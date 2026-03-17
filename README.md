# Low-Power BLE Wearable Platform (Custom Smartwatch)

A custom smartwatch built on the XIAO nRF52840 Sense, featuring a 240×240 SPI TFT display, LVGL-based touch UI, step tracking, and full BLE connectivity for iOS integration (notifications, music control, messaging).

---

## Hardware

- **MCU:** Seeed XIAO nRF52840 Sense (ARM Cortex-M4F, BLE 5.0)
- **Display:** Waveshare SPI TFT (GC9A01 round, 240×240)
- **Touch:** CST816S capacitive touch (I2C)
- **IMU:** LSM6DS3TR-C (onboard) — step counting / pedometer
- **Heart Rate:** MAX30102 optical sensor *(not yet connected)*
- **Haptics:** Coin vibration motor *(planned)*
- **Power:** LiPo battery (target ≥ 24h runtime)

---

## Software Stack

- **Framework:** Arduino (PlatformIO)
- **Language:** C++
- **UI Framework:** LVGL 8 (partial framebuffer rendering)
- **BLE:** Adafruit Bluefruit — ANCS (iOS notifications), NUS (companion app), HID (media keys)
- **Architecture:** Event-driven firmware with circular message queue and modular screen management

---

## Features

### Done
- Watch face — time, date, day of week
- Lockscreen with tap-to-unlock
- Home screen — app icon grid
- Notifications screen — ANCS (native iOS notifications, no companion app needed) + NUS push; swipe to dismiss
- Music screen — track title, artist, elapsed/duration progress bar, prev/play-pause/next controls via BLE HID
- Messages screen — conversation list with unread indicators, thread view
- Activity screen — live step count + distance (LSM6DS3 pedometer); heart rate placeholder (`-- BPM`)
- Settings screen
- Clock overlay on all non-lockscreen views
- Top-edge swipe to open notifications from anywhere
- BLE time sync (RTC updated by companion app on connect)

### Pending / Roadmap
- Screen timeout + display sleep (battery life)
- Heart rate polling (MAX30102, hardware not yet connected)
- Haptic feedback on notifications / interactions
- ANCS dismiss action (clear notification on iPhone when dismissed on watch)
- Calorie tracking (steps × MET × weight)
- iOS companion app (Swift) — music metadata push, message sync, time sync

---

## Firmware Architecture

```
app::loop()
  ├── imu::poll() — reads LSM6DS3 step counter (1 Hz)
  ├── ble::poll() — drains ANCS attribute fetch queue
  ├── buttons::poll()
  ├── touch::poll()
  └── event queue dispatch
        ├── MEDIA_* -> ble:: (HID keys)
        └── everything else -> ui::handle()

ui::handle()
  └── screen state machine
        LOCKSCREEN -> HOMESCREEN -> HEALTH / MUSIC / NOTIF / MESSAGES / SETTINGS
```

HAL modules: `ble`, `imu`, `rtc`, `touch`, `buttons`
Screen modules: `lockscreen`, `homescreen`, `health`, `music`, `notif`, `messages`, `message_thread`, `settings`

---

## BLE Protocol (NUS)

Commands sent from the iOS companion app over Nordic UART Service:

| Command | Format | Description |
|---------|--------|-------------|
| `T` | `T\|HH\|MM\|SS\|YYYY\|MM\|DD\|wday` | Time sync |
| `N` | `N\|appid\|title\|body` | Push notification |
| `NC` | `NC` | Clear all notifications |
| `MC` | `MC\|title\|artist\|elapsed\|duration\|playing` | Music metadata |
| `MS` | `MS\|elapsed\|duration` | Music seek/progress update |
| `MG` | `MG\|peer\|preview\|unread` | Message conversation |
| `MGC` | `MGC` | Clear conversations |
| `MT` | `MT\|peer\|sent\|text` | Add message to thread |
| `MTC` | `MTC` | Clear thread |

ANCS runs in parallel for direct iOS notification delivery without the companion app.

---

## Status

Active development — core UI and BLE stack complete. IMU step tracking live. Heart rate sensor and screen timeout pending.
