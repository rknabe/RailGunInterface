#include <Joystick.h>
#include "InputDebounce.h"

#define BUTTON_DEBOUNCE_DELAY 45  //[ms]
#define RECOIL_RELEASE_MS 50      //[ms]

const uint8_t buttonCount = 7;
Joystick_ controller(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, buttonCount,
                     0, true, true, false,
                     false, false, false,
                     false, false, false,
                     false, false);

static InputDebounce btnTrigger;  // not enabled yet, setup has to be called first, see setup() below

const int BTN_TRIGGER = 2;
const int BTN_LEFT = 3;
const int BTN_BOTTOM = 4;
const int BTN_D_PIN = 5;
const int BTN_E_PIN = 6;
const int BTN_F_PIN = 7;
const int BTN_K_PIN = 8;
const int RECOIL_RELAY_PIN = 9;
const int LIGHT_RELAY_PIN = 10;
const int AXIS_X_PIN = A0;
const int AXIS_Y_PIN = A1;

#define SERIAL_BAUDRATE 9600

const int buttonPins[buttonCount] = {
  BTN_TRIGGER,
  BTN_LEFT,
  BTN_BOTTOM,
  BTN_D_PIN,
  BTN_E_PIN,
  BTN_F_PIN,
  BTN_K_PIN
};

unsigned long HOLD_MS = 1000;
unsigned long RECOIL_REPEAT_MS = 100;
unsigned long lastHoldTimeMs = 0;
int lastXAxisValue = -1;
int lastYAxisValue = -1;

int getButtonNumFromPin(int pin) {
  for (int i = 0; i < buttonCount; i++) {
    if (pin == buttonPins[i]) {
      return i;
    }
  }
  return 0;
}

void engageRecoil() {
  digitalWrite(RECOIL_RELAY_PIN, HIGH);
  delay(RECOIL_RELEASE_MS);
  digitalWrite(RECOIL_RELAY_PIN, LOW);
}

void pressedCallback(uint8_t pinIn) {
  controller.setButton(getButtonNumFromPin(pinIn), HIGH);

  if (pinIn == BTN_TRIGGER) {
    engageRecoil();
  }
}

void releasedCallback(uint8_t pinIn) {
  controller.setButton(getButtonNumFromPin(pinIn), LOW);
  lastHoldTimeMs = 0;
}

void pressedDurationCallback(uint8_t pinIn, unsigned long duration) {
  if (pinIn == BTN_TRIGGER && duration > HOLD_MS) {
    if (millis() - lastHoldTimeMs > RECOIL_REPEAT_MS) {
      controller.setButton(getButtonNumFromPin(pinIn), HIGH);
      engageRecoil();
      controller.setButton(getButtonNumFromPin(BTN_TRIGGER), LOW);
      lastHoldTimeMs = millis();
    }
  }
}

void releasedDurationCallback(uint8_t pinIn, unsigned long duration) {
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);

  controller.setYAxisRange(0, 1023);
  controller.setYAxisRange(0, 1023);
  controller.begin();

  btnTrigger.registerCallbacks(pressedCallback, releasedCallback, pressedDurationCallback, releasedDurationCallback);
  btnTrigger.setup(BTN_TRIGGER, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);

  pinMode(RECOIL_RELAY_PIN, OUTPUT);
  digitalWrite(RECOIL_RELAY_PIN, LOW);

  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  digitalWrite(LIGHT_RELAY_PIN, LOW);
}

void loop() {
  //bool sendUpdate = false;
  unsigned long now = millis();
  btnTrigger.process(now);

  const int currentXAxisValue = analogRead(AXIS_X_PIN);
  if (currentXAxisValue != lastXAxisValue) {
    controller.setXAxis(currentXAxisValue);
    lastXAxisValue = currentXAxisValue;
    //sendUpdate = true;
  }

  const int currentYAxisValue = analogRead(AXIS_Y_PIN);
  if (currentYAxisValue != lastYAxisValue) {
    controller.setYAxis(currentYAxisValue);
    lastYAxisValue = currentYAxisValue;
    //sendUpdate = true;
  }

  //if (sendUpdate) {
  //  controller.sendState();
  //}

  //failsafe to release coil
  digitalWrite(RECOIL_RELAY_PIN, LOW);
}
