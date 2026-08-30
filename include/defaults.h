#pragma once

#if __has_include("secrets.h")
#include "secrets.h"
#endif

#define FIRMWARE_VERSION    "2.0.0"
#define HOSTNAME            "ph-monitor"

// --- Default Admin Credentials ---
#ifndef DEFAULT_ADMIN_USER
#define DEFAULT_ADMIN_USER  "admin"
#endif

#ifndef DEFAULT_ADMIN_PASS
#define DEFAULT_ADMIN_PASS  "admin"
#endif

// --- ADC / Voltage ---
#define VOLTAGE_DIVIDER     0.6667f   // Hardware divider ratio (10k/20k)

// --- pH Sampling ---
#define SAMPLE_COUNT        32
#define SAMPLE_INTERVAL_MS  2000
#define EMA_ALPHA           0.05f

// --- Calibration defaults ---
#define DEFAULT_VOLTAGE_PH7  2500     // mV
#define DEFAULT_VOLTAGE_PH4  3038     // mV
#define DEFAULT_VOLTAGE_PH9  2108     // mV
#define DEFAULT_PH7_VALUE    7.00f
#define DEFAULT_PH4_VALUE    4.01f
#define DEFAULT_PH9_VALUE    9.18f

// --- Calibration stability ---
#define CAL_STABILITY_SAMPLES     30
#define CAL_STABILITY_PERIOD_MS   3000  // 3 seconds
#define CAL_STABILITY_MAX_DEV_MV  50.0f // Tolerant to ADC/wiring ripple

// --- Calibration validation ---
#define CAL_MIN_VOLTAGE_MV    500     // Minimum sensible voltage
#define CAL_MAX_VOLTAGE_MV    4500    // Maximum sensible voltage
#define CAL_MIN_POINT_DIFF_MV 100     // Minimum difference between cal points

// --- Alarm defaults ---
#define DEFAULT_ALARM_LOW     6.0f
#define DEFAULT_ALARM_HIGH    8.0f
#define DEFAULT_HYSTERESIS    0.10f   // pH units
#define DEFAULT_ALARM_HOLD_S  15      // Seconds before alarm activates

// --- History ---
#define HISTORY_SIZE          60      // In-RAM ring buffer for TFT chart
#define HISTORY_LOG_INTERVAL  60000   // Log to LittleFS every 60s
#define HISTORY_MAX_RECORDS   1440    // Max records in file (24h at 1/min)
#define HISTORY_FILE          "/history.jsonl"

// --- Display ---
#define TFT_WIDTH   320
#define TFT_HEIGHT  240

// --- MQTT defaults ---
#define DEFAULT_MQTT_PORT   1883

// --- NTP ---
#define NTP_SERVER1     "pool.ntp.org"
#define NTP_SERVER2     "time.nist.gov"
#define NTP_TZ          "CET-1CEST,M3.5.0,M10.5.0/3"
