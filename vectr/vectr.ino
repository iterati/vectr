#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include "LowPower.h"
#include "elapsedMillis.h"
#include "vectr.h"


//********************************************************************************
// Globals
//********************************************************************************
PackedMode mode;
/* PatternState pattern_state; */
/* PatternState pattern_state2; */
PatternState pattern_state[2];
AccelData adata;
Led led = {PIN_R, PIN_G, PIN_B, 0, 0, 0};
int8_t (*pattern_funcs[7]) (PatternState*);

elapsedMicros limiter = 0;
uint8_t accel_tick = 0;
uint32_t since_trans = 0;
uint8_t op_state, new_state;

uint8_t cur_mode = 0;
uint8_t brightness = 0;
uint8_t variant = 0;

bool comm_link = false;
int8_t gui_set = -1;
int8_t gui_color = -1;


//********************************************************************************
// Utility functions
//********************************************************************************
uint32_t deadbeef_seed;
uint32_t deadbeef_beef = 0xdeadbeef;
uint32_t deadbeef_rand() {
  deadbeef_seed = (deadbeef_seed << 7) ^ ((deadbeef_seed >> 25) + deadbeef_beef);
  deadbeef_beef = (deadbeef_beef << 7) ^ ((deadbeef_beef >> 25) + 0xdeadbeef);
  return deadbeef_seed;
}

uint32_t rand(int _max) {
  return rand(0, _max);
}

uint32_t rand(int _min, int _max) {
  return _min + (deadbeef_rand() % (_max - _min));
}

// EEPROM wrappers
void ee_update(uint16_t addr, uint8_t val) {
  while (!eeprom_is_ready());
  EEPROM.update(addr, val);
}

uint8_t ee_read(uint16_t addr) {
  while (!eeprom_is_ready());
  return EEPROM.read(addr);
}


// Sleep functions
void _push_interrupt() {}

void enter_sleep() {
  wdt_enable(WDTO_15MS);
  write_frame(0, 0, 0);
  ee_update(ADDR_SLEEPING, 1);
  accel_standby();
  digitalWrite(PIN_LDO, LOW);
  delay(640000);
}


// 8-bit interp
inline uint8_t interp(uint8_t m, uint8_t n, uint8_t d, uint8_t D) {
  int16_t o = n - m;
  return m + ((o * d) / D);
}


// Accelerometer functions
void accel_send(uint8_t addr, uint8_t data) {
  Wire.beginTransmission(V2_ACCEL_ADDR);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
}

void accel_init() {
  accel_send(0x2A, 0x00);        // Standby to accept new settings
  accel_send(0x0E, 0x01);        // Set +-4g range
  accel_send(0x2B, 0b00011000);  // Low Power SLEEP
  accel_send(0x2A, 0b00100001);  // Set 50 samples/sec (every 40 frames) and active
}

void accel_standby() {
  accel_send(0x2A, 0x00);
}

// If write_frame isn't called every 15ms, the chip will reset
void write_frame(uint8_t r, uint8_t g, uint8_t b) {
  /* if (limiter > 64000) { Serial.print(limiter); Serial.print(F("\t")); Serial.println(accel_tick); } */
  while (limiter < 64000) {}
  limiter = 0;

  analogWrite(led.pin_r, r >> brightness);
  analogWrite(led.pin_g, g >> brightness);
  analogWrite(led.pin_b, b >> brightness);
}

void flash(uint8_t r, uint8_t g, uint8_t b, uint8_t flashes) {
  for (uint16_t i = 0; i < flashes * 100; i++) {
    if (i % 100 < 50) write_frame(r, g, b);
    else              write_frame(0, 0, 0);
  }
  since_trans += flashes * 100;
}

void load_mode(uint8_t m) {
  for (uint8_t b = 0; b < MODE_SIZE; b++) {
    mode.data[b] = ee_read((m * MODE_SIZE) + b);
  }
}

void calc_primer_states() {
  for (uint8_t s = 0; s < 2; s++) {
    pattern_state[s].pattern = constrain(mode.pm.pattern[s], 0, 6);
    pattern_state[s].numc = mode.pm.numc[s];
    for (uint8_t i = 0; i < 8; i++) {
      if (i < 5) {
        pattern_state[s].args[i] = mode.pm.args[s][i];
      }
      pattern_state[s].timings[i] = mode.pm.timings[s][i];
    }
  }
  /* pattern_state.pattern = constrain(mode.pm.pattern[0], 0, 6); */
  /* pattern_state2.pattern = constrain(mode.pm.pattern[1], 0, 6); */
  /* pattern_state.numc = mode.pm.numc[0]; */
  /* pattern_state2.numc = mode.pm.numc[1]; */
  /* for (uint8_t i = 0; i < 8; i++) { */
  /*   if (i < 5) { */
  /*     pattern_state.args[i] = mode.pm.args[0][i]; */
  /*     pattern_state2.args[i] = mode.pm.args[1][i]; */
  /*   } */
  /*   pattern_state.timings[i] = mode.pm.timings[0][i]; */
  /*   pattern_state2.timings[i] = mode.pm.timings[1][i]; */
  /* } */
}

void calc_pattern_state() {
  pattern_state[0].pattern = constrain(mode.vm.pattern, 0, 6);

  if (adata.velocity <= mode.vm.tr_flux[0])      pattern_state[0].numc = mode.vm.numc[0];
  else if (adata.velocity < mode.vm.tr_flux[1])  pattern_state[0].numc = min(mode.vm.numc[0], mode.vm.numc[1]);
  else if (adata.velocity <= mode.vm.tr_flux[2]) pattern_state[0].numc = mode.vm.numc[1];
  else if (adata.velocity < mode.vm.tr_flux[3])  pattern_state[0].numc = min(mode.vm.numc[1], mode.vm.numc[2]);
  else                                           pattern_state[0].numc = mode.vm.numc[2];

  uint8_t v, d, s;
  if (adata.velocity <= mode.vm.tr_meta[0]) {
    v = 0; d = 1; s = 0;
  } else if (adata.velocity < mode.vm.tr_meta[1]) {
    v = adata.velocity - mode.vm.tr_meta[0]; d = mode.vm.tr_meta[1] - mode.vm.tr_meta[0]; s = 0;
  } else if (adata.velocity <= mode.vm.tr_meta[2]) {
    v = 0; d = 1; s = 1;
  } else if (adata.velocity < mode.vm.tr_meta[3]) {
    v = adata.velocity - mode.vm.tr_meta[2]; d = mode.vm.tr_meta[3] - mode.vm.tr_meta[2]; s = 1;
  } else {
    v = 1; d = 1; s = 1;
  }

  for (uint8_t i = 0; i < 8; i++) {
    if (i < 5) pattern_state[0].args[i] = mode.vm.args[i];
    pattern_state[0].timings[i] = interp(mode.vm.timings[s][i], mode.vm.timings[s + 1][i], v, d);
  }
}

void calc_color(uint8_t color) {
  uint8_t v, d, s;
  if (adata.velocity <= mode.vm.tr_flux[0]) {
    v = 0; d = 1; s = 0;
  } else if (adata.velocity < mode.vm.tr_flux[1]) {
    v = adata.velocity - mode.vm.tr_flux[0]; d = mode.vm.tr_flux[1] - mode.vm.tr_flux[0]; s = 0;
  } else if (adata.velocity <= mode.vm.tr_flux[2]) {
    v = 0; d = 1; s = 1;
  } else if (adata.velocity < mode.vm.tr_flux[3]) {
    v = adata.velocity - mode.vm.tr_flux[2]; d = mode.vm.tr_flux[3] - mode.vm.tr_flux[2]; s = 1;
  } else {
    v = 1; d = 1; s = 1;
  }
  led.r = interp(mode.vm.colors[s][color][0], mode.vm.colors[s + 1][color][0], v, d);
  led.g = interp(mode.vm.colors[s][color][1], mode.vm.colors[s + 1][color][1], v, d);
  led.b = interp(mode.vm.colors[s][color][2], mode.vm.colors[s + 1][color][2], v, d);
}

void change_mode(uint8_t i) {
  if (i < NUM_MODES) cur_mode = i;
  else if (i == 99)  cur_mode = (cur_mode + NUM_MODES - 1) % NUM_MODES;
  else if (i == 101) cur_mode = (cur_mode + 1) % NUM_MODES;

  load_mode(cur_mode);
  reset_mode();
}

inline void reset_state(PatternState *state) {
  state->tick = state->trip = state->cidx = state->cntr = state->segm = 0;
}

void reset_mode() {
  gui_set = gui_color = -1;
  variant = 0;
  reset_state(&pattern_state[0]);
  reset_state(&pattern_state[1]);
  if (mode.data[0] == 0) {
    calc_pattern_state();
  } else if (mode.data[0] == 1) {
    calc_primer_states();
  }
}


//********************************************************************************
// Pattern functions
//********************************************************************************
int8_t pattern_strobe(PatternState *state) {
  uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  uint8_t pick = constrain((state->args[0] == 0) ? numc : state->args[0], 1, numc);
  uint8_t skip = constrain((state->args[1] == 0) ? pick : state->args[1], 1, pick);
  uint8_t repeat = constrain(state->args[2], 1, MAX_REPEATS);

  uint8_t st = state->timings[0];
  uint8_t bt = state->timings[1];
  uint8_t tt = state->timings[2];

  if (st == 0 && bt == 0 && tt == 0) tt = 1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == 0) calc_pattern_state();
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

  int8_t color = -1;
  if (state->segm % 2 == 1) color = (state->segm / 2) + state->cidx;
  else                      color = -1;

  if (color >= numc) color = (pick == skip) ? -1 : color % numc;
  return color;
}

int8_t pattern_vexer(PatternState *state) {
  uint8_t numc = constrain(state->numc, 2, MAX_COLORS) - 1;

  uint8_t repeat_c = constrain(state->args[0], 1, MAX_REPEATS);
  uint8_t repeat_t = constrain(state->args[1], 1, MAX_REPEATS);

  uint8_t cst = state->timings[0];
  uint8_t cbt = state->timings[1];
  uint8_t tst = state->timings[2];
  uint8_t tbt = state->timings[3];
  uint8_t sbt = state->timings[4];

  if (cst == 0 && cbt == 0 && tst == 0 && tbt == 0 && sbt == 0) sbt = 1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == 0) calc_pattern_state();
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

  int8_t color = -1;
  if (state->segm == 0) {       color = -1;
  } else {
    if (state->cntr < repeat_c) color = state->cidx + 1;
    else                        color = 0;
  }
  return color;
}

int8_t pattern_edge(PatternState *state) {
  uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  uint8_t pick = constrain((state->args[0] == 0) ? numc : state->args[0], 1, numc);

  uint8_t cst = state->timings[0];
  uint8_t cbt = state->timings[1];
  uint8_t est = state->timings[2];
  uint8_t ebt = state->timings[3];

  if (cst == 0 && cbt == 0 && est == 0 && ebt == 0) ebt = 1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == 0) calc_pattern_state();
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

  int8_t color = -1;
  if (state->segm == 0) color = -1;
  else                  color = abs((int)(pick - 1) - state->cntr) + state->cidx;

  if (color >= numc) color = -1;
  return color;
}

int8_t pattern_triple(PatternState *state) {
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

  if (ast == 0 && abt == 0 && bst == 0 && bbt == 0 && cst == 0 && cbt == 0 && sbt == 0) sbt = 0;

  if (state->tick >= state->trip) {
    if (mode.data[0] == 0) calc_pattern_state();
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

int8_t pattern_runner(PatternState *state) {
  uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  uint8_t pick = constrain((state->args[0] == 0) ? numc - 1 : state->args[0], 1, numc);
  uint8_t skip = constrain((state->args[1] == 0) ? pick : state->args[1], 1, pick);
  uint8_t repeat = constrain((state->args[2] == 0) ? pick : state->args[2], 1, MAX_REPEATS);

  uint8_t cst = state->timings[0];
  uint8_t cbt = state->timings[1];
  uint8_t rst = state->timings[2];
  uint8_t rbt = state->timings[3];
  uint8_t sbt = state->timings[4];

  if (cst == 0 && cbt == 0 && rst == 0 && rbt == 0 && sbt == 0) sbt = 1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == 0) calc_pattern_state();
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

int8_t pattern_stepper(PatternState *state) {
  uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  uint8_t steps = constrain(state->args[0], 1, 7);
  uint8_t random_step = state->args[1];
  uint8_t random_color = state->args[2];

  uint8_t bt = state->timings[0];
  uint8_t ct[7] = {state->timings[1], state->timings[2], state->timings[3],
    state->timings[4], state->timings[5], state->timings[6], state->timings[7]};

  if (bt == 0 && ct[0] == 0 && ct[1] == 0 && ct[2] == 0 && ct[3] == 0 && ct[4] == 0 && ct[5] == 0 && ct[6] == 0) bt = 1;

  if (state->tick >= state->trip) {
    if (mode.data[0] == 0) calc_pattern_state();
    state->tick = state->trip = 0;
    while (state->trip == 0) {
      state->segm++;
      if (state->segm >= 2) {
        state->segm = 0;
        state->cidx = (random_color) ? rand(0, numc) : (state->cidx + 1) % numc;
        state->cntr = (random_step) ? rand(0, steps) : (state->cntr + 1) % steps;
      }

      if (state->segm == 0) state->trip = bt;
      else                  state->trip = ct[state->cntr];
    }
  }

  return (state->segm == 0) ? -1 : state->cidx;
}

int8_t pattern_random(PatternState *state) {
  uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  uint8_t random_color = state->args[0];
  uint8_t multiplier = constrain(state->args[1], 1, 10);

  uint8_t ctl = min(state->timings[0], state->timings[1]);
  uint8_t cth = max(state->timings[0], state->timings[1]);
  uint8_t btl = min(state->timings[2], state->timings[3]);
  uint8_t bth = max(state->timings[2], state->timings[3]);

  if (state->tick >= state->trip) {
    if (mode.data[0] == 0) calc_pattern_state();
    state->tick = state->trip = 0;
    while (state->trip == 0) {
      state->segm++;
      if (state->segm >= 2) {
        state->segm = 0;
        state->cidx = (random_color) ? rand(0, numc) : (state->cidx + 1) % numc;
      }

      if (state->segm == 0) state->trip = rand(ctl, cth + 1) * multiplier;
      else                  state->trip = rand(btl, bth + 1) * multiplier;
    }
  }

  return (state->segm == 0) ? state->cidx : -1;
}

int8_t pattern_flux(PatternState *state) {
  uint8_t numc = constrain(state->numc, 1, MAX_COLORS);

  uint8_t steps = constrain(state->args[0], 1, 100);
  uint8_t blend_dir = state->args[1] % 3;
  bool target_color = state->args[2];

  uint8_t st = state->timings[0];
  uint8_t at = state->timings[1];

  if (st == 0 && at == 0) st = 1;

  if (state->tick >= state->trip) {
  }
}


//********************************************************************************
// handle_accel
//********************************************************************************
void inline _request_axis(uint8_t axis) {
  uint8_t a = (axis == AXIS_X) ? 3 : (axis == AXIS_Y) ? 1 : 5;
  Wire.beginTransmission(V2_ACCEL_ADDR);
  Wire.write(a);
  Wire.endTransmission(false);
  Wire.requestFrom(V2_ACCEL_ADDR, (uint8_t)2);
}

void inline _read_axis(uint8_t axis) {
  if (Wire.available()) adata.gs[axis] = Wire.read() << 4;
  if (Wire.available()) adata.gs[axis] |= Wire.read() >> 4;
  adata.gs[axis] = (adata.gs[axis] < 2048) ? adata.gs[axis] : -4096 + adata.gs[axis];
  adata.gs2[axis] = adata.gs[axis] * adata.gs[axis];
}

void _update_bins() {
  uint8_t i = 0;
  uint16_t bin_thresh = ACCEL_ONEG;
  adata.velocity = 0;

  while (i < ACCEL_BINS) {
    bin_thresh += ACCEL_BIN_SIZE;
    if (adata.mag > bin_thresh) {
      adata.velocity_last[i] = 0;
      adata.velocity_cntr[i] = min(adata.velocity_cntr[i] + 1, 128);
    }

    if (adata.velocity_last[i] >= ACCEL_FALLOFF) adata.velocity_cntr[i] = 0;
    else                                         adata.velocity_last[i]++;

    if (adata.velocity_cntr[i] > ACCEL_TARGET) {
      adata.velocity = i + 1;
    }
    i++;
  }
}

void _update_variant() {
  uint8_t v = 0;

  if (mode.pm.trigger_mode == 1)      v = adata.velocity;
  else if (mode.pm.trigger_mode == 2) v = adata.pitch;
  else if (mode.pm.trigger_mode == 3) v = adata.roll;
  else if (mode.pm.trigger_mode == 4) v = adata.flip;

  if ((variant == 0 && v >= mode.pm.trigger_thresh[0]) ||
      (variant == 1 && v <= mode.pm.trigger_thresh[1])) {
    adata.prime_last = 0;
    adata.prime_cntr = min(adata.prime_cntr + 1, 128);
  }

  if (adata.prime_last >= ACCEL_FALLOFF) adata.prime_cntr = 0;
  else                                   adata.prime_last++;

  if (adata.prime_cntr >= ACCEL_TARGET) {
    adata.prime_last = adata.prime_cntr = 0;
    variant = (variant + 1) % 2;
  }
}

void handle_accel() {
  switch (accel_tick % ACCEL_COUNTS) {
    case 0:
      _request_axis(AXIS_X);
      break;
    case 1:
      _read_axis(AXIS_X);
      break;
    case 2:
      _request_axis(AXIS_Y);
      break;
    case 3:
      _read_axis(AXIS_Y);
      break;
    case 4:
      _request_axis(AXIS_Z);
      break;
    case 5:
      _read_axis(AXIS_Z);
      break;
    case 7:
      adata.mag = sqrt(adata.gs2[0] + adata.gs2[1] + adata.gs2[2]);
      break;
    case 8:
      _update_bins();
      break;
    case 9:
      adata.roll = atan2(-adata.gs[1], adata.gs[2]);
      break;
    case 10:
      adata.pitch = sqrt(adata.gs2[1] + adata.gs2[2]);
      break;
    case 11:
      adata.pitch = atan2(adata.gs[0], adata.pitch);
      break;
    case 12:
      adata.flip = 32 - ((constrain(adata.gs[2], -496, 496) + 496) / 31);
      adata.pitch = constrain((adata.pitch + 1.396) * 11.817, 0, 32);
      break;
    case 13:
      if (adata.roll > M_PI_2) {
        adata.roll = M_PI - adata.roll;
      } else if (adata.roll < -M_PI_2) {
        adata.roll = -M_PI - adata.roll;
      }
      adata.roll = constrain((adata.roll + 1.396) * 11.817, 0, 32);
      break;
    case 14:
      if (mode.data[0] == 1) {
        _update_variant();
      }
      break;

    default:
      break;
  }

  accel_tick++;
  if (accel_tick >= ACCEL_COUNTS) accel_tick = 0;
}


//********************************************************************************
// handle_button
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
// handle_render
//********************************************************************************
void _render_mode() {
  int8_t color;
  if (mode.data[0] == 0) {
    // Run pattern func to get color index and increment tick
    color = pattern_funcs[pattern_state[0].pattern](&pattern_state[0]);
    pattern_state[0].tick++;

    // Write color to led buffer
    if (color < 0) led.r = led.g = led.b = 0;
    else           calc_color(color);

  } else if (mode.data[0] == 1) {
    color = pattern_funcs[pattern_state[variant].pattern](&pattern_state[variant]);
    pattern_funcs[pattern_state[!variant].pattern](&pattern_state[!variant]);
    pattern_state[0].tick++;
    pattern_state[1].tick++;
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
// handle_serial
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

  // Start up and initialize the acceleromater
  Wire.begin();
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
  pattern_funcs[P_STROBE] = &pattern_strobe;
  pattern_funcs[P_VEXER] = &pattern_vexer;
  pattern_funcs[P_EDGE] = &pattern_edge;
  pattern_funcs[P_TRIPLE] = &pattern_triple;
  pattern_funcs[P_RUNNER] = &pattern_runner;
  pattern_funcs[P_STEPPER] = &pattern_stepper;
  pattern_funcs[P_RANDOM] = &pattern_random;
}


bool version_match() {
  for (uint8_t i = 0; i < 4; i++) {
    if (EEPROM_VERSION[i] != ee_read(ADDR_VERSION[i])) {
      return false;
    }
  }
  return true;
}

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
// Main functions
//********************************************************************************
void setup() {
  setup_state();
  setup_pins();
  setup_timers();
  setup_funcs();

  if (!version_match()) reset_memory();
  brightness = ee_read(ADDR_BRIGHTNESS);

  Serial.begin(115200);
  Serial.write(SER_HANDSHAKE); Serial.write(SER_VERSION); Serial.write(SER_VERSION);

  change_mode(cur_mode);
  deadbeef_seed = analogRead(0);
  delay(6400);
}

void loop() {
  handle_serial();
  handle_accel();
  handle_button();
  handle_render();
}
