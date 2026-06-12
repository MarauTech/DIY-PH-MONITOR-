#pragma once

#define WIFI_SSID           "your ssid"
#define WIFI_PASSWORD       "your ssid pass"
#define HOSTNAME            "ph-monitor"

#define PIN_MOSI   23
#define PIN_SCLK   18
#define PIN_CS      5
#define PIN_DC     17
#define PIN_RST    16
#define PIN_BL      4
#define PIN_BUZZER     27
#define PIN_DS18B20    14

#define PIN_PH_ANALOG  34
#define PIN_PH_ALARM   35

#define ADC_VREF            3.3f
#define ADC_RESOLUTION      4095.0f
#define VOLTAGE_DIVIDER     0.6667f
#define PH_NEUTRAL_VOLTAGE  2.5f
#define PH_SLOPE            0.18f

#define SAMPLE_COUNT        10
#define SAMPLE_INTERVAL_MS  2000

#define HISTORY_SIZE        720

#define PH_ALARM_LOW_DEFAULT   6.0f
#define PH_ALARM_HIGH_DEFAULT  8.0f

#define PUSHOVER_TOKEN      ""
#define PUSHOVER_USER       ""