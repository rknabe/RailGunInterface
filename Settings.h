#ifndef SETTINGS_h
#define SETTINGS_h

#include <Arduino.h>
#include "PIDReportType.h"

#define JOYSTICK_DEFAULT_AXIS_MINIMUM 0
#define JOYSTICK_DEFAULT_AXIS_MAXIMUM 1023
#define BTN_TRIGGER 4
#define BTN_LEFT 5
#define BTN_BOTTOM 6
#define BTN_START 7
#define BTN_COIN 8
#define RECOIL_RELAY_PIN 9
#define LIGHT_RELAY_PIN 10
#define AXIS_X_PIN A0
#define AXIS_Y_PIN A1
#define BUTTON_DEBOUNCE_DELAY 50  //[ms]
#define SERIAL_BAUDRATE 115200
#define RECOIL_MS 40
#define RECOIL_RELEASE_MS 40

typedef struct {
  char id[10];
  char ver[6];
  int16_t xAxisMinimum;
  int16_t xAxisMaximum;
  int16_t yAxisMinimum;
  int16_t yAxisMaximum;
  bool autoRecoil;
  int16_t triggerRepeatRate;
  int16_t triggerHoldTime;
} Settings;  //this total needs to match size given in PidDesciptor.h line 631, and size of Settings struct in Settings.h

//all settings
class SettingsEEPROM {
public:
  Settings data;
  uint8_t checksum;

  Settings load(bool defaults);
  Settings getDefaults();
  void save(Settings settings);
  uint8_t calcChecksum();
};

/*
#define bullet_width 50
#define bullet_height 10
const static unsigned char bullet[] PROGMEM = {
  0x00, 0xcc, 0xff, 0xff, 0x00, 0x00, 0x00, 0x63, 0xfa, 0x00, 0x80, 0x03,
  0x00, 0x00, 0x26, 0x92, 0x00, 0x80, 0xfe, 0x0f, 0x00, 0x34, 0x92, 0x00,
  0x80, 0x44, 0x78, 0x00, 0x1c, 0x92, 0x00, 0x80, 0x44, 0xc0, 0x03, 0x1c,
  0x92, 0x00, 0x80, 0x44, 0xc0, 0x03, 0x36, 0x92, 0x00, 0x80, 0x44, 0x78,
  0x00, 0x62, 0x92, 0x00, 0x80, 0xfe, 0x0f, 0x00, 0x43, 0xf2, 0x00, 0x80,
  0x03, 0x00, 0x00, 0x00, 0xcc, 0xff, 0xff, 0x01, 0x00, 0x00
};

#define logo_width 60
#define logo_height 30
const static unsigned char logo[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x60,
  0xe0, 0x0f, 0xfe, 0x0f, 0x00, 0x00, 0x20, 0xf0, 0xe0, 0x3f, 0xfe, 0x0f,
  0xff, 0x71, 0x30, 0xf0, 0xc0, 0x3f, 0xfe, 0x07, 0xfe, 0x73, 0x30, 0xf0,
  0xc0, 0x7f, 0xfe, 0x07, 0xfe, 0x73, 0x38, 0xf0, 0xc1, 0x7f, 0xfe, 0x07,
  0xfe, 0x73, 0x38, 0xf8, 0xc1, 0x7f, 0xfe, 0x07, 0xfe, 0x73, 0x3c, 0xf8,
  0x81, 0xff, 0xfe, 0x03, 0x80, 0x77, 0x3e, 0xf8, 0x03, 0xf0, 0x0e, 0x00,
  0x80, 0x77, 0x1e, 0xfc, 0x03, 0xf0, 0x1e, 0x00, 0xfe, 0x77, 0x1f, 0xfc,
  0xc3, 0xe1, 0xfe, 0x01, 0xfe, 0xf3, 0x0f, 0xfc, 0xc3, 0xe1, 0xfe, 0x01,
  0xfe, 0xf3, 0x0f, 0xfe, 0xc7, 0xe1, 0xfe, 0x01, 0xfe, 0xf3, 0x0f, 0xbe,
  0xc7, 0xe1, 0xfe, 0x01, 0xfe, 0xf1, 0x1f, 0x9e, 0xc7, 0xe1, 0xfe, 0x00,
  0xfe, 0xf1, 0x1f, 0x9e, 0xcf, 0xe1, 0xfe, 0x00, 0xce, 0xf3, 0x1f, 0x1f,
  0xcf, 0xf1, 0x0e, 0x00, 0xce, 0xf3, 0x1d, 0x0f, 0xcf, 0xf1, 0x0e, 0x00,
  0x8e, 0xf3, 0x3c, 0x0f, 0xdf, 0xff, 0x7e, 0x00, 0x8e, 0xf7, 0xb8, 0x0f,
  0xdf, 0x7f, 0x7e, 0x00, 0x8e, 0x77, 0xb8, 0xff, 0xdf, 0x7f, 0x7e, 0x00,
  0x0e, 0x77, 0xf8, 0xf7, 0xff, 0x7f, 0x3e, 0x00, 0x06, 0x37, 0xf8, 0xf7,
  0xff, 0x3f, 0x3e, 0x00, 0x02, 0x10, 0xc0, 0xff, 0xff, 0x1f, 0x3e, 0x00,
  0x00, 0x10, 0xc0, 0xfb, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xfb,
  0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x7f, 0x00, 0x00, 0x00
};*/

#endif  // SETTINGS_h
