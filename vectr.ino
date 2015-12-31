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
#define ACCEL_BINS 16
#define PALETTE_SIZE 48
#define NUM_MODES    7
#define NUM_COLORS   9


// These are the pattern bases
#define P_STROBE 0
#define P_TRACER 1
#define P_VEXER  2
#define P_EDGE   3

#define C_STATIC 0
#define C_FLUX   1
/* #define C_GEO    2 */

elapsedMicros limiter = 0;
uint8_t state, new_state;
uint16_t since_trans = 0;

uint8_t accel_addr, accel_counts, accel_wrap, accel_tick;
float thresh_bins_p[16] = {
  /* 1.1, 1.4, 1.7, 2.0, 2.3, 2.6, 2.9, 3.2, 3.5, 3.8, 4.1, 4.4, 4.7, 5.0, 5.3, 5.6, */
  1.1, 1.425, 1.75, 2.075, 2.4, 2.725, 3.05, 3.375, 3.7, 4.025, 4.35, 4.675, 5.0, 5.325, 5.65, 5.975,
};
float thresh_bins_n[16] = {
  0.9, 0.845, 0.79, 0.735, 0.68, 0.625, 0.57, 0.515, 0.46, 0.405, 0.35, 0.295, 0.24, 0.185, 0.13, 0.075,
};
uint8_t thresh_falloff;
uint8_t thresh_target;
uint8_t thresh_last[ACCEL_BINS];
uint8_t thresh_cnts[ACCEL_BINS];

int16_t gs[3];
float fgs[3];
float a_mag;
uint8_t a_speed = 0;
/* float a_pitch_temp = 0.0; */
/* float a_pitch = 0.0; */

typedef struct Mode {
  uint8_t color_func;               // 1 byte
  uint8_t pattern;                  // 1 byte
  uint8_t num_colors;               // 1 byte
  uint8_t thresh_cs[2];             // 2 bytes
  uint8_t thresh_ce[2];             // 2 bytes
  uint8_t thresh_ps;                // 1 byte
  uint8_t thresh_pe;                // 1 bytes
  uint8_t args[2][4];               // 8 bytes
  uint8_t extr[4];                  // 4 bytes
  uint8_t colors[3][NUM_COLORS][3]; // 3 * 3 * 9 = 81 bytes
} Mode;                             // 102 bytes per mode

#define MODE_SIZE 102
typedef union PackedMode {
  Mode m;
  uint8_t d[MODE_SIZE];
} PackedMode;

/* Mode modes[NUM_MODES]; */
PackedMode pms[NUM_MODES];
Mode* mode;
PROGMEM const uint8_t factory_modes[NUM_MODES][MODE_SIZE] = {
  {C_STATIC, P_STROBE, 3,   // color func, pattern, and num_colors
    2, 8, 6, 14,            // thresh starts and ends for the colors
    0, 16,
    0, 50, 0, 0,
    50, 0, 0, 0,
    0, 0, 0, 0,
    48, 0, 208,  0, 0, 16,  8, 0, 248,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
    48, 0, 208,  0, 0, 16,  8, 0, 248,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
    48, 0, 208,  0, 0, 16,  8, 0, 248,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
  },
  {C_FLUX, P_VEXER, 4,
    8, 12, 12, 16,
    8, 16,
    5, 0, 3, 42,
    5, 0, 45, 0,
    0, 0, 0, 0,
    8, 0, 0,  0, 255, 0,  0, 128, 128,  0, 0, 255,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
    0, 8, 0,  0, 0, 255,  128, 0, 128,  255, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
    0, 0, 8,  255, 0, 0,  128, 128, 0,  0, 255, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
  },
  {C_FLUX, P_STROBE, 3,
    0, 8, 8, 16,
    0, 16,
    5, 0, 75, 0,
    5, 25, 0, 0,
    0, 0, 0, 0,
    0, 0, 255,  0, 16, 0,  16, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
    0, 0, 255,  0, 128, 0,  128, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
    0, 0, 255,  0, 256, 0,  256, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
  },
  {C_FLUX, P_STROBE, 4,
    0, 8, 8, 16,
    0, 16,
    5, 0, 135, 0,
    5, 0, 35, 0,
    1, 2, 2, 0,
    255, 0, 0,  0, 16, 0,  255, 0, 0,  0, 0, 16,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
    255, 0, 0,  0, 32, 0,  255, 0, 0,  0, 0, 32,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
    255, 0, 0,  0, 64, 0,  255, 0, 0,  0, 0, 64,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
  },
  {C_FLUX, P_EDGE, 9,
    0, 8, 8, 16,
    0, 16,
    0, 0, 4, 90,
    4, 0, 4, 90,
    3, 0, 0, 0,
    255, 0, 0,  0, 0, 0,  0, 0, 0,  0, 255, 0,  0, 0, 0,  0, 0, 0,  0, 0, 255,  0, 0, 0,  0, 0, 0,
    255, 0, 0,  4, 4, 0,  0, 8, 0,  0, 255, 0,  0, 4, 4,  0, 0, 8,  0, 0, 255,  4, 0, 4,  8, 0, 0,
    255, 0, 0,  8, 8, 0,  0, 32, 0,  0, 255, 0,  0, 8, 8,  0, 0, 32,  0, 0, 255,  8, 0, 8,  32, 0, 0,
  },
  {C_STATIC, P_EDGE, 9,
    0, 8, 8, 16,
    0, 16,
    2, 0, 12, 72,
    6, 4, 8, 52,
    3, 0, 0, 0,
    255, 0, 0,  16, 16, 0,  0, 64, 0,  0, 255, 0,  0, 16, 16,  0, 0, 64,  0, 0, 255,  16, 0, 16,  64, 0, 0,
    0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
    0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
  },
  {C_FLUX, P_TRACER, 4,
    0, 8, 8, 16,
    0, 16,
    5, 0, 95, 0,
    5, 0, 45, 0,
    0, 0, 0, 0,
    0, 0, 0,  255, 0, 0,  0, 255, 0,  0, 0, 255,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
    0, 0, 8,  0, 255, 0,  0, 0, 255,  255, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
    0, 0, 32,  0, 0, 255,  255, 0, 0,  0, 255, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
  },
};

uint8_t cur_mode = 0;

uint8_t r, g, b;
uint8_t r0, g0, b0;
uint8_t r1, g1, b1;

uint16_t tick;
uint8_t cidx, cntr, segm;
uint8_t arg0 = 25;
uint8_t arg1 = 25;
uint8_t arg2 = 25;
uint8_t arg3 = 25;

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

  memoryReset();
  /* if (EEPROM_VERSION != EEPROM.read(ADDR_VERSION))  memoryReset(); */
  /* else                                              memoryLoad(); */

  detectAccelModel();
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
  handleSerial();
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
    r = mode->colors[gui_set][gui_color][0];
    g = mode->colors[gui_set][gui_color][1];
    b = mode->colors[gui_set][gui_color][2];
  } else {
    r = g = b = 0;
  }
  writeFrame(r, g, b);
}

void writeFrame(uint8_t r, uint8_t g, uint8_t b) {
  // Wait for half a millisecond to pass
  if (limiter > 32000) Serial.println(accel_tick);
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
  for (uint8_t i = 0; i < NUM_MODES; i++) modeReset(i);
  /* EEPROM.update(ADDR_VERSION, EEPROM_VERSION); */
}

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


//******************************************************************************
//******************************************************************************
//******************************************************************************
// Button and sleep core
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
  switch (accel_tick % accel_counts) {
    case 0:
      accelReadAxis(0);
      break;
    case 1:
      accelReadAxis(1);
      break;
    case 2:
      accelReadAxis(2);
      break;
    case 3:
      gs[0] = (gs[0] < 2048) ? gs[0] : -4096 + gs[0];
      gs[1] = (gs[1] < 2048) ? gs[1] : -4096 + gs[1];
      gs[2] = (gs[2] < 2048) ? gs[2] : -4096 + gs[2];
      fgs[0] = gs[0] / 512.0;
      fgs[1] = gs[1] / 512.0;
      fgs[2] = gs[2] / 512.0;
      break;
    case 4:
      a_mag = sqrt((fgs[0] * fgs[0]) + (fgs[1] * fgs[1]) + (fgs[2] * fgs[2]));
      break;
    case 5:
      accelUpdateBins();
      /* a_pitch_temp = sqrt((fgs[1] * fgs[1]) + (fgs[2] * fgs[2])); */
      break;
    case 6:
      /* a_pitch_temp = atan2(-fgs[0], a_pitch_temp); */
      break;
    case 7:
      /* a_pitch_temp = (a_pitch_temp * 180) / M_PI; */
      break;
    case 8:
      /* a_pitch = (a_pitch * 0.9) + (a_pitch_temp * 0.1); */
      break;

    // Can have no higher than case 15
    default:
      break;
  }

  accel_tick++;
  if (accel_tick >= accel_wrap) accel_tick = 0;
}

void accelSend(uint8_t addr, uint8_t data) {
  Wire.beginTransmission(accel_addr);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
}

void accelInit() {
  accelSend(0x2A, 0x00);        // Standby to accept new settings
  accelSend(0x0E, 0x01);        // Set +-4g range
  accelSend(0x2A, 0b00011001);  // Set 100 samples/sec (every 20 frames) and active
}

void accelReadAxis(uint8_t axis) {
  Wire.beginTransmission(accel_addr);
  // v2 is a 12 bit value from 0 - 2047, -2048 to -1
  Wire.write(0x01 + (2 * axis));
  Wire.endTransmission(false);
  Wire.requestFrom((int)accel_addr, 2);

  // v2 stores the 8 MSB in the first register and the 4 LSB at the top of the second
  while (!Wire.available()); gs[axis] = Wire.read() << 4;
  while (!Wire.available()); gs[axis] |= Wire.read() >> 4;
}

void accelUpdateBins() {
  // Tracks the magnitude of acceleration
  // v1 max is ~ 2.55 - 2.64
  // v2 max is - 3.46
  a_speed = 0;
  for (uint8_t i = 0; i < ACCEL_BINS; i++) {
    if (a_mag > thresh_bins_p[i] || a_mag < thresh_bins_n[i]) {
      thresh_last[i] = 0;
      thresh_cnts[i] = constrain(thresh_cnts[i] + 1, 0, 200);
    }
    if (thresh_last[i] >= thresh_falloff) thresh_cnts[i] = 0;
    if (thresh_cnts[i] > thresh_target) a_speed = i + 1;
    thresh_last[i]++;
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
  }
  // v2 updates at 100/s or every 20 frames
  accel_addr = V2_ACCEL_ADDR;
  accel_counts = 20;
  accel_wrap = 20;
  thresh_falloff = 10;
  thresh_target = 5;
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
  if (a_speed < mode->thresh_ps) {
    arg0 = mode->args[0][0];
    arg1 = mode->args[0][1];
    arg2 = mode->args[0][2];
    arg3 = mode->args[0][3];
  } else if (a_speed < mode->thresh_pe) {
    as = a_speed - mode->thresh_ps;
    d = mode->thresh_pe - mode->thresh_ps;
    arg0 = interp(mode->args[0][0], mode->args[1][0], as, d);
    arg1 = interp(mode->args[0][1], mode->args[1][1], as, d);
    arg2 = interp(mode->args[0][2], mode->args[1][2], as, d);
    arg3 = interp(mode->args[0][3], mode->args[1][3], as, d);
  } else {
    arg0 = mode->args[1][0];
    arg1 = mode->args[1][1];
    arg2 = mode->args[1][2];
    arg3 = mode->args[1][3];
  }
}

inline void colorStatic(int8_t color) {
  r = mode->colors[0][color][0];
  g = mode->colors[0][color][1];
  b = mode->colors[0][color][2];
}

inline void colorFlux(int8_t color) {
  uint8_t as, d;
  if (a_speed <= mode->thresh_cs[0]) {
    r = mode->colors[0][color][0];
    g = mode->colors[0][color][1];
    b = mode->colors[0][color][2];
  } else if (a_speed < mode->thresh_ce[0]) {
    as = a_speed - mode->thresh_cs[0];
    d = mode->thresh_ce[0] - mode->thresh_cs[0];
    r = interp(mode->colors[0][color][0], mode->colors[1][color][0], as, d);
    g = interp(mode->colors[0][color][1], mode->colors[1][color][1], as, d);
    b = interp(mode->colors[0][color][2], mode->colors[1][color][2], as, d);
  } else if (a_speed <= mode->thresh_cs[1]) {
    r = mode->colors[1][color][0];
    g = mode->colors[1][color][1];
    b = mode->colors[1][color][2];
  } else if (a_speed < mode->thresh_ce[1]) {
    as = a_speed - mode->thresh_cs[1];
    d = mode->thresh_ce[1] - mode->thresh_cs[1];
    r = interp(mode->colors[1][color][0], mode->colors[2][color][0], as, d);
    g = interp(mode->colors[1][color][1], mode->colors[2][color][1], as, d);
    b = interp(mode->colors[1][color][2], mode->colors[2][color][2], as, d);
  } else {
    r = mode->colors[2][color][0];
    g = mode->colors[2][color][1];
    b = mode->colors[2][color][2];
  }
}

/* inline void colorGeo(int8_t color) { */
/*   uint8_t v; */
/*   if (a_pitch < 0) { */
/*     v = constrain(abs(a_pitch) / 10, 0, 8); */
/*     r = interp(mode->colors[0][color][0], mode->colors[1][color][0], v, 8); */
/*     g = interp(mode->colors[0][color][1], mode->colors[1][color][1], v, 8); */
/*     b = interp(mode->colors[0][color][2], mode->colors[1][color][2], v, 8); */
/*   } else { */
/*     v = constrain(a_pitch / 10, 0, 8); */
/*     r = interp(mode->colors[1][color][0], mode->colors[2][color][0], v, 8); */
/*     g = interp(mode->colors[1][color][1], mode->colors[2][color][1], v, 8); */
/*     b = interp(mode->colors[1][color][2], mode->colors[2][color][2], v, 8); */
/*   } */
/* } */

int8_t patternStrobe(uint8_t numc, uint8_t st, uint8_t bt, uint8_t lt,
    uint8_t repeat, uint8_t pick, uint8_t skip) {

  int8_t rtn = -1;
  uint8_t trip = 0;

  repeat = (repeat == 0) ? 1 : repeat;
  pick = (pick == 0) ? numc : pick;
  skip = (skip == 0 || skip > pick) ? pick : skip;

  // Keep trying until we find a valid trip (>0)
  while (trip == 0) {
    // c_c_..-- for numc
    if (segm == 2 * pick) {   trip = lt; rtn = -1; }
    else if (segm % 2 == 1) { trip = bt; rtn = -1; }
    else {                    trip = st; rtn = (segm / 2) + cidx; }

    // Go to next counter if trip is 0 (aka - don't show this segment!)
    if (trip == 0) {
      /* segm = (segm + 1) % ((2 * numc) + 1); */
      segm++;
      if (segm >= ((2 * pick) + 1)) {
        segm = 0;
        cntr++;
        if (cntr >= repeat) {
          cntr = 0;
          cidx += skip;
          if (cidx >= numc) {
            cidx = (pick == skip) ? 0 : cidx % numc;
          }
        }
      }
    }
  }

  // Increment tick and see if we tripped the wire.
  // If we tripped the wire, reset the tick and go to the next counter.
  // Recalculate the timings.
  tick++;
  if (tick >= trip) {
    tick = 0;
    segm++;
    if (segm >= ((2 * pick) + 1)) {
      segm = 0;
      cntr++;
      if (cntr >= repeat) {
        cntr = 0;
        cidx += skip;
        if (cidx >= numc) {
          cidx = (pick == skip) ? 0 : cidx % numc;
        }
      }
    }
    recalcArgs();
  }
  if (rtn >= numc) rtn = (pick == skip) ? -1 : rtn % numc;
  return rtn;
}

int8_t patternTracer(uint8_t numc, uint8_t cst, uint8_t cbt, uint8_t tst) {
  int8_t rtn = -1;
  uint8_t trip = 0;

  // Keep trying until we find a valid trip (>0)
  while (trip == 0) {
    // _c_t incremenet color
    if (segm == 0 || segm == 2) { trip = cbt; rtn = -1; }
    else if (segm == 1) {         trip = cst; rtn = cidx + 1; }
    else {                        trip = tst; rtn = 0; }

    // Go to next counter if trip is 0 (aka - don't show this segment!)
    // If we have to incremenet the color counter, do that as well.
    if (trip == 0) {
      segm++;
      if (segm >= 4) {
        segm = 0;
        cidx = (cidx + 1) % (numc - 1);
      }
    }
  }

  // Increment tick and see if we tripped the wire.
  // If we tripped the wire, reset the tick and go to the next counter.
  // When the counter wraps, increment the color counter.
  // Recalculate the timings.
  tick++;
  if (tick >= trip) {
    tick = 0;
    segm++;
    if (segm >= 4) {
      segm = 0;
      cidx = (cidx + 1) % (numc - 1);
    }
    recalcArgs();
  }
  return rtn;
}

int8_t patternVexer(uint8_t numc, uint8_t cst, uint8_t cbt, uint8_t tst, uint8_t tbt) {
  int8_t rtn = -1;
  uint8_t trip = 0;

  // Keep trying until we find a valid trip (>0)
  while (trip == 0) {
    if (segm == 0 || segm == 2) {   trip = cbt; rtn = -1; }
    else if (segm == 1) {           trip = cst; rtn = cidx + 1; }
    else if ((segm - 3) % 2 == 0) { trip = tbt; rtn = -1; }
    else {                          trip = tst; rtn = 0; }

    // Go to next counter if trip is 0 (aka - don't show this segment!)
    // If we have to incremenet the color counter, do that as well.
    // Recalculate the timings.
    if (trip == 0) {
      segm++;
      if (segm >= 12) {
        segm = 0;
        cidx = (cidx + 1) % (numc - 1);
      }
    }
  }

  // Increment tick and see if we tripped the wire.
  // If we tripped the wire, reset the tick and go to the next counter.
  // When the counter wraps, increment the color counter.
  // Recalculate the timings.
  tick++;
  if (tick >= trip) {
    tick = 0;
    segm++;
    if (segm >= 12) {
      segm = 0;
      cidx = (cidx + 1) % (numc - 1);
    }
    recalcArgs();
  }

  return rtn;
}

int8_t patternEdge(uint8_t numc, uint8_t cst, uint8_t cbt, uint8_t est, uint8_t ebt, uint8_t pick) {
  int8_t rtn = -1;
  uint8_t segm2;
  uint8_t trip = 0;

  pick = (pick == 0) ? numc : pick;

  // Keep trying until we find a valid trip (>0)
  while (trip == 0) {
    // Even segments are blanks. The first blank is the edge blank, the rest are color blank
    // Odd segments go from (numc-1) to 1, 0, 1 to (numc - 1)
    segm2 = segm / 2;
    if (segm % 2 == 0) {
      trip = (segm2 == 0) ? ebt : cbt;
      rtn = -1;
    } else {
      if (segm2 < (pick - 1)) {       trip = cst; rtn = (pick - 1) - segm2 + cidx; }
      else if (segm2 == (pick - 1)) { trip = est; rtn = cidx; }
      else {                          trip = cst; rtn = segm2 - (pick - 1) + cidx; }
    }

    // Go to next counter if trip is 0 (aka - don't show this segment!)
    if (trip == 0) {
      segm++;
      if (segm >= ((pick * 4) - 2)) {
        segm = 0;
        cidx += pick;
        if (cidx >= numc) {
          cidx = 0;
        }
      }
    }
  }

  // Increment tick and see if we tripped the wire.
  // If we tripped the wire, reset the tick and go to the next counter.
  // Recalculate the timings.
  tick++;
  if (tick >= trip) {
    tick = 0;
    segm++;
    if (segm >= ((pick * 4) - 2)) {
      segm = 0;
      cidx += pick;
      if (cidx >= numc) {
        cidx = 0;
      }
    }
    recalcArgs();
  }
  if (rtn >= numc) rtn = -1;
  return rtn;
}

void renderMode() {
  int8_t color = -1;

  /* Serial.println(arg0); */
  if (mode->pattern == P_STROBE)
    color = patternStrobe(mode->num_colors, arg0, arg1, arg2,
        mode->extr[0], mode->extr[1], mode->extr[2]);
  else if (mode->pattern == P_TRACER)
    color = patternTracer(mode->num_colors, arg0, arg1, arg2);
  else if (mode->pattern == P_VEXER)
    color = patternVexer(mode->num_colors, arg0, arg1, arg2, arg3);
  else if (mode->pattern == P_EDGE)
    color = patternEdge(mode->num_colors, arg0, arg1, arg2, arg3, mode->extr[0]);

  if (color < 0) {
    r = g = b = 0;
  } else {
    if (mode->color_func == C_STATIC)    colorStatic(color);
    else if (mode->color_func == C_FLUX) colorFlux(color);
    /* else if (mode->color_func == C_GEO)  colorGeo(color); */
  }
}


void changeMode(uint8_t i) {
  if (i < NUM_MODES) {
    cur_mode = i;
  } else if (i == SER_MODE_PREV) {
    cur_mode = (cur_mode + NUM_MODES - 1) % NUM_MODES;
  } else if (i == SER_MODE_NEXT) {
    cur_mode = (cur_mode + 1) % NUM_MODES;
  }

  tick = cidx = cntr = segm = 0;
  mode = &pms[cur_mode].m;
  recalcArgs();
}

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
