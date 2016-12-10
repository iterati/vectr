#ifndef __HUEY_H
#define __HUEY_H


#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

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

#define SER_VERSION       34    // Current serial version for UI
#define SER_WRITE         100   // Write command: addr, value
#define SER_HANDSHAKE     200   // Handshake command: SER_VERSION, value, value (values must be equal)
#define SER_DISCONNECT    210   // Disconnect command
#define SER_VIEW_MODE     220   // View in-memory mode command
#define SER_VIEW_COLOR    230   // View in-memory color command: color set, color slot
#define SER_INIT          240   // Call init_mode()

#define STATE_PLAY        0     // Normal operation
#define STATE_WAKE        1     // Waking from sleep
#define STATE_GUI_MODE    2     // Viewing in-memory mode
#define STATE_GUI_COLOR   3     // Viewing in-memory color

#define ACCEL_COUNTS      40    // 40 frames between accel reads (50hz)
#define ACCEL_BINS        100
#define ACCEL_ONEG        256   // +- 4g range
#define ACCEL_MAX_GS      11
#define ACCEL_FALLOFF     8     // 20ms cycles before falloff
#define ACCEL_TARGET      1     // 20ms cycles before triggering


typedef union Settings {
  struct {
    unsigned sleeping : 1;
    unsigned locked   : 1;
    unsigned conjure  : 1;
    unsigned bundle   : 1;
    unsigned mode     : 8;
  };
  uint8_t settings[2];
} Settings;

typedef struct Color {
  unsigned r        : 8;
  unsigned g        : 8;
  unsigned b        : 8;
} Color;

typedef union Mode {
  struct {
    uint8_t pattern;                  // 0
    uint8_t args[4];                  // 1 - 4
    uint8_t timings[3][8];            // 5 - 28
    uint8_t meta[4];                  // 29 - 32
    uint8_t flux[4];                  // 33 - 36
    uint8_t numc[3];                  // 37 - 39
    uint8_t colors[3][NUM_COLORS][3]; // 40 - 255
  };
  uint8_t data[MODE_SIZE];            // 256
} Mode;

typedef struct PatternState {
  uint8_t args[4];                            // Pattern arguments
  uint8_t timings[8];                         // Pattern timings
  uint8_t numc;                               // Number of active colors
  uint8_t colors[NUM_COLORS][3];              // RGB values for colors

  uint16_t trip;                              // Frames until next segment
  uint8_t cidx;                               // Current color index
  uint8_t cnt0;
  uint8_t cnt1;
  uint8_t segm;                               // Current pattern segment
} PatternState;

typedef struct AccelData {
  uint8_t falloff[ACCEL_BINS];                // Falloff values for vectr
  uint8_t trigger[ACCEL_BINS];                // Trigger values (how many frames have we seen a signal this strong) for vectr
  uint8_t velocity;
  uint16_t magnitude;                         // magnitude of acceleration (sqrt(x^2 + y^2 + z^2)
  int16_t axis_x, axis_y, axis_z;             // raw accel values from the accelerometer
  uint32_t axis_x2, axis_y2, axis_z2;         // accel values from accelerometer squared
  uint8_t g, v, d, s;
} AccelData;

#endif
