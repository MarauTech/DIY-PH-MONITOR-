# pH Monitor - ESP32

Professional pH monitoring system with real-time display, temperature sensor, and smart alarm notifications.

![Status](https://img.shields.io/badge/status-active-brightgreen)
![Platform](https://img.shields.io/badge/platform-ESP32-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## Features

✨ **Hardware Monitoring**
- Real-time pH measurement with analog sensor
- Temperature monitoring via DS18B20 sensor
- Alarm detection with audible buzzer notification

📊 **User Interface**
- 3.5" TFT display (320×240 landscape)
- Live pH and temperature values with color-coded status
- Visual pH scale bar with 14 color segments
- IP address display for remote access

🔔 **Smart Alerts**
- Configurable pH alarm thresholds (low/high)
- Audio buzzer on pin 27 with adjustable volume
- Visual alarm indicators (orange text)
- Optional Pushover notifications

🌐 **Web Interface**
- Async web server for remote monitoring
- RESTful API endpoints
- Real-time data synchronization
- Mobile-friendly dashboard

## Hardware Requirements

### ESP32 Development Board
- Dual-core processor
- Wi-Fi connectivity
- Built-in ADC for analog readings

### External Components

| Component | Pin | Purpose |
|-----------|-----|---------|
| pH Sensor | GPIO 34 (PO pin) (use voltage divider 1kohm & 2kohm)  | Analog pH input |
| DS18B20 | GPIO 14 | Temperature sensor (OneWire) |
| Buzzer | GPIO 27 | Alarm notification |
| TFT Display | SPI (18, 23, 5, 17, 16, 4) | 3.5" display output |

### SPI Display Pinout

```
Pin 18  → SCLK (Clock)
Pin 23  → MOSI (Data)
Pin 5   → CS   (Chip Select)
Pin 17  → DC   (Data/Command)
Pin 16  → RST  (Reset)
Pin 4   → BL   (Backlight)
```

## Installation

### 1. Prerequisites
- PlatformIO IDE or CLI
- Python 3.6+
- ESP32 board drivers

### 2. Clone Repository
```bash
git clone https://github.com/yourusername/ph-monitor.git
cd ph-monitor
```

### 3. Configure WiFi & Credentials
Edit `include/config.h`:
```cpp
#define WIFI_SSID           "your_wifi_name"
#define WIFI_PASSWORD       "your_wifi_password"
#define PUSHOVER_TOKEN      "your_token"    // Optional
#define PUSHOVER_USER       "your_user"     // Optional
```

### 4. Build & Upload
```bash
# Build
platformio run

# Build and upload
platformio run --target upload

# Monitor serial output
platformio device monitor --baud 115200
```

## Configuration

### pH Calibration
The system uses a two-point calibration method:
```cpp
#define PH_NEUTRAL_VOLTAGE  2.5f  // Voltage at pH 7
#define PH_SLOPE            0.18f // mV per pH unit
```

### ADC Settings
```cpp
#define ADC_VREF            3.3f      // Reference voltage
#define ADC_RESOLUTION      4095.0f   // 12-bit ADC
#define VOLTAGE_DIVIDER     0.6667f   // Voltage scaling
```

### Sampling
```cpp
#define SAMPLE_COUNT        10     // Averaging samples
#define SAMPLE_INTERVAL_MS  2000   // Sample interval (ms)
```

### Alarm Thresholds
```cpp
#define PH_ALARM_LOW_DEFAULT   6.0f  // Minimum pH
#define PH_ALARM_HIGH_DEFAULT  8.0f  // Maximum pH
```

### Color Coding
- **Red** (pH < 4): Strongly acidic
- **Orange** (pH 4-6): Acidic
- **Yellow** (pH 6-7): Weakly acidic
- **Green** (pH 7-8): Neutral
- **Cyan** (pH 8-10): Weakly basic
- **Blue** (pH > 10): Strongly basic

## Usage

### Web API

**Get Current Readings**
```bash
curl http://<ip>/api
```

Response:
```json
{
  "ph": 7.45,
  "temp": 25.30,
  "voltage": 2.487,
  "alarmLow": 6.0,
  "alarmHigh": 8.0,
  "uptime": 3600
}
```

**Calibrate pH**
```bash
POST /api/calibrate
Content-Type: application/json

{
  "voltage": 2.5,
  "ph": 7.0
}
```

### Web Interface
Access the dashboard at: `http://<device-ip>/`

- View real-time metrics
- Configure alarm thresholds
- Calibrate sensors
- View historical data

## Troubleshooting

### No WiFi Connection
- Verify SSID and password in `config.h`
- Check AP is broadcasting (2.4GHz, not 5GHz)
- Reset ESP32 after configuration change

### Inaccurate pH Readings
- Verify analog sensor is calibrated
- Check for noise on ADC pin (add 100nF capacitor)
- Ensure voltage divider is properly connected

### Weak Buzzer Sound
- Buzzer is 3.3V driven from GPIO pin
- For 5V buzzer, use NPN transistor (2N2222) for amplification
- Circuit: `ESP32(GPIO27) → 1kΩ → 2N2222 Base`, `Emitter → GND`, `Collector → Buzzer(+)`

### Display Not Appearing
- Verify SPI pins match config.h
- Check display initialization in serial monitor
- Ensure backlight is powered (GPIO 4 must be HIGH)

## Project Structure

```
ph-monitor/
├── include/
│   └── config.h              # Hardware & WiFi config
├── src/
│   └── main.cpp              # Main application code
├── lib/                       # Custom libraries
├── data/                      # Web interface files
├── platformio.ini            # Build configuration
└── README.md                 # This file
```

## Libraries Used

| Library | Version | Purpose |
|---------|---------|---------|
| AsyncTCP | 3.2.14 | Async socket library |
| ESPAsyncWebServer | 3.3.15 | Web server framework |
| OneWire | 2.3.8 | OneWire protocol |
| DallasTemperature | 3.9.1 | DS18B20 sensor driver |

## Performance Metrics

- **Sampling Rate**: 10 samples every 2 seconds
- **Update Frequency**: Display refresh ~2 Hz
- **Memory**: ~16% RAM, ~75% Flash
- **Uptime**: 24/7 operation capability
- **Response Time**: <100ms API response


## Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Commit changes
4. Push to branch
5. Open a Pull Request

## License

MIT License - see LICENSE file for details

## Author

Created with ❤️ Wisnia 

---

**Want to support?** ⭐ Star this repository!
