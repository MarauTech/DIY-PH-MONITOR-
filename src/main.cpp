#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

#define CLR_BLACK    0x0000
#define CLR_WHITE    0xFFFF
#define CLR_RED      0xF800
#define CLR_GREEN    0x07E0
#define CLR_BLUE     0x001F
#define CLR_ORANGE   0xFD20
#define CLR_CYAN     0x07FF
#define CLR_PANEL    0x1082
#define CLR_MUTED    0x7BEF
#define CLR_DARKGRAY 0x2104

static inline void tft_cs_low()  { digitalWrite(PIN_CS, LOW);  }
static inline void tft_cs_high() { digitalWrite(PIN_CS, HIGH); }
static inline void tft_dc_cmd()  { digitalWrite(PIN_DC, LOW);  }
static inline void tft_dc_data() { digitalWrite(PIN_DC, HIGH); }

void tft_cmd(uint8_t cmd) {
    tft_dc_cmd();
    tft_cs_low();
    SPI.transfer(cmd);
    tft_cs_high();
}

void tft_data8(uint8_t d) {
    tft_dc_data();
    tft_cs_low();
    SPI.transfer(d);
    tft_cs_high();
}

void tft_data16(uint16_t d) {
    tft_dc_data();
    tft_cs_low();
    SPI.transfer16(d);
    tft_cs_high();
}

void tft_init() {
    pinMode(PIN_CS,  OUTPUT); digitalWrite(PIN_CS,  HIGH);
    pinMode(PIN_DC,  OUTPUT); digitalWrite(PIN_DC,  HIGH);
    pinMode(PIN_RST, OUTPUT); digitalWrite(PIN_RST, HIGH);
    pinMode(PIN_BL,  OUTPUT); digitalWrite(PIN_BL,  HIGH);

    digitalWrite(PIN_RST, LOW);  delay(100);
    digitalWrite(PIN_RST, HIGH); delay(200);

    SPI.begin(PIN_SCLK, -1, PIN_MOSI, PIN_CS);
    SPI.setFrequency(40000000);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);

    tft_cmd(0x01); delay(150);
    tft_cmd(0x11); delay(500);

    tft_cmd(0x36);
    tft_data8(0xB0);

    tft_cmd(0x3A);
    tft_data8(0x55);

    tft_cmd(0xB2);
    tft_data8(0x0C); tft_data8(0x0C);
    tft_data8(0x00); tft_data8(0x33); tft_data8(0x33);

    tft_cmd(0xB7);
    tft_data8(0x35);

    tft_cmd(0xBB);
    tft_data8(0x19);

    tft_cmd(0xC0);
    tft_data8(0x2C);

    tft_cmd(0xC2);
    tft_data8(0x01);

    tft_cmd(0xC3);
    tft_data8(0x12);

    tft_cmd(0xC4);
    tft_data8(0x20);

    tft_cmd(0xC6);
    tft_data8(0x0F);

    tft_cmd(0xD0);
    tft_data8(0xA4); tft_data8(0xA1);

    tft_cmd(0x21);

    tft_cmd(0x29); delay(100);
}

void tft_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    tft_cmd(0x2A);
    tft_data8(x0 >> 8); tft_data8(x0 & 0xFF);
    tft_data8(x1 >> 8); tft_data8(x1 & 0xFF);
    tft_cmd(0x2B);
    tft_data8(y0 >> 8); tft_data8(y0 & 0xFF);
    tft_data8(y1 >> 8); tft_data8(y1 & 0xFF);
    tft_cmd(0x2C);
}

void tft_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    tft_set_window(x, y, x + w - 1, y + h - 1);
    uint32_t n = (uint32_t)w * h;
    tft_dc_data();
    tft_cs_low();
    for (uint32_t i = 0; i < n; i++) {
        SPI.transfer16(color);
    }
    tft_cs_high();
}

void tft_fill_screen(uint16_t color) {
    tft_fill_rect(0, 0, 320, 240, color);
}

void tft_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    tft_set_window(x, y, x, y);
    tft_data16(color);
}

void tft_draw_hline(uint16_t x, uint16_t y, uint16_t w, uint16_t color) {
    tft_fill_rect(x, y, w, 1, color);
}

void tft_draw_vline(uint16_t x, uint16_t y, uint16_t h, uint16_t color) {
    tft_fill_rect(x, y, 1, h, color);
}

void tft_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    tft_draw_hline(x, y, w, color);
    tft_draw_hline(x, y + h - 1, w, color);
    tft_draw_vline(x, y, h, color);
    tft_draw_vline(x + w - 1, y, h, color);
}

void tft_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0), dy = abs(y1 - y0);
    int16_t sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    int16_t err = dx - dy;
    while (true) {
        if (x0 >= 0 && x0 < 320 && y0 >= 0 && y0 < 240)
            tft_draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

static const uint8_t font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x5F,0x00,0x00},
    {0x00,0x07,0x00,0x07,0x00},
    {0x14,0x7F,0x14,0x7F,0x14},
    {0x24,0x2A,0x7F,0x2A,0x12},
    {0x23,0x13,0x08,0x64,0x62},
    {0x36,0x49,0x55,0x22,0x50},
    {0x00,0x05,0x03,0x00,0x00},
    {0x00,0x1C,0x22,0x41,0x00},
    {0x00,0x41,0x22,0x1C,0x00},
    {0x14,0x08,0x3E,0x08,0x14},
    {0x08,0x08,0x3E,0x08,0x08},
    {0x00,0x50,0x30,0x00,0x00},
    {0x08,0x08,0x08,0x08,0x08},
    {0x00,0x60,0x60,0x00,0x00},
    {0x20,0x10,0x08,0x04,0x02},
    {0x3E,0x51,0x49,0x45,0x3E},
    {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46},
    {0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10},
    {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30},
    {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36},
    {0x06,0x49,0x49,0x29,0x1E},
    {0x00,0x36,0x36,0x00,0x00},
    {0x00,0x56,0x36,0x00,0x00},
    {0x08,0x14,0x22,0x41,0x00},
    {0x14,0x14,0x14,0x14,0x14},
    {0x00,0x41,0x22,0x14,0x08},
    {0x02,0x01,0x51,0x09,0x06},
    {0x32,0x49,0x79,0x41,0x3E},
    {0x7E,0x11,0x11,0x11,0x7E},
    {0x7F,0x49,0x49,0x49,0x36},
    {0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x22,0x1C},
    {0x7F,0x49,0x49,0x49,0x41},
    {0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x49,0x49,0x7A},
    {0x7F,0x08,0x08,0x08,0x7F},
    {0x00,0x41,0x7F,0x41,0x00},
    {0x20,0x40,0x41,0x3F,0x01},
    {0x7F,0x08,0x14,0x22,0x41},
    {0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x04,0x02,0x7F},
    {0x7F,0x04,0x08,0x10,0x7F},
    {0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06},
    {0x3E,0x41,0x51,0x21,0x5E},
    {0x7F,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31},
    {0x01,0x01,0x7F,0x01,0x01},
    {0x3F,0x40,0x40,0x40,0x3F},
    {0x1F,0x20,0x40,0x20,0x1F},
    {0x3F,0x40,0x38,0x40,0x3F},
    {0x63,0x14,0x08,0x14,0x63},
    {0x07,0x08,0x70,0x08,0x07},
    {0x61,0x51,0x49,0x45,0x43},
    {0x00,0x7F,0x41,0x41,0x00},
    {0x02,0x04,0x08,0x10,0x20},
    {0x00,0x41,0x41,0x7F,0x00},
    {0x04,0x02,0x01,0x02,0x04},
    {0x40,0x40,0x40,0x40,0x40},
    {0x00,0x01,0x02,0x04,0x00},
    {0x20,0x54,0x54,0x54,0x78},
    {0x7F,0x48,0x44,0x44,0x38},
    {0x38,0x44,0x44,0x44,0x20},
    {0x38,0x44,0x44,0x48,0x7F},
    {0x38,0x54,0x54,0x54,0x18},
    {0x08,0x7E,0x09,0x01,0x02},
    {0x0C,0x52,0x52,0x52,0x3E},
    {0x7F,0x08,0x04,0x04,0x78},
    {0x00,0x44,0x7D,0x40,0x00},
    {0x20,0x40,0x44,0x3D,0x00},
    {0x7F,0x10,0x28,0x44,0x00},
    {0x00,0x41,0x7F,0x40,0x00},
    {0x7C,0x04,0x18,0x04,0x78},
    {0x7C,0x08,0x04,0x04,0x78},
    {0x38,0x44,0x44,0x44,0x38},
    {0x7C,0x14,0x14,0x14,0x08},
    {0x08,0x14,0x14,0x18,0x7C},
    {0x7C,0x08,0x04,0x04,0x08},
    {0x48,0x54,0x54,0x54,0x20},
    {0x04,0x3F,0x44,0x40,0x20},
    {0x3C,0x40,0x40,0x20,0x7C},
    {0x1C,0x20,0x40,0x20,0x1C},
    {0x3C,0x40,0x30,0x40,0x3C},
    {0x44,0x28,0x10,0x28,0x44},
    {0x0C,0x50,0x50,0x50,0x3C},
    {0x44,0x64,0x54,0x4C,0x44},
    {0x00,0x08,0x36,0x41,0x00},
    {0x00,0x00,0x7F,0x00,0x00},
    {0x00,0x41,0x36,0x08,0x00},
    {0x10,0x08,0x08,0x10,0x08},
};

void tft_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t scale) {
    if (c < 32 || c > 126) c = '?';
    const uint8_t *glyph = font5x7[c - 32];
    for (uint8_t col = 0; col < 5; col++) {
        uint8_t line = glyph[col];
        for (uint8_t row = 0; row < 7; row++) {
            uint16_t px = x + col * scale;
            uint16_t py = y + row * scale;
            if (line & (1 << row))
                tft_fill_rect(px, py, scale, scale, color);
            else if (bg != color)
                tft_fill_rect(px, py, scale, scale, bg);
        }
    }
}

uint16_t tft_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t scale) {
    uint16_t cx = x;
    while (*str) {
        tft_draw_char(cx, y, *str++, color, bg, scale);
        cx += (5 + 1) * scale;
    }
    return cx;
}

void tft_draw_big_number(uint16_t x, uint16_t y, const char *str, uint16_t color) {
    while (*str) {
        tft_draw_char(x, y, *str++, color, CLR_BLACK, 4);
        x += (5 + 1) * 4;
    }
}

float currentPH      = 7.0f;
float smoothedPH     = 7.0f;
float currentVoltage = 0.0f;
float currentTemp    = -127.0f;
bool  alarmActive    = false;
bool  wifiConnected  = false;
char  wifiIP[16]     = "---";

float phHistory[HISTORY_SIZE];
int   historyIndex = 0;
int   historyCount = 0;

unsigned long lastSampleTime = 0;

float phNeutralVoltage = 2.5f;
float phSlope          = 0.18f;
float phAlarmLow       = 6.0f;
float phAlarmHigh      = 8.0f;
bool  pushoverEnabled  = false;
char  pushoverUser[64]  = "";
char  pushoverToken[64] = "";

AsyncWebServer server(80);

OneWire oneWire(14);
DallasTemperature sensors(&oneWire);

static void insertionSort(int *arr, int n) {
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j   = i - 1;
        while (j >= 0 && arr[j] > key) { arr[j + 1] = arr[j]; j--; }
        arr[j + 1] = key;
    }
}

float readPH() {
    int samples[SAMPLE_COUNT];
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        samples[i] = analogRead(PIN_PH_ANALOG);
        delay(5);
    }

    insertionSort(samples, SAMPLE_COUNT);
    int trim    = SAMPLE_COUNT / 4;
    long sum    = 0;
    int  count  = 0;
    for (int i = trim; i < SAMPLE_COUNT - trim; i++) {
        sum += samples[i];
        count++;
    }
    float raw  = sum / (float)count;

    float vADC = raw * ADC_VREF / ADC_RESOLUTION;
    float vPO  = vADC / VOLTAGE_DIVIDER;
    currentVoltage = vPO;
    float ph   = 7.0f + (phNeutralVoltage - vPO) / phSlope;
    return constrain(ph, 0.0f, 14.0f);
}

float applyEMA(float prev, float newVal) {
    return EMA_ALPHA * newVal + (1.0f - EMA_ALPHA) * prev;
}

void updateHistory(float ph) {
    phHistory[historyIndex] = ph;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    if (historyCount < HISTORY_SIZE) historyCount++;
}

uint16_t phColor(float ph) {
    if (ph < phAlarmLow)  return CLR_RED;
    if (ph > phAlarmHigh) return CLR_BLUE;
    return CLR_GREEN;
}

const char* phLabel(float ph) {
    if (ph < 4.0f) return "SILNIE KWASNE";
    if (ph < 6.0f) return "KWASNE";
    if (ph < 6.5f) return "LEKKO KWASNE";
    if (ph < 7.5f) return "NEUTRALNE";
    if (ph < 8.5f) return "LEKKO ZASADOWE";
    if (ph < 10.f) return "ZASADOWE";
    return "SILNIE ZASADOWE";
}

void drawStaticUI() {
    tft_fill_screen(CLR_BLACK);
    tft_fill_rect(0, 0, 320, 24, CLR_PANEL);
    tft_draw_string(6, 6, "pH MONITOR", CLR_MUTED, CLR_PANEL, 2);
    tft_draw_hline(0, 24, 320, CLR_MUTED);

    tft_draw_string(10, 28, "pH ODCZYT:", CLR_MUTED, CLR_BLACK, 2);
    tft_draw_string(10, 104, "NAPIECIE:", CLR_MUTED, CLR_BLACK, 1);
    tft_draw_string(10, 138, "TEMPERATURA:", CLR_MUTED, CLR_BLACK, 1);
    tft_draw_string(10, 172, "STATUS SIECI:", CLR_MUTED, CLR_BLACK, 1);
    tft_draw_string(150, 32, "HISTORIA (Ostatnie 60):", CLR_MUTED, CLR_BLACK, 1);
    tft_draw_string(10, 230, "Made by Wisnia", CLR_MUTED, CLR_BLACK, 1);
}

void drawPHValue() {
    tft_fill_rect(10, 48, 130, 36, CLR_BLACK);
    uint16_t col = phColor(smoothedPH);
    char buf[8];
    dtostrf(smoothedPH, 5, 2, buf);
    tft_draw_big_number(10, 50, buf, col);

    tft_fill_rect(10, 88, 130, 12, CLR_BLACK);
    tft_draw_string(10, 89, phLabel(smoothedPH), col, CLR_BLACK, 1);
}

void drawVoltage() {
    tft_fill_rect(10, 116, 130, 18, CLR_BLACK);
    char buf[12];
    snprintf(buf, sizeof(buf), "%.3f V", currentVoltage);
    tft_draw_string(10, 118, buf, CLR_WHITE, CLR_BLACK, 2);
}

void drawTemperature() {
    tft_fill_rect(10, 150, 130, 18, CLR_BLACK);
    char buf[16];
    if (currentTemp < -50.0f) {
        snprintf(buf, sizeof(buf), "---");
    } else {
        snprintf(buf, sizeof(buf), "%.1f C", currentTemp);
    }
    tft_draw_string(10, 152, buf, CLR_WHITE, CLR_BLACK, 2);
}

void drawAlarm() {
    if (alarmActive) {
        tft_fill_rect(140, 4, 50, 16, CLR_ORANGE);
        tft_draw_string(143, 8, "ALARM", CLR_BLACK, CLR_ORANGE, 1);
    } else {
        tft_fill_rect(140, 4, 50, 16, CLR_PANEL);
    }
}

void drawHistory() {
    int x0 = 150, y0 = 48, w = 164, h = 180;
    tft_fill_rect(x0, y0, w, h, CLR_PANEL);
    tft_draw_rect(x0, y0, w, h, CLR_MUTED);

    int yMid = y0 + h / 2;
    tft_draw_hline(x0 + 1, yMid, w - 2, CLR_DARKGRAY);

    int yLow  = y0 + h - 2 - (int)((phAlarmLow / 14.0f) * (h - 4));
    int yHigh = y0 + h - 2 - (int)((phAlarmHigh / 14.0f) * (h - 4));
    yLow  = constrain(yLow, y0 + 1, y0 + h - 2);
    yHigh = constrain(yHigh, y0 + 1, y0 + h - 2);

    tft_draw_hline(x0 + 1, yLow, w - 2, 0x4000);
    tft_draw_hline(x0 + 1, yHigh, w - 2, 0x4000);

    if (historyCount < 2) return;

    int count = historyCount;
    float xStep = (float)(w - 2) / (float)(HISTORY_SIZE - 1);
    int xOffset = (HISTORY_SIZE - count);

    for (int i = 1; i < count; i++) {
        int idxA = (historyIndex - count + i - 1 + HISTORY_SIZE) % HISTORY_SIZE;
        int idxB = (historyIndex - count + i     + HISTORY_SIZE) % HISTORY_SIZE;

        float phA = phHistory[idxA];
        float phB = phHistory[idxB];

        int yA = y0 + h - 2 - (int)((phA / 14.0f) * (h - 4));
        int yB = y0 + h - 2 - (int)((phB / 14.0f) * (h - 4));
        int xA = x0 + 1 + (int)((xOffset + i - 1) * xStep);
        int xB = x0 + 1 + (int)((xOffset + i    ) * xStep);

        yA = constrain(yA, y0 + 1, y0 + h - 2);
        yB = constrain(yB, y0 + 1, y0 + h - 2);

        tft_draw_line(xA, yA, xB, yB, phColor(phB));
    }

    tft_draw_string(x0 + w - 18, y0 + 2,     "14", CLR_MUTED, CLR_PANEL, 1);
    tft_draw_string(x0 + w - 12, yMid - 3,   "7",  CLR_MUTED, CLR_PANEL, 1);
    tft_draw_string(x0 + w - 12, y0 + h - 9, "0",  CLR_MUTED, CLR_PANEL, 1);
}

void drawWifi() {
    tft_fill_rect(10, 184, 130, 40, CLR_BLACK);
    if (wifiConnected) {
        tft_draw_string(10, 184, "IP:", CLR_MUTED, CLR_BLACK, 1);
        tft_draw_string(28, 184, wifiIP, CLR_GREEN, CLR_BLACK, 1);
        tft_draw_string(10, 198, "ph-monitor.local", CLR_MUTED, CLR_BLACK, 1);
    } else {
        tft_draw_string(10, 184, "WiFi: BRAK", CLR_MUTED, CLR_BLACK, 1);
    }
}

void drawUptime() {
    tft_fill_rect(200, 6, 110, 12, CLR_PANEL);
    char buf[24];
    uint32_t s = millis() / 1000UL;
    if (s < 3600) {
        snprintf(buf, sizeof(buf), "UP: %lus", (unsigned long)s);
    } else {
        snprintf(buf, sizeof(buf), "UP: %luh %02lum", (unsigned long)(s/3600), (unsigned long)((s%3600)/60));
    }
    tft_draw_string(200, 8, buf, CLR_MUTED, CLR_PANEL, 1);
}

String urlEncode(String str) {
    String encodedString = "";
    char c;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (isalnum(c)) {
            encodedString += c;
        } else if (c == ' ') {
            encodedString += "+";
        } else {
            char code0 = "0123456789ABCDEF"[(c >> 4) & 0xF];
            char code1 = "0123456789ABCDEF"[c & 0xF];
            encodedString += "%";
            encodedString += code0;
            encodedString += code1;
        }
    }
    return encodedString;
}

void sendPushoverNotification(String msg) {
    if (!pushoverEnabled || strlen(pushoverUser) == 0 || strlen(pushoverToken) == 0) return;

    struct PushoverData {
        String message;
        String user;
        String token;
    };
    PushoverData* data = new PushoverData();
    data->message = msg;
    data->user = String(pushoverUser);
    data->token = String(pushoverToken);

    xTaskCreate([](void* param) {
        PushoverData* d = (PushoverData*)param;
        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;
        if (http.begin(client, "https://api.pushover.net/1/messages.json")) {
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            String payload = "token=" + d->token + "&user=" + d->user + "&message=" + urlEncode(d->message);
            http.POST(payload);
            http.end();
        }
        delete d;
        vTaskDelete(NULL);
    }, "pushover_task", 8192, data, 1, NULL);
}

String buildHTML() {
    String html = R"rawhtml(
<!DOCTYPE html>
<html lang="pl">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>pH Monitor</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700&display=swap" rel="stylesheet">
<style>
  *{box-sizing:border-box;margin:0;padding:0}
  body{font-family:'Inter',system-ui,sans-serif;background:#0b0e18;color:#dde1ef;min-height:100vh;padding:1.2rem}
  header{display:flex;align-items:center;justify-content:space-between;margin-bottom:1.2rem}
  header h1{font-size:.85rem;font-weight:600;letter-spacing:.18em;color:#4a5070;text-transform:uppercase}
  #status{font-size:.7rem;color:#3a4060;display:flex;align-items:center;gap:.4rem}
  #dot{width:7px;height:7px;border-radius:50%;background:#22c55e;animation:pulse 2s infinite}
  @keyframes pulse{0%,100%{opacity:1}50%{opacity:.3}}
  .cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:.8rem;margin-bottom:1rem}
  .card{background:#131726;border:1px solid #1e2235;border-radius:14px;padding:1rem;position:relative;overflow:hidden;transition:border-color .3s}
  .card:hover{border-color:#2e3450}
  .card::before{content:'';position:absolute;inset:0;background:linear-gradient(135deg,rgba(255,255,255,.03),transparent);pointer-events:none}
  .lbl{font-size:.6rem;font-weight:600;letter-spacing:.12em;color:#3a4060;text-transform:uppercase;margin-bottom:.5rem}
  .val{font-size:2rem;font-weight:700;line-height:1;transition:color .4s}
  .unit{font-size:.75rem;color:#4a5070;margin-top:.3rem}
  .acid{color:#f87171} .neutral{color:#4ade80} .base{color:#60a5fa}
  .alarm-on{color:#fb923c} .alarm-ok{color:#4ade80}
  .chart-wrap{background:#131726;border:1px solid #1e2235;border-radius:14px;padding:1rem;margin-bottom:.8rem}
  .chart-title{font-size:.6rem;letter-spacing:.12em;color:#3a4060;text-transform:uppercase;margin-bottom:.6rem}
  canvas{display:block;width:100%;border-radius:8px}
  footer{text-align:center;font-size:.6rem;color:#2a2f45;margin-top:.5rem}
  .badge{display:inline-block;padding:.15rem .5rem;border-radius:999px;font-size:.65rem;font-weight:600}
  .badge-ok{background:#0f2e1a;color:#4ade80;border:1px solid #166534}
  .badge-alarm{background:#2e1a0f;color:#fb923c;border:1px solid #7c2d12;animation:blink .8s step-start infinite}
  @keyframes blink{50%{opacity:.4}}
  input[type="number"], input[type="text"] {
    width: 100%; background: #0b0e18; border: 1px solid #1e2235; color: #dde1ef;
    padding: 0.4rem; border-radius: 6px; margin-top: 0.25rem; font-family: inherit; font-size: 0.8rem;
  }
  input[type="number"]:focus, input[type="text"]:focus {
    border-color: #3b82f6; outline: none;
  }
  .form-group { margin-bottom: 0.8rem; }
  .btn-submit {
    background: #2563eb; border: none; color: white; padding: 0.5rem 1.2rem;
    border-radius: 6px; font-weight: 600; cursor: pointer; font-size: 0.75rem; transition: background 0.2s;
  }
  .btn-submit:hover { background: #1d4ed8; }
</style>
</head>
<body>
<header>
  <h1>&#x2627; pH Monitor</h1>
  <div id="status"><div id="dot"></div><span id="ts">--</span></div>
</header>
<div class="cards">
  <div class="card">
    <div class="lbl">Odczyt pH</div>
    <div class="val" id="ph-val">--</div>
    <div class="unit" id="ph-lbl">--</div>
  </div>
  <div class="card">
    <div class="lbl">Napięcie czujnika</div>
    <div class="val" id="volt-val" style="font-size:1.4rem">--</div>
    <div class="unit">wyjście PO</div>
  </div>
  <div class="card">
    <div class="lbl">Temperatura</div>
    <div class="val" id="temp-val" style="font-size:1.4rem">--</div>
    <div class="unit">DS18B20</div>
  </div>
  <div class="card">
    <div class="lbl">Alarm pH</div>
    <div class="val" id="alarm-val" style="font-size:1rem;padding-top:.4rem"></div>
    <div class="unit" id="alarm-limits">próg: -- pH</div>
  </div>
  <div class="card">
    <div class="lbl">Uptime</div>
    <div class="val" id="uptime-val" style="font-size:1.4rem">--</div>
    <div class="unit">od uruchomienia</div>
  </div>
</div>
<div class="chart-wrap">
  <div class="chart-title">Historia pH (ostatnie 60 pomiarów)</div>
  <canvas id="chart" height="140"></canvas>
</div>

<div class="card" style="grid-column: 1 / -1; margin-bottom: 1rem;">
  <div class="lbl" style="font-size: 0.75rem; margin-bottom: 1rem;">Ustawienia i Alerty</div>
  <form action="/save-config" method="POST" style="display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 1.5rem;">
    <div>
      <h3 style="font-size: 0.8rem; margin-bottom: 0.5rem; color: #4a5070; text-transform: uppercase; letter-spacing: 0.05em;">Progi Alarmowe pH</h3>
      <div class="form-group">
        <label style="font-size: 0.7rem; color: #4a5070;">Dolny próg alarmu</label>
        <input type="number" step="0.1" name="alarm_low" id="in-alarm-low" required>
      </div>
      <div class="form-group">
        <label style="font-size: 0.7rem; color: #4a5070;">Górny próg alarmu</label>
        <input type="number" step="0.1" name="alarm_high" id="in-alarm-high" required>
      </div>
    </div>
    <div>
      <h3 style="font-size: 0.8rem; margin-bottom: 0.5rem; color: #4a5070; text-transform: uppercase; letter-spacing: 0.05em;">Powiadomienia Pushover</h3>
      <div class="form-group" style="margin-top: 0.5rem; margin-bottom: 0.8rem;">
        <label style="font-size: 0.75rem; color: #dde1ef; display: flex; align-items: center; gap: 0.4rem; cursor: pointer;">
          <input type="checkbox" name="po_enabled" value="1" id="in-po-enabled"> Włącz powiadomienia
        </label>
      </div>
      <div class="form-group">
        <label style="font-size: 0.7rem; color: #4a5070;">Klucz użytkownika (User Key)</label>
        <input type="text" name="po_user" id="in-po-user" placeholder="Wklej User Key">
      </div>
      <div class="form-group">
        <label style="font-size: 0.7rem; color: #4a5070;">Token aplikacji (API Token)</label>
        <input type="text" name="po_token" id="in-po-token" placeholder="Wklej API Token">
      </div>
    </div>
    <div style="grid-column: 1 / -1; text-align: right; border-top: 1px solid #1e2235; padding-top: 1rem;">
      <button type="submit" class="btn-submit">Zapisz Ustawienia</button>
    </div>
  </form>
</div>

<div class="card" style="grid-column: 1 / -1; margin-bottom: 1rem;">
  <div class="lbl" style="font-size: 0.75rem; margin-bottom: 1rem;">Kalibracja Sondy pH</div>
  <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 1.5rem;">
    <div>
      <h3 style="font-size: 0.8rem; margin-bottom: 0.5rem; color: #4a5070; text-transform: uppercase; letter-spacing: 0.05em;">Parametry Kalibracji</h3>
      <p style="font-size: 0.75rem; margin-bottom: 0.4rem; color: #888;">Napięcie neutralne (pH 7.00): <strong id="val-neutral" style="color: #dde1ef;">--</strong> V</p>
      <p style="font-size: 0.75rem; margin-bottom: 0.4rem; color: #888;">Nachylenie (Slope): <strong id="val-slope" style="color: #dde1ef;">--</strong> V/pH</p>
      <p style="font-size: 0.75rem; color: #888;">Bieżące napięcie PO: <strong id="val-current-v" style="color: #4ade80;">--</strong> V</p>
    </div>
    <div style="display: flex; flex-direction: column; gap: 0.5rem; justify-content: center;">
      <button onclick="calibrate('neutral')" style="background: #10b981; border: none; color: white; padding: 0.5rem; border-radius: 6px; cursor: pointer; font-size: 0.7rem; font-weight: 600;">Kalibruj pH 7.00 (Neutralny)</button>
      <button onclick="calibrate('acid')" style="background: #f59e0b; border: none; color: white; padding: 0.5rem; border-radius: 6px; cursor: pointer; font-size: 0.7rem; font-weight: 600;">Kalibruj pH 4.01 (Kwaśny)</button>
      <button onclick="calibrate('base')" style="background: #3b82f6; border: none; color: white; padding: 0.5rem; border-radius: 6px; cursor: pointer; font-size: 0.7rem; font-weight: 600;">Kalibruj pH 9.18 (Zasadowy)</button>
      <button onclick="calibrate('reset')" style="background: #ef4444; border: none; color: white; padding: 0.5rem; border-radius: 6px; cursor: pointer; font-size: 0.7rem; font-weight: 600;">Resetuj kalibrację</button>
    </div>
  </div>
</div>

<footer>Made by Wisnia &bull; Odświeżanie: co 2s &bull; )rawhtml";
    html += String(wifiIP);
    html += R"rawhtml( &bull; <a href="/api" style="color:#2a2f45">/api JSON</a></footer>
<script>
let configLoaded = false;
let PH_LOW  = 6.0;
let PH_HIGH = 8.0;

const history = [];
const MAX_H   = 60;

function phColor(ph){
  if(ph < PH_LOW)  return '#f87171';
  if(ph > PH_HIGH) return '#60a5fa';
  return '#4ade80';
}

function fmtUptime(s){
  if(s<60)  return s+'s';
  if(s<3600) return Math.floor(s/60)+'m '+('0'+s%60).slice(-2)+'s';
  return Math.floor(s/3600)+'h '+('0'+Math.floor((s%3600)/60)).slice(-2)+'m';
}

function drawChart(){
  const cv = document.getElementById('chart');
  const dpr = window.devicePixelRatio||1;
  const W   = cv.parentElement.clientWidth - 32;
  const H   = 140;
  cv.width  = W * dpr;
  cv.height = H * dpr;
  cv.style.width  = W + 'px';
  cv.style.height = H + 'px';
  const cx = cv.getContext('2d');
  cx.scale(dpr, dpr);

  const p = {t:12,b:20,l:32,r:12};
  const iW = W - p.l - p.r;
  const iH = H - p.t - p.b;

  cx.fillStyle='#0e1120';
  cx.beginPath(); cx.roundRect(0,0,W,H,8); cx.fill();

  const marks=[0,2,4,6,7,8,10,12,14];
  marks.forEach(v=>{
    const y = p.t + iH - (v/14)*iH;
    cx.strokeStyle = v===7 ? '#2a3050' : '#151825';
    cx.lineWidth   = v===7 ? 1.5 : 1;
    cx.setLineDash(v===7?[4,4]:[]);
    cx.beginPath(); cx.moveTo(p.l,y); cx.lineTo(p.l+iW,y); cx.stroke();
    cx.setLineDash([]);
    cx.fillStyle='#2e3550';
    cx.font=`${8*dpr/dpr}px Inter,system-ui`;
    cx.textAlign='right';
    cx.fillText(v, p.l-4, y+3);
  });

  if(history.length < 2) return;
  const step = iW / (MAX_H-1);

  const grad = cx.createLinearGradient(0,p.t,0,p.t+iH);
  grad.addColorStop(0,'rgba(74,222,128,.15)');
  grad.addColorStop(1,'rgba(74,222,128,.01)');
  cx.beginPath();
  history.forEach((v,i)=>{
    const off = MAX_H - history.length;
    const x = p.l + (off+i)*step;
    const y = p.t + iH - (v/14)*iH;
    i===0 ? cx.moveTo(x,y) : cx.lineTo(x,y);
  });
  cx.lineTo(p.l+(MAX_H-1)*step, p.t+iH);
  cx.lineTo(p.l+(MAX_H-history.length)*step, p.t+iH);
  cx.closePath();
  cx.fillStyle=grad; cx.fill();

  for(let i=1;i<history.length;i++){
    const off=MAX_H-history.length;
    const xA=p.l+(off+i-1)*step, yA=p.t+iH-(history[i-1]/14)*iH;
    const xB=p.l+(off+i  )*step, yB=p.t+iH-(history[i  ]/14)*iH;
    cx.strokeStyle=phColor(history[i]);
    cx.lineWidth=2;
    cx.lineCap='round';
    cx.beginPath(); cx.moveTo(xA,yA); cx.lineTo(xB,yB); cx.stroke();
  }

  if(history.length){
    const lv=history[history.length-1];
    const off=MAX_H-history.length;
    const lx=p.l+(off+history.length-1)*step;
    const ly=p.t+iH-(lv/14)*iH;
    cx.beginPath(); cx.arc(lx,ly,4,0,2*Math.PI);
    cx.fillStyle=phColor(lv); cx.fill();
  }
}

function setPhColor(el, ph){
  el.classList.remove('acid','neutral','base');
  if(ph < PH_LOW)  el.classList.add('acid');
  else if(ph > PH_HIGH) el.classList.add('base');
  else el.classList.add('neutral');
}

function calibrate(type) {
  if(!confirm('Czy na pewno chcesz przeprowadzic kalibracje dla: ' + type + '?')) return;
  fetch('/api/calibrate', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: 'type=' + type
  })
  .then(r => r.json())
  .then(d => {
    if(d.status === 'ok') {
      alert('Kalibracja zakonczona sukcesem!');
      refresh();
    } else {
      alert('Blad kalibracji: ' + d.message);
    }
  })
  .catch(() => alert('Blad polaczenia z API!'));
}

function refresh(){
  fetch('/api')
    .then(r=>r.json())
    .then(d=>{
      PH_LOW = d.alarm_low;
      PH_HIGH = d.alarm_high;

      const phEl = document.getElementById('ph-val');
      phEl.textContent = d.ph.toFixed(2);
      setPhColor(phEl, d.ph);
      document.getElementById('ph-lbl').textContent = d.label;
      document.getElementById('volt-val').textContent = d.voltage.toFixed(3)+' V';
      
      if (d.temp === null || d.temp === undefined) {
        document.getElementById('temp-val').textContent = "--";
      } else {
        document.getElementById('temp-val').textContent = d.temp.toFixed(1) + ' °C';
      }

      const alEl = document.getElementById('alarm-val');
      alEl.innerHTML = d.alarm
        ? '<span class="badge badge-alarm">⚠ ALARM</span>'
        : '<span class="badge badge-ok">✓ OK</span>';
      
      document.getElementById('alarm-limits').textContent = 'prog: ' + PH_LOW.toFixed(1) + ' – ' + PH_HIGH.toFixed(1) + ' pH';
      document.getElementById('uptime-val').textContent = fmtUptime(d.uptime);
      document.getElementById('ts').textContent = 'aktualizacja: '+new Date().toLocaleTimeString('pl');
      document.getElementById('dot').style.background = '#4ade80';
      setTimeout(()=>document.getElementById('dot').style.background='#22c55e', 300);

      if (!configLoaded) {
        document.getElementById('in-alarm-low').value = d.alarm_low;
        document.getElementById('in-alarm-high').value = d.alarm_high;
        document.getElementById('in-po-enabled').checked = d.po_enabled;
        document.getElementById('in-po-user').value = d.po_user;
        document.getElementById('in-po-token').value = d.po_token;
        configLoaded = true;
      }

      document.getElementById('val-neutral').textContent = d.ph_neutral.toFixed(4);
      document.getElementById('val-slope').textContent = d.ph_slope.toFixed(4);
      document.getElementById('val-current-v').textContent = d.voltage.toFixed(4);

      history.push(d.ph);
      if(history.length > MAX_H) history.shift();
      drawChart();
    })
    .catch(()=>{
      document.getElementById('dot').style.background='#f87171';
    });
}

refresh();
setInterval(refresh, 2000);
window.addEventListener('resize', drawChart);
</script>
</body>
</html>
)rawhtml";
    return html;
}

String buildJSON() {
    String tempStr = (currentTemp < -50.0f) ? "null" : String(currentTemp, 1);
    return "{\"ph\":" + String(smoothedPH,3) +
           ",\"voltage\":" + String(currentVoltage,4) +
           ",\"temp\":" + tempStr +
           ",\"alarm\":" + (alarmActive ? "true" : "false") +
           ",\"label\":\"" + phLabel(smoothedPH) + "\"" +
           ",\"uptime\":" + String(millis()/1000) +
           ",\"ph_neutral\":" + String(phNeutralVoltage,4) +
           ",\"ph_slope\":" + String(phSlope,4) +
           ",\"alarm_low\":" + String(phAlarmLow,1) +
           ",\"alarm_high\":" + String(phAlarmHigh,1) +
           ",\"po_enabled\":" + (pushoverEnabled ? "true" : "false") +
           ",\"po_user\":\"" + String(pushoverUser) + "\"" +
           ",\"po_token\":\"" + String(pushoverToken) + "\"}";
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    pinMode(PIN_PH_ALARM, INPUT);

    Preferences prefs;
    prefs.begin("ph-monitor", true);
    phNeutralVoltage = prefs.getFloat("ph_neutral", 2.5f);
    phSlope          = prefs.getFloat("ph_slope", 0.18f);
    phAlarmLow       = prefs.getFloat("alarm_low", 6.0f);
    phAlarmHigh      = prefs.getFloat("alarm_high", 8.0f);
    pushoverEnabled  = prefs.getBool("po_enabled", false);
    prefs.getString("po_user", "").toCharArray(pushoverUser, 64);
    prefs.getString("po_token", "").toCharArray(pushoverToken, 64);
    prefs.end();

    tft_init();

    tft_fill_screen(CLR_RED);   delay(400);
    tft_fill_screen(CLR_GREEN); delay(400);
    tft_fill_screen(CLR_BLUE);  delay(400);

    drawStaticUI();

    tft_draw_string(10, 184, "Laczenie WiFi...", CLR_MUTED, CLR_BLACK, 1);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 30) {
        delay(500); tries++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        WiFi.localIP().toString().toCharArray(wifiIP, 16);
        if (MDNS.begin(HOSTNAME)) {
            MDNS.addService("http", "tcp", 80);
        }
    }

    sensors.begin();
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
        req->send(200, "text/html", buildHTML());
    });
    server.on("/api", HTTP_GET, [](AsyncWebServerRequest *req){
        AsyncWebServerResponse *r = req->beginResponse(200, "application/json", buildJSON());
        r->addHeader("Access-Control-Allow-Origin","*");
        req->send(r);
    });
    server.on("/save-config", HTTP_POST, [](AsyncWebServerRequest *req){
        if(req->hasParam("alarm_low", true)) phAlarmLow = req->getParam("alarm_low", true)->value().toFloat();
        if(req->hasParam("alarm_high", true)) phAlarmHigh = req->getParam("alarm_high", true)->value().toFloat();
        pushoverEnabled = req->hasParam("po_enabled", true);
        if(req->hasParam("po_user", true)) {
            req->getParam("po_user", true)->value().toCharArray(pushoverUser, 64);
        } else {
            pushoverUser[0] = '\0';
        }
        if(req->hasParam("po_token", true)) {
            req->getParam("po_token", true)->value().toCharArray(pushoverToken, 64);
        } else {
            pushoverToken[0] = '\0';
        }
        
        Preferences prefs;
        prefs.begin("ph-monitor", false);
        prefs.putFloat("alarm_low", phAlarmLow);
        prefs.putFloat("alarm_high", phAlarmHigh);
        prefs.putBool("po_enabled", pushoverEnabled);
        prefs.putString("po_user", String(pushoverUser));
        prefs.putString("po_token", String(pushoverToken));
        prefs.end();
        
        req->redirect("/");
    });
    server.on("/api/calibrate", HTTP_POST, [](AsyncWebServerRequest *req){
        if (req->hasParam("type", true)) {
            String type = req->getParam("type", true)->value();
            float currentV = currentVoltage;
            
            if (type == "neutral") {
                phNeutralVoltage = currentV;
            } else if (type == "acid") {
                float diff = abs(phNeutralVoltage - currentV);
                if (diff > 0.1f) {
                    phSlope = diff / 2.99f;
                }
            } else if (type == "base") {
                float diff = abs(phNeutralVoltage - currentV);
                if (diff > 0.1f) {
                    phSlope = diff / 2.18f;
                }
            } else if (type == "reset") {
                phNeutralVoltage = 2.5f;
                phSlope = 0.18f;
            }
            
            Preferences prefs;
            prefs.begin("ph-monitor", false);
            prefs.putFloat("ph_neutral", phNeutralVoltage);
            prefs.putFloat("ph_slope", phSlope);
            prefs.end();
            
            currentPH = readPH();
            smoothedPH = currentPH;
            
            req->send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
            req->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing type\"}");
        }
    });
    server.onNotFound([](AsyncWebServerRequest *req){
        req->send(404,"text/plain","Nie znaleziono");
    });
    server.begin();

    currentPH = readPH();
    smoothedPH = currentPH;
    alarmActive = (smoothedPH < phAlarmLow || smoothedPH > phAlarmHigh);

    drawWifi();
}

void loop() {
    unsigned long now = millis();
    
    bool currentWifiStatus = (WiFi.status() == WL_CONNECTED);
    if (currentWifiStatus != wifiConnected) {
        wifiConnected = currentWifiStatus;
        if (wifiConnected) {
            WiFi.localIP().toString().toCharArray(wifiIP, 16);
            MDNS.begin(HOSTNAME);
            MDNS.addService("http", "tcp", 80);
        } else {
            strcpy(wifiIP, "---");
        }
        drawWifi();
    }

    if (now - lastSampleTime >= SAMPLE_INTERVAL_MS) {
        lastSampleTime = now;
        
        currentTemp = sensors.getTempCByIndex(0);
        sensors.requestTemperatures();

        currentPH  = readPH();
        smoothedPH = applyEMA(smoothedPH, currentPH);
        
        bool newAlarmActive = (smoothedPH < phAlarmLow || smoothedPH > phAlarmHigh);
        if (newAlarmActive != alarmActive) {
            alarmActive = newAlarmActive;
            if (alarmActive) {
                char msg[64];
                snprintf(msg, sizeof(msg), "Alarm! pH wynosi %.2f (progi: %.1f - %.1f)", smoothedPH, phAlarmLow, phAlarmHigh);
                sendPushoverNotification(String(msg));
            } else {
                char msg[64];
                snprintf(msg, sizeof(msg), "Norma! pH wrocilo do zakresu: %.2f", smoothedPH);
                sendPushoverNotification(String(msg));
            }
        }

        updateHistory(smoothedPH);
        drawPHValue();
        drawVoltage();
        drawTemperature();
        drawAlarm();
        drawHistory();
        drawUptime();
    }
}
