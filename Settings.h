//#include <avr/pgmspace.h>
#ifndef SETTINGS_h
#define SETTINGS_h

#include <Arduino.h>
#include <EEPROM.h>
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
#define SERIAL_BAUDRATE 9600
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

#define health_width 32
#define health_height 64
const static unsigned char health[] PROGMEM = {
  0xfc, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0,
  0x07, 0x00, 0x00, 0xe0, 0xfe, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0x7f,
  0xf8, 0xff, 0xff, 0x1f, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0xfc, 0x3f, 0x00,
  0x00, 0xfc, 0x3f, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0xf8, 0x1f, 0x00,
  0x00, 0xf0, 0x0f, 0x00
};


#define bullet_width 50
#define bullet_height 10
const static unsigned char bullet[] PROGMEM = {
  0x00, 0x00, 0xfc, 0xff, 0xcf, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x7c,
  0x19, 0x03, 0xc0, 0xff, 0x05, 0x00, 0x24, 0x91, 0x01, 0x78, 0x88, 0x04,
  0x00, 0x24, 0xb1, 0x00, 0x0f, 0x88, 0x04, 0x00, 0x24, 0xe1, 0x00, 0x0f,
  0x88, 0x04, 0x00, 0x24, 0xe1, 0x00, 0x78, 0x88, 0x04, 0x00, 0x24, 0xb1,
  0x01, 0xc0, 0xff, 0x05, 0x00, 0x24, 0x19, 0x01, 0x00, 0x00, 0x07, 0x00,
  0x3c, 0x09, 0x03, 0x00, 0x00, 0xfe, 0xff, 0xcf, 0x00, 0x00
};

#define logo_width 60
#define logo_height 30
const static unsigned char logo[] PROGMEM = {
  0x00, 0x00, 0xe0, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff,
  0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x3d, 0x80, 0x00, 0x00,
  0xc0, 0x87, 0xff, 0xff, 0x3f, 0x80, 0x00, 0x04, 0xc0, 0xc7, 0xff, 0xff,
  0xfe, 0xc1, 0x0e, 0x06, 0xc0, 0xe7, 0xff, 0xff, 0xfe, 0xe1, 0x0e, 0x07,
  0xe0, 0xe7, 0xbf, 0xff, 0xdf, 0xe1, 0x1e, 0x07, 0xe0, 0xe7, 0xbf, 0x0f,
  0xdf, 0xf1, 0x1e, 0x07, 0xe0, 0xf7, 0xbf, 0x0f, 0xcf, 0xf3, 0x1c, 0x07,
  0x00, 0xf7, 0x38, 0x0f, 0x8f, 0xfb, 0x3c, 0x07, 0x00, 0xf7, 0x38, 0x8f,
  0x8f, 0xff, 0x3c, 0x07, 0xf0, 0x77, 0x38, 0x9f, 0x87, 0xff, 0xf8, 0x07,
  0xf0, 0x77, 0x38, 0x9e, 0x87, 0xff, 0xf8, 0x07, 0xf8, 0x77, 0x38, 0xde,
  0x07, 0xff, 0xfc, 0x07, 0xf8, 0x77, 0x38, 0xfe, 0x07, 0xff, 0xfc, 0x07,
  0xf8, 0x77, 0x38, 0xfc, 0x03, 0xff, 0xfc, 0x07, 0xf8, 0x77, 0x38, 0xfc,
  0x83, 0xef, 0xfe, 0x07, 0x80, 0xf7, 0x00, 0xfc, 0x83, 0xe7, 0x1e, 0x00,
  0x00, 0xf7, 0x00, 0xfc, 0xc1, 0xe7, 0x1e, 0x00, 0xfc, 0xf7, 0x1f, 0xf8,
  0xc1, 0xe3, 0xfc, 0x07, 0xfe, 0xf7, 0x3f, 0xf8, 0xc1, 0xe1, 0xfc, 0x07,
  0xfe, 0xe7, 0x3f, 0xf8, 0xc0, 0xe1, 0xfc, 0x07, 0xfe, 0xe7, 0x3f, 0xf0,
  0xc0, 0xe0, 0xfc, 0x07, 0xfe, 0xc7, 0x3f, 0xf0, 0xc0, 0xe0, 0xf8, 0x0f,
  0xff, 0xc7, 0x7f, 0xf0, 0x40, 0x00, 0x00, 0x00, 0xff, 0x07, 0x7f, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#endif  // SETTINGS_h
