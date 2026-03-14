# Xcode Project Setup

## 1. Create the Xcode project
1. Open Xcode → **File -> New -> Project**
2. Choose **iOS -> App**
3. Set:
   - Product Name: `SmartWatchCompanion`
   - Interface: **SwiftUI**
   - Language: **Swift**
4. Save it **inside** `companion_app/ios/` so the folder becomes your project root

## 2. Add the source files
Delete the placeholder `ContentView.swift` Xcode generated, then drag all four Swift files from
this folder into the Xcode project navigator (check **"Copy items if needed"** is OFF — they are
already in place):
- `SmartWatchCompanionApp.swift`
- `BLEManager.swift`
- `MusicObserver.swift`
- `ContentView.swift`

## 3. Configure capabilities (required)

### Info.plist keys
Add these to `Info.plist` (or the Xcode "Info" tab):

| Key | Value |
|-----|-------|
| `NSBluetoothAlwaysUsageDescription` | `SmartWatch Companion needs Bluetooth to connect to your watch.` |
| `NSBluetoothPeripheralUsageDescription` | `SmartWatch Companion needs Bluetooth to connect to your watch.` |
| `NSAppleMusicUsageDescription` | `SmartWatch Companion reads the current track to display on your watch.` |

### Background Modes (Signing & Capabilities tab)
Click **+ Capability** and add **Background Modes**, then enable:
- [x] **Uses Bluetooth LE accessories**

This keeps the app alive while connected so music polling and time sync work in the background.

### MediaPlayer framework
In **Build Phases → Link Binary With Libraries**, add:
- `MediaPlayer.framework`

## 4. Build & Run
- Build to your iPhone (requires a physical device — BLE doesn't work on the Simulator)
- First launch: grant Bluetooth permission when prompted
- The app will automatically scan for "SmartWatch", connect, sync time, and start forwarding music

## 5. ANCS (iOS Notifications -> Watch)
Notifications flow **without any iOS app code** — the watch firmware subscribes to ANCS directly.
When you first connect after flashing the updated firmware, your iPhone will show a pairing
request. Accept it. After pairing, iOS will push notification data straight to the watch.

If the watch doesn't appear in Bluetooth settings after flashing, forget any old "SmartWatch"
pairing and re-scan.

## What works
| Feature | How |
|---------|-----|
| Time sync | Sent automatically on BLE connect |
| Music (Apple Music) | `MPMusicPlayerController` notification → `MC\|...` command |
| Music (Spotify / others) | 5-second `MPNowPlayingInfoCenter` poll → `MC\|...` command |
| Notifications | ANCS — watch firmware subscribes directly to iPhone, no app code needed |
| Messages | `MG\|...` / `MT\|...` — not auto-synced (iOS blocks SMS access); use nRF Connect manually for now |
