#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include "PIDReportType.h"

#pragma once

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
} Settings;

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
