#pragma once

// --- TFT ST7789 Display ---
#define PIN_MOSI        23
#define PIN_SCLK        18
#define PIN_CS           5
#define PIN_DC          17
#define PIN_RST         16
#define PIN_BL           4

#define PIN_SPI_MOSI    PIN_MOSI
#define PIN_SPI_SCK     PIN_SCLK
#define PIN_TFT_CS      PIN_CS
#define PIN_TFT_DC      PIN_DC
#define PIN_TFT_RST     PIN_RST
#define PIN_TFT_BL      PIN_BL

// --- pH Sensor (PH-4502C) ---
#define PIN_PH_ANALOG   34
#define PIN_PH_ALARM    35   // Digital output from module (unused in firmware)

// --- DS18B20 Temperature Sensor ---
#define PIN_DS18B20     14

// --- Buzzer (optional, set to -1 to disable) ---
#define PIN_BUZZER      -1
