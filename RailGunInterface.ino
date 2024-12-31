#include "Joystick.h"
#include "InputDebounce.h"
#include <arduino-timer.h>
#include "U8glib.h"
#include "avdweb_AnalogReadFast.h"
#include <digitalWriteFast.h>

#define OLED_CS 11
#define OLED_DC 12
#define OLED_RST 13
U8GLIB_SSD1309_128X64 display(OLED_CS, OLED_DC, OLED_RST);

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
int16_t lastAmmoCount = -1;
int16_t lastHealth = 0;
int8_t lastHealthPct = 0;

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
  digitalWriteFast(RECOIL_RELAY_PIN, LOW);

  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  digitalWriteFast(LIGHT_RELAY_PIN, LOW);

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
    display.drawXBMP(34, 18, logo_width, logo_height, logo);
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
    digitalWriteFast(RECOIL_RELAY_PIN, LOW);
    controller.setButton(getButtonNumFromPin(BTN_TRIGGER), LOW);
    digitalWriteFast(LIGHT_RELAY_PIN, LOW);
    sendUpdate = true;
    timer.in(RECOIL_MS, setRecoilReleased);
  }
  return false;
}

void pressFire(bool doRecoil, bool setButton) {
  if (!isFiring) {
    isFiring = true;
    if (doRecoil && controller.hasAmmo()) {
      digitalWriteFast(RECOIL_RELAY_PIN, HIGH);
    }
    if (setButton) {
      controller.setButton(getButtonNumFromPin(BTN_TRIGGER), HIGH);
      digitalWriteFast(LIGHT_RELAY_PIN, HIGH);
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
  return true;
}

void updateDisplayStats() {
  if (screenReady) {
    int8_t healthPct = round(min(((float)controller.getHealth() / (float)controller.getMaxHealth()) * (float)6.0, 6.0));
    if (controller.getHealth() > 4 && healthPct == 0) {
      healthPct = 1;
    }
    if (lastAmmoCount != controller.getAmmoCount() || lastHealthPct != healthPct) {
      screenReady = false;
      unsigned long now = millis();
      lastAmmoCount = controller.getAmmoCount();
      lastHealth = controller.getHealth();
      lastHealthPct = healthPct;
      display.firstPage();
      do {
        for (int8_t i = 0; i < healthPct; i++) {
          display.setPrintPos(6, 77 - (11 * i));
          display.print("-");
        }
        if (lastAmmoCount > 99) {
          display.setPrintPos(40, 50);
          display.setFont(u8g_font_fub35n);
          display.print(lastAmmoCount);
          display.setFont(u8g_font_fub49n);
        } else {
          if (lastAmmoCount < 10) {
            display.setPrintPos(70, 63);
          } else {
            display.setPrintPos(50, 63);
          }
          display.print(lastAmmoCount);
        }
        display.drawXBMP(67, 0, bullet_width, bullet_height, bullet);
      } while (display.nextPage());
      Serial.println(millis() - now);
      screenReady = true;
    }
  }  
}

void loop() {
  //unsigned long start = micros();
  sendUpdate = false;
  timer.tick();

  processSerial();

  unsigned long now = millis();
  btnTrigger.process(now);
  btnLeft.process(now);
  btnBottom.process(now);
  btnStart.process(now);
  btnCoin.process(now);

  const int currentXAxisValue = 1024 - analogReadFast(AXIS_X_PIN);
  if (abs(currentXAxisValue - lastXAxisValue) > 2) {
    controller.setXAxis(currentXAxisValue);
    lastXAxisValue = currentXAxisValue;
    sendUpdate = true;
  }

  const int currentYAxisValue = 1024 - analogReadFast(AXIS_Y_PIN);
  if (abs(currentYAxisValue - lastYAxisValue) > 2) {
    controller.setYAxis(currentYAxisValue);
    lastYAxisValue = currentYAxisValue;
    sendUpdate = true;
  }

  if (sendUpdate) {
    controller.sendState();
  }

  updateDisplayStats();

  //Serial.println(micros() - start);
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
