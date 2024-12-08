#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include "PIDReportType.h"

#pragma once

#define JOYSTICK_DEFAULT_AXIS_MINIMUM 0
#define JOYSTICK_DEFAULT_AXIS_MAXIMUM 1023

//all settings
class SettingsEEPROM {
public:
  Settings data;
  uint8_t checksum;

  Settings load(bool defaults);
  void save(Settings settings);
  uint8_t calcChecksum();
};
