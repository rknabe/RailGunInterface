#include <Joystick.h>
#include "InputDebounce.h"
#include <arduino-timer.h>

#define BUTTON_DEBOUNCE_DELAY 50  //[ms]

const uint8_t buttonCount = 7;
Joystick_ controller(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, buttonCount,
                     0, true, true, false,
                     false, false, false,
                     false, false, false,
                     false, false);

auto timer = timer_create_default();  // create a timer with default settings

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

unsigned long HOLD_MS = 1100;
unsigned long RECOIL_MS = 50;
bool recoilEngaged = false;
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

bool releaseRecoil(void *) {
  if (recoilEngaged) {
    digitalWrite(RECOIL_RELAY_PIN, LOW);
    controller.setButton(BTN_TRIGGER, LOW);
    recoilEngaged = false;
    Serial.println("recoil released ");
  }
  return false;
}

void engageRecoil() {
  if (!recoilEngaged) {
    digitalWrite(RECOIL_RELAY_PIN, HIGH);
    recoilEngaged = true;
    Serial.println("recoil engaged ");
    timer.in(RECOIL_MS, releaseRecoil);
  }
}

void pressedCallback(uint8_t pinIn) {
  // handle pressed state
  Serial.print("HIGH pin: ");
  Serial.println(pinIn);

  Serial.print("btn #: ");
  Serial.println(getButtonNumFromPin(pinIn));
  controller.setButton(getButtonNumFromPin(pinIn), HIGH);

  if (pinIn == BTN_TRIGGER && !recoilEngaged) {
    engageRecoil();
    //delay(50);
    //digitalWrite(RECOIL_RELAY_PIN, LOW);
  }
}

void releasedCallback(uint8_t pinIn) {
  // handle released state
  Serial.print("LOW (pin: ");
  Serial.print(pinIn);
  Serial.println(")");

  Serial.print("btn #: ");
  Serial.println(getButtonNumFromPin(pinIn));

  controller.setButton(getButtonNumFromPin(pinIn), LOW);
}

void pressedDurationCallback(uint8_t pinIn, unsigned long duration) {
  // handle still pressed state
  /*Serial.print("HIGH (pin: ");
  Serial.print(pinIn);
  Serial.print(") still pressed, duration ");
  Serial.print(duration);
  Serial.println("ms");*/

  if (pinIn == BTN_TRIGGER && duration > HOLD_MS && !recoilEngaged) {
    Serial.println("Hold trigger");
    controller.setButton(getButtonNumFromPin(pinIn), HIGH);
    engageRecoil();
    //delay(100);
    // controller.setButton(getButtonNumFromPin(pinIn), LOW);
    //digitalWrite(RECOIL_RELAY_PIN, LOW);
  }
}

void releasedDurationCallback(uint8_t pinIn, unsigned long duration) {
  // handle released state
  /*Serial.print("LOW (pin: ");
  Serial.print(pinIn);
  Serial.print("), duration ");
  Serial.print(duration);
  Serial.println("ms");*/
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
  bool sendUpdate = false;
  //digitalWrite(RECOIL_RELAY_PIN, LOW);
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

  delay(10);
  //digitalWrite(RECOIL_RELAY_PIN, LOW);
}
