#include "Joystick.h"
#include "InputDebounce.h"
#include <arduino-timer.h>

#include "U8glib.h"
U8GLIB_SH1106_128X64 display(U8G_I2C_OPT_NONE);  // I2C / TWI

#define BUTTON_DEBOUNCE_DELAY 50  //[ms]
#define SERIAL_BAUDRATE 9600

const uint8_t buttonCount = 5;
Joystick_ controller(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, buttonCount,
                     0, true, true, false,
                     false, false, false,
                     true, false, false,
                     false, false);

auto timer = timer_create_default();  // create a timer with default settings
boolean screenReady = true;
uint16_t lastAmmoCount = -1;

static InputDebounce btnTrigger;
static InputDebounce btnLeft;
static InputDebounce btnBottom;

const int LITE_PIN = 0;
const int BTN_TRIGGER = 4;
const int BTN_LEFT = 5;
const int BTN_BOTTOM = 6;
const int START_PIN = 7;
const int COIN_PIN = 8;
const int RECOIL_RELAY_PIN = 9;
const int LIGHT_RELAY_PIN = 10;
const int AXIS_X_PIN = A0;
const int AXIS_Y_PIN = A1;

const int buttonPins[buttonCount] = {
  BTN_TRIGGER,
  BTN_LEFT,
  BTN_BOTTOM,
  START_PIN,
  COIN_PIN
};


unsigned long HOLD_MS = 1000;
unsigned long RECOIL_MS = 40;
unsigned long RECOIL_RELEASE_MS = 40;
bool isFiring = false;
bool sendUpdate = false;
int lastXAxisValue = -1;
int lastYAxisValue = -1;
unsigned long lastTriggerRepeat = 0;

int getButtonNumFromPin(int pin) {
  for (int i = 0; i < buttonCount; i++) {
    if (pin == buttonPins[i]) {
      return i;
    }
  }
  return 0;
}

bool setRecoilReleased(void *) {
  isFiring = false;
  return false;
}

bool releaseFire(void *) {
  if (isFiring) {
    digitalWrite(RECOIL_RELAY_PIN, LOW);
    controller.setButton(getButtonNumFromPin(BTN_TRIGGER), LOW);
    sendUpdate = true;
    timer.in(RECOIL_MS, setRecoilReleased);
  }
  return false;
}

void pressFire(bool doRecoil, bool setButton) {
  if (!isFiring) {
    isFiring = true;
    if (doRecoil && controller.hasAmmo()) {
      digitalWrite(RECOIL_RELAY_PIN, HIGH);
    }
    if (setButton) {
      controller.setButton(getButtonNumFromPin(BTN_TRIGGER), HIGH);
    }
    sendUpdate = true;
    timer.in(RECOIL_RELEASE_MS, releaseFire);
  }
}

void pressedCallback(uint8_t pinIn) {
  controller.setButton(getButtonNumFromPin(pinIn), HIGH);
  sendUpdate = true;

  if (pinIn == BTN_TRIGGER && controller.getAutoRecoil()) {
    pressFire(true, true);
  }
}

void releasedCallback(uint8_t pinIn) {
  controller.setButton(getButtonNumFromPin(pinIn), LOW);
  sendUpdate = true;
  lastTriggerRepeat = 0;
}

void pressedDurationCallback(uint8_t pinIn, unsigned long duration) {
  if (pinIn == BTN_TRIGGER && duration >= controller.getTriggerHoldTime() && controller.getTriggerRepeatRate() > 0) {
    long now = millis();
    if (now - lastTriggerRepeat >= controller.getTriggerRepeatRate()) {
      pressFire(controller.getAutoRecoil(), true);
      lastTriggerRepeat = now;
    }
  }
}

void releasedDurationCallback(uint8_t pinIn, unsigned long duration) {
}

bool clearScreen(void *) {
  screenReady = true;
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  Serial.setTimeout(20);

  controller.begin(false);

  controller.loadSettings();

  btnTrigger.registerCallbacks(pressedCallback, releasedCallback, pressedDurationCallback, releasedDurationCallback);
  btnTrigger.setup(BTN_TRIGGER, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);

  btnLeft.registerCallbacks(pressedCallback, releasedCallback, pressedDurationCallback, releasedDurationCallback);
  btnLeft.setup(BTN_LEFT, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);

  btnBottom.registerCallbacks(pressedCallback, releasedCallback, pressedDurationCallback, releasedDurationCallback);
  btnBottom.setup(BTN_BOTTOM, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);

  pinMode(RECOIL_RELAY_PIN, OUTPUT);
  digitalWrite(RECOIL_RELAY_PIN, LOW);

  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  digitalWrite(LIGHT_RELAY_PIN, LOW);

  cli();
  TCCR3A = 0;  //set TCCR1A 0
  TCCR3B = 0;  //set TCCR1B 0
  TCNT3 = 0;   //counter init
  OCR3A = 399;
  TCCR3B |= (1 << WGM32);  //open CTC mode
  TCCR3B |= (1 << CS31);   //set CS11 1(8-fold Prescaler)
  TIMSK3 |= (1 << OCIE3A);
  sei();
}

ISR(TIMER3_COMPA_vect) {
  controller.getUSBPID();
}

void draw(char *str) {
  // graphic commands to redraw the complete screen should be placed here
  display.setFont(u8g_font_helvB24n);
  //display.setPrintPos(30, 27);
  display.setScale2x2();
  //display.print(str);
  display.drawStr180(36, 3, str);
  display.drawBox(50, 0, 20, 3);
}

void loop() {
  sendUpdate = false;
  timer.tick();

  processSerial();

  unsigned long now = millis();
  btnTrigger.process(now);
  btnLeft.process(now);
  btnBottom.process(now);

  const int currentXAxisValue = 1024 - analogRead(AXIS_X_PIN);
  if (abs(currentXAxisValue - lastXAxisValue) > 2) {
    controller.setXAxis(currentXAxisValue);
    lastXAxisValue = currentXAxisValue;
    sendUpdate = true;
  }

  const int currentYAxisValue = 1024 - analogRead(AXIS_Y_PIN);
  if (abs(currentYAxisValue - lastYAxisValue) > 2) {
    controller.setYAxis(currentYAxisValue);
    lastYAxisValue = currentYAxisValue;
    sendUpdate = true;
  }

  if (sendUpdate) {
    controller.sendState();
  }

  if (screenReady) {
    if (lastAmmoCount != controller.getAmmoCount()) {
      char ammo[3];
      sprintf(ammo, "%02d", controller.getAmmoCount());
      // picture loop
      display.firstPage();
      do {
        draw(ammo);
      } while (display.nextPage());
      //display.printFixedN(10, 4, ammo, STYLE_NORMAL, FONT_SIZE_4X);
    }
    lastAmmoCount = controller.getAmmoCount();
  }
}

//Serial port - commands and output.
void processSerial() {

  if (Serial.available()) {
    char cmd[16];
    int16_t arg1 = -32768, arg2 = -32768, arg3 = -32768;
    String line = Serial.readStringUntil('!');
    Serial.readBytes(&cmd[0], 1);  //read the ! or it will loop again
    line.toLowerCase();
    sscanf(line.c_str(), "%s %d %d %d", cmd, &arg1, &arg2, &arg3);

    if (strcmp_P(cmd, PSTR("recoil")) == 0) {
      if (arg1 == 1) {
        pressFire(true, false);
      }
    } else if (strcmp_P(cmd, PSTR("setammocount")) == 0) {
      controller.setAmmoCount(arg1);
      sendUpdate = true;
    }
    //this help match the hid device to com port from host
    else if (strcmp_P(cmd, PSTR("setuniqueid")) == 0) {
      controller.setUniqueId(arg1);
    }
    //this help match the hid device to com port from host
    else if (strcmp_P(cmd, PSTR("getuniqueid")) == 0) {
      Serial.println(controller.getUniqueId());
    }
  }
}
