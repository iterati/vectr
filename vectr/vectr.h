#include <Arduino.h>

//********************************************************************************
// Constants
//********************************************************************************
const uint8_t EEPROM_VERSION[4] = {1, 1, 1, 1};
const uint16_t ADDR_VERSION[4] = {904, 936, 968, 1000};

const uint16_t ADDR_BRIGHTNESS   = 1019;
const uint16_t ADDR_CONJURE_MODE = 1020;
const uint16_t ADDR_LOCKED       = 1021;
const uint16_t ADDR_CONJURE      = 1022;
const uint16_t ADDR_SLEEPING     = 1023;

const uint16_t MODE_SIZE = 128;
const uint8_t NUM_MODES   = 7;
const uint8_t MAX_COLORS  = 9;
const uint8_t MAX_REPEATS = 100;

const uint8_t PIN_R = 9;
const uint8_t PIN_G = 6;
const uint8_t PIN_B = 5;
const uint8_t PIN_BUTTON = 2;
const uint8_t PIN_LDO = A3;
const uint8_t V2_ACCEL_ADDR = 0x1D;

const uint16_t PRESS_DELAY    = 100;
const uint16_t SHORT_HOLD     = 500;
const uint16_t LONG_HOLD      = 1000;
const uint16_t VERY_LONG_HOLD = 3000;

const uint8_t S_PLAY_OFF          = 0;
const uint8_t S_PLAY_PRESSED      = 1;
const uint8_t S_PLAY_SLEEP_WAIT   = 2;
const uint8_t S_PLAY_CONJURE_WAIT = 3;
const uint8_t S_PLAY_LOCK_WAIT    = 5;
const uint8_t S_CONJURE_OFF       = 10;
const uint8_t S_CONJURE_PRESS     = 11;
const uint8_t S_CONJURE_PLAY_WAIT = 12;
const uint8_t S_SLEEP_WAKE        = 20;
const uint8_t S_SLEEP_BRIGHT_WAIT = 21;
const uint8_t S_SLEEP_RESET_WAIT  = 22;
const uint8_t S_SLEEP_HELD        = 23;
const uint8_t S_SLEEP_LOCK        = 25;
const uint8_t S_RESET_OFF         = 30;
const uint8_t S_RESET_PRESSED     = 31;
const uint8_t S_BRIGHT_OFF        = 35;
const uint8_t S_BRIGHT_PRESSED    = 36;
const uint8_t S_VIEW_MODE         = 250;
const uint8_t S_VIEW_COLOR        = 251;
const uint8_t S_MODE_WRITE        = 252;

const uint8_t SER_VERSION        = 101;
const uint8_t SER_DUMP           = 10;
const uint8_t SER_DUMP_LIGHT     = 11;
const uint8_t SER_SAVE           = 20;
const uint8_t SER_READ           = 30;
const uint8_t SER_WRITE          = 40;
const uint8_t SER_WRITE_LIGHT    = 41;
const uint8_t SER_WRITE_MODE     = 42;
const uint8_t SER_WRITE_MODE_END = 43;
const uint8_t SER_CHANGE_MODE    = 50;
const uint8_t SER_RESET_MODE     = 51;
const uint8_t SER_VIEW_MODE      = 100;
const uint8_t SER_VIEW_COLOR     = 110;
const uint8_t SER_DUMP_START     = 200;
const uint8_t SER_DUMP_END       = 210;
const uint8_t SER_HANDSHAKE      = 250;
const uint8_t SER_HANDSHACK      = 251;
const uint8_t SER_DISCONNECT     = 254;

const uint16_t ACCEL_BINS     = 32;
const uint16_t ACCEL_BIN_SIZE = 56;
const uint16_t ACCEL_COUNTS   = 20;
const uint16_t ACCEL_WRAP     = 20;
const uint16_t ACCEL_ONEG     = 512;
const uint16_t ACCEL_FALLOFF  = 10;
const uint16_t ACCEL_TARGET   = 2;
const uint16_t AXIS_X         = 0;
const uint16_t AXIS_Y         = 1;
const uint16_t AXIS_Z         = 2;

const uint8_t M_VECTR  = 0;
const uint8_t M_PRIMER = 1;

const uint8_t P_STROBE  = 0;
const uint8_t P_VEXER   = 1;
const uint8_t P_EDGE    = 2;
const uint8_t P_TRIPLE  = 3;
const uint8_t P_RUNNER  = 4;
const uint8_t P_STEPPER = 5;
const uint8_t P_RANDOM  = 6;

const uint8_t T_OFF   = 0;
const uint8_t T_SPEED = 1;
const uint8_t T_PITCH = 2;
const uint8_t T_ROLL  = 3;
const uint8_t T_FLIP  = 4;

//********************************************************************************
// Structs
//********************************************************************************
typedef struct VectrMode {
  uint8_t _type;                    // 0
  uint8_t pattern;                  // 1
  uint8_t args[5];                  // 2 - 6
  uint8_t tr_meta[4];               // 7 - 10
  uint8_t timings[3][8];            // 11 - 34
  uint8_t tr_flux[4];               // 35 - 38
  uint8_t numc[3];                  // 39 - 41
  uint8_t colors[3][MAX_COLORS][3]; // 42 - 122
  uint8_t _pad[5];
} VectrMode;                        // 115 bytes per mode

typedef struct PrimerMode {
  uint8_t _type;                    // 0
  uint8_t trigger_mode;             // 1
  uint8_t trigger_thresh[2];        // 2 -3
  uint8_t pattern[2];               // 4 - 5
  uint8_t args[2][5];               // 6 - 15
  uint8_t timings[2][8];            // 16 - 31
  uint8_t numc[2];                  // 32 - 33
  uint8_t colors[2][MAX_COLORS][3]; // 34 - 87
  uint8_t _pad[40];
} PrimerMode;                       // 88 bytes per mode

typedef struct DoubleMode {
  uint8_t _type;                    // 0
  uint8_t repeats[2];               // 1 - 2
  uint8_t pattern[2];               // 3 - 4
  uint8_t args[2][5];               // 5 - 14
  uint8_t timings[2][8];            // 15 - 30
  uint8_t numc[2];                  // 31 - 32
  uint8_t colors[2][MAX_COLORS][3]; // 33 - 86
  uint8_t _pad[41];
} TripleMode;                       // 87 bytes per mode

typedef union PackedMode {
  uint8_t data[MODE_SIZE];
  VectrMode vm;
  PrimerMode pm;
} PackedMode;


typedef struct AccelData {
  uint8_t velocity_last[32];
  uint8_t velocity_cntr[32];
  uint8_t prime_last;
  uint8_t prime_cntr;
  int32_t gs[3];
  uint32_t gs2[3];
  uint32_t mag, velocity;
  int32_t flip;
  float pitch, roll;
} AccelData;


typedef struct Led {
  uint8_t pin_r, pin_g, pin_b;
  uint8_t r, g, b;
} Led;


typedef struct PatternState {
  // Track the arguments to the function
  uint8_t pattern;
  uint8_t numc;
  uint8_t args[5];
  uint8_t timings[8];

  // Track the state variables
  uint16_t tick;
  uint16_t trip;
  uint8_t cidx;
  uint8_t cntr;
  uint8_t segm;
} PatternState;


//********************************************************************************
// Presets
//********************************************************************************
PROGMEM const uint8_t factory[NUM_MODES][MODE_SIZE] = {
  // Darkside of the moon
  {M_VECTR, P_STROBE, 0, 0, 0, 0, 0,
    8, 32, 32, 32,
    3, 0, 150, 0, 0, 0, 0, 0,
    6, 44, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    0, 16, 16, 32,
    6, 6, 6,
    12, 0, 0,     6, 7, 0,      0, 14, 0,
    0, 7, 8,      0, 0, 16,     6, 0, 8,
    0, 0, 0,      0, 0, 0,      0, 0, 0,

    48, 0, 0,     24, 28, 0,    0, 56, 0,
    0, 28, 32,    0, 0, 64,     24, 0, 32,
    0, 0, 0,      0, 0, 0,      0, 0, 0,

    192, 0, 0,    96, 112, 0,   0, 224, 0,
    0, 112, 128,  0, 0, 255,    96, 0, 128,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0, 0, 0},

  // Sorcery
  {M_VECTR, P_VEXER, 1, 5, 0, 0, 0,
    0, 5, 12, 32,
    0, 0, 0, 20, 5, 0, 0, 0,
    5, 0, 0, 20, 5, 0, 0, 0,
    5, 0, 20, 0, 5, 0, 0, 0,

    0, 32, 32, 32,
    4, 4, 1,
    12, 0, 0,      18, 0, 104,   0, 21, 104,
    78, 0, 24,    0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,

    2, 0, 0,      36, 0, 208,   0, 42, 208,
    156, 0, 48,   0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,

    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0, 0, 0},

  // Crosshair
  {M_VECTR, P_EDGE, 0, 0, 0, 0, 0,
    0, 24, 24, 24,
    3, 5, 15, 100, 0, 0, 0, 0,
    7, 1, 1, 100, 0, 0, 0, 0,
    3, 20, 3, 100, 0, 0, 0, 0,

    0, 24, 24, 24,
    4, 4, 4,
    192, 0, 0,    0, 112, 0,    0, 28, 0,
    0, 4, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,

    192, 0, 0,    96, 28, 0,    24, 7, 0,
    3, 1, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,

    192, 0, 0,    0, 0, 0,    192, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0, 0, 0},

  // Quantum Core
  {M_VECTR, P_TRIPLE, 4, 2, 1, 1, 1,
    0, 32, 32, 32,
    1, 15, 13, 15, 25, 0, 50, 0,
    1, 3, 2, 5, 3, 0, 50, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    0, 4, 4, 16,
    1, 2, 3,
    0, 42, 144,   0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,

    0, 42, 144,   36, 0, 144,   0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,

    0, 42, 144,   36, 0, 144,   48, 70, 96,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0, 0, 0},

  // Dashdops
  {M_VECTR, P_RUNNER, 0, 0, 0, 0, 0,
    0, 20, 20, 32,
    5, 0, 3, 22, 25, 0, 0, 0,
    5, 0, 5, 0, 25, 0, 0, 0,
    3, 22, 5, 0, 25, 0, 0, 0,

    32, 32, 32, 32,
    6, 1, 1,
    12, 91, 88,   72, 112, 0,   132, 42, 0,
    144, 0, 32,   120, 0, 64,   0, 28, 186,
    0, 0, 0,      0, 0, 0,      0, 0, 0,

    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,

    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0, 0, 0},

  // Rainbow Walker
  {M_VECTR, P_STEPPER, 7, 0, 0, 0, 0, 
    0, 16, 16, 32,
    50, 1, 2, 3, 4, 5, 6, 7,
    50, 1, 2, 4, 8, 16, 32, 64,
    50, 1, 4, 16, 64, 32, 16, 4,

    32, 32, 32, 32,
    9, 1, 1,
    144, 0, 0,    96, 56, 0,    48, 112, 0,
    0, 168, 0,    0, 112, 64,   0, 56, 128,
    0, 0, 196,    48, 0, 128,   96, 0, 64,

    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,

    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0, 0, 0},

  // Random Rainbow
  {M_VECTR, P_RANDOM, 1, 1, 0, 0, 0,
    0, 16, 16, 32,
    15, 25, 25, 25, 0, 0, 0, 0,
    5, 10, 25, 25, 0, 0, 0, 0,
    1, 3, 25, 25, 0, 0, 0, 0,

    32, 32, 32, 32,
    9, 1, 1,
    144, 0, 0,    96, 56, 0,    48, 112, 0,
    0, 168, 0,    0, 112, 64,   0, 56, 128,
    0, 0, 196,    48, 0, 128,   96, 0, 64,

    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,

    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0,      0, 0, 0,      0, 0, 0,
    0, 0, 0, 0, 0},
};
