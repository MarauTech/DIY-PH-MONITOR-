# Changelog

All notable changes to the **MarauTech pH Monitor** project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [2.0.0] - 2026-08-30

### Added
- **Native Android Application (`PhMonitorApp`)**:
  - Built with Kotlin, Jetpack Compose, Material 3, and Retrofit.
  - Direct local LAN communication (`http://<IP>`) with zero cloud dependencies.
  - Real-time **Hero Dashboard** with pH, temperature, probe voltage, and animated status badges (`NORMAL`, `LOW pH`, `HIGH pH`, `OFFLINE`).
  - **3 Android App Widgets** (Compact, Medium, and Large) with live launcher updates.
  - **Spam-free Background Monitoring** using `WorkManager` and Android Notification API with notification channels.
  - **Interactive History Chart** using custom Compose Canvas with 1h to 24h range filtering.
  - Interactive **3-Point Calibration Assistant** with progress feedback and Polish error descriptions.
  - Secure credential storage using `EncryptedSharedPreferences` / Android Keystore.
- **Branding & Presentation Suite**:
  - Brand-new geometric water-drop & measuring probe logo (SVG & dark/light variants).
  - Modern, responsive **GitHub Pages Website (`docs/`)** with live previews and hardware documentation.
  - High-resolution social media banner (`assets/branding/social-preview.svg`).
  - Updated Android adaptive icons.
- **Firmware Enhancements**:
  - Single-bin embedded Web SPA architecture (`html_content.h` in PROGMEM) eliminating LittleFS dependencies for web assets.
  - Advanced **3-point calibration engine** with live standard deviation stability analysis (`/api/calibrate/status`).
  - Strict calibration validation (`invalid_voltage`, `points_too_close`, `invalid_slope`).
  - Factory reset calibration routine (`reset`).
  - Support for custom buffer solutions (`customPH`).

### Changed
- Refactored web server endpoints to enforce HTTP Basic Authentication on sensitive configuration routes.
- Updated Home Assistant MQTT integration with auto-discovery and telemetry publishing.
- Upgraded Android Gradle configuration to SDK 36 and target SDK 36.

### Security
- Added secure credential handling across all endpoints.
- Masked Pushover tokens and MQTT passwords in web and Android forms.
- Replaced plain text credential storage with Android Keystore.

### Fixed
- Fixed browser caching issues on web UI by introducing aggressive `Cache-Control: no-cache, no-store, must-revalidate` headers.
- Fixed calibration stability timeout race conditions.
- Fixed DTO field mapping for historical JSONL logging (`t`, `p`, `te`, `v`, `a`).

---

## [1.0.0] - Initial Release
- Basic ESP32 ADC sampling for analog pH sensors.
- Basic web server on LittleFS.
- ST7789 TFT display driver.
- Simple alarm threshold logic.
