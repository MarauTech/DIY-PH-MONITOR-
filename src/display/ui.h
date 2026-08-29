#pragma once
#include <Arduino.h>

namespace UI {
    uint16_t phColor(float ph, float low, float high);
    const char* phLabel(float ph);
    void drawStaticUI(const char* deviceName);
    void drawPHValue(float ph, float alarmLow, float alarmHigh);
    void drawVoltage(float voltage);
    void drawTemperature(float temp);
    void drawAlarm(bool active);
    void drawHistory(float* history, int count, int index, int maxSize, float alarmLow, float alarmHigh);
    void drawWifi(bool connected, const char* ip);
    void drawUptime(uint32_t seconds);
    void drawOTAProgress(int percent);
    void drawOTASuccess();
    void drawBootAnimation();
    void drawAPMode(const char* ssid, const char* ip);
}
