#include "ui.h"
#include "display.h"
#include <stdio.h>

namespace UI {
    uint16_t phColor(float ph, float low, float high) {
        if (ph < low) return Display::CLR_ORANGE;
        if (ph > high) return Display::CLR_RED;
        return Display::CLR_GREEN;
    }

    const char* phLabel(float ph) {
        if (ph < 7.0f) return "Kwasowy";
        if (ph > 7.0f) return "Zasadowy";
        return "Neutralny";
    }

    void drawStaticUI(const char* deviceName) {
        Display::fillScreen(Display::CLR_BLACK);
        Display::fillRect(0, 0, 320, 24, Display::CLR_PANEL);
        Display::drawString(4, 4, deviceName, Display::CLR_WHITE, Display::CLR_PANEL, 2);
        
        Display::drawHLine(0, 24, 320, Display::CLR_DARKGRAY);
        Display::drawHLine(0, 228, 320, Display::CLR_DARKGRAY);
        
        Display::drawString(4, 28, "pH Wody", Display::CLR_MUTED, Display::CLR_BLACK, 1);
        Display::drawString(4, 104, "Napiecie", Display::CLR_MUTED, Display::CLR_BLACK, 1);
        Display::drawString(4, 138, "Temperatura", Display::CLR_MUTED, Display::CLR_BLACK, 1);
        
        Display::drawRect(150, 48, 164, 180, Display::CLR_PANEL);
    }

    void drawPHValue(float ph, float alarmLow, float alarmHigh) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.2f", ph);
        uint16_t color = phColor(ph, alarmLow, alarmHigh);
        Display::fillRect(4, 44, 140, 40, Display::CLR_BLACK);
        Display::drawBigNumber(4, 44, buf, color);
        
        Display::fillRect(4, 84, 140, 16, Display::CLR_BLACK);
        Display::drawString(4, 84, phLabel(ph), color, Display::CLR_BLACK, 2);
    }

    void drawVoltage(float voltage) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.2f V", voltage);
        Display::fillRect(4, 116, 140, 16, Display::CLR_BLACK);
        Display::drawString(4, 116, buf, Display::CLR_WHITE, Display::CLR_BLACK, 2);
    }

    void drawTemperature(float temp) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f C", temp);
        Display::fillRect(4, 150, 140, 16, Display::CLR_BLACK);
        Display::drawString(4, 150, buf, Display::CLR_WHITE, Display::CLR_BLACK, 2);
    }

    void drawAlarm(bool active) {
        if (active) {
            Display::fillRect(280, 4, 32, 16, Display::CLR_RED);
            Display::drawString(284, 4, "ALARM", Display::CLR_WHITE, Display::CLR_RED, 1);
        } else {
            Display::fillRect(280, 4, 32, 16, Display::CLR_PANEL);
        }
    }

    void drawHistory(float* history, int count, int index, int maxSize, float alarmLow, float alarmHigh) {
        Display::fillRect(151, 49, 162, 178, Display::CLR_BLACK);
        if (count < 2) return;
        
        float minPh = 0.0f;
        float maxPh = 14.0f;
        
        int drawCount = count;
        if (drawCount > 162) drawCount = 162;
        
        for (int i = 0; i < drawCount - 1; i++) {
            int histIdx1 = (index - drawCount + i + maxSize) % maxSize;
            int histIdx2 = (index - drawCount + i + 1 + maxSize) % maxSize;
            
            float ph1 = history[histIdx1];
            float ph2 = history[histIdx2];
            
            int y1 = 49 + 178 - (int)((ph1 - minPh) / (maxPh - minPh) * 178);
            int y2 = 49 + 178 - (int)((ph2 - minPh) / (maxPh - minPh) * 178);
            
            if (y1 < 49) y1 = 49;
            if (y1 > 226) y1 = 226;
            if (y2 < 49) y2 = 49;
            if (y2 > 226) y2 = 226;
            
            uint16_t color = phColor(ph1, alarmLow, alarmHigh);
            Display::drawLine(151 + i, y1, 151 + i + 1, y2, color);
        }
    }

    void drawWifi(bool connected, const char* ip) {
        Display::fillRect(4, 172, 140, 40, Display::CLR_BLACK);
        if (connected) {
            Display::drawString(4, 172, "WiFi OK", Display::CLR_GREEN, Display::CLR_BLACK, 1);
            Display::drawString(4, 184, ip, Display::CLR_WHITE, Display::CLR_BLACK, 1);
        } else {
            Display::drawString(4, 172, "Brak WiFi", Display::CLR_RED, Display::CLR_BLACK, 1);
        }
    }

    void drawUptime(uint32_t seconds) {
        char buf[32];
        uint32_t d = seconds / 86400;
        uint32_t h = (seconds % 86400) / 3600;
        uint32_t m = (seconds % 3600) / 60;
        snprintf(buf, sizeof(buf), "Uptime: %ud %02uh %02um", d, h, m);
        Display::fillRect(4, 230, 312, 10, Display::CLR_BLACK);
        Display::drawString(4, 230, buf, Display::CLR_MUTED, Display::CLR_BLACK, 1);
    }

    void drawOTAProgress(int percent) {
        Display::fillScreen(Display::CLR_BLACK);
        Display::drawRect(4, 4, 312, 232, Display::CLR_RED);
        Display::drawString(100, 100, "Aktualizacja OTA", Display::CLR_WHITE, Display::CLR_BLACK, 2);
        
        Display::drawRect(60, 130, 200, 20, Display::CLR_WHITE);
        Display::fillRect(62, 132, (196 * percent) / 100, 16, Display::CLR_BLUE);
        
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", percent);
        Display::drawString(150, 156, buf, Display::CLR_WHITE, Display::CLR_BLACK, 1);
        Display::drawString(40, 200, "Nie odpisuj zasilania!", Display::CLR_ORANGE, Display::CLR_BLACK, 2);
    }

    void drawOTASuccess() {
        Display::fillScreen(Display::CLR_GREEN);
        Display::drawString(80, 110, "Zaktualizowano!", Display::CLR_BLACK, Display::CLR_GREEN, 2);
        Display::drawString(100, 140, "Restart...", Display::CLR_BLACK, Display::CLR_GREEN, 1);
    }

    void drawBootAnimation() {
        Display::fillScreen(Display::CLR_BLACK);
        Display::drawString(80, 100, "Uruchamianie...", Display::CLR_CYAN, Display::CLR_BLACK, 2);
        Display::drawRect(60, 140, 200, 10, Display::CLR_DARKGRAY);
    }

    void drawAPMode(const char* ssid, const char* ip) {
        Display::fillScreen(Display::CLR_BLUE);
        Display::drawString(20, 40, "Tryb Konfiguracji", Display::CLR_WHITE, Display::CLR_BLUE, 2);
        Display::drawString(20, 80, "Siec WiFi:", Display::CLR_WHITE, Display::CLR_BLUE, 1);
        Display::drawString(20, 100, ssid, Display::CLR_YELLOW, Display::CLR_BLUE, 2);
        Display::drawString(20, 140, "Panel pod adresem:", Display::CLR_WHITE, Display::CLR_BLUE, 1);
        Display::drawString(20, 160, ip, Display::CLR_YELLOW, Display::CLR_BLUE, 2);
    }
}
