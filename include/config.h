#pragma once

#define WIFI_SSID       "TWOJA_NAZWA_WIFI"
#define WIFI_PASSWORD   "TWOJE_HASLO_WIFI"
#define HOSTNAME        "ph-monitor"

#define PIN_MOSI   23
#define PIN_SCLK   18
#define PIN_CS      5
#define PIN_DC     17
#define PIN_RST    16
#define PIN_BL      4

#define PIN_PH_ANALOG   34
#define PIN_PH_ALARM    35

#define ADC_VREF            3.3f
#define ADC_RESOLUTION      4095.0f
#define VOLTAGE_DIVIDER     0.6667f
#define PH_NEUTRAL_VOLTAGE  2.5f
#define PH_SLOPE            0.18f

#define SAMPLE_COUNT        32
#define SAMPLE_INTERVAL_MS  2000
#define EMA_ALPHA           0.05f

#define HISTORY_SIZE        60

#define PH_ALARM_LOW    6.0f
#define PH_ALARM_HIGH   8.0f

