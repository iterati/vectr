#include <Arduino.h>
#include <EEPROM.h>
#include <avr/wdt.h>

const uint8_t EEPROM_VERSION[4] = {3, 3, 5, 8};
const uint16_t ADDR_VERSION[4]  = {904, 936, 968, 1000};

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define ADDR_BRIGHTNESS   1019
#define ADDR_CONJURE_MODE 1020
#define ADDR_LOCKED       1021
#define ADDR_CONJURE      1022
#define ADDR_SLEEPING     1023

#define MODE_SIZE         128   // Number of bytes in a mode
#define NUM_MODES         7     // Number of modes
#define MAX_REPEATS       100

#define PIN_R             9     // Red pin - timer 0
#define PIN_G             6     // Green pin - timer 1
#define PIN_B             5     // Blue pin - timer 1
#define PIN_BUTTON        2     // Pin for the button
#define PIN_LDO           A3    // Low voltage dropoff pin
#define ACCEL_ADDR        0x1D  // I2C address of accelerometer
#define SCL_PIN           A5    // Clock pin
#define SDA_PIN           A4    // Data pin
#define I2CADC_H          315   // Analog read high threshold
#define I2CADC_L          150   // Analog read low threshold

#define ACCEL_BINS        32    // 32 bins gives 33 velocity states
#define ACCEL_BIN_SIZE    56    // approx 0.1g
#define ACCEL_COUNTS      40    // 20 frames between accel reads (50hz)
#define ACCEL_ONEG        512   // +- 4g range
#define ACCEL_FALLOFF     10    // 10 cycle falloff / 200ms
#define ACCEL_TARGET      2     // 2 cycle target / 40ms
#define ACCEL_COEF        11.82 // For normalizing pitch and roll

#define SER_VERSION       200
#define SER_DUMP          10
#define SER_DUMP_LIGHT    11
#define SER_SAVE          20
#define SER_RESET         30
#define SER_WRITE         50
#define SER_WRITE_LIGHT   60
#define SER_CHANGE_MODE   90
#define SER_VIEW_MODE     100
#define SER_VIEW_COLOR    110
#define SER_DUMP_START    200
#define SER_DUMP_END      210
#define SER_HANDSHAKE     250
#define SER_HANDSHACK     251
#define SER_DISCONNECT    254

#define S_PLAY            0
#define S_WAKE            1
#define S_BRIGHT          2
#define S_RESET           3
#define S_GUI_MODE        4
#define S_GUI_COLOR       5

#define M_VECTR           0
#define M_PRIMER          1

#define P_STROBE          0
#define P_TRACER          1
#define P_MORPH           2
#define P_SCIMITAR        3
#define P_WAVE            4
#define P_DYNAMO          5
#define P_SHIFTER         6
#define P_TRIPLE          7
#define P_STEPPER         8
#define P_RANDOM          9

#define T_OFF             0
#define T_VELOCITY        1
#define T_PITCH           2
#define T_ROLL            3
#define T_FLIP            4

typedef struct PatternState {
  // Track the arguments to the function
  uint8_t numc;
  uint8_t args[5];
  uint8_t timings[8];
  uint8_t colors[9][3];

  // Track the state variables
  uint16_t trip;
  uint8_t cidx;
  uint8_t cntr;
  uint8_t segm;
} PatternState;

typedef struct VectrMode {
  uint8_t _type;             // 0
  uint8_t pattern;           // 1
  uint8_t args[5];           // 2 - 6
  uint8_t tr_meta[4];        // 7 - 10
  uint8_t timings[3][8];     // 11 - 34
  uint8_t tr_flux[4];        // 35 - 38
  uint8_t numc[3];           // 39 - 41
  uint8_t colors[3][9][3];   // 42 - 122
  uint8_t _pad[5];
} VectrMode;                 // 123 bytes per mode

typedef struct PrimerMode {
  uint8_t _type;             // 0
  uint8_t trigger_mode;      // 1
  uint8_t trigger_thresh[2]; // 2 -3
  uint8_t pattern[2];        // 4 - 5
  uint8_t args[2][5];        // 6 - 15
  uint8_t timings[2][8];     // 16 - 31
  uint8_t numc[2];           // 32 - 33
  uint8_t colors[2][9][3];   // 34 - 87
  uint8_t _pad[40];
} PrimerMode;                // 88 bytes per mode

typedef union PackedMode {
  uint8_t data[MODE_SIZE];
  VectrMode vm;
  PrimerMode pm;
} PackedMode;

typedef struct AccelData {
  uint8_t vectr_falloff[ACCEL_BINS];
  uint8_t vectr_trigger[ACCEL_BINS];
  uint8_t prime_falloff;
  uint8_t prime_trigger;
  uint8_t velocity, pitch, roll, flip;
  uint16_t magnitude;
  int16_t axis_x, axis_y, axis_z;
  uint32_t axis_x2, axis_y2, axis_z2;
  float fpitch, froll;
} AccelData;


void (*pattern_funcs[10]) (PatternState*, uint8_t*, uint8_t*, uint8_t*, bool);
PackedMode mode;
AccelData adata;
PatternState patterns[2];

uint32_t limiter_us = 500;  // Frame time in us
uint32_t last_write = 0;
uint8_t rgb_r = 0;
uint8_t rgb_g = 0;
uint8_t rgb_b = 0;
uint8_t accel_tick = 0;     // Tick to track what accel action to perform
uint8_t cur_mode = 0;       // Current mode index
uint8_t brightness = 0;     // Global brightness value
bool locked = false;        // Is the light locked?
bool conjure = false;       // Is conjure enabled?
uint8_t active_pattern = 0; // Which pattern state is the one that needs rendered?
uint8_t op_state = 0;       // Current op state
uint32_t since_press = 0;   // How many ticks since the button state changed?
bool was_pressed = false;   // Was the button pressed last frame?
bool comm_link = false;     // Are we connected to the GUI?
uint8_t gui_set = 0;        // Which color set to display for GUI
uint8_t gui_slot = 0;       // Which color slot to display for GUI

PROGMEM const uint8_t factory[NUM_MODES][MODE_SIZE] = {
  {
    M_VECTR, P_WAVE, 64, 2, 1, 0, 0,
    0, 32, 32, 32,
    10, 0, 1, 0, 0, 0, 0, 0,
    3, 0, 1, 0, 0, 0, 0, 0,
    3, 0, 1, 0, 0, 0, 0, 0,
    0, 32, 32, 32,
    3, 3, 3,

    0, 84, 150,     0, 28, 210,     26, 0, 210,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    182, 0, 30,     182, 28, 0,     130, 84, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0, 0, 0},
  {
    M_VECTR, P_DYNAMO, 16, 1, 1, 0, 0,
    0, 32, 32, 32,
    4, 4, 4, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 0,
    32, 32, 32, 32,
    6, 6, 6,

    0, 84, 150,     0, 28, 210,     26, 0, 210,
    182, 0, 30,     182, 28, 0,     130, 84, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0, 0, 0},
  {
    M_VECTR, P_SHIFTER, 4, 2, 0, 0, 0,
    0, 32, 32, 32,
    5, 10, 5, 50, 0, 0, 0, 0,
    5, 10, 5, 200, 0, 0, 0, 0,
    5, 10, 5, 200, 0, 0, 0, 0,
    32, 32, 32, 32,
    3, 3, 3,

    0, 84, 150,     26, 0, 210,     182, 28, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0, 0, 0},
  {
    M_VECTR, P_MORPH, 16, 0, 0, 0, 0,
    0, 32, 32, 32,
    50, 0, 0, 0, 0, 0, 0, 0,
    3, 47, 0, 0, 0, 0, 0, 0,
    3, 47, 0, 0, 0, 0, 0, 0,
    32, 32, 32, 32,
    6, 6, 6,

    208, 0, 0,      104, 112, 0,    0, 224, 0,
    0, 112, 120,    0, 0, 240,      104, 0, 120,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0, 0, 0},
  {
    M_VECTR, P_TRACER, 2, 1, 5, 0, 0,
    0, 32, 32, 32,
    6, 44, 0, 25, 5, 0, 0, 0,
    6, 44, 25, 0, 5, 0, 0, 0,
    6, 44, 25, 0, 5, 0, 0, 0,
    32, 32, 32, 32,
    7, 7, 7,

    4, 4, 4,        208, 0, 0,      104, 112, 0,
    0, 224, 0,      0, 112, 120,    0, 0, 240,
    104, 0, 120,    0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0, 0, 0},
  {
    M_VECTR, P_STROBE, 3, 1, 16, 0, 0,
    0, 32, 32, 32,
    6, 44, 0, 0, 0, 0, 0, 0,
    3, 22, 0, 0, 0, 0, 0, 0,
    3, 22, 0, 0, 0, 0, 0, 0,
    32, 32, 32, 32,
    6, 6, 6,

    208, 0, 0,      104, 112, 0,    0, 224, 0,
    0, 112, 120,    0, 0, 240,      104, 0, 120,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0, 0, 0},
  {
    M_VECTR, P_STROBE, 0, 0, 0, 0, 0,
    0, 32, 32, 32,
    2, 0, 200, 0, 0, 0, 0, 0,
    6, 44, 0, 0, 0, 0, 0, 0,
    6, 44, 0, 0, 0, 0, 0, 0,
    32, 32, 32, 32,
    6, 6, 6,

    208, 0, 0,      104, 112, 0,    0, 224, 0,
    0, 112, 120,    0, 0, 240,      104, 0, 120,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,
    0, 0, 0,        0, 0, 0,        0, 0, 0,

    0, 0, 0, 0, 0},
};

const uint16_t _reciprocals[] = {
  0x0000, 0xFFFF, 0x8000, 0x5555, 0x4000, 0x3333, 0x2AAA, 0x2492,
  0x2000, 0x1C71, 0x1999, 0x1745, 0x1555, 0x13B1, 0x1249, 0x1111,
  0x1000, 0x0F0F, 0x0E38, 0x0D79, 0x0CCC, 0x0C30, 0x0BA2, 0x0B21,
  0x0AAA, 0x0A3D, 0x09D8, 0x097B, 0x0924, 0x08D3, 0x0888, 0x0842,
  0x0800, 0x07C1, 0x0787, 0x0750, 0x071C, 0x06EB, 0x06BC, 0x0690,
  0x0666, 0x063E, 0x0618, 0x05F4, 0x05D1, 0x05B0, 0x0590, 0x0572,
  0x0555, 0x0539, 0x051E, 0x0505, 0x04EC, 0x04D4, 0x04BD, 0x04A7,
  0x0492, 0x047D, 0x0469, 0x0456, 0x0444, 0x0432, 0x0421, 0x0410,
  0x0400, 0x03F0, 0x03E0, 0x03D2, 0x03C3, 0x03B5, 0x03A8, 0x039B,
  0x038E, 0x0381, 0x0375, 0x0369, 0x035E, 0x0353, 0x0348, 0x033D,
  0x0333, 0x0329, 0x031F, 0x0315, 0x030C, 0x0303, 0x02FA, 0x02F1,
  0x02E8, 0x02E0, 0x02D8, 0x02D0, 0x02C8, 0x02C0, 0x02B9, 0x02B1,
  0x02AA, 0x02A3, 0x029C, 0x0295, 0x028F, 0x0288, 0x0282, 0x027C,
  0x0276, 0x0270, 0x026A, 0x0264, 0x025E, 0x0259, 0x0253, 0x024E,
  0x0249, 0x0243, 0x023E, 0x0239, 0x0234, 0x0230, 0x022B, 0x0226,
  0x0222, 0x021D, 0x0219, 0x0214, 0x0210, 0x020C, 0x0208, 0x0204,
  0x0200, 0x01FC, 0x01F8, 0x01F4, 0x01F0, 0x01EC, 0x01E9, 0x01E5,
  0x01E1, 0x01DE, 0x01DA, 0x01D7, 0x01D4, 0x01D0, 0x01CD, 0x01CA,
  0x01C7, 0x01C3, 0x01C0, 0x01BD, 0x01BA, 0x01B7, 0x01B4, 0x01B2,
  0x01AF, 0x01AC, 0x01A9, 0x01A6, 0x01A4, 0x01A1, 0x019E, 0x019C,
  0x0199, 0x0197, 0x0194, 0x0192, 0x018F, 0x018D, 0x018A, 0x0188,
  0x0186, 0x0183, 0x0181, 0x017F, 0x017D, 0x017A, 0x0178, 0x0176,
  0x0174, 0x0172, 0x0170, 0x016E, 0x016C, 0x016A, 0x0168, 0x0166,
  0x0164, 0x0162, 0x0160, 0x015E, 0x015C, 0x015A, 0x0158, 0x0157,
  0x0155, 0x0153, 0x0151, 0x0150, 0x014E, 0x014C, 0x014A, 0x0149,
  0x0147, 0x0146, 0x0144, 0x0142, 0x0141, 0x013F, 0x013E, 0x013C,
  0x013B, 0x0139, 0x0138, 0x0136, 0x0135, 0x0133, 0x0132, 0x0130,
  0x012F, 0x012E, 0x012C, 0x012B, 0x0129, 0x0128, 0x0127, 0x0125,
  0x0124, 0x0123, 0x0121, 0x0120, 0x011F, 0x011E, 0x011C, 0x011B,
  0x011A, 0x0119, 0x0118, 0x0116, 0x0115, 0x0114, 0x0113, 0x0112,
  0x0111, 0x010F, 0x010E, 0x010D, 0x010C, 0x010B, 0x010A, 0x0109,
  0x0108, 0x0107, 0x0106, 0x0105, 0x0104, 0x0103, 0x0102, 0x0101,
};

float fast_atan2(float y, float x) {
  float coeff_1 = PI * 0.25;
  float coeff_2 = 3 * coeff_1;
  float abs_y = fabs(y) + 1e-10;
  float angle, r;

  if (x >= 0) {
    r = (x - abs_y) / (x + abs_y);
    angle = coeff_1 - coeff_1 * r;
  } else {
    r = (x + abs_y) / (abs_y - x);
    angle = coeff_2 - coeff_1 * r;
  }
  return (y < 0) ? -angle : angle;
}

uint16_t fast_sqrt(uint32_t v) {
  union mylong {
    uint32_t v;
    struct {
      uint32_t b0: 30;
      uint8_t b1: 2;
    };
  } val;

  val.v = v;
  uint16_t rem = 0;
  uint16_t res = 0;

  uint8_t i = 16;
  while (i--) {
    res <<= 1;
    rem <<= 2;
    rem += val.b1;
    val.v <<= 2;
    res++;
    if (res <= rem) {
      rem -= res;
      res++;
    } else {
      res--;
    }
  }
  return (uint16_t)(res >> 1);
}

uint8_t fast_interp(uint8_t s, uint8_t e, uint8_t d, uint8_t D) {
  if (s == e || d == 0 || D == 0) return s;
  if (d >= D) return e;

  union mylong {
    uint32_t v;
    uint8_t b[4];
  } tmp;

  if (s < e) {
    tmp.v = e - s;
    tmp.v *= d;
    tmp.v *= _reciprocals[D];
    return s + tmp.b[2];
  } else {
    tmp.v = s - e;
    tmp.v *= d;
    tmp.v *= _reciprocals[D];
    return s - tmp.b[2];
  }
}


inline void ee_update(uint16_t addr, uint8_t val) {
  while (!eeprom_is_ready()) {}
  EEPROM.update(addr, val);
}

inline uint8_t ee_read(uint16_t addr) {
  while (!eeprom_is_ready()) {}
  return EEPROM.read(addr);
}


inline void I2CADC_SDA_H_OUTPUT() { DDRC &= ~(1 << 4); }
inline void I2CADC_SDA_L_INPUT()  { DDRC |=  (1 << 4); }
inline void I2CADC_SCL_H_OUTPUT() { DDRC &= ~(1 << 5); }
inline void I2CADC_SCL_L_INPUT()  { DDRC |=  (1 << 5); }

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
    I2CADC_SCL_L_INPUT();  __asm__("nop\n\t");
    I2CADC_SDA_L_INPUT();
    I2CADC_SCL_H_OUTPUT(); __asm__("nop\n\t");
    I2CADC_SCL_L_INPUT();  __asm__("nop\n\t");
  } else {
AckThis:
    I2CADC_SCL_L_INPUT();  __asm__("nop\n\t");
    I2CADC_SCL_H_OUTPUT(); __asm__("nop\n\t");
    int result = analogRead(SCL_PIN);
    if (result < I2CADC_L) {
      goto AckThis;
    }
    I2CADC_SCL_L_INPUT();  __asm__("nop\n\t");

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

void TWADC_write_r(uint8_t data) {
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

void TWADC_send(uint8_t dev_addr, uint8_t addr, uint8_t data) {
  TWADC_beginTransmission(ACCEL_ADDR);
  TWADC_write(addr);
  TWADC_write(data);
  TWADC_endTransmission();
  delay(1);
}


void write_frame() {
  uint32_t cus = micros();
  while (cus - last_write < limiter_us) cus = micros();
  last_write = cus;

  analogWrite(PIN_R, rgb_r >> brightness);
  analogWrite(PIN_G, rgb_g >> brightness);
  analogWrite(PIN_B, rgb_b >> brightness);
}

void write_frame(uint8_t r, uint8_t g, uint8_t b) {
  rgb_r = r;
  rgb_g = g;
  rgb_b = b;
  write_frame();
}

void flash(uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t i = 0; i < 5; i++) {
    for (uint8_t j = 0; j < 200; j++) {
      if (j < 100) write_frame(0, 0, 0);
      else         write_frame(r, g, b);
    }
  }
}


void pattern_strobe(PatternState *state, uint8_t *r, uint8_t *g, uint8_t *b, bool rend) {
  uint8_t numc = constrain(state->numc, 1, 9);

  uint8_t pick = constrain((state->args[0] == 0) ? numc : state->args[0], 1, numc);
  uint8_t skip = constrain((state->args[1] == 0) ? pick : state->args[1], 1, pick);
  uint8_t repeat = constrain(state->args[2], 1, MAX_REPEATS);

  uint8_t st = state->timings[0];
  uint8_t bt = state->timings[1];
  uint8_t tt = state->timings[2];

  if (st == 0 && bt == 0 && tt == 0) tt = 1;

  while (state->trip == 0) {
    state->segm++;
    if (state->segm >= (2 * pick) + 1) {
      state->segm = 0;
      state->cntr++;
      if (state->cntr >= repeat) {
        state->cntr = 0;
        state->cidx += skip;
        if (state->cidx >= numc) {
          if (pick == skip) {
            state->cidx = 0;
          } else {
            state->cidx -= numc;
          }
        }
      }
    }

    if (state->segm == 0) {
      state->trip = tt;
    } else if (state->segm & 1 == 1) {
      state->trip = st;
    } else {
      state->trip = bt;
    }
  }

  bool show_blank = !(state->segm & 1 == 1);
  uint8_t color = state->cidx + (state->segm / 2);

  if (rend) {
    if (show_blank) {
      *r = 0;
      *g = 0;
      *b = 0;
    } else {
      *r = state->colors[color][0];
      *g = state->colors[color][1];
      *b = state->colors[color][2];
    }
  }

  state->trip--;
}

void pattern_tracer(PatternState *state, uint8_t *r, uint8_t *g, uint8_t *b, bool rend) {
  uint8_t numc = constrain(state->numc, 2, 9) - 1;

  uint8_t pick = constrain((state->args[0] == 0) ? numc : state->args[0], 1, numc);
  uint8_t skip = constrain((state->args[1] == 0) ? pick : state->args[1], 1, pick);
  uint8_t repeat = constrain(state->args[2], 1, MAX_REPEATS);

  uint8_t cst = state->timings[0];
  uint8_t cbt = state->timings[1];
  uint8_t tst = state->timings[2];
  uint8_t tbt = state->timings[3];
  uint8_t gt  = state->timings[4];

  if (cst == 0 && cbt == 0 && tst == 0 && tbt == 0 && gt == 0) gt = 1;

  while (state->trip == 0) {
    state->segm++;
    if (state->segm >= 2) {
      state->segm = 0;
      state->cntr++;
      if (state->cntr >= pick + repeat) {
        state->cntr = 0;
        state->cidx += skip;
        if (state->cidx >= numc) {
          state->cidx -= numc;
        }
      }
    }

    if (state->segm == 0) {
      if (state->cntr == 0) {
        state->trip = gt;
      } else if (state->cntr < pick) {
        state->trip = cbt;
      } else if (state->cntr == pick) {
        state->trip = gt;
      } else {
        state->trip = tbt;
      }
    } else {
      if (state->cntr < pick) {
        state->trip = cst;
      } else {
        state->trip = tst;
      }
    }
  }

  if (rend) {
    bool show_blank = state->segm == 0;
    uint8_t color = 0;
    if (state->cntr < pick) {
      color = state->cidx + state->cntr;
      if (color >= numc) show_blank = true;
      color++;
    }

    if (show_blank) {
      *r = 0;
      *g = 0;
      *b = 0;
    } else {
      *r = state->colors[color][0];
      *g = state->colors[color][1];
      *b = state->colors[color][2];
    }
  }

  state->trip--;
}

void pattern_morph(PatternState *state, uint8_t *r, uint8_t *g, uint8_t *b, bool rend) {
  uint8_t numc = constrain(state->numc, 1, 9);

  uint8_t steps = constrain(state->args[0], 1, MAX_REPEATS);
  uint8_t direc = constrain(state->args[1], 0, 1);

  uint8_t st = state->timings[0];
  uint8_t bt = state->timings[1];
  uint8_t ct = state->timings[2];
  uint8_t gt = state->timings[3];

  if (st == 0 && bt == 0 && ct == 0 && gt == 0) gt = 1;

  while (state->trip == 0) {
    state->segm++;
    if (state->segm >= 2) {
      state->segm = 0;
      state->cntr++;
      if (state->cntr >= steps + 1) {
        state->cntr = 0;
        state->cidx++;
        if (state->cidx == numc) {
          state->cidx = 0;
        }
      }
    }

    if (state->segm == 0) {
      if (state->cntr == 0) {
        state->trip = gt;
      } else {
        state->trip = bt;
      }
    } else {
      if (state->cntr == 0) {
        state->trip = ct;
      } else {
        state->trip = st;
      }
    }
  }

  if (rend) {
    uint8_t c1 = state->cidx;
    uint8_t c2 = state->cidx;
    if (direc == 0) {
      c2++;
      if (c2 == state->numc) c2 = 0;
    } else {
      c1++;
      if (c1 == state->numc) c1 = 0;
    }

    if (state->segm == 0) {
      *r = 0;
      *g = 0;
      *b = 0;
    } else {
      if (state->cntr == 0) {
        *r = state->colors[c1][0];
        *g = state->colors[c1][1];
        *b = state->colors[c1][2];
      } else {
        uint16_t D = steps * (st + bt);
        uint16_t d = (state->cntr * (st + bt)) - state->trip;

        while (D > 256) {
          D >>= 1;
          d >>= 1;
        }

        *r = fast_interp(state->colors[c1][0], state->colors[c2][0], (uint8_t)d, (uint8_t)D);
        *g = fast_interp(state->colors[c1][1], state->colors[c2][1], (uint8_t)d, (uint8_t)D);
        *b = fast_interp(state->colors[c1][2], state->colors[c2][2], (uint8_t)d, (uint8_t)D);
      }
    }
  }

  state->trip--;
}

void pattern_scimitar(PatternState *state, uint8_t *r, uint8_t *g, uint8_t *b, bool rend) {
  uint8_t numc = constrain(state->numc, 1, 9);

  uint8_t pick = constrain((state->args[0] == 0) ? numc : state->args[0], 1, numc);

  uint8_t st = state->timings[0];
  uint8_t bt = state->timings[1];
  uint8_t ct = state->timings[2];
  uint8_t gt = state->timings[3];

  if (st == 0 && bt == 0 && ct == 0 && gt == 0) gt = 1;

  while (state->trip == 0) {
    state->segm++;
    if (state->segm >= 2) {
      state->segm = 0;
      state->cntr++;
      if (state->cntr >= (pick - 1) * 2) {
        state->cntr = 0;
        state->cidx += pick;
        if (state->cidx >= numc) {
          state->cidx -= numc;
        }
      }
    }

    if (state->segm == 0) {
      if (state->cntr == 0) {
        state->trip = gt;
      } else {
        state->trip = bt;
      }
    } else {
      if (state->cntr == pick - 1) {
        state->trip = ct;
      } else {
        state->trip = st;
      }
    }
  }

  if (rend) {
    bool show_blank = state->segm == 0;
    uint8_t color = state->cidx;
    if (state->cntr < pick) {
      color += numc - state->cntr - 1;
    } else {
      color += state->cntr - numc + 1;
    }
    if (color >= numc) show_blank = true;

    if (show_blank) {
      *r = 0;
      *g = 0;
      *b = 0;
    } else {
      *r = state->colors[color][0];
      *g = state->colors[color][1];
      *b = state->colors[color][2];
    }
  }

  state->trip--;
}

void pattern_wave(PatternState *state, uint8_t *r, uint8_t *g, uint8_t *b, bool rend) {
  uint8_t numc = constrain(state->numc, 1, 9);

  uint8_t steps = constrain(state->args[0], 1, MAX_REPEATS);
  uint8_t direc = constrain(state->args[1], 0, 2);
  uint8_t alter = constrain(state->args[2], 0, 1);
  uint8_t every = constrain(state->args[3], 0, 1);

  uint8_t st = state->timings[0];
  uint8_t bt = state->timings[1];
  uint8_t ct = state->timings[2];

  if (st == 0 && bt == 0 && ct == 0) bt = 1;
  uint8_t tsteps = (direc == 2) ? steps * 2 : steps;

  while (state->trip == 0) {
    state->segm++;
    if (state->segm >= 2) {
      state->segm = 0;
      if (every == 0) {
        state->cntr++;
        state->cidx++;
        if (state->cntr >= tsteps) state->cntr = 0;
        if (state->cidx >= numc) state->cidx = 0;
      } else {
        state->cntr++;
        if (state->cntr >= tsteps) {
          state->cntr = 0;
          state->cidx++;
          if (state->cidx >= numc) state->cidx = 0;
        }
      }
    }

    uint8_t len = 0;
    if (direc == 0) {
      len = state->cntr;
    } else if (direc == 1) {
      len = steps - state->cntr - 1;
    } else {
      if (state->cntr < steps) {
        len = state->cntr;
      } else {
        len = tsteps - state->cntr - 1;
      }
    }

    if (state->segm == 0) {
      if (alter == 0) {
        state->trip = st + (len * ct);
      } else {
        state->trip = st;
      }
    } else {
      if (alter == 0) {
        state->trip = bt;
      } else {
        state->trip = bt + (len * ct);
      }
    }
  }

  if (rend) {
    if (state->segm == 0) {
      *r = state->colors[state->cidx][0];
      *g = state->colors[state->cidx][1];
      *b = state->colors[state->cidx][2];
    } else {
      *r = 0;
      *g = 0;
      *b = 0;
    }
  }

  state->trip--;
}

void pattern_dynamo(PatternState *state, uint8_t *r, uint8_t *g, uint8_t *b, bool rend) {
  uint8_t numc = constrain(state->numc, 1, 9);

  uint8_t steps = constrain(state->args[0], 1, MAX_REPEATS);
  uint8_t direc = constrain(state->args[1], 0, 2);
  uint8_t every = constrain(state->args[2], 0, 1);

  uint8_t st = state->timings[0];
  uint8_t bt = state->timings[1];
  uint8_t ct = state->timings[2];

  if (st == 0 && bt == 0 && ct == 0) bt = 1;
  uint8_t tsteps = steps;
  if (direc == 2) tsteps <<= 1;

  while (state->trip == 0) {
    state->segm++;
    if (state->segm >= 2) {
      state->segm = 0;
      if (every == 0) {
        state->cntr++;
        state->cidx++;
        if (state->cntr >= tsteps) state->cntr = 0;
        if (state->cidx >= numc) state->cidx = 0;
      } else {
        state->cntr++;
        if (state->cntr >= tsteps) {
          state->cntr = 0;
          state->cidx++;
          if (state->cidx >= numc) state->cidx = 0;
        }
      }
    }

    uint8_t len_s;
    if (direc == 0) {
      len_s = state->cntr;
    } else if (direc == 1) {
      len_s = steps - state->cntr - 1;
    } else {
      if (state->cntr < steps) {
        len_s = state->cntr;
      } else {
        len_s = tsteps - state->cntr - 1;
      }
    }
    uint8_t len_b = steps - len_s - 1;

    if (state->segm == 0) {
      state->trip = st + (len_s * ct);
    } else {
      state->trip = bt + (len_b * ct);
    }
  }

  if (rend) {
    if (state->segm == 0) {
      *r = state->colors[state->cidx][0];
      *g = state->colors[state->cidx][1];
      *b = state->colors[state->cidx][2];
    } else {
      *r = 0;
      *g = 0;
      *b = 0;
    }
  }

  state->trip--;
}

void pattern_shifter(PatternState *state, uint8_t *r, uint8_t *g, uint8_t *b, bool rend) {
  uint8_t numc = constrain(state->numc, 1, 9);

  uint8_t steps = constrain(state->args[0], 1, MAX_REPEATS);
  uint8_t direc = constrain(state->args[1], 0, 2);

  uint8_t st = state->timings[0];
  uint8_t bt = state->timings[1];
  uint8_t ct = state->timings[2];
  uint8_t gt = state->timings[3];

  if (st == 0 && bt == 0 && ct == 0 && gt == 0) gt = 1;
  uint8_t tsteps = (direc == 2) ? steps * 2 : steps;

  while (state->trip == 0) {
    state->segm++;
    if (state->segm >= (2 * numc) + 1) {
      state->segm = 0;
      state->cntr++;
      if (state->cntr >= tsteps) state->cntr = 0;
    }

    uint8_t len;
    if (direc == 0) {
      len = state->cntr;
    } else if (direc == 1) {
      len = steps - state->cntr - 1;
    } else {
      if (state->cntr < steps) {
        len = state->cntr;
      } else {
        len = tsteps - state->cntr - 1;
      }
    }

    if (state->segm & 1) {
      state->trip = st + (len * ct);
    } else {
      if (state->segm == 0) {
        state->trip = gt;
      } else {
        state->trip = bt;
      }
    }
  }

  if (rend) {
    if (state->segm & 1) {
      *r = state->colors[state->segm / 2][0];
      *g = state->colors[state->segm / 2][1];
      *b = state->colors[state->segm / 2][2];
    } else {
      *r = 0;
      *g = 0;
      *b = 0;
    }
  }

  state->trip--;
}

void pattern_triple(PatternState *state, uint8_t *r, uint8_t *g, uint8_t *b, bool rend) {
  uint8_t numc = constrain(state->numc, 1, 9);

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

  if (ast == 0 && abt == 0 && bst == 0 && bbt == 0 && cst == 0 && cbt == 0 && sbt == 0) sbt = 1;

  while (state->trip == 0) {
    state->segm++;
    if (state->segm >= 2) {
      state->segm = 0;
      state->cntr++;
      if (state->cntr >= repeats) {
        state->cntr = 0;
        state->cidx++;
        if (state->cidx >= numc) {
          state->cidx = 0;
        }
      }
    }

    if (state->segm == 0) {
      if (state->cntr == 0)                         state->trip = sbt;
      else if (state->cntr < repeat_a)              state->trip = abt;
      else if (state->cntr == repeat_a)             state->trip = sbt;
      else if (state->cntr < repeat_a + repeat_b)   state->trip = bbt;
      else if (state->cntr == repeat_a + repeat_b)  state->trip = sbt;
      else                                          state->trip = cbt;
    } else {
      if (state->cntr < repeat_a)                   state->trip = ast;
      else if (state->cntr < repeat_b)              state->trip = bst;
      else                                          state->trip = cst;
    }
  }

  if (rend) {
    if (state->segm == 0) {
      *r = 0;
      *g = 0;
      *b = 0;
    } else {
      if (state->cntr < repeat_a) {
        *r = state->colors[state->cidx][0];
        *g = state->colors[state->cidx][1];
        *b = state->colors[state->cidx][2];
      } else if (state->cntr < repeat_a + repeat_b) {
        *r = state->colors[(state->cidx + skip) % numc][0];
        *g = state->colors[(state->cidx + skip) % numc][1];
        *b = state->colors[(state->cidx + skip) % numc][2];
      } else {
        *r = state->colors[(state->cidx + skip + skip) % numc][0];
        *g = state->colors[(state->cidx + skip + skip) % numc][1];
        *b = state->colors[(state->cidx + skip + skip) % numc][2];
      }
    }
  }

  state->trip--;
}

void pattern_stepper(PatternState *state, uint8_t *r, uint8_t *g, uint8_t *b, bool rend) {
  uint8_t numc = constrain(state->numc, 1, 9);

  uint8_t steps = constrain(state->args[0], 1, 7);
  uint8_t random_step = state->args[1];
  uint8_t random_color = state->args[2];

  uint8_t bt = state->timings[0];
  uint8_t ct[7] = {
    state->timings[1],
    state->timings[2],
    state->timings[3],
    state->timings[4],
    state->timings[5],
    state->timings[6],
    state->timings[7]};

  if (bt == 0 && ct[0] == 0 && ct[1] == 0 && ct[2] == 0 && ct[3] == 0 && ct[4] == 0 && ct[5] == 0 && ct[6] == 0) bt = 1;

  while (state->trip == 0) {
    state->segm++;
    if (state->segm >= 2) {
      state->segm = 0;
      state->cidx = (rend && random_color) ? random(0, numc ) : (state->cidx + 1) % numc;
      state->cntr = (rend && random_step)  ? random(0, steps) : (state->cntr + 1) % steps;
    }

    if (state->segm == 0) state->trip = bt;
    else                  state->trip = ct[state->cntr];
  }

  if (rend) {
    if (state->segm == 0) {
      *r = 0;
      *g = 0;
      *b = 0;
    } else {
      *r = state->colors[state->cidx][0];
      *g = state->colors[state->cidx][1];
      *b = state->colors[state->cidx][2];
    }
  }

  state->trip--;
}

void pattern_random(PatternState *state, uint8_t *r, uint8_t *g, uint8_t *b, bool rend) {
  uint8_t numc = constrain(state->numc, 1, 9);

  uint8_t random_color = state->args[0];
  uint8_t multiplier = constrain(state->args[1], 1, 10);

  uint8_t ctl = min(state->timings[0], state->timings[1]);
  uint8_t cth = max(state->timings[0], state->timings[1]);
  uint8_t btl = min(state->timings[2], state->timings[3]);
  uint8_t bth = max(state->timings[2], state->timings[3]);

  if (ctl == 0 && cth == 0 && btl == 0 && bth == 0) btl = bth = 1;
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

  if (rend) {
    if (state->segm == 0) {
      *r = state->colors[state->cidx][0];
      *g = state->colors[state->cidx][1];
      *b = state->colors[state->cidx][2];
    } else {
      *r = 0;
      *g = 0;
      *b = 0;
    }
  }

  state->trip--;
}


bool version_match() {
  if (ee_read(ADDR_VERSION[0]) != EEPROM_VERSION[0]) return false;
  if (ee_read(ADDR_VERSION[1]) != EEPROM_VERSION[1]) return false;
  if (ee_read(ADDR_VERSION[2]) != EEPROM_VERSION[2]) return false;
  if (ee_read(ADDR_VERSION[3]) != EEPROM_VERSION[3]) return false;
  return true;
}

void reset_memory() {
  for (int i = 0; i < 1024; i++) ee_update(i, 0);

  for (uint8_t m = 0; m < NUM_MODES; m++) {
    for (uint8_t b = 0; b < MODE_SIZE; b++) {
      ee_update((m * MODE_SIZE) + b, pgm_read_byte(&factory[m][b]));
    }
  }

  ee_update(ADDR_VERSION[0], EEPROM_VERSION[0]);
  ee_update(ADDR_VERSION[1], EEPROM_VERSION[1]);
  ee_update(ADDR_VERSION[2], EEPROM_VERSION[2]);
  ee_update(ADDR_VERSION[3], EEPROM_VERSION[3]);
}


void init_mode() {
  gui_set = gui_slot = 0;
  active_pattern = 0;

  if (mode.data[0] == M_VECTR) {
    patterns[0].numc = mode.vm.numc[0];
    patterns[1].numc = mode.vm.numc[0];
    for (uint8_t i = 0; i < 9; i++) {
      if (i < 5) patterns[0].args[i] = mode.vm.args[i];
      if (i < 5) patterns[1].args[i] = mode.vm.args[i];
      if (i < 8) patterns[0].timings[i] = mode.vm.timings[0][i];
      if (i < 8) patterns[1].timings[i] = mode.vm.timings[0][i];
      patterns[0].colors[i][0] = mode.vm.colors[0][i][0];
      patterns[0].colors[i][1] = mode.vm.colors[0][i][1];
      patterns[0].colors[i][2] = mode.vm.colors[0][i][2];
      patterns[1].colors[i][0] = mode.vm.colors[0][i][0];
      patterns[1].colors[i][1] = mode.vm.colors[0][i][1];
      patterns[1].colors[i][2] = mode.vm.colors[0][i][2];
    }
  } else {
    patterns[0].numc = mode.pm.numc[0];
    patterns[1].numc = mode.pm.numc[1];
    for (uint8_t i = 0; i < 9; i++) {
      if (i < 5) patterns[0].args[i] = mode.pm.args[0][i];
      if (i < 5) patterns[1].args[i] = mode.pm.args[1][i];
      if (i < 8) patterns[0].timings[i] = mode.pm.timings[0][i];
      if (i < 8) patterns[1].timings[i] = mode.pm.timings[1][i];
      patterns[0].colors[i][0] = mode.pm.colors[0][i][0];
      patterns[0].colors[i][1] = mode.pm.colors[0][i][1];
      patterns[0].colors[i][2] = mode.pm.colors[0][i][2];
      patterns[1].colors[i][0] = mode.pm.colors[1][i][0];
      patterns[1].colors[i][1] = mode.pm.colors[1][i][1];
      patterns[1].colors[i][2] = mode.pm.colors[1][i][2];
    }
  }

  patterns[0].trip = patterns[0].cidx = patterns[0].cntr = patterns[0].segm = 0;
  patterns[1].trip = patterns[1].cidx = patterns[1].cntr = patterns[1].segm = 0;
}

void change_mode(uint8_t i) {
  if (i < NUM_MODES) cur_mode = i;
  else if (i == 99)  cur_mode = (cur_mode + NUM_MODES - 1) % NUM_MODES;
  else if (i == 101) cur_mode = (cur_mode + 1) % NUM_MODES;

  for (uint8_t b = 0; b < MODE_SIZE; b++) {
    mode.data[b] = ee_read((cur_mode * MODE_SIZE) + b);
  }

  init_mode();
}


void accel_init() {
  TWADC_begin();
  delay(1);
  TWADC_send(ACCEL_ADDR, 0x2A, B00000000); // Standby to accept new settings
  TWADC_send(ACCEL_ADDR, 0x0E, B00000001); // Set +-4g range
  TWADC_send(ACCEL_ADDR, 0x2B, B00011011); // Low Power SLEEP
  TWADC_send(ACCEL_ADDR, 0x2C, B00111000);
  TWADC_send(ACCEL_ADDR, 0x2D, B00000000);
  TWADC_send(ACCEL_ADDR, 0x2A, B00100001); // Set 50 samples/sec (every 40 frames) and active
}

void accel_standby() {
  TWADC_send(ACCEL_ADDR, 0x2A, 0x00);
}


void _push_interrupt() {}

void enter_sleep() {
  wdt_enable(WDTO_15MS);        // Enable the watchdog
  write_frame(0, 0, 0);         // Blank the LED
  ee_update(ADDR_SLEEPING, 1);  // Set the sleeping bit
  accel_standby();              // Standby the acceleromater
  digitalWrite(PIN_LDO, LOW);   // Deactivate the LDO
  while (true) {}               // Loop until watchdog bites
}


void dump_mode() {
  Serial.write(SER_DUMP_START); Serial.write(cur_mode); Serial.write(SER_DUMP);
  for (uint8_t b = 0; b < MODE_SIZE; b++) {
    Serial.write(cur_mode); Serial.write(b); Serial.write(mode.data[b]);
  }
  Serial.write(SER_DUMP_END); Serial.write(cur_mode); Serial.write(SER_DUMP);
}

void handle_serial() {
  uint8_t cmd, in0, in1, in2;
  while (Serial.available() >= 4) {
    cmd = Serial.read();
    in0 = Serial.read();
    in1 = Serial.read();
    in2 = Serial.read();

    if (cmd == SER_HANDSHAKE) {
      if (in0 == SER_VERSION && in1 == SER_VERSION && in2 == SER_VERSION) {
        op_state = S_GUI_MODE;
        comm_link = true;
        Serial.write(SER_HANDSHACK); Serial.write(cur_mode); Serial.write(SER_VERSION);
      }
    } else if (comm_link) {
      if (cmd == SER_DUMP) {
        dump_mode();
      } else if (cmd == SER_DUMP_LIGHT) {
        Serial.write(SER_DUMP_START); Serial.write(cur_mode); Serial.write(SER_DUMP_LIGHT);
        for (uint8_t m = 0; m < NUM_MODES; m++) {
          for (uint8_t b = 0; b < MODE_SIZE; b++) {
            Serial.write(m); Serial.write(b); Serial.write(ee_read((m * MODE_SIZE) + b));
          }
        }
        Serial.write(SER_DUMP_END); Serial.write(cur_mode); Serial.write(SER_DUMP_LIGHT);
      } else if (cmd == SER_SAVE) {
        for (uint8_t b = 0; b < MODE_SIZE; b++) {
          ee_update((cur_mode * MODE_SIZE) + b, mode.data[b]);
        }
        flash(255, 255, 255);
      } else if (cmd == SER_RESET) {
        change_mode(cur_mode);
      } else if (cmd == SER_WRITE) {
        if (in0 < MODE_SIZE) {
          mode.data[in0] = in1;
        }
        init_mode();
      } else if (cmd == SER_WRITE_LIGHT) {
        if (in0 < NUM_MODES && in1 < MODE_SIZE) {
          ee_update((in0 * MODE_SIZE) + in1, in2);
        }
      } else if (cmd == SER_CHANGE_MODE) {
        change_mode(in0);
        dump_mode();
      } else if (cmd == SER_VIEW_MODE) {
        op_state = S_GUI_MODE;
      } else if (cmd == SER_VIEW_COLOR) {
        gui_set = in0;
        gui_slot = in1;
        op_state = S_GUI_COLOR;
      } else if (cmd == SER_DISCONNECT) {
        op_state = S_PLAY;
        comm_link = false;
      }
    }
  }
}


void get_vectr_vals(uint8_t thresh[4], uint8_t *g, uint8_t *v, uint8_t *d, uint8_t *s) {
  if (adata.velocity <= thresh[0]) {
    *g = 0; *s = 0; *v = 0; *d = 1;
  } else if (adata.velocity <= thresh[1]) {
    *g = 1; *s = 0; *v = adata.velocity - thresh[0]; *d = thresh[1] - thresh[0];
  } else if (adata.velocity <= thresh[2]) {
    *g = 2; *s = 1; *v = 0; *d = 1;
  } else if (adata.velocity <= thresh[3]) {
    *g = 3; *s = 1; *v = adata.velocity - thresh[2]; *d = thresh[3] - thresh[2];
  } else {
    *g = 4; *s = 1; *v = 1; *d = 1;
  }
}

void accel_velocity() {
  uint16_t bin_thresh = ACCEL_ONEG;
  uint8_t velocity = 0;
  uint8_t i = 0;
  uint16_t check_thresh;

  while (i < ACCEL_BINS) {
    bin_thresh += ACCEL_BIN_SIZE;
    check_thresh = bin_thresh;

    // Enlarge bin for current velocity
    if (i == adata.velocity - 3) check_thresh -= 4;
    if (i == adata.velocity - 2) check_thresh -= 12;
    if (i == adata.velocity - 1) check_thresh -= 28;
    if (i == adata.velocity)     check_thresh += 28;
    if (i == adata.velocity + 1) check_thresh += 12;
    if (i == adata.velocity + 2) check_thresh += 4;

    if (adata.magnitude > check_thresh) {
      adata.vectr_falloff[i] = 0;
      adata.vectr_trigger[i] = min(adata.vectr_trigger[i] + 1, 128);
    }

    if (adata.vectr_falloff[i] > ACCEL_FALLOFF) adata.vectr_trigger[i] = 0;
    if (adata.vectr_trigger[i] > ACCEL_TARGET)  velocity = i + 1;

    adata.vectr_falloff[i]++;
    i++;
  }

  adata.velocity = velocity;
}

void accel_variant() {
  if (mode.data[0] == M_PRIMER) {
    uint8_t value = 0;
    if (mode.pm.trigger_mode == T_VELOCITY)   value = adata.velocity;
    else if (mode.pm.trigger_mode == T_PITCH) value = adata.pitch;
    else if (mode.pm.trigger_mode == T_ROLL)  value = adata.roll;
    else if (mode.pm.trigger_mode == T_FLIP)  value = adata.flip;

    if ((active_pattern == 0 && value > mode.pm.trigger_thresh[0]) ||
        (active_pattern == 1 && value < mode.pm.trigger_thresh[1])) {
      adata.prime_falloff = 0;
      adata.prime_trigger = min(adata.prime_trigger + 1, 128);
    }

    if (adata.prime_falloff > ACCEL_FALLOFF) adata.prime_trigger = 0;
    if (adata.prime_trigger > ACCEL_TARGET) {
      adata.prime_falloff = 0;
      adata.prime_trigger = 0;
      active_pattern = !active_pattern;
    }
  } else {
    active_pattern = !active_pattern;
    patterns[active_pattern].trip = patterns[!active_pattern].trip;
    patterns[active_pattern].cidx = patterns[!active_pattern].cidx;
    patterns[active_pattern].cntr = patterns[!active_pattern].cntr;
    patterns[active_pattern].segm = patterns[!active_pattern].segm;
  }
}

void accel_timings() {
  if (mode.data[0] == M_VECTR) {
    uint8_t update_pattern = !active_pattern;
    uint8_t mg, mv, md, ms;
    uint8_t fg, fv, fd, fs;
    get_vectr_vals(mode.vm.tr_meta, &mg, &mv, &md, &ms);
    get_vectr_vals(mode.vm.tr_flux, &fg, &fv, &fd, &fs);

    if (fg == 0) {
      patterns[update_pattern].numc = mode.vm.numc[0];
    } else if (fg == 1) {
      patterns[update_pattern].numc = min(mode.vm.numc[0], mode.vm.numc[1]);
    } else if (fg == 2) {
      patterns[update_pattern].numc = mode.vm.numc[1];
    } else if (fg == 3) {
      patterns[update_pattern].numc = min(mode.vm.numc[1], mode.vm.numc[2]);
    } else {
      patterns[update_pattern].numc = mode.vm.numc[2];
    }

    for (uint8_t i = 0; i < 9; i++) {
      patterns[update_pattern].colors[i][0] = fast_interp(
        mode.vm.colors[fs][i][0], mode.vm.colors[fs + 1][i][0], fv, fd);
      patterns[update_pattern].colors[i][1] = fast_interp(
        mode.vm.colors[fs][i][1], mode.vm.colors[fs + 1][i][1], fv, fd);
      patterns[update_pattern].colors[i][2] = fast_interp(
        mode.vm.colors[fs][i][2], mode.vm.colors[fs + 1][i][2], fv, fd);
      if (i < 8) patterns[update_pattern].timings[i] = fast_interp(
          mode.vm.timings[ms][i], mode.vm.timings[ms + 1][i], mv, md);
    }
  }
}

void handle_accel() {
  if (accel_tick == 0) {
    TWADC_begin();
    TWADC_write_w(ACCEL_ADDR);
    TWADC_write((uint8_t)1);
  } else if (accel_tick == 1) {
    TWADC_begin();
    TWADC_write_r(ACCEL_ADDR);
  } else if (accel_tick == 2) {
    adata.axis_y = (int16_t)TWADC_read(1) << 8;
  } else if (accel_tick == 3) {
    adata.axis_y = (adata.axis_y | TWADC_read(0)) >> 4;
  } else if (accel_tick == 4) {
    TWADC_begin();
    TWADC_write_w(ACCEL_ADDR);
    TWADC_write((uint8_t)3);
  } else if (accel_tick == 5) {
    TWADC_begin();
    TWADC_write_r(ACCEL_ADDR);
  } else if (accel_tick == 6) {
    adata.axis_x = (int16_t)TWADC_read(1) << 8;
  } else if (accel_tick == 7) {
    adata.axis_x = (adata.axis_x | TWADC_read(0)) >> 4;
  } else if (accel_tick == 8) {
    TWADC_begin();
    TWADC_write_w(ACCEL_ADDR);
    TWADC_write((uint8_t)5);
  } else if (accel_tick == 9) {
    TWADC_begin();
    TWADC_write_r(ACCEL_ADDR);
  } else if (accel_tick == 10) {
    adata.axis_z = (int16_t)TWADC_read(1) << 8;
  } else if (accel_tick == 11) {
    adata.axis_z = (adata.axis_z | TWADC_read(0)) >> 4;
  } else if (accel_tick == 12) {
    adata.axis_x2 = pow(adata.axis_x, 2);
    adata.axis_y2 = pow(adata.axis_y, 2);
    adata.axis_z2 = pow(adata.axis_z, 2);;
    adata.magnitude = fast_sqrt(adata.axis_x2 + adata.axis_y2 + adata.axis_z2);
    adata.fpitch = fast_sqrt(adata.axis_y2 + adata.axis_z2);
    adata.froll = fast_sqrt(adata.axis_x2 + adata.axis_z2);
  } else if (accel_tick == 13) {
    adata.fpitch = fast_atan2(-adata.axis_x, adata.fpitch);
  } else if (accel_tick == 14) {
    adata.froll = fast_atan2(adata.axis_y, adata.froll);
  } else if (accel_tick == 15) {
    adata.pitch = 16 + constrain(adata.fpitch * ACCEL_COEF, -16, 16);
    adata.roll  = 16 + constrain(adata.froll  * ACCEL_COEF, -16, 16);
    adata.flip  = 16 + constrain(adata.axis_z / 30,         -16, 16);
  } else if (accel_tick == 16) {
    accel_velocity();
  } else if (accel_tick == 17) {
    accel_timings();
  } else if (accel_tick == 18) {
    accel_variant();
  }

  accel_tick++;
  if (accel_tick >= ACCEL_COUNTS) accel_tick = 0;
}

void handle_button() {
  bool pressed = digitalRead(PIN_BUTTON) == LOW;
  bool changed = pressed != was_pressed;
  was_pressed  = pressed;

  if (op_state == S_PLAY) {
    if (pressed) {
      if (since_press == 2000)      flash(0, 0, 255);
      else if (since_press == 4000) flash(255, 0, 0);
      else if (since_press == 6000) flash(255, 255, 255);
    } else if (changed) {
      if (since_press < 1000) {
        if (conjure) {
          enter_sleep();
        } else {
          change_mode(cur_mode + 1);
        }
      } else if (since_press < 2000) {
        enter_sleep();
      } else if (since_press < 4000) {
        conjure = !conjure;
        ee_update(ADDR_CONJURE, conjure);
        ee_update(ADDR_CONJURE_MODE, cur_mode);
      } else if (since_press < 6000) {
        ee_update(ADDR_LOCKED, true);
        enter_sleep();
      } else {
        enter_sleep();
      }
    }
  } else if (op_state == S_WAKE) {
    if (locked) {
      if (pressed) {
        if (since_press == 6000) flash(0, 255, 0);
      } else if (changed) {
        if (since_press < 6000) {
          flash(255, 0, 0);
          enter_sleep();
        } else if (since_press < 8000) {
          ee_update(ADDR_LOCKED, false);
          op_state = S_PLAY;
        } else {
          flash(255, 0, 0);
          enter_sleep();
        }
      }
    } else {
      if (pressed) {
        if (since_press == 2000)      flash(255, 255, 255);
        else if (since_press == 4000) flash(255, 0, 255);
      } else if (changed) {
        if (since_press < 2000) {
          op_state = S_PLAY;
        } else if (since_press < 4000) {
          op_state = S_BRIGHT;
        } else if (since_press < 6000) {
          op_state = S_RESET;
        } else {
          enter_sleep();
        }
      }
    }
  } else if (op_state == S_BRIGHT) {
    if (pressed) {
      if (since_press == 2000) flash(255, 255, 255);
    } else if (changed) {
      if (since_press < 2000) {
        brightness = (brightness + 1) % 3;
        since_press = 0;
      } else {
        ee_update(ADDR_BRIGHTNESS, brightness);
        op_state = S_PLAY;
      }
    }
  } else if (op_state == S_RESET) {
    if (pressed) {
      if (since_press == 6000) flash(255, 0, 255);
    } else if (changed) {
      if (since_press < 6000) {
        enter_sleep();
      } else if (since_press < 8000) {
        reset_memory();
        op_state = S_PLAY;
      } else {
        enter_sleep();
      }
    }
  }

  since_press++;
  if (changed) since_press = 0;
}


void render_mode() {
  // For Vectr modes we only render the active pattern
  // For Primer modes, we run both patterns to increment state but only render the active
  if (mode.data[0] == M_VECTR) {
    pattern_funcs[mode.vm.pattern](&patterns[active_pattern], &rgb_r, &rgb_g, &rgb_b, true);
  } else {
    pattern_funcs[mode.pm.pattern[0]](&patterns[0], &rgb_r, &rgb_g, &rgb_b, active_pattern == 0);
    pattern_funcs[mode.pm.pattern[1]](&patterns[1], &rgb_r, &rgb_g, &rgb_b, active_pattern == 1);
  }
}

void handle_render() {
  rgb_r = 0;
  rgb_g = 0;
  rgb_b = 0;

  if (op_state == S_PLAY) {
    if (!was_pressed) {
      render_mode();
    }
  } else if (op_state == S_BRIGHT) {
    rgb_r = 255;
    rgb_g = 255;
    rgb_b = 255;
  } else if (op_state == S_RESET) {
    rgb_r = 255;
    rgb_g = 0;
    rgb_b = 255;
  } else if (op_state == S_GUI_MODE) {
    render_mode();
  } else if (op_state == S_GUI_COLOR) {
    if (mode.data[0] == M_VECTR) {
      rgb_r = mode.vm.colors[gui_set][gui_slot][0];
      rgb_g = mode.vm.colors[gui_set][gui_slot][1];
      rgb_b = mode.vm.colors[gui_set][gui_slot][2];
    } else {
      rgb_r = mode.pm.colors[gui_set][gui_slot][0];
      rgb_g = mode.pm.colors[gui_set][gui_slot][1];
      rgb_b = mode.pm.colors[gui_set][gui_slot][2];
    }
  }
  write_frame();
}


void power_down() {
  ADCSRA &= ~(1 << ADEN);
  set_sleep_mode(SLEEP_FOREVER);
  cli();
  sleep_enable();
  sleep_bod_disable();
  sei();
  sleep_cpu();
  sleep_disable();
  sei();
  ADCSRA |= (1 << ADEN);
}

void setup() {
  // Check the sleeping bit, if it's set, go into low power mode until button press
  attachInterrupt(0, _push_interrupt, FALLING);
  if (ee_read(ADDR_SLEEPING)) {
    ee_update(ADDR_SLEEPING, 0);
    power_down();
    op_state = S_WAKE;
  } else {
    op_state = S_PLAY;
  }
  detachInterrupt(0);

  Serial.begin(115200);
  randomSeed(analogRead(0));

  pinMode(PIN_BUTTON, INPUT);             // Setup pins 
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_LDO, OUTPUT);
  digitalWrite(PIN_LDO, HIGH);            // Power on Low-Voltage Dropoff

  sbi(ADCSRA, ADPS2);                     // Configure ADC for TWACD functions
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  noInterrupts();                         // Configure timers for fastest PWM
  TCCR0B = (TCCR0B & 0b11111000) | 0b001; // no prescaler ~64/ms
  TCCR1B = (TCCR1B & 0b11111000) | 0b001; // no prescaler ~32/ms
  sbi(TCCR1B, WGM12);                     // fast PWM ~64/ms
  limiter_us <<= 6;                       // Since the clock timer is 64x normal, compensate
  interrupts();

  if (!version_match()) reset_memory();   // check verision and reset memory if no match

  locked     = ee_read(ADDR_LOCKED);      // Read in stored settings
  conjure    = ee_read(ADDR_CONJURE);
  brightness = ee_read(ADDR_BRIGHTNESS);
  if (conjure) cur_mode = ee_read(ADDR_CONJURE_MODE);

  pattern_funcs[P_STROBE]   = &pattern_strobe;
  pattern_funcs[P_TRACER]   = &pattern_tracer;
  pattern_funcs[P_MORPH]    = &pattern_morph;
  pattern_funcs[P_SCIMITAR] = &pattern_scimitar;
  pattern_funcs[P_WAVE]     = &pattern_wave;
  pattern_funcs[P_DYNAMO]   = &pattern_dynamo;
  pattern_funcs[P_SHIFTER]  = &pattern_shifter;
  pattern_funcs[P_TRIPLE]   = &pattern_triple;
  pattern_funcs[P_STEPPER]  = &pattern_stepper;
  pattern_funcs[P_RANDOM]   = &pattern_random;

  accel_init();                           // initialize the accelerometer
  change_mode(cur_mode);                  // initialize current mode

  Serial.write(SER_HANDSHAKE);            // Send handshake to probe for GUI
  Serial.write(SER_VERSION);
  Serial.write(SER_VERSION);

  last_write = micros();                  // Reset the limiter
}

void loop() {
  handle_serial();
  handle_button();
  handle_accel();
  handle_render();
}
