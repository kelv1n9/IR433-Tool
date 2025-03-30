#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define USE_LGT_EEPROM_API

#include <EncButton.h>
#include <RCSwitch.h>
#include <IRremote.hpp>
#include <EEPROM.h>

#define VIBRO D9

#define RED_PIN D5
#define GREEN_PIN D6
#define BLUE_PIN D13

#define BTN_1 D8
#define BTN_2 D10
#define BTN_3 D11

#define RA_TX D12
#define RA_RX D2
#define IR_TX D7
#define IR_RX D3

#define BAT A0
#define R1 9.74
#define R2 1.003
#define VREF 1.024
#define VOLTAGE_COEF 1.004455
#define VOLTAGE_SAMPLE_COUNT 10

#define MAIN_PAGES_COUNT 2
#define IR_PAGES_COUNT 2
#define RA_PAGES_COUNT 4

#define MAX_IR_SIGNALS 4

Button Btn_ok(BTN_1);
Button Btn_up(BTN_2);
Button Btn_down(BTN_3);

uint8_t page_main = 0;
uint8_t page_RA = 0;
uint8_t page_IR = 0;

float voltage;

enum MenuState {
  MENU_MAIN,
  MENU_RA,
  MENU_IR
};

struct BreathState {
  unsigned long lastTime = 0;
  int brightness = 0;
  int direction = 1;
};

MenuState menuState = MENU_MAIN;

struct VibroState {
  bool active = false;
  uint32_t startTime = 0;
  uint16_t duration = 0;
  uint8_t intensity = 0;
  uint8_t repeatCount = 0;
  uint32_t pauseTime = 150;
  bool waitingPause = false;
};

VibroState vibroState;

/* ============= RA ================== */

bool pageSelected = false;

RCSwitch mySwitch = RCSwitch();

uint16_t sendInterval = 300;

unsigned long capturedCode;
unsigned int capturedLength;
unsigned int capturedProtocol;
unsigned int capturedDelay;

/* ============= IR ================== */

struct SimpleIRData {
  uint8_t protocol;
  uint16_t address;
  uint16_t command;
};

int lastUsedSlot = 0;

/* =================================== */


void vibro(uint8_t intensity = 120, uint16_t duration = 100, uint8_t repeat = 1, uint32_t pause = 150) {
  vibroState.intensity = intensity;
  vibroState.duration = duration;
  vibroState.repeatCount = repeat;
  vibroState.pauseTime = pause;
  vibroState.startTime = millis();
  vibroState.active = true;
  vibroState.waitingPause = false;
  analogWrite(VIBRO, intensity);
}

void updateVibro() {
  if (!vibroState.active) return;

  uint32_t now = millis();

  if (!vibroState.waitingPause) {
    if (now - vibroState.startTime >= vibroState.duration) {
      analogWrite(VIBRO, 0);
      vibroState.waitingPause = true;
      vibroState.startTime = now;
    }
  } else {
    if (now - vibroState.startTime >= vibroState.pauseTime) {
      vibroState.repeatCount--;
      if (vibroState.repeatCount == 0) {
        vibroState.active = false;
      } else {
        vibroState.waitingPause = false;
        vibroState.startTime = now;
        analogWrite(VIBRO, vibroState.intensity);
      }
    }
  }
}


void menuButtons(uint8_t &page, uint8_t MAX_PAGES) {
  bool changed = false;
  if (Btn_up.click()) {
    page = (page + MAX_PAGES - 1) % MAX_PAGES;
    changed = true;
  }
  if (Btn_down.click()) {
    page = (page + 1) % MAX_PAGES;
    changed = true;
  }
  if (changed) {
    vibro(150, 100, page + 1, 120);
  }
}

void setColor(int r, int g, int b) {
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void breathe(BreathState &state, uint32_t now, uint8_t r, uint8_t g, uint8_t b, uint16_t delayMs = 10, uint8_t step = 5) {
  if (now - state.lastTime > delayMs) {
    state.brightness += state.direction * step;

    if (state.brightness <= 0 || state.brightness >= 255) {
      state.direction = -state.direction;
      state.brightness = constrain(state.brightness, 0, 255);
    }

    setColor(
      (r * state.brightness) / 255,
      (g * state.brightness) / 255,
      (b * state.brightness) / 255);

    state.lastTime = now;
  }
}

float readVoltage() {
  int raw = analogRead(BAT);
  if (raw < 0 || raw > 4095) return 0;
  float vReal = VOLTAGE_COEF * (((float)raw / 4095.0) * VREF) * (1 + (R1 / R2));
  if (Serial) {
    Serial.print(vReal);
    Serial.println(" V");
  }
  return vReal;
}

void setColorGreen() {
  setColor(0, 255, 0);
}
void setColorRed() {
  setColor(255, 0, 0);
}
void setColorBlue() {
  setColor(0, 0, 255);
}
void setColorOrange() {
  setColor(255, 150, 0);
}
void setColorOff() {
  setColor(0, 0, 0);
}

void checkVoltage() {
  float voltage = readVoltage();
  if (voltage >= 3.8) {
    setColorGreen();
  } else if (voltage >= 3.5) {
    setColorOrange();
  } else {
    setColorRed();
  }
}

void writeIRData(uint8_t protocol, uint16_t address, uint16_t command, uint8_t slot) {
  uint8_t addr = slot * 5 + 5;
  EEPROM.write(addr++, protocol);
  EEPROM.write(addr++, address & 0xFF);        
  EEPROM.write(addr++, (address >> 8) & 0xFF); 
  EEPROM.write(addr++, command & 0xFF);       
  EEPROM.write(addr++, (command >> 8) & 0xFF); 
}

SimpleIRData readIRData(uint8_t slot) {
  uint8_t addr = slot * 5 + 5;
  SimpleIRData data;
  data.protocol = EEPROM.read(addr++);
  uint8_t addrLow = EEPROM.read(addr++);
  uint8_t addrHigh = EEPROM.read(addr++);
  data.address = (addrHigh << 8) | addrLow;
  uint8_t cmdLow = EEPROM.read(addr++);
  uint8_t cmdHigh = EEPROM.read(addr++);
  data.command = (cmdHigh << 8) | cmdLow;

  return data;
}

void getVoltageColor(float voltage, uint8_t &r, uint8_t &g, uint8_t &b) {
  if (voltage >= 3.8) {
    r = 0;
    g = 255;
    b = 0;
  } else if (voltage >= 3.5) {
    r = 255;
    g = 150;
    b = 0;
  } else {
    r = 255;
    g = 0;
    b = 0;
  }
}

#endif