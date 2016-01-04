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
#define S_VIEW_MODE       250
#define S_VIEW_COLOR      251

// Serial stuff for gui comms
#define SER_VERSION     100
#define SER_LOAD        10
#define SER_SAVE        20
#define SER_READ        30
#define SER_WRITE       40
#define SER_DUMP        90
#define SER_MODE_SET    100
#define SER_MODE_PREV   101
#define SER_MODE_NEXT   102
#define SER_VIEW_MODE   110
#define SER_VIEW_COLOR  111
#define SER_HANDSHAKE   200
#define SER_DISCONNECT  210

// This defines the number of acceleration bins used for speed tracking
#define ONEG 512
#define ACCEL_BINS 32
#define ACCEL_BIN_SIZE 56
#define ACCEL_COUNTS 20
#define ACCEL_WRAP   20
#define ACCEL_FALLOFF 10
#define ACCEL_TARGET  5

#define PALETTE_SIZE 48
#define NUM_MODES    7
#define NUM_COLORS   9

// These are the pattern bases
#define P_STROBE  0
#define P_VEXER   1
#define P_EDGE    2
#define P_DOUBLE  3
#define P_RUNNER  4

elapsedMicros limiter = 0;
uint8_t state, new_state;
uint16_t since_trans = 0;

uint8_t accel_tick;
uint8_t thresh_last[ACCEL_BINS];
uint8_t thresh_cnts[ACCEL_BINS];

int32_t gs[3];
int32_t a_mag;
uint8_t a_speed = 0;

typedef struct Mode {
  // Which pattern is used for the animations
  uint8_t pattern;                  // 0
  // How many colors in the animation
  uint8_t num_colors;               // 1
  // What arguments define the animation
  uint8_t args[3];                  // 2 - 4
  // What are the timing thresholds
  // First 2x2 array is for color morphing and the second for pattern
  // First 2 values indicate the start and end of the morph from 0->1, second two for 1->2
  uint8_t thresh[2][2][2];          // 5 - 11, color/pattern, first/second, start/end
  // These are the timings that define the animation. 3 sets to be morphed between
  uint8_t timings[3][4];            // 13 - 24, timing sets
  // These are the colors that define the animation. 3 sets of up to 9
  uint8_t colors[NUM_COLORS][3][3]; // 25 - 105
} Mode;                             // 106 bytes per mode

#define MODE_SIZE 106
typedef union PackedMode {
  Mode m;
  uint8_t d[MODE_SIZE];
} PackedMode;

/* PackedMode pms[NUM_MODES]; */
Mode* mode;
PackedMode pm;
PROGMEM const uint8_t factory_modes[NUM_MODES][MODE_SIZE] = {
  // Darkside of the moon
  {P_STROBE, 6, 0, 0, 0,
    0, 16, 16, 32,
    0, 8, 8, 32,
    10, 0, 180, 0,
    5, 0, 120, 0,
    10, 40, 0, 0,
    12, 0, 0,       48, 0, 0,       192, 0, 0,
    6, 7, 0,        24, 28, 0,      96, 112, 0,
    0, 14, 0,       0, 28, 0,       0, 224, 0,
    0, 7, 8,        0, 14, 16,      0, 112, 128,
    0, 0, 16,       0, 0, 32,       0, 0, 255,
    6, 0, 8,        12, 0, 16,      96, 0, 128,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
  },
  // Electric Dops
  {P_EDGE, 4, 2, 0, 0,
    0, 16, 16, 32,
    0, 12, 12, 24,
    1, 0, 3, 95,
    1, 0, 2, 46,
    1, 0, 1, 22,
    36, 14, 192,    36, 14, 192,    26, 14, 192,
    18, 0, 24,      48, 0, 32,      120, 0, 32,
    36, 168, 16,    36, 168, 16,    36, 168, 16,
    18, 21, 0,      48, 28, 0,      120, 28, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
  },
  // Self Healing
  {P_VEXER, 3, 1, 4, 0,
    1, 32, 32, 32,
    0, 24, 24, 32,
    5, 5, 0, 50,
    5, 5, 50, 0,
    5, 5, 25, 0,
    6, 0, 0,        0, 0, 8,      0, 0, 0,
    48, 154, 16,    144, 56, 0,   0, 0, 0,
    132, 56, 16,    192, 0, 0,    0, 0, 0,
    90, 105, 16,    96, 112, 0,   0, 0, 0,
    0, 0, 0,        0, 0, 0,      0, 0, 0,
    0, 0, 0,        0, 0, 0,      0, 0, 0,
    0, 0, 0,        0, 0, 0,      0, 0, 0,
    0, 0, 0,        0, 0, 0,      0, 0, 0,
    0, 0, 0,        0, 0, 0,      0, 0, 0,
  },
  // Sorcery
  {P_STROBE, 3, 0, 0, 0,
    0, 32, 32, 32,
    1, 6, 6, 32,
    0, 25, 0, 0,
    5, 20, 0, 0,
    10, 65, 0, 0,
    18, 0, 104,   36, 0, 208,  0, 0, 0,
    0, 21, 104,   0, 42, 208,  0, 0, 0,
    78, 0, 24,    156, 0, 48,  0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
  },
  // Candy Strobe
  {P_STROBE, 9, 3, 1, 32,
    32, 32, 32, 32,
    0, 16, 16, 32,
    9, 41, 0, 0,
    25, 25, 0, 0,
    3, 47, 0, 0,
    144, 0, 0,    0, 0, 0,  0, 0, 0,
    96, 56, 0,    0, 0, 0,  0, 0, 0,
    48, 112, 0,   0, 0, 0,  0, 0, 0,
    0, 168, 0,    0, 0, 0,  0, 0, 0,
    0, 112, 64,   0, 0, 0,  0, 0, 0,
    0, 56, 128,   0, 0, 0,  0, 0, 0,
    0, 0, 196,    0, 0, 0,  0, 0, 0,
    48, 0, 128,   0, 0, 0,  0, 0, 0,
    96, 0, 64,    0, 0, 0,  0, 0, 0,
  },
  // Quantum Core
  {P_DOUBLE, 3, 1, 1, 1,
    32, 32, 32, 32,
    0, 32, 32, 32,
    50, 50, 50, 50,
    90, 50, 10, 50,
    5, 45, 5, 45,
    0, 42, 144,   0, 0, 0,      0, 0, 0,
    6, 0, 184,    0, 0, 0,      0, 0, 0,
    48, 70, 96,   0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
  },
  // Dashdops
  {P_RUNNER, 7, 0, 0, 0,
    32, 32, 32, 32,
    16, 16, 32, 32,
    5, 0, 3, 22,
    3, 22, 5, 0,
    5, 0, 5, 0,
    60, 70, 80,   0, 0, 0,  0, 0, 0,
    192, 0, 0,    0, 0, 0,  0, 0, 0,
    96, 112, 0,   0, 0, 0,  0, 0, 0,
    0, 224, 0,    0, 0, 0,  0, 0, 0,
    0, 112, 128,  0, 0, 0,  0, 0, 0,
    0, 0, 255,    0, 0, 0,  0, 0, 0,
    96, 0, 128,   0, 0, 0,  0, 0, 0,
    0, 0, 0,  0, 0, 0,  0, 0, 0,
    0, 0, 0,  0, 0, 0,  0, 0, 0,
  },
  /*
  // Ghosts
  {P_STROBE, 9, 3, 3, 1,
    0, 8, 8, 32,
    0, 32, 32, 32,
    5, 0, 135, 0,
    5, 0, 60, 0,
    5, 0, 85, 0,
    174, 4, 16,     174, 4, 16,     174, 4, 16,
    0, 0, 4,        0, 0, 16,       0, 0, 64,
    0, 0, 0,        0, 0, 4,        0, 0, 16,
    174, 4, 16,     174, 4, 16,     174, 4, 16,
    0, 2, 2,        0, 7, 8,        0, 28, 32,
    0, 0, 0,        0, 2, 2,        0, 7, 8,
    174, 4, 16,     174, 4, 16,     174, 4, 16,
    0, 4, 0,        0, 16, 0,       0, 64, 0,
    0, 0, 0,        0, 4, 0,        0, 16, 0,
  },
  // Halos
  {P_EDGE, 9, 3, 0, 0,
    0, 16, 16, 32,
    0, 16, 16, 32,
    4, 0, 8, 90,
    2, 0, 8, 90,
    2, 0, 6, 140,
    0, 28, 224,   0, 28, 224,   0, 28, 224,
    24, 0, 0,     24, 0, 0,     24, 0, 0,
    48, 0, 0,     48, 0, 0,     48, 0, 0,
    0, 28, 224,   0, 28, 224,   0, 28, 224,
    12, 14, 0,    12, 14, 0,    12, 14, 0,
    24, 28, 0,    24, 28, 0,    24, 28, 0,
    0, 28, 224,   0, 28, 224,   0, 28, 224,
    0, 28, 8,     0, 28, 8,     0, 28, 0,
    0, 56, 0,     0, 56, 0,     0, 56, 0,
  },
  // Star Tripping
  {P_VEXER, 3, 1, 5, 0,
    16, 32, 32, 32,
    0, 16, 16, 32,
    , 20, 1, 0,
    25, 20, 5, 0,
    25, 0, 1, 24,
    60, 0, 24,      120, 0, 36,     0, 0, 0,
    0, 28, 160,     0, 14, 80,      0, 0, 0,
    36, 0, 144,     18, 0, 72,      0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
  },
  // PacMan
  {P_VEXER, 5, 3, 3, 0,
    0, 16, 16, 32,
    0, 16, 16, 32,
    2, 0, 1, 19,
    2, 0, 20, 0,
    5, 5, 10, 0,
    0, 0, 0,      0, 0, 4,      0, 0, 16,
    144, 42, 48,  144, 42, 48,  144, 42, 48,
    192, 0, 0,    192, 0, 0,    192, 0, 0,
    96, 112, 0,   96, 112, 0,   96, 112, 0,
    0, 112, 128,  0, 112, 128,  0, 112, 128,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
  },
  */
};

uint8_t cur_mode = 0;

uint8_t r, g, b;

uint16_t tick;
uint8_t trip = 0;
uint8_t cidx, cntr, segm;
uint8_t timing0 = 25;
uint8_t timing1 = 25;
uint8_t timing2 = 25;
uint8_t timing3 = 25;

bool comm_link = false;
uint8_t gui_set, gui_color;


void setup() {
  pinMode(PIN_BUTTON, INPUT);

  attachInterrupt(0, pushInterrupt, FALLING);
  if (EEPROM.read(ADDR_SLEEPING)) {
    EEPROM.write(ADDR_SLEEPING, 0);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    state = new_state = S_SLEEP_WAKE;
  } else {
    state = new_state = S_PLAY_OFF;
  }
  detachInterrupt(0);

  Wire.begin();
  Serial.begin(57600);

  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_LDO, OUTPUT);
  digitalWrite(PIN_LDO, HIGH);

  /* memoryReset(); */
  /* if (EEPROM_VERSION != EEPROM.read(ADDR_VERSION))  memoryReset(); */
  /* else                                              memoryLoad(); */

  accelInit();
  changeMode(0);

  noInterrupts();
  TCCR0B = (TCCR0B & 0b11111000) | 0b001;  // no prescaler ~64/ms
  TCCR1B = (TCCR1B & 0b11111000) | 0b001;  // no prescaler ~32/ms
  bitSet(TCCR1B, WGM12); // enable fast PWM                ~64/ms
  interrupts();

  delay(64);
}

void loop() {
  /* handleSerial(); */
  handlePress(digitalRead(PIN_BUTTON) == LOW);
  handleAccel();
  render();
}

void render() {
  if (state == S_PLAY_OFF) {
    renderMode();
  } else if (state == S_VIEW_MODE) {
    renderMode();
  } else if (state == S_VIEW_COLOR) {
    r = mode->colors[gui_color][gui_set][0];
    g = mode->colors[gui_color][gui_set][1];
    b = mode->colors[gui_color][gui_set][2];
  } else {
    r = g = b = 0;
  }
  writeFrame(r, g, b);
}

void writeFrame(uint8_t r, uint8_t g, uint8_t b) {
  // Wait for half a millisecond to pass
  /* if (limiter > 32000) { Serial.print(limiter); Serial.print(F("\t")); Serial.println(accel_tick); } */
  while (limiter < 32000) {}
  limiter = 0;

  // Write the values to the PWM buffers for updating the LEDs
  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
}


//******************************************************************************
//******************************************************************************
//******************************************************************************
// Memory core
void memoryReset() {
  /* for (int i = 0; i < 1024; i++) EEPROM.write(i, 0); */
  /* for (uint8_t i = 0; i < NUM_MODES; i++) modeReset(i); */
  /* EEPROM.update(ADDR_VERSION, EEPROM_VERSION); */
}

void loadMode(uint8_t i) {
  for (uint8_t j = 0; j < MODE_SIZE; j++) {
    pm.d[j] = pgm_read_byte(&factory_modes[i][j]);
  }
  /*
  for (uint8_t j = 0; j < MODE_SIZE; j++) {
    pm.d[j] = EEPROM.read((i * 128) + j);
  }
  */
}

/*
void memoryLoad() {
  for (uint8_t i = 0; i < NUM_MODES; i++) modeLoad(i);
}

void modeReset(uint8_t i) {
  for (uint8_t j = 0; j < MODE_SIZE; j++) {
    pms[i].d[j] = pgm_read_byte(&factory_modes[i][j]);
  }
}

void modeLoad(uint8_t i) {
  for (uint8_t j = 0; j < MODE_SIZE; j++) {
    pms[i].d[j] = EEPROM.read((i * 128) + j);
  }
}

void modeSave(uint8_t i) {
  for (uint8_t j = 0; j < MODE_SIZE; j++) {
    EEPROM.update((i * 128) + j, pms[i].d[j]);
  }
}

void modeRead(uint8_t i, uint8_t addr) {
  if (i < NUM_MODES) {
    Serial.write(i);
    Serial.write(addr);
    Serial.write(pms[i].d[addr]);
  }
}

void modeWrite(uint8_t i, uint8_t addr, uint8_t val) {
  if (i < NUM_MODES) {
    pms[i].d[addr] = val;
  }
}

void modeDump(uint8_t i) {
  Serial.write(200); Serial.write(i); Serial.write(SER_VERSION);
  for (uint8_t j = 0; j < MODE_SIZE; j++) modeRead(i, j);
  Serial.write(210); Serial.write(i); Serial.write(SER_VERSION);
}
*/


//******************************************************************************
//******************************************************************************
//******************************************************************************
// Button and sleep core
void enterSleep() {
  // Write a blank to the light.
  writeFrame(0, 0, 0);
  // Write the sleeping bit.
  EEPROM.write(ADDR_SLEEPING, 1);
  accelStandby();
  // Turn off the Low Voltage Dropoff.
  digitalWrite(PIN_LDO, LOW);
  // Delay 10ms for the light to be ready.
  delay(640);
  // Enable the watchdog to bite after ~15ms.
  wdt_enable(WDTO_15MS);
  // Now delay long enough for the watchdog to bite. (100ms)
  delay(6400);
}

// This function is a noop that is used to wake from low power mode on button press.
void pushInterrupt() {}

void handlePress(bool pressed) {
  if (state == S_PLAY_OFF) {
    // On press, we go to the PRESSED state to wait for release or hold.
    if (pressed && since_trans >= PRESS_DELAY) new_state = S_PLAY_PRESSED;
  } else if (state == S_PLAY_PRESSED) {
    if (!pressed) {
      // This is a press while the light is playing. You can switch modes here.
      changeMode(SER_MODE_NEXT);
      new_state = S_PLAY_OFF;
    } else if (since_trans >= SHORT_HOLD) {
      // If the button is held longer than the short wait (.5s) we move to the next state.
      // In this case, we wait for sleeping.
      new_state = S_PLAY_SLEEP_WAIT;
    }
  } else if (state == S_PLAY_SLEEP_WAIT) {
    if (!pressed) {
      // On release, we go to sleep.
      enterSleep();
    } else if (since_trans >= SHORT_HOLD) {
      // We can add more transitions here.
    }
  } else if (state == S_SLEEP_WAKE) {
    if (!pressed) {
      // On release after sleep, we start playing.
      new_state = S_PLAY_OFF;
    } else if (since_trans >= LONG_HOLD) {
      // Here we can go to other states from sleep.
    }
  }

  // If a state change has occured, we reset the transition time and move to the new state.
  // If the state remains the same, we just incrememnt the transition counter.
  if (state != new_state) {
    state = new_state;
    since_trans = 0;
  } else {
    since_trans++;
  }
}


//******************************************************************************
//******************************************************************************
//******************************************************************************
// Accelerometer Core
void handleAccel() {
  switch (accel_tick % ACCEL_COUNTS) {
    case 0:
      Wire.beginTransmission(V2_ACCEL_ADDR);
      Wire.write(0x01);
      Wire.endTransmission(false);
      break;
    case 1:
      Wire.requestFrom(V2_ACCEL_ADDR, 2);
      while (!Wire.available()); gs[0] = Wire.read() << 4;
      while (!Wire.available()); gs[0] |= Wire.read() >> 4;
      gs[0] = (gs[0] < 2048) ? gs[0] : -4096 + gs[0];
      break;
    case 2:
      Wire.beginTransmission(V2_ACCEL_ADDR);
      Wire.write(0x03);
      Wire.endTransmission(false);
      break;
    case 3:
      Wire.requestFrom(V2_ACCEL_ADDR, 2);
      while (!Wire.available()); gs[1] = Wire.read() << 4;
      while (!Wire.available()); gs[1] |= Wire.read() >> 4;
      gs[1] = (gs[1] < 2048) ? gs[1] : -4096 + gs[1];
      break;
    case 4:
      Wire.beginTransmission(V2_ACCEL_ADDR);
      Wire.write(0x05);
      Wire.endTransmission(false);
      break;
    case 5:
      Wire.requestFrom(V2_ACCEL_ADDR, 2);
      while (!Wire.available()); gs[2] = Wire.read() << 4;
      while (!Wire.available()); gs[2] |= Wire.read() >> 4;
      gs[2] = (gs[2] < 2048) ? gs[2] : -4096 + gs[2];
      break;
    case 6:
      a_mag = sqrt((gs[0] * gs[0]) + (gs[1] * gs[1]) + (gs[2] * gs[2]));
      break;
    case 7:
      accelUpdateBins();
      break;

    // Can have no higher than case 19
    default:
      break;
  }

  accel_tick++;
  if (accel_tick >= ACCEL_COUNTS) accel_tick = 0;
}

void accelSend(uint8_t addr, uint8_t data) {
  Wire.beginTransmission(V2_ACCEL_ADDR);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
}

void accelInit() {
  accelSend(0x2A, 0x00);        // Standby to accept new settings
  accelSend(0x0E, 0x01);        // Set +-4g range
  accelSend(0x2A, 0b00011001);  // Set 100 samples/sec (every 20 frames) and active
}

void accelStandby() {
  accelSend(0x2A, 0x00);
}

void accelUpdateBins() {
  // Tracks the magnitude of acceleration
  int16_t cur_thresh = ACCEL_BIN_SIZE;
  uint8_t i = 0;
  a_speed = 0;
  while (i < ACCEL_BINS) {
    if (abs(a_mag - ONEG) > cur_thresh) {
      thresh_last[i] = 0;
      thresh_cnts[i] = max(thresh_cnts[i], 128);
    }
    if (thresh_last[i] >= ACCEL_FALLOFF) thresh_cnts[i] = 0;
    if (thresh_cnts[i] > ACCEL_TARGET) a_speed = i + 1;
    thresh_last[i]++;
    i++;
    cur_thresh += ACCEL_BIN_SIZE;
  }
}


//******************************************************************************
//******************************************************************************
//******************************************************************************
// Rendering Core
inline uint8_t interp(uint8_t m, uint8_t n, uint8_t d, uint8_t D) {
  return m + (((int16_t)(n - m) * d) / D);
}

inline void recalcArgs() {
  uint8_t as, d;
  if (a_speed <= mode->thresh[1][0][0]) {
    timing0 = mode->timings[0][0];
    timing1 = mode->timings[0][1];
    timing2 = mode->timings[0][2];
    timing3 = mode->timings[0][3];
  } else if (a_speed < mode->thresh[1][0][1]) {
    as = a_speed - mode->thresh[1][0][0];
    d = mode->thresh[1][0][1] - mode->thresh[1][0][0];
    timing0 = interp(mode->timings[0][0], mode->timings[1][0], as, d);
    timing1 = interp(mode->timings[0][1], mode->timings[1][1], as, d);
    timing2 = interp(mode->timings[0][2], mode->timings[1][2], as, d);
    timing3 = interp(mode->timings[0][3], mode->timings[1][3], as, d);
  } else if (a_speed <= mode->thresh[1][1][0]) {
    timing0 = mode->timings[1][0];
    timing1 = mode->timings[1][1];
    timing2 = mode->timings[1][2];
    timing3 = mode->timings[1][3];
  } else if (a_speed < mode->thresh[1][1][1]) {
    as = a_speed - mode->thresh[1][1][0];
    d = mode->thresh[1][1][1] - mode->thresh[1][1][0];
    timing0 = interp(mode->timings[1][0], mode->timings[2][0], as, d);
    timing1 = interp(mode->timings[1][1], mode->timings[2][1], as, d);
    timing2 = interp(mode->timings[1][2], mode->timings[2][2], as, d);
    timing3 = interp(mode->timings[1][3], mode->timings[2][3], as, d);
  } else {
    timing0 = mode->timings[2][0];
    timing1 = mode->timings[2][1];
    timing2 = mode->timings[2][2];
    timing3 = mode->timings[2][3];
  }
}

inline void colorStatic(int8_t color) {
  r = mode->colors[color][0][0];
  g = mode->colors[color][0][1];
  b = mode->colors[color][0][2];
}

inline void colorFlux(int8_t color) {
  uint8_t as, d;
  if (a_speed <= mode->thresh[0][0][0]) {
    r = mode->colors[color][0][0];
    g = mode->colors[color][0][1];
    b = mode->colors[color][0][2];
  } else if (a_speed < mode->thresh[0][0][1]) {
    as = a_speed - mode->thresh[0][0][0];
    d = mode->thresh[0][0][1] - mode->thresh[0][0][0];
    r = interp(mode->colors[color][0][0], mode->colors[color][1][0], as, d);
    g = interp(mode->colors[color][0][1], mode->colors[color][1][1], as, d);
    b = interp(mode->colors[color][0][2], mode->colors[color][1][2], as, d);
  } else if (a_speed <= mode->thresh[0][1][0]) {
    r = mode->colors[color][1][0];
    g = mode->colors[color][1][1];
    b = mode->colors[color][1][2];
  } else if (a_speed < mode->thresh[0][1][1]) {
    as = a_speed - mode->thresh[0][1][0];
    d = mode->thresh[0][1][1] - mode->thresh[0][1][0];
    r = interp(mode->colors[color][1][0], mode->colors[color][2][0], as, d);
    g = interp(mode->colors[color][1][1], mode->colors[color][2][1], as, d);
    b = interp(mode->colors[color][1][2], mode->colors[color][2][2], as, d);
  } else {
    r = mode->colors[color][2][0];
    g = mode->colors[color][2][1];
    b = mode->colors[color][2][2];
  }
}

int8_t patternStrobe(uint8_t numc, uint8_t pick, uint8_t skip, uint8_t repeat,
    uint8_t st, uint8_t bt, uint8_t lt) {

  int8_t rtn = -1;
  numc = constrain(numc, 1, NUM_COLORS);
  pick = (pick == 0) ? numc : pick;
  skip = (skip == 0) ? pick : skip;
  repeat = max(1, repeat);

  if (tick >= trip) {
    recalcArgs();
    tick = trip = 0;
    while (trip == 0) {
      segm++;
      if (segm >= ((2 * pick) + 1)) {
        segm = 0;
        cntr++;
        if (cntr >= repeat) {
          cntr = 0;
          cidx += skip;
          if (cidx >= numc) cidx = (pick == skip) ? 0 : cidx % numc;
        }
      }

      if (segm == 2 * pick)   trip = lt;
      else if (segm % 2 == 1) trip = st;
      else                    trip = bt;
    }
  }

  if (segm % 2 == 1) rtn = (segm / 2) + cidx;
  else               rtn = -1;

  if (rtn >= numc) rtn = (pick == skip) ? -1 : rtn % numc;
  return rtn;
}

int8_t patternVexer(uint8_t numc, uint8_t repeat_c, uint8_t repeat_t,
    uint8_t cst, uint8_t cbt, uint8_t tst, uint8_t tbt) {

  int8_t rtn = -1;
  numc = constrain(numc, 1, NUM_COLORS);
  repeat_c = max(1, repeat_c);
  repeat_t = max(1, repeat_t);

  if (tick >= trip) {
    recalcArgs();
    tick = trip = 0;
    while (trip == 0) {
      segm++;
      if (segm >= (2 * (repeat_c + repeat_t + 1))) {
        segm = 0;
        cidx = (cidx + 1) % (numc - 1);
      }

      if (segm < (2 * repeat_c) + 1) {
        if (segm % 2 == 0) trip = cbt;
        else               trip = cst;
      } else {
        if (segm % 2 == 1) trip = tbt;
        else               trip = tst;
      }
    }
  }

  if (segm < (2 * repeat_c) + 1) {
    if (segm % 2 == 0) rtn = -1;
    else               rtn = cidx + 1;
  } else {
    if (segm % 2 == 1) rtn = -1;
    else               rtn = 0;
  }
  return rtn;
}

int8_t patternEdge(uint8_t numc, uint8_t pick,
    uint8_t cst, uint8_t cbt, uint8_t est, uint8_t ebt) {

  int8_t rtn = -1;
  numc = constrain(numc, 1, NUM_COLORS);
  pick = (pick == 0) ? numc : pick;

  // -c_c_t_c_c
  // 0123456789
  if (tick >= trip) {
    recalcArgs();
    tick = trip = 0;
    while (trip == 0) {
      segm++;
      if (segm >= (4 * pick) - 2) {
        segm = 0;
        cidx += pick;
        if (cidx >= numc) cidx = 0;
      }

      if (segm == 0)                   trip = ebt;
      else if (segm == (2 * pick) - 1) trip = est;
      else if (segm % 2 == 0)          trip = cbt;
      else                             trip = cst;
    }
  }

  if (segm % 2 == 0)               rtn = -1;
  else if (segm == (2 * pick) - 1) rtn = cidx;
  else                             rtn = abs((segm / 2) - (pick - 1)) + cidx;

  if (rtn >= numc) rtn = -1;
  return rtn;
}

int8_t patternDouble(uint8_t numc, uint8_t repeat_c, uint8_t repeat_d, uint8_t skip,
    uint8_t cst, uint8_t cbt, uint8_t dst, uint8_t dbt) {

  int8_t rtn = -1;
  numc = constrain(numc, 1, NUM_COLORS);
  repeat_c = max(1, repeat_c);
  repeat_d = max(1, repeat_d);
  skip = min(numc - 1, skip);

  if (tick >= trip) {
    recalcArgs();
    tick = trip = 0;
    while (trip == 0) {
      segm++;
      if (segm >= 2 * (repeat_c + repeat_d)) {
        segm = 0;
        cidx = (cidx + 1) % numc;
      }

      if (segm < (2 * repeat_d)) {
        if (segm % 2 == 0) trip = dst;
        else               trip = dbt;
      } else {
        if (segm % 2 == 0) trip = cst;
        else               trip = cbt;
      }
    }
  }

  if (segm < (2 * repeat_d)) {
    if (segm % 2 == 0) rtn = (cidx + skip) % numc;
    else               rtn = -1;
  } else {
    if (segm % 2 == 0) rtn = cidx;
    else               rtn = -1;
  }
  return rtn;
}

int8_t patternRunner(uint8_t numc, uint8_t pick, uint8_t skip, uint8_t repeat,
    uint8_t cst, uint8_t cbt, uint8_t rst, uint8_t rbt) {

  int8_t rtn = -1;
  numc = constrain(numc, 1, NUM_COLORS);
  pick = (pick == 0) ? numc - 1 : pick;
  skip = (skip == 0) ? pick : skip;
  repeat = (repeat == 0) ? pick : repeat;

  if (tick >= trip) {
    recalcArgs();
    tick = trip = 0;
    while (trip == 0) {
      segm++;
      if (segm >= (2 * (repeat + pick + 1))) {
        segm = 0;
        cidx += skip;
        if (cidx >= (numc - 1)) cidx = (pick == skip) ? 0 : rtn % (numc - 1);
      }

      if (segm < (2 * pick) + 1) {
        if (segm % 2 == 0) trip = cbt;
        else               trip = cst;
      } else {
        if (segm % 2 == 1) trip = rbt;
        else               trip = rst;
      }
    }
  }

  if (segm < (2 * pick) + 1) {
    if (segm % 2 == 0) rtn = -1;
    else               rtn = (segm / 2) + 1 + cidx;
  } else {
    if (segm % 2 == 1) rtn = -1;
    else               rtn = 0;
  }

  if (rtn >= numc) rtn = (pick == skip) ? -1 : (rtn % (numc - 1) + 1);
  return rtn;
}


void renderMode() {
  int8_t color = -1;

  if (mode->pattern == P_STROBE)
    color = patternStrobe(mode->num_colors, mode->args[0], mode->args[1], mode->args[2],
        timing0, timing1, timing2);
  else if (mode->pattern == P_VEXER)
    color = patternVexer(mode->num_colors, mode->args[0], mode->args[1],
        timing0, timing1, timing2, timing3);
  else if (mode->pattern == P_EDGE)
    color = patternEdge(mode->num_colors, mode->args[0],
        timing0, timing1, timing2, timing3);
  else if (mode->pattern == P_DOUBLE)
    color = patternDouble(mode->num_colors, mode->args[0], mode->args[1], mode->args[2],
        timing0, timing1, timing2, timing3);
  else if (mode->pattern == P_RUNNER)
    color = patternRunner(mode->num_colors, mode->args[0], mode->args[1], mode->args[2],
        timing0, timing1, timing2, timing3);

  if (color < 0) {
    r = g = b = 0;
  } else {
    colorFlux(color);
  }

  tick++;
}


void changeMode(uint8_t i) {
  if (i < NUM_MODES)            cur_mode = i;
  else if (i == SER_MODE_PREV)  cur_mode = (cur_mode + NUM_MODES - 1) % NUM_MODES;
  else if (i == SER_MODE_NEXT)  cur_mode = (cur_mode + 1) % NUM_MODES;

  tick = trip = cidx = cntr = segm = 0;
  loadMode(cur_mode);
  mode = &pm.m;
  recalcArgs();
}

/*
void handleSerial() {
  // Handshake by sending 200 100 same same
  // command, target, address, value
  uint8_t in0, in1, in2, in3;
  while (Serial.available() >= 4) {
    if (in0 == SER_HANDSHAKE) {
      if (in1 == SER_VERSION && in2 == in3) {
        new_state = S_VIEW_MODE;
        comm_link = true;
      }
    } else if (comm_link) {
      if (in0 == SER_LOAD) {
        modeLoad(in1);
      } else if (in0 == SER_SAVE) {
        modeSave(in1);
      } else if (in0 == SER_READ) {
        modeRead(in1, in2);
      } else if (in0 == SER_WRITE) {
        modeWrite(in1, in2, in3);
      } else if (in0 == SER_DUMP) {
        modeDump(in1);
      } else if (in0 == SER_MODE_SET) {
        changeMode(in1);
      } else if (in0 == SER_MODE_PREV) {
        changeMode(SER_MODE_PREV);
      } else if (in0 == SER_MODE_NEXT) {
        changeMode(SER_MODE_NEXT);
      } else if (in0 == SER_VIEW_MODE) {
        new_state = S_VIEW_MODE;
      } else if (in0 == SER_VIEW_COLOR) {
        new_state = S_VIEW_COLOR;
        gui_set = in1;
        gui_color = in2;
      } else if (in0 == SER_DISCONNECT) {
        new_state = S_PLAY_OFF;
        comm_link = false;
      }
    }
  }
}
*/
