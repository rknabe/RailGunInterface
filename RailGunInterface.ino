#include "Joystick.h"
#include "InputDebounce.h"
#include <arduino-timer.h>
#include "U8glib.h"

U8GLIB_SH1106_128X64 display(U8G_I2C_OPT_DEV_0);

const uint8_t buttonCount = 5;
Joystick_ controller(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, buttonCount,
                     0, true, true, false,
                     false, false, false,
                     true, false, false,
                     false, false);

auto timer = timer_create_default();  // create a timer with default settings

static InputDebounce btnTrigger;
static InputDebounce btnLeft;
static InputDebounce btnBottom;
static InputDebounce btnStart;
static InputDebounce btnCoin;

const int buttonPins[buttonCount] = {
  BTN_TRIGGER,
  BTN_LEFT,
  BTN_BOTTOM,
  BTN_START,
  BTN_COIN
};

bool isFiring = false;
bool sendUpdate = false;
boolean screenReady = false;
int lastXAxisValue = -1;
int lastYAxisValue = -1;
unsigned long lastTriggerRepeat = 0;
uint16_t lastAmmoCount = -1;
uint16_t lastHealth = 0;

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

  btnStart.registerCallbacks(pressedCallback, releasedCallback, pressedDurationCallback, releasedDurationCallback);
  btnStart.setup(BTN_START, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);

  btnCoin.registerCallbacks(pressedCallback, releasedCallback, pressedDurationCallback, releasedDurationCallback);
  btnCoin.setup(BTN_COIN, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);

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

  display.firstPage();
  do {
    display.drawXBMP(32, 20, logo_width, logo_height, logo);
  } while (display.nextPage());
  timer.in(2000, clearDisplay);

  display.setFont(u8g_font_fub49n);
}

ISR(TIMER3_COMPA_vect) {
  controller.getUSBPID();
}

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
    digitalWrite(LIGHT_RELAY_PIN, LOW);
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
      digitalWrite(LIGHT_RELAY_PIN, HIGH);
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

bool clearDisplay(void *) {
  screenReady = true;
}

void updateDisplayStats() {
  if (screenReady) {
    if (lastAmmoCount != controller.getAmmoCount() || lastHealth != controller.getHealth()) {
      char ammoStr[3];
      sprintf(ammoStr, "%02d", controller.getAmmoCount());
      int16_t pct = 0.50 * min(((float)controller.getHealth() / (float)controller.getMaxHealth()) * 100.00, 100.0);  //max height 64 at 100%
      pct = max(pct, 1);                                                                                             //min of 1
      // picture loop
      display.firstPage();
      do {
        display.drawXBMP(18, 54, bullet_width, bullet_height, bullet);
        display.drawStr180(80, 0, ammoStr);
        display.drawXBMP(88, 0, health_width, health_height, health);
        display.drawBox(93, 4, 22, pct);
      } while (display.nextPage());
    }
    lastAmmoCount = controller.getAmmoCount();
    lastHealth = controller.getHealth();
  }
}

void loop() {
  sendUpdate = false;
  timer.tick();

  processSerial();

  unsigned long now = millis();
  btnTrigger.process(now);
  btnLeft.process(now);
  btnBottom.process(now);
  btnStart.process(now);
  btnCoin.process(now);

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

  updateDisplayStats();
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
    } else if (strcmp_P(cmd, PSTR("useammocount")) == 0) {
      controller.setUseAmmoCount(arg1 > 0);
      sendUpdate = true;
    } else if (strcmp_P(cmd, PSTR("sethealth")) == 0) {
      controller.setHealth(arg1);
      sendUpdate = true;
    } else if (strcmp_P(cmd, PSTR("setmaxhealth")) == 0) {
      controller.setMaxHealth(arg1);
      sendUpdate = true;
    } else if (strcmp_P(cmd, PSTR("setuniqueid")) == 0) {
      //this help match the hid device to com port from host
      controller.setUniqueId(arg1);
    } else if (strcmp_P(cmd, PSTR("getuniqueid")) == 0) {
      //this help match the hid device to com port from host
      Serial.println(controller.getUniqueId());
    }
  }
}
