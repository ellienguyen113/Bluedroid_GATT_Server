# Smart Door Access Control System

An ESP32-based smart door control system with multiple access methods including keypad PIN entry, automatic detection, and Bluetooth remote control.

## Features

- **Three Access Modes:**
  - **Keypad Mode:** Enter 4-digit PIN to open doors
  - **Auto Mode:** Automatic door opening when person detected within 25cm
  - **Remote Mode:** Bluetooth-controlled door operation

- **Three Independent Doors:** Each with customizable 4-digit PIN codes
- **Multi-sensor Integration:** Ultrasonic sensors for presence detection
- **Feedback Systems:** Buzzer for success/failure notifications and LED indicators
- **Bluetooth Control:** Remote door operation via BLE

## Hardware Components

- ESP32 microcontroller
- 4x4 Matrix Keypad
- Ultrasonic sensors (HC-SR04) - 3 units
- Servo motors - 3 units (for door mechanisms)
- Buzzer
- LED indicators - 3 units
- Bluetooth module (built-in ESP32)

## PIN Configuration

Default PIN codes for each door:
- **Door 1:** 1234
- **Door 2:** 4567  
- **Door 3:** 6789

## Bluetooth Commands

| Command | Function |
|---------|----------|
| 1, 2, 3 | Select Door 1/2/3 |
| 4 | Switch to Auto Mode |
| 5 | Switch to Remote Mode |
| 6 | Switch to Keypad Mode |
| 7 | Open selected door |
| 8 | Close selected door |

## Keypad Controls

- **0-9:** Enter PIN digits
- __*__: Clear entered PIN

## Mode Descriptions

### Auto Mode
- Automatically opens door when person detected within 25cm
- Door closes 5 seconds after person leaves detection area

### Keypad Mode
- Enter 4-digit PIN to open door
- Success/failure indicated by buzzer sounds
- Door closes after 5 seconds

### Remote Mode
- Open/close doors via Bluetooth commands
- Immediate response to remote commands

## Dependencies

- FreeRTOS
- ESP-IDF framework
- Component libraries:
  - buzzer.h
  - keypad.h
  - ultrasonic.h
  - servo.h
  - light.h
  - bluetooth.h
