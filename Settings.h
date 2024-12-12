#ifndef SETTINGS_h
#define SETTINGS_h

#include <Arduino.h>
#include <EEPROM.h>
#include "PIDReportType.h"

#define JOYSTICK_DEFAULT_AXIS_MINIMUM 0
#define JOYSTICK_DEFAULT_AXIS_MAXIMUM 1023

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
} Settings; //this total needs to match size given in PidDesciptor.h line 631, and size of Settings struct in Settings.h

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

#endif  // SETTINGS_h
