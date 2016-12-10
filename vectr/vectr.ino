#include <Arduino.h>
#include <EEPROM.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define ADDR_SETTINGS 800
#define NUM_MODES     16

#define PIN_R             9
#define PIN_G             6
#define PIN_B             5
#define PIN_BUTTON        2
#define PIN_LDO           A3
#define ACCEL_ADDR        0x1D
#define ACCEL_ADDR_W      0x3A // 0x1D << 1
#define ACCEL_ADDR_R      0x3B // 0x1D << 1 + 1
#define SCL_PIN           A5
#define SDA_PIN           A4
#define I2CADC_H          315
#define I2CADC_L          150

#define STATE_PLAY        0
#define STATE_WAKE        1

#define ACCEL_COUNTS      40
#define ACCEL_BINS        100
#define ACCEL_ONEG        256
#define ACCEL_MAX_GS      10
uint32_t ACCEL_BIN_SIZE = (ACCEL_MAX_GS * ACCEL_ONEG) / ACCEL_BINS;
#define ACCEL_FALLOFF     8
#define ACCEL_TARGET      1
uint16_t bin_thresh = ACCEL_ONEG;


typedef struct RGBColor {
  uint8_t r, g, b;
} RGBColor;

typedef struct HSVColor {
  uint8_t h, s, v;
}

typedef union Settings {
  struct {
    unsigned sleeping :1;
    unsigned mode     :6;
  };
  uint8_t settings;
} Settings;

typedef struct PatternState {
  uint16_t trip;
  uint16_t segm;
  uint16_t cnt0;
  uint16_t cnt1;
  uint16_t cnt2;
} PatternState;

typedef struct AccelData {
  uint8_t falloff[ACCEL_BINS];
  uint8_t trigger[ACCEL_BINS];
  uint8_t velocity;
  uint16_t magnitude;
  int16_t axis_x, axis_y, axis_z;
  uint32_t axis_x2, axis_y2, axis_z2;
} AccelData;

Settings settings;
PatternState state;
AccelData accel;

uint8_t ledr, ledg, ledb;
uint32_t limiter_us = 500;
uint32_t last_write = 0;
uint32_t since_press = 0;
bool was_pressed = false;
uint8_t op_state = STATE_PLAY;
uint8_t accel_tick = 0;


/* UTILITY FUNCTIONS */
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

const float COEFF1 = PI * 0.25;
const float COEFF2 = 3 *  COEFF1;

float fast_atan2(float y, float x) {
  float abs_y = fabs(y) + 1e-10;
  float angle, r;

  if (x >= 0) {
    r = (x - abs_y) / (x + abs_y);
    angle = COEFF1 - COEFF1 * r;
  } else {
    r = (x + abs_y) / (abs_y - x);
    angle = COEFF2 - COEFF1 * r;
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

uint8_t fast_hsv_sat(uint8_t s, uint8_t e, uint8_t sat) {
  if (
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
    I2CADC_SCL_L_INPUT();
    I2CADC_SDA_L_INPUT();
    I2CADC_SCL_H_OUTPUT();
    I2CADC_SCL_L_INPUT();
  } else {
AckThis:
    I2CADC_SCL_L_INPUT();
    I2CADC_SCL_H_OUTPUT();
    int result = analogRead(SCL_PIN);
    if (result < I2CADC_L) {
      goto AckThis;
    }
    I2CADC_SCL_L_INPUT();
  }
  return data;
}

void TWADC_begin() {
  I2CADC_SCL_H_OUTPUT();
  I2CADC_SDA_H_OUTPUT();
  I2CADC_SDA_L_INPUT();
  I2CADC_SCL_H_OUTPUT();
  I2CADC_SCL_L_INPUT();
}

void TWADC_endTransmission() {
  I2CADC_SDA_L_INPUT();
  I2CADC_SCL_H_OUTPUT();
  I2CADC_SDA_H_OUTPUT();
}

void TWADC_send(uint8_t addr, uint8_t data) {
  TWADC_begin();
  TWADC_write(ACCEL_ADDR_W);
  TWADC_write(addr);
  TWADC_write(data);
  TWADC_endTransmission();
  delay(1);
}

void accel_init() {
  TWADC_begin();
  delay(1);
  TWADC_send(0x2A, B00000000);
  TWADC_send(0x0E, B00000010);
  TWADC_send(0x2B, B00011011);
  TWADC_send(0x2C, B00000000);
  TWADC_send(0x2D, B00000000);
  TWADC_send(0x2E, B00000000);
  TWADC_send(0x2A, B00100001);
}

void accel_standby() {
  TWADC_send(0x2A, 0x00);
}


void init_mode(uint8_t dst) {
  states[dst].trip = 0;
  states[dst].segm = 0;
  states[dst].cnt0 = 0;
  states[dst].cnt1 = 0;
  states[dst].cnt2 = 0;
}

void next_mode() {
  settings.mode++;
  if (settings.mode >= NUM_MODES) settings.mode = 0;
  init_mode();
}


/* LED OUTPUT FUNCTIONS */
void write_frame(uint8_t r, uint8_t g, uint8_t b) {
  uint32_t cus = micros();
  while (cus - last_write < limiter_us) cus = micros();
  last_write = cus;

  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
}

void flash(uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t i = 0; i < 5; i++) {
    for (uint8_t j = 0; j < 200; j++) {
      if (j < 100) write_frame(0, 0, 0);
      else         write_frame(r, g, b);
    }
  }
  since_press += 1000;
}


/* SLEEP FUNCTIONS */
void _push_interrupt() {
  sleep_disable();
  detachInterrupt(0);
}

void power_down() {
  // Set up sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(0, _push_interrupt, FALLING);
  ADCSRA = 0;

  // Go to sleep here
  cli();
  sleep_bod_disable();
  sei();
  sleep_cpu();

  // Wake up here
  sleep_disable();
  detachInterrupt(0);
  settings.sleeping = 0;
}


void enter_sleep() {
  settings.sleeping = 1;
  while (!eeprom_is_ready()) {}
  EEPROM.update(ADDR_SETTINGS, settings.settings);
  write_frame(0, 0, 0);
  accel_standby();
  digitalWrite(PIN_LDO, LOW);
  wdt_enable(WDTO_15MS);
  while (true) {}
}


void accel_velocity(uint8_t start) {
  uint8_t i = start;
  uint8_t _end = start + 50;

  if (start == 0) {
    bin_thresh = ACCEL_ONEG;
    accel.velocity = 0;
  }

  while (i < _end) {
    bin_thresh += ACCEL_BIN_SIZE;
    if (accel.magnitude > bin_thresh) {
      accel.falloff[i] = 0;
      accel.trigger[i] = min(accel.trigger[i] + 1, 128);
    }
    if (accel.falloff[i] > ACCEL_FALLOFF) accel.trigger[i] = 0;
    if (accel.trigger[i] > ACCEL_TARGET) accel.velocity = i + 1;
    accel.falloff[i]++;
    i++;
  }
}


const uint8_t VALUE_R[64] = {
    3,   6,   9,  13,  16,  19,  22,  26,  29,  32,  35,  39,  42,  45,  48,  52,
   55,  58,  61,  65,  68,  71,  74,  78,  81,  84,  87,  91,  94,  97, 100, 104,
  107, 110, 113, 117, 120, 123, 126, 130, 133, 136, 139, 143, 146, 149, 152, 156,
  159, 162, 165, 169, 172, 175, 178, 182, 185, 188, 191, 195, 198, 201, 204, 208,
};

const uint8_t VALUE_G[64] = {
    3,   7,  10,  14,  17,  21,  24,  28,  31,  35,  38,  42,  45,  49,  52,  56,
   59,  63,  66,  70,  73,  77,  80,  84,  87,  91,  94,  98, 101, 105, 108, 112,
  115, 119, 122, 126, 129, 133, 136, 140, 143, 147, 150, 154, 157, 161, 164, 168,
  171, 175, 178, 182, 185, 189, 192, 196, 199, 203, 206, 210, 213, 217, 220, 224,
};

const uint8_t VALUE_B[64] = {
    3,   7,  11,  15,  18,  22,  26,  30,  33,  37,  41,  45,  48,  52,  56,  60,
   63,  67,  71,  75,  78,  82,  86,  90,  93,  97, 101, 105, 108, 112, 116, 120,
  123, 127, 131, 135, 138, 142, 146, 150, 153, 157, 161, 165, 168, 172, 176, 180,
  183, 187, 191, 195, 198, 202, 206, 210, 213, 217, 221, 225, 228, 232, 236, 240,
};

RGBColor hsv_to_rgb(HSVColor *hsv) {
  RGBColor rgb;

  if (hsv->h >= 192) hsv->h -= 192;
  if (hsv->h < 64) {
    rgb.r = VALUES_R[63 - hsv->h];
    rgb.g = (x == 0) ? 0 : VALUES_G[hsv->h];
    rgb.b = 0;

  } else if (hsv->h < 128) {
    rgb.r = 0;
    rgb.g = VALUES_G[127 - hsv->h];
    rgb.b = (x == 64) ? 0 : VALUES_B[hsv->h - 64];
  } else {
    rgb.r = (hsv->h == 128) ? 0 : VALUES_R[hsv->h - 128];
    rgb.g = 0;
    rgb.b = VALUES_B[191 - hsv->h];
  }

  if (hsv->s >= 8) {
    rgb.r = 52;
    rgb.g = 56;
    rgb.b = 60;
  } else {
    rgb.r = interp(rgb.r, 52, hsv->s, 8);
    rgb.g = interp(rgb.g, 56, hsv->s, 8);
    rgb.b = interp(rgb.b, 56, hsv->s, 8);
  }
}

void mode_darkside() {

}


void handle_button() {
  bool pressed = digitalRead(PIN_BUTTON) == LOW;
  bool changed = pressed != was_pressed;

  if (op_state == STATE_PLAY) {
    if (pressed) {
    } else if (changed) {
      if (since_press < 1000) {
        next_mode();
      } else {
        enter_sleep();
      }
    }
  } else if (op_state == STATE_WAKE) {
    if (pressed) {
    } else if (changed) {
      op_state = STATE_PLAY;
    }
  }

  since_press++;
  if (changed) since_press = 0;
  was_pressed  = pressed;
}

void handle_accel() {
  switch (accel_tick) {
    case 0:
      TWADC_begin();
      TWADC_write(ACCEL_ADDR_W);
      break;

    case 1:
      TWADC_write((uint8_t)1);
      break;

    case 2:
      TWADC_begin();
      TWADC_write(ACCEL_ADDR_R);
      break;

    case 3:
      accel.axis_y = (int16_t)TWADC_read(1) << 8;
      break;

    case 4:
      accel.axis_y = (accel.axis_y | TWADC_read(0)) >> 4;
      break;

    case 5:
      TWADC_begin();
      TWADC_write(ACCEL_ADDR_W);
      break;

    case 6:
      TWADC_write((uint8_t)3);
      break;

    case 7:
      TWADC_begin();
      TWADC_write(ACCEL_ADDR_R);
      break;

    case 8:
      accel.axis_x = (int16_t)TWADC_read(1) << 8;
      break;

    case 9:
      accel.axis_x = (accel.axis_x | TWADC_read(0)) >> 4;
      break;

    case 10:
      TWADC_begin();
      TWADC_write(ACCEL_ADDR_W);
      break;

    case 11:
      TWADC_write((uint8_t)5);
      break;

    case 12:
      TWADC_begin();
      TWADC_write(ACCEL_ADDR_R);
      break;

    case 13:
      accel.axis_z = (int16_t)TWADC_read(1) << 8;
      break;

    case 14:
      accel.axis_z = (accel.axis_z | TWADC_read(0)) >> 4;
      break;

    case 15:
      accel.axis_x2 = pow(accel.axis_x, 2);
      accel.axis_y2 = pow(accel.axis_y, 2);
      break;

    case 16:
      accel.magnitude = fast_sqrt(accel.axis_x2 + accel.axis_y2);
      break;

    case 17:
      accel_velocity(0);
      break;

    case 18:
      accel_velocity(50);
      break;

    default:
      accel_tick = 0;
      break;
  }

  accel_tick++;
  if (accel_tick >= ACCEL_COUNTS) accel_tick = 0;
}

void render_mode() {
  switch (settings.mode) {
    case 0:
      break;

    default:
      settings.mode = 0;
      write_frame(0, 0, 0);
      break;
  }
}

void handle_render() {
  if (op_state == STATE_PLAY && !was_pressed) render_mode();
  else                                        write_frame(0, 0, 0);
}


void setup() {
  // Get settings
  while (!eeprom_is_ready()) {}
  settings.settings = EEPROM.read(ADDR_SETTINGS);

  // Handle sleeping
  pinMode(PIN_BUTTON, INPUT);
  if (settings.sleeping) { power_down(); op_state = STATE_WAKE; }
  else                   {               op_state = STATE_PLAY; }

  // LEDs
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  noInterrupts();                                 // Configure timers for fastest PWM
  TCCR0B = (TCCR0B & 0b11111000) | 0b001;         // no prescaler ~64/ms
  TCCR1B = (TCCR1B & 0b11111000) | 0b001;         // no prescaler ~32/ms
  sbi(TCCR1B, WGM12);                             // fast PWM ~64/ms
  limiter_us <<= 6;                               // Since the clock timer is 64x normal, compensate
  interrupts();

  // Accel
  pinMode(PIN_LDO, OUTPUT);
  digitalWrite(PIN_LDO, HIGH);
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
  accel_init();

  // General
  Serial.begin(115200);
  settings.mode = 0;
  last_write = micros();
}

void loop() {
  handle_button();
  handle_accel();
  handle_render();
}
