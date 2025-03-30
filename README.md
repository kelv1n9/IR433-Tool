# IR433 Tool

**IR433 Tool** is a compact and flexible embedded utility for capturing, storing, and transmitting both 433 MHz RF and IR (infrared) signals. Designed for boards such as the LGT8F328P or similar Arduino-compatible MCUs, it provides a menu-driven interface with user feedback through LEDs and a vibro motor.

## Features

- **433 MHz Support:**
  - Scan for and capture RF codes using RCSwitch
  - Transmit captured codes with configurable timing
  - Generate RF noise (jammer-like behavior)
  - Built-in self-test for TX/RX hardware loopback

- **Infrared Support (IR):**
  - Scan and decode common IR protocols using IRremote
  - Store and replay up to 4 IR commands in EEPROM
  - Visual and haptic feedback for captured signals

- **User Interface:**
  - Three-button control: OK / UP / DOWN
  - RGB LED for color-based feedback (status, breathing animation)
  - Vibro motor for haptic signaling (non-blocking)
  - OLED support can be added (not included by default)

- **EEPROM Usage:**
  - Optimized to support LGT8F328P 
  - IR command storage: protocol + address + command
  - Persistent storage across reboots

- **Power Monitoring:**
  - Reads voltage through resistor divider
  - Visual voltage indication using LED color

## Dependencies

- [IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote)
- [RCSwitch](https://github.com/sui77/rc-switch)
- [EncButton](https://github.com/GyverLibs/EncButton)
- EEPROM (native or LGT-specific API)

## Pin Configuration

| Function       | Pin   |
|----------------|-------|
| Vibro Motor    | D9    |
| RGB LED        | D5/D6/D13 |
| Buttons        | D8/D10/D11 |
| RF TX/RX       | D12 / D2 |
| IR TX/RX       | D7 / D3 |
| Battery Input  | A0    |

> Adjust pins if using a different board or layout.

## Menu Structure

- **Main Menu**
  - RA (433 MHz)
    - Scan
    - Attack (replay)
    - Noise
    - Self-Test
  - IR
    - Scan
    - Attack

## How to Use

1. **Scan RF**: Go to RA > Scan and press OK. When a valid RF signal is received, it is stored for later use.
2. **Replay RF**: Go to RA > Attack. Captured signal is sent at intervals.
3. **Scan IR**: Go to IR > Scan. When an IR code is received, it is saved to EEPROM sequentially.
4. **Replay IR**: Go to IR > Attack and click 1–4 times to select the saved slot.

## License

MIT License

---

This tool is intended for educational and testing purposes. Use responsibly and comply with your local RF and IR transmission regulations.

