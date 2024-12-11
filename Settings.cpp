#include "Settings.h"

uint8_t SettingsEEPROM::calcChecksum() {
  uint8_t i, checksum;

  checksum = ~((uint8_t*)this)[0];
  for (i = 1; i < sizeof(SettingsEEPROM) - 1; i++)
    checksum = checksum ^ ~((uint8_t*)this)[i];
  return checksum;
}

Settings SettingsEEPROM::getDefaults() {
  SettingsEEPROM settingsE;
  settingsE.data.xAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
  settingsE.data.xAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
  settingsE.data.yAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
  settingsE.data.yAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
  settingsE.data.autoRecoil = true;
  settingsE.data.triggerRepeatRate = 100;
  settingsE.data.triggerHoldTime = 1000;
  return settingsE.data;
}

Settings SettingsEEPROM::load(bool defaults) {
  SettingsEEPROM settingsE;
  uint8_t checksum;
  EEPROM.get(0, settingsE);
  checksum = settingsE.calcChecksum();

  //Loading defaults
  if (defaults || (settingsE.checksum != checksum)) {
    return getDefaults();
  }
  return settingsE.data;
}

void SettingsEEPROM::save(Settings settings) {
  SettingsEEPROM settingsE;
  settingsE.data = settings;
  settingsE.checksum = settingsE.calcChecksum();
  EEPROM.put(0, settingsE);
}
