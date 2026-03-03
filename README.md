# Low-Power BLE Wearable Platform (Custom Smartwatch)

An embedded wearable system built on the XIAO nRF52840 Sense, featuring a 240×240/240×280 SPI TFT display, LVGL-based UI, motion sensing, and BLE connectivity for smartphone integration.

This project explores low-power embedded firmware design, real-time UI rendering, and hardware/software integration on a resource-constrained microcontroller platform.

---

## Hardware

- **MCU:** Seeed XIAO nRF52840 Sense (ARM Cortex-M4F, BLE 5.0)
- **Display:** Waveshare SPI TFT (ST7789 / GC9A01)
- **Touch:** CST816S capacitive touch (I2C) *(pending replacement)*
- **Sensors:**
  - Onboard IMU (motion / step tracking)
  - MAX30102 optical heart-rate sensor
- **Haptics:** Coin vibration motor (planned)
- **Power:** LiPo battery (target ≥ 24h runtime)

---

## Software Stack

- **Framework:** Arduino (PlatformIO)
- **Language:** C/C++
- **UI Framework:** LVGL (partial framebuffer rendering)
- **Communication:** SPI (display), I2C (touch + sensors), BLE (planned iOS integration)
- **Architecture:** Event-driven firmware with circular message queue and modular screen management

---

## Features (Planned / In Progress)

### V1
- Watch face (time, date)
- Health app (step tracking + heart rate)
- Notifications display
- Music control
- Settings menu
- Screen timeout + low-power idle state

### Roadmap
- iOS notification integration (BLE)
- Media control (play/pause/skip)
- Call + text interaction
- Gesture-based navigation
- Power-optimized sleep/wake via interrupts

---

## Firmware Architecture

The firmware is structured around:

- Event-driven message queue
- Screen state machine (watchface, health, notifications, music, settings)
- Partial framebuffer LVGL rendering
- Modular hardware abstraction layers (display, sensors, power, input)

This design allows clean separation between hardware drivers and UI logic, enabling scalability and maintainability.

---

## Power Strategy

- Screen-off timeout when idle
- Interrupt-driven wake (button / touch)
- Sensor sampling rate optimization
- BLE connection interval tuning (planned)

Target: ≥ 24 hours typical usage.

---

## Project Goals

- Build a fully custom BLE wearable from the ground up
- Implement smooth LVGL UI on constrained hardware
- Optimize for low-power operation
- Explore real-world embedded system architecture

---


## Status

Active development.
Touch-enabled round display replacement pending.
