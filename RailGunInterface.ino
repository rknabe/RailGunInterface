#include <Joystick.h>

const uint8_t buttonCount = 7;
Joystick_ controller(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, buttonCount,
                     0, true, true, false,
                     false, false, false,
                     false, false, false,
                     false, false);

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

#define SERIAL_BAUDRATE 2000000
const int BTN_PRESSED = 0;
const int BTN_RELEASED = 1;

const int buttonPins[buttonCount] = {
  BTN_TRIGGER,
  BTN_LEFT,
  BTN_BOTTOM,
  BTN_D_PIN,
  BTN_E_PIN,
  BTN_F_PIN,
  BTN_K_PIN
};
int lastButtonValue[buttonCount];
unsigned long triggerHeldTime = 0;
unsigned long HOLD_MS = 1100;
int lastXAxisValue = -1;
int lastYAxisValue = -1;

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  Serial.setTimeout(50);
  triggerHeldTime = 0;

  controller.setYAxisRange(0, 1023);
  controller.setYAxisRange(0, 1023);
  controller.begin(false);

  for (int i = 0; i < buttonCount; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    lastButtonValue[i] = -1;
  }

  pinMode(RECOIL_RELAY_PIN, OUTPUT);
  digitalWrite(RECOIL_RELAY_PIN, LOW);

  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  digitalWrite(LIGHT_RELAY_PIN, LOW);
}

void loop() {
  bool sendUpdate = false;
  bool buttonChanged = false;
  digitalWrite(RECOIL_RELAY_PIN, LOW);

  for (int i = 0; i < buttonCount; i++) {
    const int buttonValue = digitalRead(buttonPins[i]);

    if (buttonValue != lastButtonValue[i]) {
      controller.setButton(i, !buttonValue);
      lastButtonValue[i] = buttonValue;
      sendUpdate = true;
      buttonChanged = true;
    }

    if (buttonPins[i] == BTN_TRIGGER) {
      if (buttonValue == BTN_PRESSED) {
        if (buttonChanged) {
          digitalWrite(RECOIL_RELAY_PIN, HIGH);
          Serial.println("Trigger pressed");
          triggerHeldTime = millis();
        } else {
          if (millis() - triggerHeldTime >= HOLD_MS) {
            controller.setButton(i, BTN_PRESSED);
            digitalWrite(RECOIL_RELAY_PIN, HIGH);
            Serial.println("Trigger held");
            sendUpdate = true;
          } else {
            // Serial.println("Trigger held < 2 secs");
          }
        }
        delay(50);
        digitalWrite(RECOIL_RELAY_PIN, LOW);
      } else {
        //digitalWrite(RECOIL_RELAY_PIN, LOW);
        triggerHeldTime = 0;
      }
    }
    //digitalWrite(RECOIL_RELAY_PIN, LOW);
  }

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

  delay(50);
  //digitalWrite(RECOIL_RELAY_PIN, LOW);
}
