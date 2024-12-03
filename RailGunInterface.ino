#include "RailGun.h"
#include "InputDebounce.h"
#include <arduino-timer.h>

#define BUTTON_DEBOUNCE_DELAY 50  //[ms]

const uint8_t buttonCount = 7;
RailGun controller(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, buttonCount,
                      0, true, true, false,
                      false, false, false,
                      false, false, false,
                      false, false);

auto timer = timer_create_default();  // create a timer with default settings

static InputDebounce btnTrigger;  // not enabled yet, setup has to be called first, see setup() below

const int LITE_PIN = 0;
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
unsigned long RECOIL_MS = 62;
bool recoilEngaged = false;
bool sendUpdate = false;
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

bool setRecoilReleased(void *) {
  recoilEngaged = false;
  return false;
}

bool releaseRecoil(void *) {
  if (recoilEngaged) {
    digitalWrite(RECOIL_RELAY_PIN, LOW);
    controller.setButton(getButtonNumFromPin(BTN_TRIGGER), LOW);
    sendUpdate = true;
    timer.in(RECOIL_MS, setRecoilReleased);
  }
  return false;
}

void engageRecoil() {
  if (!recoilEngaged) {
    recoilEngaged = true;
    digitalWrite(RECOIL_RELAY_PIN, HIGH);
    controller.setButton(getButtonNumFromPin(BTN_TRIGGER), HIGH);
    sendUpdate = true;
    timer.in(RECOIL_MS, releaseRecoil);
  }
}

void pressedCallback(uint8_t pinIn) {
  controller.setButton(getButtonNumFromPin(pinIn), HIGH);
  sendUpdate = true;

  if (pinIn == BTN_TRIGGER) {
    engageRecoil();
  }
}

void releasedCallback(uint8_t pinIn) {
  controller.setButton(getButtonNumFromPin(pinIn), LOW);
  sendUpdate = true;
}

void pressedDurationCallback(uint8_t pinIn, unsigned long duration) {
  if (pinIn == BTN_TRIGGER && duration > HOLD_MS) {
    engageRecoil();
  }
}

void releasedDurationCallback(uint8_t pinIn, unsigned long duration) {
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);

  controller.setYAxisRange(0, 1023);
  controller.setYAxisRange(0, 1023);
  controller.begin(false);

  btnTrigger.registerCallbacks(pressedCallback, releasedCallback, pressedDurationCallback, releasedDurationCallback);
  btnTrigger.setup(BTN_TRIGGER, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);

  pinMode(RECOIL_RELAY_PIN, OUTPUT);
  digitalWrite(RECOIL_RELAY_PIN, LOW);

  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  digitalWrite(LIGHT_RELAY_PIN, LOW);
}

void loop() {
  sendUpdate = false;
  timer.tick();

  unsigned long now = millis();
  btnTrigger.process(now);

  const int currentXAxisValue = analogRead(AXIS_X_PIN);
  if (currentXAxisValue != lastXAxisValue) {
    controller.setXAxis(currentXAxisValue);
    lastXAxisValue = currentXAxisValue;
    sendUpdate = true;
  }

  const int currentYAxisValue = analogRead(AXIS_Y_PIN);
  if (currentYAxisValue != lastYAxisValue) {
    controller.setYAxis(currentYAxisValue);
    lastYAxisValue = currentYAxisValue;
    sendUpdate = true;
  }

  if (sendUpdate) {
    controller.sendState();
  }
}
