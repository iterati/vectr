#include <Arduino.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include "LowPower.h"
#include "elapsedMillis.h"
#include "vectr.h"

Led led = {PIN_R, PIN_G, PIN_B, 0, 0, 0};
PackedMode mode;
AccelData adata;

PatternState pattern_state[2];                    // Vectr uses 0, Primer both 0 and 1
int8_t (*pattern_funcs[7]) (PatternState*, bool); // Function pointers for the pattern functions

elapsedMicros limiter = 0; // Tracks the number of us since last frame was written
uint8_t accel_tick = 0;    // Tracks which stage of the accelerometer handling to perform
uint32_t since_trans = 0;  // Tracks frames since operating state change
uint8_t op_state;          // Tracks the current operating state
uint8_t new_state;         // Change this when you want to change operating state
uint8_t cur_mode = 0;      // Tracks the currently playing mode
uint8_t variant = 0;       // Tracks primer mode variant
uint8_t brightness = 0;    // Global brightness value

// Flux values for color set blending
uint8_t flux_v; // Distance into flux interpolation
uint8_t flux_d; // Distance between flux flux floor and ceiling
uint8_t flux_s; // First target set for flux (0 or 1)

// These are for linking to the UI
bool comm_link = false;    // True is a serial link has been established
int8_t gui_set = -1;       // For color preview, set to -1 to reset
int8_t gui_color = -1;     // For color preview, set to -1 to reset


//********************************************************************************
// Utilities
//********************************************************************************
// EEPROM wrappers
inline void ee_update(uint16_t addr, uint8_t val) {
  // Wait for EEPROM to be ready before updating
  while (!eeprom_is_ready());
  EEPROM.update(addr, val);
}

inline uint8_t ee_read(uint16_t addr) {
  // Wait for EEPROM to be ready before reading
  while (!eeprom_is_ready());
  return EEPROM.read(addr);
}


// Sleep functions
void _push_interrupt() {
  // Noop - just needs to run to intercept button press
}

void enter_sleep() {
  wdt_enable(WDTO_15MS);        // Enable the watchdog
  write_frame(0, 0, 0);         // Blank the LED
  ee_update(ADDR_SLEEPING, 1);  // Set the sleeping bit
  accel_standby();              // Standby the acceleromater
  digitalWrite(PIN_LDO, LOW);   // Deactivate the LDO
  while (true) {}               // Loop until watchdog bites
  // After this, the light will reset. The actual sleeping occurs in the setup state function.
}


// 8-bit interp
inline uint8_t interp(uint8_t m, uint8_t n, uint8_t d, uint8_t D) {
  int16_t o = n - m;
  return m + ((o * d) / D);
}


// NOP delay - delays a single cycle using a nop
void Ndelay(int cycles) {
  while (cycles > 0) {
    cycles--;
    __asm__("nop\n\t");
  }
}

// TWADC functions
inline void I2CADC_SDA_H_OUTPUT() { DDRC &= ~(1 << 4); }
inline void I2CADC_SDA_L_INPUT()  { DDRC |= (1 << 4); }
inline void I2CADC_SCL_H_OUTPUT() { DDRC &= ~(1 << 5); }
inline void I2CADC_SCL_L_INPUT()  { DDRC |= (1 << 5); }

void TWADC_write(uint8_t data) {
  uint8_t data_r = ~data;
  uint8_t i = 8;
  while (i > 0) {
    i--;
    pinMode(SDA_PIN, bitRead(data_r, i));
    I2CADC_SCL_H_OUTPUT();
    I2CADC_SCL_L_INPUT(); 
  }

AckThis:
  I2CADC_SCL_L_INPUT(); 
  I2CADC_SCL_H_OUTPUT();
  int ADCresult = analogRead(SCL_PIN);
  if (ADCresult < I2CADC_L) {
    goto AckThis;
  }
  I2CADC_SCL_L_INPUT();
}

uint8_t TWADC_read(bool ack) {
  uint8_t data = 0;
  uint8_t i = 8;
  while (i > 0) {
    i--;
    I2CADC_SDA_H_OUTPUT();
    I2CADC_SCL_H_OUTPUT();
    int result = analogRead(SDA_PIN);
    if (result < I2CADC_L) {
      data &= ~(1 << i);
    } else {
      data |= (1 << i);
    }
    I2CADC_SCL_L_INPUT();
  }

  if (ack) {
    I2CADC_SCL_L_INPUT();  Ndelay(TICK_DELAY);
    I2CADC_SDA_L_INPUT();
    I2CADC_SCL_H_OUTPUT(); Ndelay(TICK_DELAY);
    I2CADC_SCL_L_INPUT();  Ndelay(TICK_DELAY);
  } else {
AckThis:
    I2CADC_SCL_L_INPUT();  Ndelay(TICK_DELAY);
    I2CADC_SCL_H_OUTPUT(); Ndelay(TICK_DELAY);
    int result = analogRead(SCL_PIN);
    if (result < I2CADC_L) {
      goto AckThis;
    }
    I2CADC_SCL_L_INPUT();  Ndelay(TICK_DELAY);
  }
  return data;
}

void TWADC_write_w(uint8_t data) {
  uint8_t data_r = ~data;
  uint8_t i = 7;
  while (i > 0) {
    i--;
    pinMode(SDA_PIN, bitRead(data_r, i));
    I2CADC_SCL_H_OUTPUT();
    I2CADC_SCL_L_INPUT(); 
  }
  I2CADC_SDA_L_INPUT();
  I2CADC_SCL_H_OUTPUT();
  I2CADC_SCL_L_INPUT();
AckThis:
  I2CADC_SCL_L_INPUT();
  I2CADC_SCL_H_OUTPUT();
  int result = analogRead(SCL_PIN);
  if (result < I2CADC_L) {
    goto AckThis;
  }
  I2CADC_SCL_L_INPUT();
}

void TWADC_write_r(byte data) {
  uint8_t data_r = ~data;
  uint8_t i = 7;
  while (i > 0) {
    i--;
    pinMode(SDA_PIN, bitRead(data_r, i));
    I2CADC_SCL_H_OUTPUT();
    I2CADC_SCL_L_INPUT();
  }
  I2CADC_SDA_H_OUTPUT();
  I2CADC_SCL_H_OUTPUT();
  I2CADC_SCL_L_INPUT();
AckThis:
  I2CADC_SCL_L_INPUT();
  I2CADC_SCL_H_OUTPUT();
  int result = analogRead(SCL_PIN);
  if (result < I2CADC_L) {
    goto AckThis;
  }
  I2CADC_SCL_L_INPUT();
}

void TWADC_begin() {
  I2CADC_SCL_H_OUTPUT();

  I2CADC_SDA_H_OUTPUT();
  I2CADC_SDA_L_INPUT();

  I2CADC_SCL_H_OUTPUT();
  I2CADC_SCL_L_INPUT();
}

void TWADC_beginTransmission(uint8_t addr) {
  TWADC_begin();
  TWADC_write_w(addr);
}

void TWADC_endTransmission() {
  I2CADC_SDA_L_INPUT();
  I2CADC_SCL_H_OUTPUT();
  I2CADC_SDA_H_OUTPUT();
}

void TWADC_requestFrom(uint8_t addr, uint8_t data) {
  TWADC_begin();
  TWADC_write_w(addr);
  TWADC_write(data);
  TWADC_begin();
  TWADC_write_r(addr);
}

void TWADC_send(uint8_t dev_addr, uint8_t addr, uint8_t data) {
  TWADC_beginTransmission(ACCEL_ADDR);
  TWADC_write(addr);
  TWADC_write(data);
  TWADC_endTransmission();
  delay(1);
}

// Returns true if the EEPROM version matches the hex version
bool version_match() {
  for (uint8_t i = 0; i < 4; i++) {
    if (EEPROM_VERSION[i] != ee_read(ADDR_VERSION[i])) {
      return false;
    }
  }
  return true;
}

// Resets the EEPROM to "factory" settings
void reset_memory() {
  // Clear all memory
  for (int i = 0; i < 1024; i++) ee_update(i, 0);

  // Rewrite factory and version
  for (uint8_t m = 0; m < NUM_MODES; m++) {
    for (uint8_t b = 0; b < MODE_SIZE; b++) {
      ee_update((m * MODE_SIZE) + b, pgm_read_byte(&factory[m][b]));
    }

    if (m < 4) ee_update(ADDR_VERSION[m], EEPROM_VERSION[m]);
  }
}


//********************************************************************************
// Pattern functions
//********************************************************************************
int8_t pattern_strobe(PatternState *state, bool rend) {
  uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  uint8_t pick = constrain((state->args[0] == 0) ? numc : state->args[0], 1, numc);
  uint8_t skip = constrain((state->args[1] == 0) ? pick : state->args[1], 1, pick);
  uint8_t repeat = constrain(state->args[2], 1, MAX_REPEATS);

  uint8_t st = state->timings[0];
  uint8_t bt = state->timings[1];
  uint8_t tt = state->timings[2];

  if (st == 0 && bt == 0 && tt == 0) return -1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == M_VECTR) calc_vectr_state();
    state->tick = state->trip = 0;
    while (state->trip == 0) {
      state->segm++;
      if (state->segm >= ((2 * pick) + 1)) {
        state->segm = 0;
        state->cntr++;
        if (state->cntr >= repeat) {
          state->cntr = 0;
          state->cidx += skip;
          if (state->cidx >= numc) {
            state->cidx = (pick == skip) ? 0 : state->cidx % numc;
          }
        }
      }

      if (state->segm == 2 * pick)   state->trip = tt;
      else if (state->segm % 2 == 1) state->trip = st;
      else                           state->trip = bt;
    }
  }
  state->tick++;

  if (!rend) return -1;

  int8_t color = -1;
  if (state->segm % 2 == 1) color = (state->segm / 2) + state->cidx;
  else                      color = -1;

  if (color >= numc) color = (pick == skip) ? -1 : color % numc;
  return color;
}

int8_t pattern_vexer(PatternState *state, bool rend) {
  uint8_t numc = constrain(state->numc, 2, MAX_COLORS) - 1;

  uint8_t repeat_c = constrain(state->args[0], 1, MAX_REPEATS);
  uint8_t repeat_t = constrain(state->args[1], 1, MAX_REPEATS);

  uint8_t cst = state->timings[0];
  uint8_t cbt = state->timings[1];
  uint8_t tst = state->timings[2];
  uint8_t tbt = state->timings[3];
  uint8_t sbt = state->timings[4];

  if (cst == 0 && cbt == 0 && tst == 0 && tbt == 0 && sbt == 0) return -1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == M_VECTR) calc_vectr_state();
    state->tick = state->trip = 0;
    while (state->trip == 0) {
      state->segm++;
      if (state->segm >= 2) {
        state->segm = 0;
        state->cntr++;
        if (state->cntr >= repeat_c + repeat_t) {
          state->cntr = 0;
          state->cidx = (state->cidx + 1) % numc;
        }
      }

      if (state->segm == 0) {
        if (state->cntr == 0 || state->cntr == repeat_c) state->trip = sbt;
        else if (state->cntr < repeat_c)                 state->trip = cbt;
        else                                             state->trip = tbt;
      } else {
        if (state->cntr < repeat_c)                      state->trip = cst;
        else                                             state->trip = tst;
      }
    }
  }
  state->tick++;

  if (!rend) return -1;

  int8_t color = -1;
  if (state->segm == 0) {       color = -1;
  } else {
    if (state->cntr < repeat_c) color = state->cidx + 1;
    else                        color = 0;
  }
  return color;
}

int8_t pattern_edge(PatternState *state, bool rend) {
  uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  uint8_t pick = constrain((state->args[0] == 0) ? numc : state->args[0], 1, numc);

  uint8_t cst = state->timings[0];
  uint8_t cbt = state->timings[1];
  uint8_t est = state->timings[2];
  uint8_t ebt = state->timings[3];

  if (cst == 0 && cbt == 0 && est == 0 && ebt == 0) return -1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == M_VECTR) calc_vectr_state();
    state->tick = state->trip = 0;
    while (state->trip == 0) {
      state->segm++;
      if (state->segm > 2) {
        state->segm = 0;
        state->cntr++;
        if (state->cntr >= (2 * pick) - 1) {
          state->cntr = 0;
          state->cidx += pick;
          if (state->cidx >= numc) {
            state->cidx = 0;
          }
        }
      }

      if (state->segm == 0) {
        if (state->cntr == 0)          state->trip = ebt;
        else                           state->trip = cbt;
      } else {
        if (state->cntr == (pick - 1)) state->trip = est;
        else                           state->trip = cst;
      }
    }
  }
  state->tick++;

  if (!rend) return -1;

  int8_t color = -1;
  if (state->segm == 0) color = -1;
  else                  color = abs((int)(pick - 1) - state->cntr) + state->cidx;

  if (color >= numc) color = -1;
  return color;
}

int8_t pattern_triple(PatternState *state, bool rend) {
  uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  uint8_t repeat_a = constrain(state->args[0], 1, MAX_REPEATS);
  uint8_t repeat_b = constrain(state->args[1], 1, MAX_REPEATS);
  uint8_t repeat_c = constrain(state->args[2], 1, MAX_REPEATS);
  uint8_t skip = constrain(state->args[3], 0, numc - 1);
  uint8_t use_c = state->args[4];

  uint8_t ast = state->timings[0];
  uint8_t abt = state->timings[1];
  uint8_t bst = state->timings[2];
  uint8_t bbt = state->timings[3];
  uint8_t cst = state->timings[4];
  uint8_t cbt = state->timings[5];
  uint8_t sbt = state->timings[6];

  uint8_t repeats = repeat_a + repeat_b;
  if (use_c) repeats += repeat_c;

  if (ast == 0 && abt == 0 && bst == 0 && bbt == 0 && cst == 0 && cbt == 0 && sbt == 0) return -1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == M_VECTR) calc_vectr_state();
    state->tick = state->trip = 0;
    while (state->trip == 0) {
      state->segm++;
      if (state->segm >= 2) {
        state->segm = 0;
        state->cntr++;
        if (state->cntr >= repeats) {
          state->cntr = 0;
          state->cidx = (state->cidx + 1) % numc;
        }
      }

      if (state->segm == 0) {
        if (state->cntr == 0)                        state->trip = sbt;
        else if (state->cntr < repeat_a)             state->trip = abt;
        else if (state->cntr == repeat_a)            state->trip = sbt;
        else if (state->cntr < repeat_a + repeat_b)  state->trip = bbt;
        else if (state->cntr == repeat_a + repeat_b) state->trip = sbt;
        else                                         state->trip = cbt;
      } else {
        if (state->cntr < repeat_a)                  state->trip = ast;
        else if (state->cntr < repeat_a + repeat_b)  state->trip = bst;
        else                                         state->trip = cst;
      }
    }
  }
  state->tick++;

  if (!rend) return -1;

  int8_t color = -1;
  if (state->segm == 0) {
    color = -1;
  } else {
    if (state->cntr < repeat_a)                 color = state->cidx;
    else if (state->cntr < repeat_a + repeat_b) color = (state->cidx + skip) % numc;
    else                                        color = (state->cidx + skip + skip) % numc;
  }
  return color;
}

int8_t pattern_runner(PatternState *state, bool rend) {
  uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  uint8_t pick = constrain((state->args[0] == 0) ? numc - 1 : state->args[0], 1, numc);
  uint8_t skip = constrain((state->args[1] == 0) ? pick : state->args[1], 1, pick);
  uint8_t repeat = constrain((state->args[2] == 0) ? pick : state->args[2], 1, MAX_REPEATS);

  uint8_t cst = state->timings[0];
  uint8_t cbt = state->timings[1];
  uint8_t rst = state->timings[2];
  uint8_t rbt = state->timings[3];
  uint8_t sbt = state->timings[4];

  if (cst == 0 && cbt == 0 && rst == 0 && rbt == 0 && sbt == 0) return -1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == M_VECTR) calc_vectr_state();
    state->tick = state->trip = 0;
    while (state->trip == 0) {
      state->segm++;
      if (state->segm >= 2) {
        state->segm = 0;
        state->cntr++;
        if (state->cntr >= repeat + pick) {
          state->cntr = 0;
          state->cidx += skip;
          if (state->cidx >= (numc - 1)) {
            state->cidx = (pick == skip) ? 0 : state->cidx % (numc - 1);
          }
        }
      }

      if (state->segm == 0) {
        if (state->cntr == 0 || state->cntr == pick) state->trip = sbt;
        else if (state->cntr < pick)                 state->trip = cbt;
        else                                         state->trip = rbt;
      } else {
        if (state->cntr < pick)                      state->trip = cst;
        else                                         state->trip = rst;
      }
    }
  }
  state->tick++;

  if (!rend) return -1;

  int8_t color = -1;
  if (state->segm == 0) {
    color = -1;
  } else {
    if (state->cntr < pick) color = state->cidx + state->cntr + 1;
    else                    color = 0;
  }

  if (color >= numc) color = (pick == skip) ? -1 : (color % (numc - 1) + 1);
  return color;
}

int8_t pattern_stepper(PatternState *state, bool rend) {
  uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  uint8_t steps = constrain(state->args[0], 1, 7);
  uint8_t random_step = state->args[1];
  uint8_t random_color = state->args[2];

  uint8_t bt = state->timings[0];
  uint8_t ct[7] = {state->timings[1], state->timings[2], state->timings[3],
    state->timings[4], state->timings[5], state->timings[6], state->timings[7]};

  if (bt == 0 && ct[0] == 0 && ct[1] == 0 && ct[2] == 0 && ct[3] == 0 && ct[4] == 0 && ct[5] == 0 && ct[6] == 0) return -1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == M_VECTR) calc_vectr_state();
    state->tick = state->trip = 0;
    while (state->trip == 0) {
      state->segm++;
      if (state->segm >= 2) {
        state->segm = 0;
        state->cidx = (rend && random_color) ? random(0, numc) : (state->cidx + 1) % numc;
        state->cntr = (rend && random_step) ? random(0, steps) : (state->cntr + 1) % steps;
      }

      if (state->segm == 0) state->trip = bt;
      else                  state->trip = ct[state->cntr];
    }
  }
  state->tick++;

  return (state->segm == 0) ? -1 : state->cidx;
}

int8_t pattern_random(PatternState *state, bool rend) {
  uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  uint8_t random_color = state->args[0];
  uint8_t multiplier = constrain(state->args[1], 1, 10);

  uint8_t ctl = min(state->timings[0], state->timings[1]);
  uint8_t cth = max(state->timings[0], state->timings[1]);
  uint8_t btl = min(state->timings[2], state->timings[3]);
  uint8_t bth = max(state->timings[2], state->timings[3]);

  if (ctl == 0 && cth == 0 && btl == 0 && bth ==0) return -1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == M_VECTR) calc_vectr_state();
    state->tick = state->trip = 0;
    while (state->trip == 0) {
      state->segm++;
      if (state->segm >= 2) {
        state->segm = 0;
        state->cidx = (rend && random_color) ? random(0, numc) : (state->cidx + 1) % numc;
      }

      if (rend) {
        if (state->segm == 0) state->trip = random(ctl, cth + 1) * multiplier;
        else                  state->trip = random(btl, bth + 1) * multiplier;
      } else {
        state->trip = 1;
      }
    }
  }
  state->tick++;

  return (state->segm == 0) ? state->cidx : -1;
}

int8_t pattern_flux(PatternState *state, bool rend) {
  const uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  const uint8_t steps = constrain(state->args[0], 1, MAX_REPEATS);
  const uint8_t s_or_b = state->args[1] % 2;
  const uint8_t direc = state->args[2] % 3;

  const uint8_t st = state->timings[0];
  const uint8_t bt = state->timings[1];
  const uint8_t ft = state->timings[2];

  const uint8_t t_steps = (direc == 2) ? 2 * steps : steps + 1;

  if (st == 0 && bt == 0) return -1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == M_VECTR) calc_vectr_state();
    state->tick = state->trip = 0;
    while (state->trip == 0) {
      state->segm++;
      if (state->segm >= 3) {
        state->segm = 0;
        state->cidx = (state->cidx + 1) % numc;
        state->cntr = (state->cntr + 1) % t_steps;
      }

      if (state->segm == 0) {
        state->trip = bt;
      } else if (state->segm == 1) {
        if (direc == 0) {
          state->trip = state->cntr * ft;
        } else if (direc == 1) {
          state->trip = (steps - state->cntr) * ft;
        } else {
          if (state->cntr < steps) {
            state->trip = state->cntr * ft;
          } else {
            state->trip = (steps - (state->cntr - steps)) * ft;
          }
        }
      } else {
        state->trip = st;
      }
    }
  }
  state->tick++;

  if (!rend) return -1;

  if (state->segm == 0) {
    return -1;
  } else if (state->segm == 1) {
    if (s_or_b == 0) {
      return state->cidx;
    } else {
      return -1;
    }
  } else {
    return state->cidx;
  }
}


// Core call here - this is what limits the speed of the firmware.
// This is necessary so that flash is a blocking call but adheres to the speed limit.
void write_frame(uint8_t r, uint8_t g, uint8_t b) {
  /* if (limiter > 64000) { Serial.print(limiter); Serial.print(F("\t")); Serial.println(accel_tick); } */

  // Hold until it's been 1ms since last write, then reset limiter
  while (limiter < 64000) {}
  limiter = 0;

  // Write out color factoring in brightness setting
  analogWrite(led.pin_r, r >> brightness);
  analogWrite(led.pin_g, g >> brightness);
  analogWrite(led.pin_b, b >> brightness);
}

// Flashes 50ms on/50ms off flashes times
void flash(uint8_t r, uint8_t g, uint8_t b, uint8_t flashes) {
  for (uint16_t i = 0; i < flashes * 100; i++) {
    if (i % 100 < 50) write_frame(r, g, b);
    else              write_frame(0, 0, 0);
  }

  // Update how long since last operating state change
  since_trans += flashes * 100;
}


// Accelerometer functions
// Initializes the accelerometer settings
void accel_init() {
  TWADC_begin(); delay(1);
  TWADC_send(ACCEL_ADDR, 0x2A, B00000000); // Standby to accept new settings
  TWADC_send(ACCEL_ADDR, 0x0E, B00000001); // Set +-4g range
  TWADC_send(ACCEL_ADDR, 0x2B, B00011011); // Low Power SLEEP
  TWADC_send(ACCEL_ADDR, 0x2C, B00111000);
  TWADC_send(ACCEL_ADDR, 0x2D, B00000000); 
  TWADC_send(ACCEL_ADDR, 0x2A, B00100001); // Set 50 samples/sec (every 20 frames) and active
}

void accel_standby() {
  TWADC_send(ACCEL_ADDR, 0x2A, 0x00);
}


//********************************************************************************
// Calculate pattern states and colors
//********************************************************************************
// Pattern State setup functions
// Primer states just need to be initialized when the mode changes
void calc_primer_states() {
  for (uint8_t s = 0; s < 2; s++) {
    pattern_state[s].pattern = constrain(mode.pm.pattern[s], 0, NUM_MODES - 1);
    pattern_state[s].numc = mode.pm.numc[s];
    for (uint8_t i = 0; i < 8; i++) {
      if (i < 5) {
        pattern_state[s].args[i] = mode.pm.args[s][i];
      }
      pattern_state[s].timings[i] = mode.pm.timings[s][i];
    }
  }
}

// Vectr state needs updated whenever a pattern segment finishes
// This must be called from inside the pattern function itself
void calc_vectr_state() {
  pattern_state[0].pattern = constrain(mode.vm.pattern, 0, NUM_MODES - 1);

  // Determine the color set based on velocity and flux values
  if (adata.velocity <= mode.vm.tr_flux[0]) {
    // Before first slider (all 0)
    pattern_state[0].numc = mode.vm.numc[0];
    flux_v = 0;
    flux_d = 1;
    flux_s = 0;
  } else if (adata.velocity < mode.vm.tr_flux[1]) {
    // Between first and second slider (blend 0 to 1)
    pattern_state[0].numc = min(mode.vm.numc[0], mode.vm.numc[1]);
    flux_v = adata.velocity - mode.vm.tr_flux[0];
    flux_d = mode.vm.tr_flux[1] - mode.vm.tr_flux[0];
    flux_s = 0;
  } else if (adata.velocity <= mode.vm.tr_flux[2]) {
    // Between second and third slider (all 1)
    pattern_state[0].numc = mode.vm.numc[1];
    flux_v = 0;
    flux_d = 1;
    flux_s = 1;
  } else if (adata.velocity < mode.vm.tr_flux[3]) {
    // Between third and fourth slider (blend 1 to 2)
    pattern_state[0].numc = min(mode.vm.numc[1], mode.vm.numc[2]);
    flux_v = adata.velocity - mode.vm.tr_flux[2];
    flux_d = mode.vm.tr_flux[3] - mode.vm.tr_flux[2];
    flux_s = 1;
  } else {
    // After fourth sider (all 2)
    pattern_state[0].numc = mode.vm.numc[2];
    flux_v = 1;
    flux_d = 1;
    flux_s = 1;
  }

  // Determine pattern timings based on meta values
  uint8_t v, d, s;
  if (adata.velocity <= mode.vm.tr_meta[0]) {
    // Before first slider (all 0)
    v = 0;
    d = 1;
    s = 0;
  } else if (adata.velocity < mode.vm.tr_meta[1]) {
    // Between first and second slider (blend 0 to 1)
    v = adata.velocity - mode.vm.tr_meta[0];
    d = mode.vm.tr_meta[1] - mode.vm.tr_meta[0];
    s = 0;
  } else if (adata.velocity <= mode.vm.tr_meta[2]) {
    // Between second and third slider (all 1)
    v = 0;
    d = 1;
    s = 1;
  } else if (adata.velocity < mode.vm.tr_meta[3]) {
    // Between third and fourth slider (blend 1 to 2)
    v = adata.velocity - mode.vm.tr_meta[2];
    d = mode.vm.tr_meta[3] - mode.vm.tr_meta[2];
    s = 1;
  } else {
    // After fourth slider (all 2)
    v = 1;
    d = 1;
    s = 1;
  }

  for (uint8_t i = 0; i < 8; i++) {
    // Patterns are just pulled in - no interpolation needed
    if (i < 5) pattern_state[0].args[i] = mode.vm.args[i];
    // interpolate the pattern timings
    pattern_state[0].timings[i] =
      interp(mode.vm.timings[s][i], mode.vm.timings[s + 1][i], v, d);
  }
}

// Interpolates the color based on the color index
void calc_color(uint8_t color) {
  led.r = interp(mode.vm.colors[flux_s][color][0], mode.vm.colors[flux_s + 1][color][0], flux_v, flux_d);
  led.g = interp(mode.vm.colors[flux_s][color][1], mode.vm.colors[flux_s + 1][color][1], flux_v, flux_d);
  led.b = interp(mode.vm.colors[flux_s][color][2], mode.vm.colors[flux_s + 1][color][2], flux_v, flux_d);
}


//********************************************************************************
// Functions for changing modes
//********************************************************************************
void reset_state(PatternState *state) {
  state->tick = state->trip = state->cidx = state->cntr = state->segm = 0;
}

void load_mode(uint8_t m) {
  for (uint8_t b = 0; b < MODE_SIZE; b++) {
    mode.data[b] = ee_read((m * MODE_SIZE) + b);
  }
}

void reset_mode() {
  gui_set = gui_color = -1;
  variant = 0;
  reset_state(&pattern_state[0]);
  reset_state(&pattern_state[1]);
  if (mode.data[0] == M_VECTR) {
    calc_vectr_state();
  } else if (mode.data[0] == M_PRIMER) {
    calc_primer_states();
  }
}

void change_mode(uint8_t i) {
  if (i < NUM_MODES) cur_mode = i;
  else if (i == 99)  cur_mode = (cur_mode + NUM_MODES - 1) % NUM_MODES;
  else if (i == 101) cur_mode = (cur_mode + 1) % NUM_MODES;

  load_mode(cur_mode);
  reset_mode();
}


//********************************************************************************
// handle_accel
// These functions handle spreading the accelerometer handling over just enough
// frames to keep the overall execution time of the accel handling to under 250us
// per frame. This allows at least 750us for handling other aspects of the light.
// Assume you will always have 750us to do all non-accel parts of a frame.
// Due to the logic being spread over 13 frames, this adds a 13ms delay to Primer
// variant selection and 10ms delay to Vectr
//********************************************************************************
void accel_velocity() {
  uint8_t i = 0;
  uint16_t bin_thresh = ACCEL_ONEG;
  adata.velocity = 0;

  // Go through each bin, if the accel magnitude is over the treshold,
  // reset the falloff counter and increment the trigger counter
  // If the falloff counter is over the falloff value, reset the trigger counter
  // Otherwise increment the falloff counter
  // If the trigger counter is over the trigger value, the velocity is at least this high
  while (i < ACCEL_BINS) {
    bin_thresh += ACCEL_BIN_SIZE;
    if (adata.mag > bin_thresh) {
      adata.vectr_falloff[i] = 0;
      adata.vectr_trigger[i] = min(adata.vectr_trigger[i] + 1, 128);
    }

    if (adata.vectr_falloff[i] >= ACCEL_FALLOFF) { adata.vectr_trigger[i] = 0; }
    else                                         { adata.vectr_falloff[i]++; }

    if (adata.vectr_trigger[i] > ACCEL_TARGET) {
      adata.velocity = i + 1;
    }
    i++;
  }
}

void accel_variant() {
  uint8_t v = 0;

  // Figure out what value to trigger on
  if (mode.pm.trigger_mode == T_VELOCITY)   { v = adata.velocity; }
  else if (mode.pm.trigger_mode == T_PITCH) { v = adata.pitch; }
  else if (mode.pm.trigger_mode == T_ROLL)  { v = adata.roll; }
  else if (mode.pm.trigger_mode == T_FLIP)  { v = adata.flip; }

  // See if we're over/under the trigger threshold
  // If we are, reset the falloff counter and add to the trigger counter
  if ((variant == 0 && v > mode.pm.trigger_thresh[0]) ||
      (variant == 1 && v < mode.pm.trigger_thresh[1])) {
    adata.prime_falloff = 0;
    adata.prime_trigger = min(adata.prime_trigger + 1, 128);
  }

  // If we're past the falloff, reset the counter, otherwise just increment the falloff counter
  if (adata.prime_falloff >= ACCEL_FALLOFF) { adata.prime_trigger = 0; }
  else                                      { adata.prime_falloff++; }

  // If the counter is over the target, switch primes and reset the counters
  if (adata.prime_trigger >= ACCEL_TARGET) {
    adata.prime_falloff = adata.prime_trigger = 0;
    variant = (variant + 1) % 2;
  }
}

void handle_accel() {
  switch (accel_tick % ACCEL_COUNTS) {
    case 0:  // Request X axis
      TWADC_requestFrom(ACCEL_ADDR, 3);
      break;

    case 1:  // Get X MSB
      adata.gs[0] = (int16_t)TWADC_read(1) << 8;
      break;

    case 2:  // Get X LSB
      adata.gs[0] = (adata.gs[0] | TWADC_read(0)) >> 4;
      TWADC_endTransmission();
      break;

    case 3:  // Request Y axis
      TWADC_requestFrom(ACCEL_ADDR, 1);
      break;

    case 4:  // Get Y MSB
      adata.gs[1] = (int16_t)TWADC_read(1) << 8;
      break;

    case 5:  // Get Y LSB
      adata.gs[1] = (adata.gs[1] | TWADC_read(0)) >> 4;
      TWADC_endTransmission();
      break;

    case 6:  // Request Z axis
      TWADC_requestFrom(ACCEL_ADDR, 5);
      break;

    case 7:  // Get Z MSB
      adata.gs[2] = (int16_t)TWADC_read(1) << 8;
      break;

    case 8:  // Get Z LSB
      adata.gs[2] = (adata.gs[2] | TWADC_read(0)) >> 4;
      TWADC_endTransmission();
      break;

    case 9:  // Calculate the velocity and do the first half of the pitch calculation
             // This finishes Vectr mode ish
      adata.gs2[0] = pow(adata.gs[0], 2);
      adata.gs2[1] = pow(adata.gs[1], 2);
      adata.gs2[2] = pow(adata.gs[2], 2);
      adata.mag = sqrt(adata.gs2[0] + adata.gs2[1] + adata.gs2[2]);
      adata.pitch = sqrt(adata.gs2[1] + adata.gs2[2]);
      accel_velocity();
      break;

    case 10: // Calculate the roll
      adata.roll = atan2(-adata.gs[1], adata.gs[2]);
      break;

    case 11: // Finish calculating pitch
      adata.pitch = atan2(adata.gs[0], adata.pitch);
      break;

    case 12: // Fimish Primer mode ish by normalizing the flip, pitch, and roll to 0-32
      adata.flip = 32 - ((constrain(adata.gs[2], -496, 496) + 496) / 31);
      adata.pitch = constrain((adata.pitch + ACCEL_OFFSET) * ACCEL_COEF, 0, 32);
      adata.roll = (adata.roll > M_PI_2) ?  M_PI - adata.roll : -M_PI - adata.roll;
      adata.roll = constrain((adata.roll + ACCEL_OFFSET) * ACCEL_COEF, 0, 32);
      accel_variant();
      break;

    default:
      break;
  }

  accel_tick++;
  if (accel_tick >= ACCEL_COUNTS) accel_tick = 0;
}


//********************************************************************************
// handle_button
// This large ass switch statement is the operating state machine for handling
// button presses and interacting with the light.
// To add new features, you basically need to add a new transition into a _WAIT
// state for the feature that will trigger when the button is released.
//********************************************************************************
void handle_button() {
  bool pressed = digitalRead(PIN_BUTTON) == LOW;
  switch (op_state) {
    //****************************************************************************
    // Play Mode
    //****************************************************************************
    case S_PLAY_OFF:
      if (pressed && since_trans >= PRESS_DELAY) {
        new_state = S_PLAY_PRESSED;
      }
      break;

    case S_PLAY_PRESSED:
      if (!pressed) {
        change_mode(101);
        new_state = S_PLAY_OFF;
      } else if (since_trans >= SHORT_HOLD) {
        new_state = S_PLAY_SLEEP_WAIT;
      }
      break;

    case S_PLAY_SLEEP_WAIT:
      if (!pressed) {
        enter_sleep();
      } else if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_CONJURE_WAIT;
      }
      break;

    case S_PLAY_CONJURE_WAIT:
      if (since_trans == 0) flash(0, 0, 128, 5);
      if (!pressed) {
        ee_update(ADDR_CONJURE, 1);
        new_state = S_CONJURE_OFF;
      } else if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_LOCK_WAIT;
      }
      break;

    case S_PLAY_LOCK_WAIT:
      if (since_trans == 0) flash(128, 0, 0, 5);
      if (!pressed) {
        ee_update(ADDR_LOCKED, 1);
        enter_sleep();
      } else if (since_trans >= LONG_HOLD) {
        flash(48, 48, 48, 5);
        new_state = S_PLAY_SLEEP_WAIT;
      }
      break;

    //****************************************************************************
    // Conjure Mode
    //****************************************************************************
    case S_CONJURE_OFF:
      if (pressed && since_trans >= PRESS_DELAY) {
        new_state = S_CONJURE_PRESS;
      }
      break;

    case S_CONJURE_PRESS:
      if (!pressed) {
        ee_update(ADDR_CONJURE_MODE, cur_mode);
        enter_sleep();
      } else if (since_trans >= LONG_HOLD) {
        new_state = S_CONJURE_PLAY_WAIT;
      }
      break;

    case S_CONJURE_PLAY_WAIT:
      if (since_trans == 0) flash(0, 0, 128, 5);
      if (!pressed) {
        ee_update(ADDR_CONJURE, 0);
        new_state = S_PLAY_OFF;
      }
      break;

    //****************************************************************************
    //  Sleeping
    //****************************************************************************
    case S_SLEEP_WAKE:
      if (!pressed) {
        if (ee_read(ADDR_CONJURE)) {
          change_mode(ee_read(ADDR_CONJURE_MODE));
          new_state = S_CONJURE_OFF;
        } else {
          new_state = S_PLAY_OFF;
        }
      } else if (since_trans >= LONG_HOLD) {
        new_state = S_SLEEP_BRIGHT_WAIT;
      }
      break;

    case S_SLEEP_BRIGHT_WAIT:
      if (since_trans == 0) flash(128, 128, 128, 5);
      if (!pressed) {
        new_state = S_BRIGHT_OFF;
      } else if (since_trans >= VERY_LONG_HOLD) {
        new_state = S_SLEEP_RESET_WAIT;
      }
      break;

    case S_SLEEP_RESET_WAIT:
      if (since_trans == 0) flash(128, 0, 0, 5);
      if (!pressed) {
        new_state = S_RESET_OFF;
      } else if (since_trans >= VERY_LONG_HOLD) {
        new_state = S_SLEEP_HELD;
      }
      break;

    case S_SLEEP_HELD:
      if (!pressed) {
        enter_sleep();
      }
      break;

    //****************************************************************************
    // Sleep lock
    //****************************************************************************
    case S_SLEEP_LOCK:
      if (since_trans == VERY_LONG_HOLD) flash(0, 128, 0, 5);
      if (!pressed) {
        if (since_trans > VERY_LONG_HOLD) {
          ee_update(ADDR_LOCKED, 0);
          if (ee_read(ADDR_CONJURE)) {
            change_mode(ee_read(ADDR_CONJURE_MODE));
            new_state = S_CONJURE_OFF;
          } else {
            new_state = S_PLAY_OFF;
          }
        } else {
          flash(128, 0, 0, 5);
          enter_sleep();
        }
      }
      break;

    //****************************************************************************
    // Master Reset
    //****************************************************************************
    case S_RESET_OFF:
      if (pressed) {
        new_state = S_RESET_PRESSED;
      } else if (since_trans >= VERY_LONG_HOLD) {
        enter_sleep();
      }
      break;

    case S_RESET_PRESSED:
      if (since_trans == VERY_LONG_HOLD) flash(128, 0, 0, 5);
      if (!pressed) {
        if (since_trans >= VERY_LONG_HOLD) {
          reset_memory();
          flash(128, 128, 128, 5);
          enter_sleep();
        } else {
          enter_sleep();
        }
      } else if (since_trans >= VERY_LONG_HOLD * 4) {
        enter_sleep();
      }
      break;

    //****************************************************************************
    // Brightness Settings
    //****************************************************************************
    case S_BRIGHT_OFF:
      if (pressed && since_trans >= PRESS_DELAY) {
        new_state = S_BRIGHT_PRESSED;
      }
      break;

    case S_BRIGHT_PRESSED:
      if (since_trans == LONG_HOLD) flash(128, 128, 128, 5);
      if (!pressed) {
        if (since_trans >= LONG_HOLD) {
          ee_update(ADDR_BRIGHTNESS, brightness);
          new_state = S_PLAY_OFF;
        } else {
          brightness = (brightness + 1) % 3;
          new_state = S_BRIGHT_OFF;
        }
      }
      break;

    default:
      break;
  }

  if (op_state != new_state) {
    op_state = new_state;
    since_trans = 0;
  } else {
    since_trans++;
  }
}


//********************************************************************************
// handle_serial
// These functions are all for handling serial communications. Unless you're
// writing your own UI for Vectr, do not fuck with this. If you are writing your
// own GUI for Vectr you should probably still not fuck with this and instead use
// the existing serial protocol.
//********************************************************************************
void ser_dump() {
  Serial.write(200); Serial.write(cur_mode); Serial.write(200);
  for (uint8_t b = 0; b < MODE_SIZE; b++) {
    Serial.write(cur_mode); Serial.write(b); Serial.write(mode.data[b]);
  }
  Serial.write(210); Serial.write(cur_mode); Serial.write(210);
}

void ser_dump_light() {
  for (uint8_t m = 0; m < NUM_MODES; m++) {
    for (uint8_t b = 0; b < MODE_SIZE; b++) {
      Serial.write(m); Serial.write(b); Serial.write(ee_read((m * MODE_SIZE) + b));
    }
  }
  Serial.write(SER_DUMP_LIGHT); Serial.write(0); Serial.write(0);
}

void ser_save() {
  for (uint8_t b = 0; b < MODE_SIZE; b++) {
    ee_update((cur_mode * MODE_SIZE) + b, mode.data[b]);
  }
}

void ser_read(uint8_t b) {
  if (b < MODE_SIZE) {
    Serial.write(cur_mode); Serial.write(b); Serial.write(mode.data[b]);
  }
}

void ser_write(uint8_t b, uint8_t v) {
  if (b < MODE_SIZE) {
    mode.data[b] = v;
  }
}

void ser_write_light(uint8_t m, uint8_t b, uint8_t v) {
  if (m < NUM_MODES && b < MODE_SIZE) {
    ee_update((m * MODE_SIZE) + b, v);
  }
}

void handle_serial() {
  uint8_t cmd, in0, in1, in2;
  while (Serial.available() >= 4) {
    cmd = Serial.read();
    in0 = Serial.read();
    in1 = Serial.read();
    in2 = Serial.read();

    if (cmd == SER_HANDSHAKE) {
      // Initial handshake: 250 VERSION VERSION
      if (in0 == SER_VERSION && in1 == SER_VERSION) {
        new_state = S_VIEW_MODE;
        comm_link = true;
        Serial.write(251); Serial.write(cur_mode); Serial.write(SER_VERSION);
      }
    } else if (comm_link) {
      if (cmd == SER_DUMP) {
        ser_dump();
      } else if (cmd == SER_DUMP_LIGHT) {
        ser_dump_light();
      } else if (cmd == SER_SAVE) {
        ser_save();
        flash(128, 128, 128, 5);
      } else if (cmd == SER_READ) {
        ser_read(in0);
      } else if (cmd == SER_WRITE) {
        ser_write(in0, in1);
        if (mode.data[0] == 1) {
          calc_primer_states();
        }
      } else if (cmd == SER_WRITE_LIGHT) {
        ser_write_light(in0, in1, in2);
      } else if (cmd == SER_WRITE_MODE) {
        new_state = S_MODE_WRITE;
      } else if (cmd == SER_WRITE_MODE_END) {
        reset_mode();
        new_state = S_VIEW_MODE;
      } else if (cmd == SER_CHANGE_MODE) {
        change_mode(in0);
        ser_dump();
      } else if (cmd == SER_RESET_MODE) {
        reset_mode();
      } else if (cmd == SER_VIEW_MODE) {
        new_state = S_VIEW_MODE;
      } else if (cmd == SER_VIEW_COLOR) {
        new_state = S_VIEW_COLOR;
        gui_set = in0;
        gui_color = in1;
      } else if (cmd == SER_DISCONNECT) {
        new_state = S_PLAY_OFF;
        comm_link = false;
      }
    }
  }
}


//********************************************************************************
// handle_render
//********************************************************************************
void _render_mode() {
  int8_t color;

  // Handle Vectr mode render
  if (mode.data[0] == M_VECTR) {
    // Run pattern func to get color index
    color = pattern_funcs[pattern_state[0].pattern](&pattern_state[0], true);

    // If color index is negative, render blank, otherwise perform color flux
    if (color < 0) {
      led.r = led.g = led.b = 0;
    } else {
      calc_color(color);
    }

  // Handle Primer mode render
  } else if (mode.data[0] == M_PRIMER) {
    // Render the color with the active variant and dummy render the inactive one
    color = pattern_funcs[pattern_state[variant].pattern](&pattern_state[variant], true);
    pattern_funcs[pattern_state[!variant].pattern](&pattern_state[!variant], false);

    // If color index is negative, render blank, otherwise render color for active variant
    if (color < 0) {
      led.r = led.g = led.b = 0;
    } else {
      led.r = mode.pm.colors[variant][color][0];
      led.g = mode.pm.colors[variant][color][1];
      led.b = mode.pm.colors[variant][color][2];
    }
  }
}

void handle_render() {
  switch (op_state) {
    case S_PLAY_OFF:
    case S_CONJURE_OFF:
    case S_VIEW_MODE:
      _render_mode();
      break;

    case S_VIEW_COLOR:
      if (mode.data[0] == 0) {
        gui_set = constrain(gui_set, 0, 2);
        gui_color = constrain(gui_color, 0, 8);
        led.r = mode.vm.colors[gui_set][gui_color][0];
        led.g = mode.vm.colors[gui_set][gui_color][1];
        led.b = mode.vm.colors[gui_set][gui_color][2];
      } else {
        gui_set = constrain(gui_set, 0, 1);
        gui_color = constrain(gui_color, 0, 8);
        led.r = mode.pm.colors[gui_set][gui_color][0];
        led.g = mode.pm.colors[gui_set][gui_color][1];
        led.b = mode.pm.colors[gui_set][gui_color][2];
      }
      break;

    case S_BRIGHT_OFF:
      led.r = led.g = led.b = 128;
      break;

    case S_RESET_OFF:
      led.r = 128; led.g = led.b = 0;
      break;

    default:
      led.r = led.g = led.b = 0;
      break;
  }
  write_frame(led.r, led.g, led.b);
}


//********************************************************************************
// Setup helpers
//********************************************************************************
void setup_state() {
  // Listen for button press
  attachInterrupt(0, _push_interrupt, FALLING);
  if (ee_read(ADDR_SLEEPING)) {
    // Disable sleeping bit
    ee_update(ADDR_SLEEPING, 0);

    // This is where we actually go to sleep
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);

    // On wake up, check lock state
    if (ee_read(ADDR_LOCKED)) {
      op_state = new_state = S_SLEEP_LOCK;
    } else {
      op_state = new_state = S_SLEEP_WAKE;
    }
  } else {
    // On reboot, check for conjure vs play state
    if (ee_read(ADDR_CONJURE)) {
      // Load current mode from memory when conjuring
      cur_mode = ee_read(ADDR_CONJURE_MODE);
      op_state = new_state = S_CONJURE_OFF;
    } else {
      op_state = new_state = S_PLAY_OFF;
    }
  }

  // Can stop listening to button now
  detachInterrupt(0);
}

void setup_pins() {
  // Set the button for input
  pinMode(PIN_BUTTON, INPUT);

  // Set the LED pins for output
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);

  // Power on the Low Drop Out voltage regulator
  pinMode(PIN_LDO, OUTPUT);
  digitalWrite(PIN_LDO, HIGH);

  // Set ADC prescale to 16 (fastest that TWADC will work with)
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  // Start up and initialize the acceleromater
  accel_init();
}

void setup_timers() {
  // Setup PWM speeds for timers
  noInterrupts();
  TCCR0B = (TCCR0B & 0b11111000) | 0b001;  // no prescaler ~64/ms
  TCCR1B = (TCCR1B & 0b11111000) | 0b001;  // no prescaler ~32/ms
  bitSet(TCCR1B, WGM12); // enable fast PWM                ~64/ms
  interrupts();
}

void setup_funcs() {
  pattern_funcs[P_STROBE]  = &pattern_strobe;
  pattern_funcs[P_VEXER]   = &pattern_vexer;
  pattern_funcs[P_EDGE]    = &pattern_edge;
  pattern_funcs[P_TRIPLE]  = &pattern_triple;
  pattern_funcs[P_RUNNER]  = &pattern_runner;
  pattern_funcs[P_STEPPER] = &pattern_stepper;
  pattern_funcs[P_RANDOM]  = &pattern_random;
}


//********************************************************************************
// Main functions
// Setup is the first thing that's ran when the light boots up.
// Loop is called over an over again until the world ends (or your batteries die)
//********************************************************************************
void setup() {
  randomSeed(analogRead(0));

  setup_state();
  setup_pins();
  setup_timers();
  setup_funcs();

  if (!version_match()) reset_memory();
  brightness = ee_read(ADDR_BRIGHTNESS);

  // Attempt handshake with GUI
  Serial.begin(115200);
  Serial.write(SER_HANDSHAKE); Serial.write(SER_VERSION); Serial.write(SER_VERSION);

  change_mode(cur_mode);
}

void loop() {
  handle_serial();
  handle_accel();
  handle_button();
  handle_render();
}
