#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include "LowPower.h"
#include "elapsedMillis.h"

// These are the pin and address assignments
#define PIN_R 9
#define PIN_G 6
#define PIN_B 5
#define PIN_BUTTON 2
#define PIN_LDO A3
#define V1_ACCEL_ADDR 0x4C
#define V2_ACCEL_ADDR 0x1D

// This is the version used for loading factory settings on first bootup
#define EEPROM_VERSION  100

// These are the EEPROM addresses where the model, version, locked, and sleeping flags are stored
#define ADDR_VERSION    1022
#define ADDR_SLEEPING   1023

// These are the timings for changing between button states
#define PRESS_DELAY     100
#define SHORT_HOLD      1000
#define LONG_HOLD       2000
#define VERY_LONG_HOLD  6000

// These are the different light states
#define S_PLAY_OFF        0
#define S_PLAY_PRESSED    1
#define S_PLAY_SLEEP_WAIT 2
#define S_SLEEP_WAKE      3

// This defines the number of acceleration bins used for speed tracking
#define ACCEL_BINS 16

// This limiter variable is used to rate limit frame updates
elapsedMicros limiter = 0;

// The state buttons are used to track the current state of the light.
// Since trans tracks how long since the last state transition.
uint8_t button_state, new_state;
uint16_t since_trans = 0;

// These variables are for handling the accelerometer for both v1 and v2 chips
uint8_t accel_model;
uint8_t accel_addr;

uint8_t accel_counts;
uint8_t accel_wrap;
uint8_t accel_tick = 0;

// These variables track the acceleration values read from the accelerometer
int16_t xg, yg, zg;
// These track the last acceleration values read
int16_t lxg, lyg, lzg;
// These floats hold the normalized (in gs) acceleration values
float fxg, fyg, fzg;
// This vector is the product off all acceleration values. Max is ~55.5
float mag_a;
// Tiltx and y track the pitch and roll of the light.
float pitch, roll;

float thresh_bin_size;
uint8_t thresh_falloff;
uint8_t thresh_target;
// This tracks how long since the last time an acceleration in a bin has been detected.
uint8_t thresh_last[ACCEL_BINS];
// This tracks how many times an acceleration in a bin has been tracked.
uint8_t thresh_cnts[ACCEL_BINS];
// This tracks the highest "active" acceleration threshold.
uint8_t accel_high = 0;


// This is the setup function. This is the first function to run when the light starts up.
void setup() {
  // Call this once to start the I2C bus
  Wire.begin();
  // This initializes serial communication over the USB port.
  Serial.begin(57600);

  // These pins need to be enabled for the light to function.
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_LDO, OUTPUT);

  // The initial state, unless waking from sleep, is playing.
  button_state = new_state = S_PLAY_OFF;

  // This is the low-power sleep section.
  // First we attach an interrupt on the button to wake up if we happen to be sleeping.
  // If the sleeping bit is set, we unset it and go into low power mode.
  // When waking up from the button press, we go into the SLEEP WAKE state to handle holds.
  /*
  attachInterrupt(0, pushInterrupt, FALLING);
  if (EEPROM.read(ADDR_SLEEPING)) {
    EEPROM.write(ADDR_SLEEPING, 0);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    button_state = new_state = S_SLEEP_WAKE;
  }
  */

  // Now that the sleep state has been handled, we need to deactivate the interrupt and continue.
  detachInterrupt(0);
  // And enable the Low Voltage Dropout
  digitalWrite(PIN_LDO, HIGH);

  // Now we check the version. If the version doesn't match, we need to load the factory settings
  // and update the version number.
  // During the version update, we also check the chip version agains the accelerometer.
  // If the versions do match, then load from memory.
  /*
  if (EEPROM_VERSION != EEPROM.read(ADDR_VERSION)) memoryReset();
  else                                             memoryRestore();
  */

  // Now configure the accelerometer based on the detected model.
  detectAccelModel();
  accelInit();

  // Now we configure the timers. To completely sync the two timers, we need to also disable
  // PWM correction on Timer1.
  noInterrupts();
  TCCR0B = (TCCR0B & 0b11111000) | 0b001;  // no prescaler ~64/ms
  TCCR1B = (TCCR1B & 0b11111000) | 0b001;  // no prescaler ~32/ms
  bitSet(TCCR1B, WGM12); // enable fast PWM                ~64/ms
  interrupts();

  // Delay 1ms (all timers are x64 with Timer0 at no prescaler) for everything to catch up.
  delay(64);
}

void loop() {
  // Check the button to see if the state needs to be updated.
  handlePress(digitalRead(PIN_BUTTON) == LOW);

  // Now handle this frame's work for the accelerometer.
  handleAccel();

  // Write out the frame.
  render();
}

void render() {
  uint8_t r, g, b;

  if (button_state == S_PLAY_OFF) r = g = b = 128;
  else                            r = g = b = 0;

  writeFrame(r, g, b);
}

void writeFrame(uint8_t r, uint8_t g, uint8_t b) {
  // Wait for half a millisecond to pass
  while (limiter < 32000) {}
  limiter = 0;

  // Write the values to the PWM buffers for updating the LEDs
  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
}

void memoryClear() {
  for (int i = 0; i < 1024; i++) EEPROM.write(i, 0);
}

void memoryReset() {
  memoryClear();

  // Read from the factory settings and save them to the EEPROM.

  // Update the current version so stored settings are loaded next time.
  EEPROM.update(ADDR_VERSION, EEPROM_VERSION);
}

void memoryRestore() {
  // Load all your date from memory here
}


void handleAccel() {
  switch (accel_tick % accel_counts) {
    case 0:   // Get raw accel values
      accelReadXYZ();
      break;
    case 1:   // Normalize to Gs
      accelNormalize();
      break;
    case 2:   // Calculate the magnitude of all acceleration axes
      mag_a = sqrt((fxg * fxg) + (fyg * fyg) + (fzg * fzg));
      break;
    case 3:   // Track the acceleration by bins
      accelUpdateBins();
      break;
    case 4:   // Calculate vector of Y and Z axes for pitch calculation
      pitch = sqrt((fyg * fyg) + (fzg * fzg));
      break;
    case 5:   // Calculate pitch in radians
      pitch = atan2(-fxg, pitch);
      break;
    case 6:   // Convert pitch to degrees
      pitch = (pitch * 180) / M_PI;
      break;
    case 7:   // Calculate roll in radians
      roll = atan2(-fyg, fzg);
      break;
    case 8:   // Convert roll to degrees
      roll = (roll * 180.0) / M_PI;
      break;

    // Can have no higher than case 15
    default:
      break;
  }

  accel_tick++;
  if (accel_tick >= accel_wrap) accel_tick = 0;
}

void enterSleep() {
  // Write a blank to the light.
  writeFrame(0, 0, 0);
  // Write the sleeping bit.
  EEPROM.write(ADDR_SLEEPING, 1);
  // Turn off the Low Voltage Dropoff.
  digitalWrite(PIN_LDO, LOW);
  // Delay 100ms for the light to be ready.
  delay(6400);
  // Enable the watchdog to bite after ~15ms.
  wdt_enable(WDTO_15MS);
  // Now delay long enough for the watchdog to bite. (1s)
  delay(64000);
}

// This function is a noop that is used to wake from low power mode on button press.
void pushInterrupt() {}


void handlePress(bool pressed) {
  switch (button_state) {
    case S_PLAY_OFF:
      // On press, we go to the PRESSED state to wait for release or hold.
      if (pressed && since_trans >= PRESS_DELAY) new_state = S_PLAY_PRESSED;
      break;

    case S_PLAY_PRESSED:
      if (!pressed) {
        // This is a press while the light is playing. You can switch modes here.
        new_state = S_PLAY_OFF;
      } else if (since_trans >= SHORT_HOLD) {
        // If the button is held longer than the short wait (.5s) we move to the next state.
        // In this case, we wait for sleeping.
        new_state = S_PLAY_SLEEP_WAIT;
      }
      break;

    case S_PLAY_SLEEP_WAIT:
      if (!pressed) {
        // On release, we go to sleep.
        enterSleep();
      } else if (since_trans >= SHORT_HOLD) {
        // We can add more transitions here.
      }
      break;

    case S_SLEEP_WAKE:
      if (!pressed) {
        // On release after sleep, we start playing.
        new_state = S_PLAY_OFF;
      } else if (since_trans >= LONG_HOLD) {
        // Here we can go to other states from sleep.
      }
      break;

    default:
      break;
  }

  // If a state change has occured, we reset the transition time and move to the new state.
  // If the state remains the same, we just incrememnt the transition counter.
  if (button_state != new_state) {
    button_state = new_state;
    since_trans = 0;
  } else {
    since_trans++;
  }
}


void accelSend(uint8_t addr, uint8_t data) {
  Wire.beginTransmission(accel_addr);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
}

void accelInit() {
  if (accel_model == 0) {
    accelSend(0x07, 0x00);        // Standby to accept new settings
    accelSend(0x08, 0x00);        // Set 120 samples/sec (every 16 2/3 frames)
    accelSend(0x07, 0x01);        // Active mode
  } else {
    accelSend(0x2A, 0x00);        // Standby to accept new settings
    accelSend(0x0E, 0x00);        // Set +-2g range
    accelSend(0x2A, 0b00011001);  // Set 100 samples/sec (every 20 frames) and active
  }
}

void accelReadXYZ() {
  Wire.beginTransmission(accel_addr);

  // v1 is a 6 bit value from 0 - 31, -32 - -1
  if (accel_model == 0) {
    Wire.write(0x00);
    Wire.endTransmission(false);
    Wire.requestFrom((int)accel_addr, 3);

    while (!Wire.available()); xg = Wire.read();
    xg = (xg >= 32) ? -64 + xg : xg;

    while (!Wire.available()); yg = Wire.read();
    yg = (yg >= 32) ? -64 + yg : yg;

    while (!Wire.available()); zg = Wire.read();
    zg = (zg >= 32) ? -64 + zg : zg;

  // v2 is a 12 bit value from 0 - 2047, -2048 to -1
  } else {
    Wire.write(0x01);
    Wire.endTransmission(false);
    Wire.requestFrom((int)accel_addr, 6);

    // v2 stores the 8 MSB in the first register and the 4 LSB at the top of the second
    while (!Wire.available()); xg = Wire.read() << 4;
    while (!Wire.available()); xg |= Wire.read() >> 4;
    xg = (xg >= 2048) ? -4096 + xg : xg;

    while (!Wire.available()); yg = Wire.read() << 4;
    while (!Wire.available()); yg |= Wire.read() >> 4;
    yg = (yg >= 2048) ? -4096 + yg : yg;

    while (!Wire.available()); zg = Wire.read() << 4;
    while (!Wire.available()); zg |= Wire.read() >> 4;
    zg = (zg >= 2048) ? -4096 + zg : zg;
  }
}

void accelNormalize() {
  // Normalize accel values to gs
  float pg = (accel_model == 0) ? 21.0 : 1024.0;
  fxg = xg / pg; fyg = yg / pg; fzg = zg / pg;
}

void accelUpdateBins() {
  // Tracks the magnitude of acceleration
  // v1 max is ~ 2.55 - 2.64
  // v2 max is - 3.46

  // reset high tracker
  accel_high = 0;

  for (uint8_t i = 0; i < THRESH_BINS; i++) {
    // Tracking each bin
    // v1 tracks from 1.1 - 2.53
    // v2 tracks from 1.1 - 3.43
    thresh_last[i]++;
    if (mag_a > (1.1 + (i * thresh_bin_size)) || mag_a < (0.9 - (i * 0.055))) {
      // Reset time since last we heard from this bin
      thresh_last[i] = 0;

      // Increase the number of times we've heard from this bin (avoiding overflow)
      thresh_cnts[i] = constrain(thresh_cnts[i] + 1, 0, 200);
    }

    // If it's been 100ms since we've heard from a bin, it's not active
    if (thresh_last[i] >= thresh_falloff) thresh_cnts[i] = 0;

    // If we've seen 50ms of activity in a bin, it's the highest for now
    if (thresh_cnts[i] > thresh_target) accel_high = i + 1;
  }
}

void detectAccelModel() {
  // Try to talk to the v2 sensor to get it's id
  Wire.beginTransmission(V2_ACCEL_ADDR);
  Wire.write(0x0d);
  Wire.endTransmission(false);

  // Read in the ID value if it's there
  Wire.requestFrom(V2_ACCEL_ADDR, 1);
  uint8_t v = 0;
  while (!Wire.available()); v = Wire.read();

  Serial.print(F("accel id: "));
  Serial.println(v, HEX);

  // Set accelerometer properties based on the model id
  if (v == 0x4a || v == 0x5a) {
    Serial.println(F("v2 sensor detected"));
    // v2 updates at 100/s or every 20 frames
    accel_model = 1;
    accel_addr = V2_ACCEL_ADDR;
    accel_counts = 20;
    accel_count_wrap = 20;
    thresh_bin_size = 0.155;
    thresh_falloff = 10;
    thresh_target = 5;
  } else {
    Serial.println(F("v1 sensor detected"));
    // v1 updates 120/s or every 16 and 2/3 frames
    accel_model = 0;
    accel_addr = V1_ACCEL_ADDR;
    accel_counts = 17;
    accel_count_wrap = 50;
    thresh_bin_size = 0.095;
    thresh_falloff = 12;
    thresh_target = 6;
  }
}
