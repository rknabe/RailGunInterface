#include <stdint.h>
/*
  Joystick.h

  Copyright (c) 2015-2017, Matthew Heironimus

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef JOYSTICK_h
#define JOYSTICK_h

#include "DynamicHID.h"
#include "Settings.h"

#if ARDUINO < 10606
#error The Joystick library requires Arduino IDE 1.6.6 or greater. Please update your IDE.
#endif  // ARDUINO < 10606

#if ARDUINO > 10606
#if !defined(USBCON)
#error The Joystick library can only be used with a USB MCU (e.g. Arduino Leonardo, Arduino Micro, etc.).
#endif  // !defined(USBCON)
#endif  // ARDUINO > 10606

#if !defined(_USING_DYNAMIC_HID)

#warning "Using legacy HID core (non pluggable)"

#else  // !defined(_USING_DYNAMIC_HID)

//================================================================================
//  Joystick (Gamepad)

#define JOYSTICK_DEFAULT_REPORT_ID 0x01
#define JOYSTICK_DEFAULT_BUTTON_COUNT 32
#define JOYSTICK_DEFAULT_SIMULATOR_MINIMUM 0
#define JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM 1023
#define JOYSTICK_DEFAULT_HATSWITCH_COUNT 2
#define JOYSTICK_HATSWITCH_COUNT_MAXIMUM 2
#define JOYSTICK_HATSWITCH_RELEASE -1
#define JOYSTICK_TYPE_JOYSTICK 0x04
#define JOYSTICK_TYPE_GAMEPAD 0x05
#define JOYSTICK_TYPE_MULTI_AXIS 0x08

#define DIRECTION_ENABLE 0x04
#define X_AXIS_ENABLE 0x01
#define Y_AXIS_ENABLE 0x02

void pressFire(bool doRecoil, bool setButton);

class Joystick_ {
private:

  // Joystick State
  int16_t _xAxis;
  int16_t _yAxis;
  int16_t _zAxis;
  int16_t _xAxisRotation;
  int16_t _yAxisRotation;
  int16_t _zAxisRotation;
  int16_t _throttle;
  int16_t _rudder;
  int16_t _accelerator;
  int16_t _brake;
  int16_t _steering;
  int16_t _hatSwitchValues[JOYSTICK_HATSWITCH_COUNT_MAXIMUM];
  uint8_t* _buttonValues = NULL;

  // Joystick Settings
  bool _autoSendState;
  uint8_t _buttonCount;
  uint8_t _buttonValuesArraySize = 0;
  uint8_t _hatSwitchCount = 0;
  uint8_t _includeAxisFlags;
  uint8_t _includeSimulatorFlags;
  bool autoRecoil = true;
  int16_t ammoCount = 0;
  int16_t health = 0;
  int16_t maxHealth = 1000;
  bool useAmmoCount = false;
  uint16_t uniqueId = 0;
  int16_t triggerRepeatRate = 100;
  int16_t triggerHoldTime = 500;
  int16_t _xAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;  //14;
  int16_t _xAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;  //932;
  int16_t _yAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;  //91;
  int16_t _yAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;  //955;
  int16_t _zAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
  int16_t _zAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
  int16_t _rxAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
  int16_t _rxAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
  int16_t _ryAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
  int16_t _ryAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
  int16_t _rzAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
  int16_t _rzAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
  int16_t _rudderMinimum = JOYSTICK_DEFAULT_SIMULATOR_MINIMUM;
  int16_t _rudderMaximum = JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM;
  int16_t _throttleMinimum = JOYSTICK_DEFAULT_SIMULATOR_MINIMUM;
  int16_t _throttleMaximum = JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM;
  int16_t _acceleratorMinimum = JOYSTICK_DEFAULT_SIMULATOR_MINIMUM;
  int16_t _acceleratorMaximum = JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM;
  int16_t _brakeMinimum = JOYSTICK_DEFAULT_SIMULATOR_MINIMUM;
  int16_t _brakeMaximum = JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM;
  int16_t _steeringMinimum = JOYSTICK_DEFAULT_SIMULATOR_MINIMUM;
  int16_t _steeringMaximum = JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM;

  uint8_t _hidReportId;
  uint8_t _hidReportSize;

  GUI_Report USB_GUI_Report;
  SettingsEEPROM eeprom;

protected:
  int set16BitValue(int16_t value, uint8_t dataLocation[]);
  int setBoolValue(bool value, uint8_t dataLocation[]);
  int normalize(int16_t value, int16_t valueMinimum, int16_t valueMaximum, int16_t actualMinimum, int16_t actualMaximum);
  int buildAndSet16BitValue(bool includeValue, int16_t value, int16_t valueMinimum, int16_t valueMaximum, int16_t actualMinimum, int16_t actualMaximum, uint8_t dataLocation[]);
  int buildAndSetAxisValue(bool includeAxis, int16_t axisValue, int16_t axisMinimum, int16_t axisMaximum, uint8_t dataLocation[]);
  int buildAndSetSimulationValue(bool includeValue, int16_t value, int16_t valueMinimum, int16_t valueMaximum, uint8_t dataLocation[]);

public:
  Joystick_(
    uint8_t hidReportId = JOYSTICK_DEFAULT_REPORT_ID,
    uint8_t joystickType = JOYSTICK_TYPE_JOYSTICK,
    uint8_t buttonCount = JOYSTICK_DEFAULT_BUTTON_COUNT,
    uint8_t hatSwitchCount = 0,
    bool includeXAxis = true,
    bool includeYAxis = true,
    bool includeZAxis = false,
    bool includeRxAxis = false,
    bool includeRyAxis = false,
    bool includeRzAxis = false,
    bool includeRudder = false,
    bool includeThrottle = false,
    bool includeAccelerator = false,
    bool includeBrake = false,
    bool includeSteering = false);

  void begin(bool initAutoSendState = true);
  void end();

  // Set Range Functions
  inline void setXAxisRange(int16_t minimum, int16_t maximum) {
    _xAxisMinimum = minimum;
    _xAxisMaximum = maximum;
  }
  inline void setYAxisRange(int16_t minimum, int16_t maximum) {
    _yAxisMinimum = minimum;
    _yAxisMaximum = maximum;
  }
  inline void setZAxisRange(int16_t minimum, int16_t maximum) {
    _zAxisMinimum = minimum;
    _zAxisMaximum = maximum;
  }
  inline void setRxAxisRange(int16_t minimum, int16_t maximum) {
    _rxAxisMinimum = minimum;
    _rxAxisMaximum = maximum;
  }
  inline void setRyAxisRange(int16_t minimum, int16_t maximum) {
    _ryAxisMinimum = minimum;
    _ryAxisMaximum = maximum;
  }
  inline void setRzAxisRange(int16_t minimum, int16_t maximum) {
    _rzAxisMinimum = minimum;
    _rzAxisMaximum = maximum;
  }
  inline void setRudderRange(int16_t minimum, int16_t maximum) {
    _rudderMinimum = minimum;
    _rudderMaximum = maximum;
  }
  inline void setThrottleRange(int16_t minimum, int16_t maximum) {
    _throttleMinimum = minimum;
    _throttleMaximum = maximum;
  }
  inline void setAcceleratorRange(int16_t minimum, int16_t maximum) {
    _acceleratorMinimum = minimum;
    _acceleratorMaximum = maximum;
  }
  inline void setBrakeRange(int16_t minimum, int16_t maximum) {
    _brakeMinimum = minimum;
    _brakeMaximum = maximum;
  }
  inline void setSteeringRange(int16_t minimum, int16_t maximum) {
    _steeringMinimum = minimum;
    _steeringMaximum = maximum;
  }

  // Set Axis Values
  void setXAxis(int16_t value);
  void setYAxis(int16_t value);
  void setZAxis(int16_t value);
  void setRxAxis(int16_t value);
  void setRyAxis(int16_t value);
  void setRzAxis(int16_t value);

  // Set Simuation Values
  void setRudder(int16_t value);
  void setThrottle(int16_t value);
  void setAccelerator(int16_t value);
  void setBrake(int16_t value);
  void setSteering(int16_t value);

  void setButton(uint8_t button, uint8_t value);
  void pressButton(uint8_t button);
  void releaseButton(uint8_t button);
  void setHatSwitch(int8_t hatSwitch, int16_t value);

  void sendState();
  void sendGuiReport(void* data);
  // get USB PID data
  void getUSBPID();

  void processUsbCmd();
  void loadSettings();
  void saveSettings();
  void loadDefaultSettings();
  void loadSettings(Settings settings);
  bool getAutoRecoil();
  void setAutoRecoil(bool value);
  uint16_t getTriggerRepeatRate();
  void setTriggerRepeatRate(uint16_t value);
  uint16_t getTriggerHoldTime();
  void setTriggerHoldTime(uint16_t value);
  int16_t getAmmoCount();
  void setAmmoCount(int16_t value);
  int16_t getHealth();
  void setHealth(int16_t value);
  int16_t getMaxHealth();
  void setMaxHealth(int16_t value);
  bool hasAmmo();
  bool getUseAmmoCount();
  void setUseAmmoCount(bool flag);
  void setUniqueId(uint16_t);
  uint16_t getUniqueId();
};

#endif  // !defined(_USING_DYNAMIC_HID)
#endif  // JOYSTICK_h
