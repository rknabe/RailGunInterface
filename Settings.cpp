#include "Settings.h"

uint8_t SettingsEEPROM::calcChecksum() {
  uint8_t i, checksum;

  checksum = ~((uint8_t*)this)[0];
  for (i = 1; i < sizeof(SettingsEEPROM) - 1; i++)
    checksum = checksum ^ ~((uint8_t*)this)[i];
  return checksum;
}

Settings SettingsEEPROM::load(bool defaults) {
  SettingsEEPROM settingsE;
  uint8_t checksum;
  EEPROM.get(0, settingsE);
  checksum = settingsE.calcChecksum();

  //Loading defaults
  if (defaults || (settingsE.checksum != checksum)) {
    Serial.println(F("Loading defaults"));
    settingsE.data.xAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
    settingsE.data.xAxisMinimum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
    settingsE.data.yAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
    settingsE.data.yAxisMinimum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
  }
  return settingsE.data;
}

void SettingsEEPROM::save(Settings settings) {
  SettingsEEPROM settingsE;
  settingsE.data = settings;
  settingsE.checksum = settingsE.calcChecksum();
  EEPROM.put(0, settingsE);
}