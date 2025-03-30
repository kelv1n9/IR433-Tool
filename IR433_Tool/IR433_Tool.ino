#include "functions.h"

void setup() {
  analogReference(INTERNAL1V024);
  analogReadResolution(12);

  pinMode(VIBRO, OUTPUT);
  analogWrite(VIBRO, 0);
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(RA_RX, INPUT_PULLUP);
  setColorOff();

  IrReceiver.begin(IR_RX, ENABLE_LED_FEEDBACK);
  IrSender.begin(IR_TX);

  voltage = readVoltage();
}

bool initialized = false;

void loop() {
  Btn_ok.tick();
  Btn_up.tick();
  Btn_down.tick();
  updateVibro();

  static BreathState breah;
  unsigned long now = millis();
  uint8_t r = 0, g = 0, b = 0;

  switch (menuState) {
    case MENU_MAIN:
      {
        menuButtons(page_main, MAIN_PAGES_COUNT);
        getVoltageColor(voltage, r, g, b);
        breathe(breah, now, r, g, b);

        if (Btn_ok.click()) {
          switch (page_main) {
            case 0: menuState = MENU_RA; break;
            case 1: menuState = MENU_IR; break;
          }
          vibro(120, 250);
        }
        break;
      }
    /* ==================== RA MENU ==================== */
    case MENU_RA:
      {
        if (!pageSelected) {
          breathe(breah, now, 255, 255, 0);
          menuButtons(page_RA, RA_PAGES_COUNT);
          if (Btn_ok.click()) {
            pageSelected = true;
            vibro(120, 250);
          }
          if (Btn_ok.hold()) {
            menuState = MENU_MAIN;
            vibro(120, 400);
          }
        }

        if (pageSelected) {
          if (Btn_ok.hold()) {
            menuState = MENU_RA;
            pageSelected = false;
            vibro(120, 400);
            initialized = false;
          }
          switch (page_RA) {
            // Scan
            case 0:
              {
                if (!initialized) {
                  mySwitch.disableTransmit();
                  mySwitch.enableReceive(0);
                  initialized = true;
                }

                if (mySwitch.available()) {
                  capturedCode = mySwitch.getReceivedValue();
                  capturedLength = mySwitch.getReceivedBitlength();
                  capturedProtocol = mySwitch.getReceivedProtocol();
                  capturedDelay = mySwitch.getReceivedDelay();

                  mySwitch.resetAvailable();

                  vibro(150, 100, 3, 150);
                  breathe(breah, now, 0, 255, 0, 10, 50);
                } else {
                  breathe(breah, now, 255, 255, 0, 10, 30);
                }
                break;
              }
            // Attack
            case 1:
              {
                static unsigned long lastSend = 0;
                if (capturedCode == 0) {
                  breathe(breah, now, 255, 0, 0);
                  break;
                }

                if (!initialized) {
                  mySwitch.disableReceive();
                  mySwitch.enableTransmit(RA_TX);
                  mySwitch.setRepeatTransmit(5);
                  mySwitch.setProtocol(capturedProtocol);
                  mySwitch.setPulseLength(capturedDelay);
                  initialized = true;
                }

                if (now - lastSend >= sendInterval) {
                  breathe(breah, now, 255, 0, 0, 10, 30);
                  mySwitch.send(capturedCode, capturedLength);
                  lastSend = now;
                }
                break;
              }
            // Noise
            case 2:
              {
                if (!initialized) {
                  mySwitch.disableReceive();
                  mySwitch.disableTransmit();
                  pinMode(RA_TX, OUTPUT);
                  initialized = true;
                }

                static unsigned long lastNoise = 0;
                static bool noiseState = false;
                unsigned long nowMicros = micros();

                breathe(breah, now, 255, 0, 0, 10, 50);

                if (nowMicros - lastNoise > 500) {
                  noiseState = !noiseState;
                  digitalWrite(RA_TX, noiseState);
                  lastNoise = nowMicros;
                }
                break;
              }
            // Selftest
            case 3:
              {
                static unsigned long lastToggle = 0;
                static bool txState = false;
                static unsigned long lastCheck = 0;
                static unsigned long lastReport = 0;

                const unsigned long toggleInterval = 100;
                const unsigned long checkInterval = 100;
                const unsigned long reportInterval = 1000;

                static int matchCount = 0;
                static int totalChecks = 0;

                if (!initialized) {
                  mySwitch.disableReceive();
                  mySwitch.disableTransmit();
                  pinMode(RA_TX, OUTPUT);
                  pinMode(RA_RX, INPUT_PULLUP);
                  initialized = true;
                }

                if (now - lastToggle >= toggleInterval) {
                  txState = !txState;
                  digitalWrite(RA_TX, txState);
                  lastToggle = now;
                }

                if (now - lastCheck >= checkInterval) {
                  int rx = digitalRead(RA_RX);
                  if (rx == txState) {
                    matchCount++;
                  }
                  totalChecks++;
                  lastCheck = now;
                }

                if (now - lastReport >= reportInterval && totalChecks > 0) {
                  int percent = (matchCount * 100) / totalChecks;
                  if (percent >= 90) {
                    setColorGreen();
                  } else if (percent >= 50) {
                    setColorOrange();
                  } else {
                    setColorRed();
                  }
                  matchCount = 0;
                  totalChecks = 0;
                  lastReport = now;
                }
                break;
              }
          }
        }
        break;
      }
    /* ==================== IR MENU ==================== */
    case MENU_IR:
      {
        static bool entered = false;

        if (!pageSelected) {
          breathe(breah, now, 255, 255, 0);
          menuButtons(page_IR, IR_PAGES_COUNT);
          if (Btn_ok.click()) {
            pageSelected = true;
            vibro(120, 250);
          }
          if (Btn_ok.hold()) {
            menuState = MENU_MAIN;
            vibro(120, 400);
          }
        }

        if (pageSelected) {
          if (Btn_ok.hold()) {
            menuState = MENU_IR;
            pageSelected = false;
            vibro(120, 400);
            // initialized = false;
            entered = false;
          }

          switch (page_IR) {
            // Scan
            case 0:
              {
                static uint8_t red = 255;
                static uint8_t green = 255;
                static unsigned long colorChangeTime = 0;
                const unsigned long colorHoldDuration = 500;

                if (millis() - colorChangeTime > colorHoldDuration) {
                  red = 255;
                  green = 255;
                }

                breathe(breah, now, red, green, 0, 10, 30);

                if (IrReceiver.decode()) {
                  auto &data = IrReceiver.decodedIRData;

                  if (data.rawDataPtr->rawlen < 4 || data.flags & IRDATA_FLAGS_IS_REPEAT || data.protocol == UNKNOWN) {
                    IrReceiver.resume();
                    break;
                  }

                  writeIRData(data.protocol, data.address, data.command, lastUsedSlot);
                  lastUsedSlot = (lastUsedSlot + 1) % MAX_IR_SIGNALS;

                  vibro(150, 100, 3);
                  red = 0;
                  green = 255;
                  colorChangeTime = millis();
                  IrReceiver.resume();
                }
                break;
              }
            case 1:
              {
                static bool entered = false;

                if (!entered) {
                  Btn_ok.reset();
                  entered = true;
                }

                breathe(breah, now, 255, 0, 0, 10, 50);

                for (uint8_t i = 0; i < MAX_IR_SIGNALS; i++) {
                  if (Btn_ok.hasClicks(i + 1)) {
                    SimpleIRData data = readIRData(i);
                    IrSender.write(data.protocol, data.address, data.command);
                    lastUsedSlot = i;
                    vibro(150, 100, 1);
                  }
                }
                break;
              }
          }
        }
        break;
      }
  }
}
