# MarauTech pH Monitor v2.0.0 — Official Release

We are proud to announce the official **v2.0.0** release of the **MarauTech pH Monitor**! This milestone transforms the project from an embedded prototype into a complete, professional open-source water quality monitoring ecosystem.

---

## 🌟 What's New in v2.0.0

### 📱 1. Dedicated Native Android Application (`PhMonitorApp`)
- **Modern Jetpack Compose UI**: Built with Material 3, supporting dark and light themes, smooth animations, and clean data formatting.
- **Direct LAN Connection**: Connects directly to the ESP32 IP address over HTTP without subscriptions, cloud accounts, or Firebase.
- **3 Home Screen Widgets**:
  - *Compact (1x1)*: Glanceable pH and status badge.
  - *Medium (2x2)*: pH, DS18B20 temperature, device name, and manual refresh trigger.
  - *Large (4x2)*: Full telemetry including probe voltage, uptime, and quick application launcher.
- **Spam-Free Background Monitoring**: Periodically checks the ESP32 via Android `WorkManager` and issues system notifications only on alarm state changes (`NORMAL` ➔ `LOW`/`HIGH` pH) and recovery.
- **Interactive History Graphs**: Custom Canvas charts for pH, temperature, and probe voltage with 1h to 24h filtering.
- **Interactive Calibration Assistant**: Visual countdown and stability progress reporting directly from the ESP32.

### ⚡ 2. Firmware Architecture Enhancements (ESP32)
- **Single-Binary Web Architecture**: Embedded HTML/JS/CSS assets in PROGMEM, allowing the full web dashboard to be flashed in a single `firmware.bin` file without requiring a separate LittleFS upload for web assets.
- **Precision 3-Point Calibration Engine**: Calibrate using standard buffers (`pH 4.01`, `7.00`, `9.18`) or custom solutions. Includes standard deviation sampling to prevent saving noisy measurements.
- **Validation Rules**: Guard rails against invalid electrode slope, physical voltage bounds (0.5V–4.5V), and inverted buffer points.
- **Home Assistant MQTT Auto-Discovery**: Automatic entity creation in Home Assistant for pH, temperature, alarms, and RSSI.
- **Pushover Emergency Alerts**: Direct mobile push notifications when water parameters breach safety thresholds.
- **LittleFS 24h History Logging**: Automatic JSONL logging for historical telemetry.

### 🎨 3. Project Branding & Documentation
- **New Geometric Brand Identity**: Modern water-drop & measuring probe logo (SVG & dark/light variants).
- **GitHub Pages Website**: Complete landing page ready for deployment under `docs/`.
- **Professional Documentation**: Comprehensive README with wiring pinouts, REST API documentation, and installation instructions.

---

## 💾 Release Assets

| File Name | Size | Target Platform | Description |
| :--- | :--- | :--- | :--- |
| **`MarauTech-pH-Monitor-v2.0.0.apk`** | ~14.3 MB | Android 8.0+ | Official Release Signed Android APK |
| **`MarauTech-pH-Monitor-v2.0.0-firmware.bin`** | ~1.4 MB | ESP32 | Complete Firmware Binary (includes Web SPA) |
| **`MarauTech-pH-Monitor-v2.0.0-filesystem.bin`** | ~1.4 MB | ESP32 Flash | LittleFS Storage Partition (History Logging) |

---

## 🔄 Upgrade & Compatibility Notes

1. **NVS Configuration**: Upgrading from v1.x will preserve existing Wi-Fi settings. However, it is strongly recommended to run the 3-point calibration in the web dashboard or Android app to take full advantage of the new calibration engine.
2. **Web Assets**: Because the web SPA is now compiled directly into `firmware.bin`, you no longer need to upload `littlefs.bin` for the web interface to function. `littlefs.bin` is only used for the 24-hour history logger.
3. **Android App Compatibility**: `PhMonitorApp v2.0.0` is designed for `Firmware v2.0.0` REST API endpoints (`/api/status`, `/api/calibrate/status`, `/api/history`).

---

## 📥 Installation Quick Start

### Flashing ESP32 via PlatformIO:
```bash
pio run -t upload
```

### Installing Android App:
1. Download `MarauTech-pH-Monitor-v2.0.0.apk` onto your Android phone.
2. Open the APK and allow installation from unknown sources.
3. Connect your phone to the same Wi-Fi network as the ESP32, enter the device IP, and press **Connect**.

---

**Full Changelog**: [CHANGELOG.md](CHANGELOG.md)
