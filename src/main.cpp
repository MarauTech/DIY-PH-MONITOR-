#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

#define CLR_BLACK    0x0000
#define CLR_WHITE    0xFFFF
#define CLR_RED      0xF800
#define CLR_GREEN    0x07E0
#define CLR_BLUE     0x001F
#define CLR_ORANGE   0xFD20
#define CLR_YELLOW   0xFFE0
#define CLR_PANEL    0x1082
#define CLR_MUTED    0x7BEF
#define CLR_DARKGRAY 0x2104
#define CLR_LIME     0x4FE0

static inline void tft_cs_low()  { digitalWrite(PIN_CS, LOW);  }
static inline void tft_cs_high() { digitalWrite(PIN_CS, HIGH); }
static inline void tft_dc_cmd()  { digitalWrite(PIN_DC, LOW);  }
static inline void tft_dc_data() { digitalWrite(PIN_DC, HIGH); }

void tft_cmd(uint8_t cmd) {
    tft_dc_cmd(); tft_cs_low();
    SPI.transfer(cmd);
    tft_cs_high();
}
void tft_data8(uint8_t d) {
    tft_dc_data(); tft_cs_low();
    SPI.transfer(d);
    tft_cs_high();
}
void tft_data16(uint16_t d) {
    tft_dc_data(); tft_cs_low();
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
    tft_cmd(0x36); tft_data8(0xA0);
    tft_cmd(0x3A); tft_data8(0x55);
    tft_cmd(0xB2); tft_data8(0x0C); tft_data8(0x0C); tft_data8(0x00); tft_data8(0x33); tft_data8(0x33);
    tft_cmd(0xB7); tft_data8(0x35);
    tft_cmd(0xBB); tft_data8(0x19);
    tft_cmd(0xC0); tft_data8(0x2C);
    tft_cmd(0xC2); tft_data8(0x01);
    tft_cmd(0xC3); tft_data8(0x12);
    tft_cmd(0xC4); tft_data8(0x20);
    tft_cmd(0xC6); tft_data8(0x0F);
    tft_cmd(0xD0); tft_data8(0xA4); tft_data8(0xA1);
    tft_cmd(0x21);
    tft_cmd(0x29); delay(100);
}
void tft_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    tft_cmd(0x2A); tft_data8(x0>>8); tft_data8(x0&0xFF); tft_data8(x1>>8); tft_data8(x1&0xFF);
    tft_cmd(0x2B); tft_data8(y0>>8); tft_data8(y0&0xFF); tft_data8(y1>>8); tft_data8(y1&0xFF);
    tft_cmd(0x2C);
}
void tft_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (w == 0 || h == 0) return;
    tft_set_window(x, y, x+w-1, y+h-1);
    uint32_t n = (uint32_t)w * h;
    tft_dc_data(); tft_cs_low();
    for (uint32_t i = 0; i < n; i++) SPI.transfer16(color);
    tft_cs_high();
}
void tft_fill_screen(uint16_t c) { tft_fill_rect(0, 0, 320, 240, c); }
void tft_draw_pixel(uint16_t x, uint16_t y, uint16_t c) { tft_set_window(x,y,x,y); tft_data16(c); }
void tft_draw_hline(uint16_t x, uint16_t y, uint16_t w, uint16_t c) { tft_fill_rect(x,y,w,1,c); }
void tft_draw_vline(uint16_t x, uint16_t y, uint16_t h, uint16_t c) { tft_fill_rect(x,y,1,h,c); }
void tft_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c) {
    tft_draw_hline(x,y,w,c); tft_draw_hline(x,y+h-1,w,c);
    tft_draw_vline(x,y,h,c); tft_draw_vline(x+w-1,y,h,c);
}
void tft_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t c) {
    int16_t dx=abs(x1-x0), dy=abs(y1-y0), sx=x0<x1?1:-1, sy=y0<y1?1:-1, err=dx-dy;
    while(true) {
        if(x0>=0&&x0<240&&y0>=0&&y0<320) tft_draw_pixel(x0,y0,c);
        if(x0==x1&&y0==y1) break;
        int16_t e2=2*err;
        if(e2>-dy){err-=dy;x0+=sx;}
        if(e2<dx){err+=dx;y0+=sy;}
    }
}

static const uint8_t font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00},{0x00,0x00,0x5F,0x00,0x00},{0x00,0x07,0x00,0x07,0x00},
    {0x14,0x7F,0x14,0x7F,0x14},{0x24,0x2A,0x7F,0x2A,0x12},{0x23,0x13,0x08,0x64,0x62},
    {0x36,0x49,0x55,0x22,0x50},{0x00,0x05,0x03,0x00,0x00},{0x00,0x1C,0x22,0x41,0x00},
    {0x00,0x41,0x22,0x1C,0x00},{0x14,0x08,0x3E,0x08,0x14},{0x08,0x08,0x3E,0x08,0x08},
    {0x00,0x50,0x30,0x00,0x00},{0x08,0x08,0x08,0x08,0x08},{0x00,0x60,0x60,0x00,0x00},
    {0x20,0x10,0x08,0x04,0x02},{0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},{0x18,0x14,0x12,0x7F,0x10},
    {0x27,0x45,0x45,0x45,0x39},{0x3C,0x4A,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1E},{0x00,0x36,0x36,0x00,0x00},
    {0x00,0x56,0x36,0x00,0x00},{0x08,0x14,0x22,0x41,0x00},{0x14,0x14,0x14,0x14,0x14},
    {0x00,0x41,0x22,0x14,0x08},{0x02,0x01,0x51,0x09,0x06},{0x32,0x49,0x79,0x41,0x3E},
    {0x7E,0x11,0x11,0x11,0x7E},{0x7F,0x49,0x49,0x49,0x36},{0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x22,0x1C},{0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x49,0x49,0x7A},{0x7F,0x08,0x08,0x08,0x7F},{0x00,0x41,0x7F,0x41,0x00},
    {0x20,0x40,0x41,0x3F,0x01},{0x7F,0x08,0x14,0x22,0x41},{0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x04,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},{0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06},{0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7F,0x01,0x01},{0x3F,0x40,0x40,0x40,0x3F},
    {0x1F,0x20,0x40,0x20,0x1F},{0x3F,0x40,0x38,0x40,0x3F},{0x63,0x14,0x08,0x14,0x63},
    {0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43},{0x00,0x7F,0x41,0x41,0x00},
    {0x02,0x04,0x08,0x10,0x20},{0x00,0x41,0x41,0x7F,0x00},{0x04,0x02,0x01,0x02,0x04},
    {0x40,0x40,0x40,0x40,0x40},{0x00,0x01,0x02,0x04,0x00},{0x20,0x54,0x54,0x54,0x78},
    {0x7F,0x48,0x44,0x44,0x38},{0x38,0x44,0x44,0x44,0x20},{0x38,0x44,0x44,0x48,0x7F},
    {0x38,0x54,0x54,0x54,0x18},{0x08,0x7E,0x09,0x01,0x02},{0x0C,0x52,0x52,0x52,0x3E},
    {0x7F,0x08,0x04,0x04,0x78},{0x00,0x44,0x7D,0x40,0x00},{0x20,0x40,0x44,0x3D,0x00},
    {0x7F,0x10,0x28,0x44,0x00},{0x00,0x41,0x7F,0x40,0x00},{0x7C,0x04,0x18,0x04,0x78},
    {0x7C,0x08,0x04,0x04,0x78},{0x38,0x44,0x44,0x44,0x38},{0x7C,0x14,0x14,0x14,0x08},
    {0x08,0x14,0x14,0x18,0x7C},{0x7C,0x08,0x04,0x04,0x08},{0x48,0x54,0x54,0x54,0x20},
    {0x04,0x3F,0x44,0x40,0x20},{0x3C,0x40,0x40,0x20,0x7C},{0x1C,0x20,0x40,0x20,0x1C},
    {0x3C,0x40,0x30,0x40,0x3C},{0x44,0x28,0x10,0x28,0x44},{0x0C,0x50,0x50,0x50,0x3C},
    {0x44,0x64,0x54,0x4C,0x44},{0x00,0x08,0x36,0x41,0x00},{0x00,0x00,0x7F,0x00,0x00},
    {0x00,0x41,0x36,0x08,0x00},{0x10,0x08,0x08,0x10,0x08},
};

void tft_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale) {
    if (c < 32 || c > 126) c = '?';
    const uint8_t *g = font5x7[c-32];
    for (uint8_t col=0; col<5; col++) {
        uint8_t line = g[col];
        for (uint8_t row=0; row<7; row++) {
            if (line & (1<<row)) tft_fill_rect(x+col*scale, y+row*scale, scale, scale, fg);
            else if (bg != fg)   tft_fill_rect(x+col*scale, y+row*scale, scale, scale, bg);
        }
    }
}
uint16_t tft_draw_string(uint16_t x, uint16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale) {
    uint16_t cx = x;
    while (*s) { tft_draw_char(cx, y, *s++, fg, bg, scale); cx += (5+1)*scale; }
    return cx;
}
void tft_draw_big(uint16_t x, uint16_t y, const char *s, uint16_t fg) {
    while (*s) { tft_draw_char(x, y, *s++, fg, CLR_BLACK, 6); x += (5+1)*6; }
}

Preferences prefs;
AsyncWebServer server(80);

OneWire oneWire(PIN_DS18B20);
DallasTemperature sensors(&oneWire);

float currentPH       = 7.0f;
float currentTemp     = 0.0f;
float currentVoltage  = 0.0f;
bool  alarmActive     = false;
bool  wifiConnected   = false;
char  wifiIP[16]      = "---";
bool  alarmMuted      = false;

float phAlarmLow  = PH_ALARM_LOW_DEFAULT;
float phAlarmHigh = PH_ALARM_HIGH_DEFAULT;
float phNeutralV  = PH_NEUTRAL_VOLTAGE;
float phSlope     = PH_SLOPE;

char  pushoverToken[48] = PUSHOVER_TOKEN;
char  pushoverUser[48]  = PUSHOVER_USER;
bool  pushoverEnabled   = false;

struct Sample { float ph; uint32_t ts; };
Sample phHistory[HISTORY_SIZE];
int    historyIndex = 0;
int    historyCount = 0;

unsigned long lastSampleTime  = 0;
unsigned long lastAlarmBeep   = 0;
bool          alarmBlinkState = false;
unsigned long lastBlinkTime   = 0;
bool          pushoverSent    = false;

void saveSettings() {
    prefs.begin("ph", false);
    prefs.putFloat("alLow",   phAlarmLow);
    prefs.putFloat("alHigh",  phAlarmHigh);
    prefs.putFloat("neutV",   phNeutralV);
    prefs.putFloat("slope",   phSlope);
    prefs.putBool ("poEn",    pushoverEnabled);
    prefs.putString("poTok",  pushoverToken);
    prefs.putString("poUsr",  pushoverUser);
    prefs.end();
}

void loadSettings() {
    prefs.begin("ph", true);
    phAlarmLow      = prefs.getFloat("alLow",  PH_ALARM_LOW_DEFAULT);
    phAlarmHigh     = prefs.getFloat("alHigh", PH_ALARM_HIGH_DEFAULT);
    phNeutralV      = prefs.getFloat("neutV",  PH_NEUTRAL_VOLTAGE);
    phSlope         = prefs.getFloat("slope",  PH_SLOPE);
    pushoverEnabled = prefs.getBool ("poEn",   false);
    String tok = prefs.getString("poTok", PUSHOVER_TOKEN);
    String usr = prefs.getString("poUsr", PUSHOVER_USER);
    tok.toCharArray(pushoverToken, sizeof(pushoverToken));
    usr.toCharArray(pushoverUser,  sizeof(pushoverUser));
    prefs.end();
}

void saveHistory() {
    prefs.begin("hist", false);
    prefs.putInt("cnt", historyCount);
    prefs.putInt("idx", historyIndex);
    prefs.putBytes("data", phHistory, sizeof(float) * historyCount);
    prefs.end();
}

void loadHistory() {
    prefs.begin("hist", true);
    historyCount = prefs.getInt("cnt", 0);
    historyIndex = prefs.getInt("idx", 0);
    if (historyCount > 0) {
        float tmp[HISTORY_SIZE];
        prefs.getBytes("data", tmp, sizeof(float) * historyCount);
        for (int i = 0; i < historyCount; i++) phHistory[i].ph = tmp[i];
    }
    prefs.end();
}

void sendPushover(const char *msg) {
    if (!pushoverEnabled || strlen(pushoverToken) == 0 || strlen(pushoverUser) == 0) return;
    WiFiClientSecure client;
    client.setInsecure();
    if (!client.connect("api.pushover.net", 443)) return;
    String body = "token=";
    body += pushoverToken;
    body += "&user=";
    body += pushoverUser;
    body += "&title=pH+Monitor&message=";
    body += msg;
    String req = "POST /1/messages.json HTTP/1.1\r\n";
    req += "Host: api.pushover.net\r\n";
    req += "Content-Type: application/x-www-form-urlencoded\r\n";
    req += "Content-Length: " + String(body.length()) + "\r\n";
    req += "Connection: close\r\n\r\n";
    req += body;
    client.print(req);
    delay(500);
    client.stop();
}

float readPH() {
    long sum = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) { sum += analogRead(PIN_PH_ANALOG); delay(10); }
    float raw  = sum / (float)SAMPLE_COUNT;
    float vADC = raw * ADC_VREF / ADC_RESOLUTION;
    float vPO  = vADC / VOLTAGE_DIVIDER;
    currentVoltage = vPO;
    return constrain(7.0f + (phNeutralV - vPO) / phSlope, 0.0f, 14.0f);
}

float readTemperature() {
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
    if (temp == DEVICE_DISCONNECTED_C) {
        return 0.0f;
    }
    return temp;
}

void updateHistory(float ph) {
    phHistory[historyIndex].ph = ph;
    phHistory[historyIndex].ts = millis() / 1000;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    if (historyCount < HISTORY_SIZE) historyCount++;
}

uint16_t phColor(float ph) {
    if (ph < phAlarmLow)  return CLR_RED;
    if (ph > phAlarmHigh) return CLR_BLUE;
    return CLR_LIME;
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

void drawPHBar() {
    uint16_t bx = 5, by = 215, bw = 310, bh = 13;
    tft_draw_rect(bx, by, bw, bh, CLR_WHITE);
    int segments = 14;
    for (int i = 0; i < segments; i++) {
        float seg_ph = i + 0.5f;
        uint16_t col;
        if      (seg_ph < 4)  col = 0xF800;
        else if (seg_ph < 6)  col = 0xFC00;
        else if (seg_ph < 7)  col = 0xFFE0;
        else if (seg_ph < 8)  col = 0x07E0;
        else if (seg_ph < 10) col = 0x07FF;
        else                  col = 0x001F;
        uint16_t sx = bx + 1 + (uint16_t)(i * (float)(bw-2) / segments);
        uint16_t sw = (uint16_t)((bw-2) / segments) - 1;
        tft_fill_rect(sx, by+1, sw, bh-2, col);
    }
}

void drawStaticUI() {
    tft_fill_screen(CLR_BLACK);
    tft_fill_rect(0, 0, 320, 24, CLR_PANEL);
    tft_draw_string(110, 7, "PH MONITOR", CLR_WHITE, CLR_PANEL, 2);
    tft_draw_hline(0, 24, 320, CLR_WHITE);
}

void drawPHValue(bool alarmBlink) {
    tft_fill_rect(0, 26, 320, 190, CLR_BLACK);
    
    uint16_t phCol = alarmBlink ? CLR_ORANGE : phColor(currentPH);
    char phBuf[8];
    dtostrf(currentPH, 5, 2, phBuf);
    
    tft_draw_string(10, 40, "PH", CLR_WHITE, CLR_BLACK, 2);
    tft_draw_string(10, 70, phBuf, phCol, CLR_BLACK, 4);
    
    char tempBuf[8];
    snprintf(tempBuf, sizeof(tempBuf), "%.2f", currentTemp);
    
    tft_draw_string(10, 130, "TEMP.", CLR_WHITE, CLR_BLACK, 2);
    tft_draw_string(10, 160, tempBuf, CLR_LIME, CLR_BLACK, 4);
    
    drawPHBar();
}

void drawVoltage() {}

void drawAlarmBadge(bool blink) {}

void drawHistory() {}

void drawWifi() {
    tft_fill_rect(0, 229, 320, 11, CLR_BLACK);
    char ipStr[32];
    snprintf(ipStr, sizeof(ipStr), "IP: %s", wifiIP);
    tft_draw_string(10, 230, ipStr, CLR_WHITE, CLR_BLACK, 1);
}

void drawUptime() {}

void buzzerBeep(int freq, int dur) {
    ledcSetup(1, 10000, 8);
    ledcAttachPin(PIN_BUZZER, 1);
    ledcWrite(1, 255);
    delay(dur);
    ledcWrite(1, 0);
    ledcDetachPin(PIN_BUZZER);
}

String buildCSV() {
    String csv = "index,ph,uptime_s\n";
    for (int i = 0; i < historyCount; i++) {
        int idx = (historyIndex - historyCount + i + HISTORY_SIZE) % HISTORY_SIZE;
        csv += String(i) + "," + String(phHistory[idx].ph, 3) + "," + String(phHistory[idx].ts) + "\n";
    }
    return csv;
}

String buildJSON() {
    return "{\"ph\":" + String(currentPH,3) +
           ",\"voltage\":" + String(currentVoltage,4) +
           ",\"alarm\":" + (alarmActive?"true":"false") +
           ",\"label\":\"" + phLabel(currentPH) + "\"" +
           ",\"uptime\":" + String(millis()/1000) +
           ",\"alarmLow\":" + String(phAlarmLow,1) +
           ",\"alarmHigh\":" + String(phAlarmHigh,1) + "}";
}

String buildHistJSON(int minutes) {
    int maxSamples = (minutes * 60000) / SAMPLE_INTERVAL_MS;
    if (maxSamples > historyCount) maxSamples = historyCount;
    String j = "[";
    for (int i = 0; i < maxSamples; i++) {
        int idx = (historyIndex - maxSamples + i + HISTORY_SIZE) % HISTORY_SIZE;
        if (i > 0) j += ",";
        j += "{\"ph\":" + String(phHistory[idx].ph,2) + ",\"ts\":" + String(phHistory[idx].ts) + "}";
    }
    j += "]";
    return j;
}

String buildHTML() {
    String html = R"xxx(<!DOCTYPE html><html lang="pl"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>pH Monitor</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,sans-serif;background:#0a0c12;color:#ddd;padding:1rem;max-width:600px;margin:auto}
h1{font-size:.9rem;color:#555;letter-spacing:.15em;margin-bottom:1rem}
.cards{display:grid;grid-template-columns:repeat(3,1fr);gap:.6rem;margin-bottom:1rem}
.card{background:#13161f;border:1px solid #1e2130;border-radius:10px;padding:.8rem}
.lbl{font-size:.6rem;color:#444;text-transform:uppercase;letter-spacing:.1em;margin-bottom:.3rem}
.val{font-size:1.9rem;font-weight:700;line-height:1}
.sub{font-size:.65rem;color:#555;margin-top:.3rem}
.acid{color:#f44}.neutral{color:#4e8}.base{color:#46f}
.alarm-card{border-color:#f80}
section{background:#13161f;border:1px solid #1e2130;border-radius:10px;padding:.8rem;margin-bottom:.8rem}
section h2{font-size:.7rem;color:#555;letter-spacing:.1em;margin-bottom:.6rem}
canvas{width:100%;height:140px;display:block;border-radius:6px}
.tabs{display:flex;gap:.4rem;margin-bottom:.6rem}
.tab{padding:.3rem .7rem;border-radius:6px;font-size:.7rem;cursor:pointer;border:1px solid #2a2d3a;background:#0a0c12;color:#888}
.tab.active{background:#1e2130;color:#ddd;border-color:#4e8}
input[type=number],input[type=text]{width:100%;padding:.4rem;background:#0a0c12;border:1px solid #2a2d3a;border-radius:6px;color:#ddd;font-size:.8rem;margin-bottom:.4rem}
input[type=number]:focus,input[type=text]:focus{outline:none;border-color:#4e8}
label{font-size:.7rem;color:#666;display:block;margin-bottom:.2rem}
.row{display:grid;grid-template-columns:1fr 1fr;gap:.5rem}
btn,button{display:inline-block;padding:.4rem .9rem;border-radius:6px;font-size:.75rem;cursor:pointer;border:none}
.btn-g{background:#1a3a2a;color:#4e8}
.btn-r{background:#3a1a1a;color:#f44}
.btn-b{background:#1a1a3a;color:#46f}
.btn-o{background:#3a2a1a;color:#f80}
.btns{display:flex;gap:.5rem;flex-wrap:wrap;margin-top:.4rem}
.status{font-size:.65rem;color:#4e8;margin-top:.3rem;min-height:1rem}
.err{color:#f44}
.po-toggle{display:flex;align-items:center;gap:.5rem;margin-bottom:.4rem}
.po-toggle input{width:auto;margin:0}
</style></head><body>
<h1>PH MONITOR</h1>
<div class="cards">
  <div class="card" id="ph-card"><div class="lbl">Odczyt pH</div><div class="val" id="ph-val">--</div><div class="sub" id="ph-lbl">--</div></div>
  <div class="card"><div class="lbl">Napi&#x119;cie PO</div><div class="val" style="font-size:1.2rem" id="volt-val">--</div><div class="sub">wyj&#x15B;cie czujnika</div></div>
  <div class="card" id="alarm-card"><div class="lbl">Alarm</div><div class="val" style="font-size:1.2rem" id="alarm-val">--</div><div class="sub" id="alarm-range">--</div></div>
</div>

<section>
  <h2>HISTORIA</h2>
  <div class="tabs">
    <div class="tab active" onclick="setRange(60,this)">1h</div>
    <div class="tab" onclick="setRange(360,this)">6h</div>
    <div class="tab" onclick="setRange(1440,this)">24h</div>
  </div>
  <canvas id="chart"></canvas>
</section>

<section>
  <h2>PROGI ALARMU</h2>
  <div class="row">
    <div><label>pH min</label><input type="number" id="al-low" step="0.1" min="0" max="14" value="6.0"></div>
    <div><label>pH max</label><input type="number" id="al-high" step="0.1" min="0" max="14" value="8.0"></div>
  </div>
  <div class="btns">
    <button class="btn-g" onclick="saveAlarms()">Zapisz progi</button>
    <button class="btn-r" onclick="resetHistory()">Resetuj histori&#x119;</button>
    <button class="btn-b" onclick="exportCSV()">Eksport CSV</button>
  </div>
  <div class="status" id="alarm-status"></div>
</section>

<section>
  <h2>KALIBRACJA</h2>
  <div class="row">
    <div><label>Warto&#x15B;&#x107; pH buforu</label><input type="number" id="cal-ph" step="0.1" min="0" max="14" value="7.0"></div>
    <div><label>Napi&#x119;cie PO (V)</label><input type="number" id="cal-v" step="0.001" min="0" max="5"></div>
  </div>
  <p style="font-size:.65rem;color:#555;margin-bottom:.5rem">W&#x142;&#xF3;&#x17C; elektrod&#x119; do buforu, poczekaj 2 min, przepisz napi&#x119;cie z pola &quot;Napi&#x119;cie PO&quot; powy&#x17C;ej, wpisz znane pH buforu i kliknij Kalibruj.</p>
  <div class="btns">
    <button class="btn-g" onclick="calibrate()">Kalibruj</button>
  </div>
  <div class="status" id="cal-status"></div>
</section>

<section>
  <h2>POWIADOMIENIA PUSHOVER</h2>
  <div class="po-toggle"><input type="checkbox" id="po-en"><label for="po-en" style="margin:0;color:#aaa">W&#x142;&#x105;cz Pushover</label></div>
  <label>App Token</label><input type="text" id="po-tok" placeholder="a1b2c3...">
  <label>User Key</label><input type="text" id="po-usr" placeholder="u1v2w3...">
  <div class="btns">
    <button class="btn-g" onclick="savePushover()">Zapisz</button>
    <button class="btn-o" onclick="testPushover()">Test</button>
  </div>
  <div class="status" id="po-status"></div>
</section>

<script>
let rangeMin = 60;
let chartData = [];

function setRange(m, el) {
  rangeMin = m;
  document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
  el.classList.add('active');
  fetchHistory();
}

async function fetchStatus() {
  try {
    const r = await fetch('/api');
    const d = await r.json();
    const phEl = document.getElementById('ph-val');
    phEl.textContent = d.ph.toFixed(2);
    phEl.className = 'val ' + (d.ph < d.alarmLow ? 'acid' : d.ph > d.alarmHigh ? 'base' : 'neutral');
    document.getElementById('ph-lbl').textContent = d.label;
    document.getElementById('volt-val').textContent = d.voltage.toFixed(3) + ' V';
    const ac = document.getElementById('alarm-card');
    const av = document.getElementById('alarm-val');
    if (d.alarm) {
      ac.classList.add('alarm-card');
      av.innerHTML = "<span style='color:#f80'>AKTYWNY</span>";
    } else {
      ac.classList.remove('alarm-card');
      av.innerHTML = "<span style='color:#4e8'>OK</span>";
    }
    document.getElementById('alarm-range').textContent = 'prog: ' + d.alarmLow + ' - ' + d.alarmHigh;
    document.getElementById('al-low').value  = d.alarmLow;
    document.getElementById('al-high').value = d.alarmHigh;
  } catch(e) {}
}

async function fetchHistory() {
  try {
    const r = await fetch('/history?min=' + rangeMin);
    chartData = await r.json();
    drawChart();
  } catch(e) {}
}

function drawChart() {
  const cv = document.getElementById('chart');
  const cx = cv.getContext('2d');
  cv.width = cv.offsetWidth; cv.height = 140;
  const W = cv.width, H = 140, p = 12;
  cx.fillStyle = '#0a0c12'; cx.fillRect(0,0,W,H);
  cx.strokeStyle = '#1e2130'; cx.lineWidth = 1; cx.beginPath();
  const y7 = H-p-((7/14)*(H-2*p));
  cx.moveTo(p,y7); cx.lineTo(W-p,y7); cx.stroke();
  cx.fillStyle='#333'; cx.font='9px system-ui'; cx.fillText('pH 7',W-p-22,y7-2);
  if (chartData.length > 1) {
    const s = (W-2*p)/(chartData.length-1);
    cx.lineWidth=2; cx.lineJoin='round'; cx.beginPath();
    chartData.forEach((v,i) => {
      const x=p+i*s, y=H-p-((v.ph/14)*(H-2*p));
      i?cx.lineTo(x,y):cx.moveTo(x,y);
    });
    cx.strokeStyle='#4e8'; cx.stroke();
    const lx=p+(chartData.length-1)*s;
    const ly=H-p-((chartData[chartData.length-1].ph/14)*(H-2*p));
    cx.beginPath(); cx.arc(lx,ly,3,0,Math.PI*2); cx.fillStyle='#4e8'; cx.fill();
  }
}

async function saveAlarms() {
  const low  = parseFloat(document.getElementById('al-low').value);
  const high = parseFloat(document.getElementById('al-high').value);
  const st = document.getElementById('alarm-status');
  if (low >= high) { st.textContent='Min musi byc mniejszy niz max'; st.className='status err'; return; }
  try {
    await fetch('/set-alarms?low='+low+'&high='+high);
    st.textContent = 'Zapisano progi: ' + low + ' - ' + high;
    st.className = 'status';
  } catch(e) { st.textContent='Blad'; st.className='status err'; }
}

async function resetHistory() {
  if (!confirm('Na pewno wyczyscic historie?')) return;
  await fetch('/reset-history');
  chartData = [];
  drawChart();
  document.getElementById('alarm-status').textContent = 'Historia wyczyszczona';
}

function exportCSV() {
  window.location.href = '/export.csv';
}

async function calibrate() {
  const ph = parseFloat(document.getElementById('cal-ph').value);
  const v  = parseFloat(document.getElementById('cal-v').value);
  const st = document.getElementById('cal-status');
  if (isNaN(ph) || isNaN(v) || v <= 0) { st.textContent='Podaj prawidlowe wartosci'; st.className='status err'; return; }
  try {
    await fetch('/calibrate?ph='+ph+'&v='+v);
    st.textContent = 'Kalibracja zapisana';
    st.className = 'status';
  } catch(e) { st.textContent='Blad'; st.className='status err'; }
}

async function savePushover() {
  const en  = document.getElementById('po-en').checked;
  const tok = document.getElementById('po-tok').value.trim();
  const usr = document.getElementById('po-usr').value.trim();
  const st  = document.getElementById('po-status');
  try {
    await fetch('/set-pushover?en='+(en?1:0)+'&tok='+encodeURIComponent(tok)+'&usr='+encodeURIComponent(usr));
    st.textContent = 'Ustawienia Pushover zapisane';
    st.className = 'status';
  } catch(e) { st.textContent='Blad'; st.className='status err'; }
}

async function testPushover() {
  const st = document.getElementById('po-status');
  await fetch('/pushover-test');
  st.textContent = 'Wyslano testowe powiadomienie';
  st.className = 'status';
}

async function loadPushoverSettings() {
  try {
    const r = await fetch('/pushover-config');
    const d = await r.json();
    document.getElementById('po-en').checked = d.enabled;
    document.getElementById('po-tok').value  = d.token;
    document.getElementById('po-usr').value  = d.user;
  } catch(e) {}
}

setInterval(fetchStatus,  2000);
setInterval(fetchHistory, 10000);
fetchStatus();
fetchHistory();
loadPushoverSettings();
</script></body></html>)xxx";
    return html;
}

void setupServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
        req->send(200, "text/html", buildHTML());
    });

    server.on("/api", HTTP_GET, [](AsyncWebServerRequest *req) {
        AsyncWebServerResponse *r = req->beginResponse(200, "application/json", buildJSON());
        r->addHeader("Access-Control-Allow-Origin","*");
        req->send(r);
    });

    server.on("/history", HTTP_GET, [](AsyncWebServerRequest *req) {
        int minutes = 60;
        if (req->hasParam("min")) minutes = req->getParam("min")->value().toInt();
        AsyncWebServerResponse *r = req->beginResponse(200, "application/json", buildHistJSON(minutes));
        r->addHeader("Access-Control-Allow-Origin","*");
        req->send(r);
    });

    server.on("/export.csv", HTTP_GET, [](AsyncWebServerRequest *req) {
        AsyncWebServerResponse *r = req->beginResponse(200, "text/csv", buildCSV());
        r->addHeader("Content-Disposition","attachment; filename=ph_data.csv");
        req->send(r);
    });

    server.on("/set-alarms", HTTP_GET, [](AsyncWebServerRequest *req) {
        if (req->hasParam("low") && req->hasParam("high")) {
            phAlarmLow  = req->getParam("low")->value().toFloat();
            phAlarmHigh = req->getParam("high")->value().toFloat();
            saveSettings();
        }
        req->send(200, "text/plain", "OK");
    });

    server.on("/reset-history", HTTP_GET, [](AsyncWebServerRequest *req) {
        historyCount = 0;
        historyIndex = 0;
        saveHistory();
        req->send(200, "text/plain", "OK");
    });

    server.on("/calibrate", HTTP_GET, [](AsyncWebServerRequest *req) {
        if (req->hasParam("ph") && req->hasParam("v")) {
            float calPH = req->getParam("ph")->value().toFloat();
            float calV  = req->getParam("v")->value().toFloat();
            phNeutralV = calV - (7.0f - calPH) * phSlope;
            saveSettings();
        }
        req->send(200, "text/plain", "OK");
    });

    server.on("/set-pushover", HTTP_GET, [](AsyncWebServerRequest *req) {
        if (req->hasParam("en"))  pushoverEnabled = req->getParam("en")->value().toInt() == 1;
        if (req->hasParam("tok")) req->getParam("tok")->value().toCharArray(pushoverToken, sizeof(pushoverToken));
        if (req->hasParam("usr")) req->getParam("usr")->value().toCharArray(pushoverUser,  sizeof(pushoverUser));
        saveSettings();
        req->send(200, "text/plain", "OK");
    });

    server.on("/pushover-test", HTTP_GET, [](AsyncWebServerRequest *req) {
        sendPushover("Test powiadomienia z pH Monitor");
        req->send(200, "text/plain", "OK");
    });

    server.on("/pushover-config", HTTP_GET, [](AsyncWebServerRequest *req) {
        String j = "{\"enabled\":" + String(pushoverEnabled?"true":"false") +
                   ",\"token\":\"" + String(pushoverToken) + "\"" +
                   ",\"user\":\"" + String(pushoverUser) + "\"}";
        req->send(200, "application/json", j);
    });

    server.onNotFound([](AsyncWebServerRequest *req) {
        req->send(404, "text/plain", "Nie znaleziono");
    });

    server.begin();
}

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    pinMode(PIN_PH_ALARM, INPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    loadSettings();
    loadHistory();
    
    sensors.begin();

    tft_init();
    tft_fill_screen(CLR_RED);   delay(300);
    tft_fill_screen(CLR_GREEN); delay(300);
    tft_fill_screen(CLR_BLUE);  delay(300);

    drawStaticUI();

    tft_draw_string(6, 302, "Laczenie WiFi...", CLR_MUTED, CLR_BLACK, 1);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 30) { delay(500); tries++; }
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        WiFi.localIP().toString().toCharArray(wifiIP, 16);
    }

    setupServer();
    drawWifi();

    buzzerBeep(1000, 80);
    delay(50);
    buzzerBeep(1500, 80);
}

void loop() {
    unsigned long now = millis();

    if (now - lastSampleTime >= SAMPLE_INTERVAL_MS) {
        lastSampleTime = now;
        currentPH   = readPH();
        currentTemp = readTemperature();
        bool wasAlarm = alarmActive;
        alarmActive = (currentPH < phAlarmLow || currentPH > phAlarmHigh);
        updateHistory(currentPH);

        if (alarmActive && !wasAlarm) {
            pushoverSent = false;
        }
        if (alarmActive && !pushoverSent && wifiConnected) {
            char msg[64];
            snprintf(msg, sizeof(msg), "pH=%.2f poza zakresem %.1f-%.1f", currentPH, phAlarmLow, phAlarmHigh);
            sendPushover(msg);
            pushoverSent = true;
        }
        if (!alarmActive) {
            pushoverSent = false;
            alarmMuted   = false;
        }

        if (now % 30000 < SAMPLE_INTERVAL_MS) saveHistory();

        drawVoltage();
        drawHistory();
        drawUptime();
    }

    if (now - lastBlinkTime >= 500) {
        lastBlinkTime  = now;
        alarmBlinkState = !alarmBlinkState;
        drawPHValue(alarmActive && alarmBlinkState);
        drawAlarmBadge(alarmActive && alarmBlinkState);
    }

    if (alarmActive && !alarmMuted && now - lastAlarmBeep >= 600) {
        lastAlarmBeep = now;
        buzzerBeep(800, 150);
        delay(50);
        buzzerBeep(880, 60);
    }
}