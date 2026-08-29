#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <functional>
#include "config/settings.h"
#include "data/history_logger.h"
#include "sensor/ph_sensor.h"
#include "network/pushover.h"

struct SystemState {
    float ph = 7.0f;
    float voltage = 0.0f;
    float temperature = -127.0f;
    bool tempConnected = false;
    uint8_t alarmState = 0;  // 0=normal, 1=low, 2=high
    bool wifiConnected = false;
    int32_t rssi = 0;
    String ip = "---";
    bool ntpSynced = false;
    String ntpTime = "--:--:--";
    uint32_t uptime = 0;
    bool calibrated = false;
    uint8_t pushoverStatus = 0;
    bool buzzerMuted = false;
    float minPH = 14.0f;
    float maxPH = 0.0f;
    float minTemp = 100.0f;
    float maxTemp = -50.0f;
    bool statsInitialized = false;
};

class WebServer {
public:
    WebServer(uint16_t port = 80);
    void begin(bool apMode);
    void setAuthCredentials(const char* user, const char* pass);
    void setSystemState(SystemState* state);
    void setHistoryLogger(HistoryLogger* logger);
    void setPhSensor(PhSensor* sensor);
    void setPushover(Pushover* po);
    void setCalibrationCallback(std::function<void(const String& type, float customPH)> cb);
    void setConfigChangeCallback(std::function<void()> cb);
    void setOTAProgressCallback(std::function<void(int percent)> cb);
    void setOTACompleteCallback(std::function<void(bool success)> cb);

private:
    AsyncWebServer server;
    SystemState* sysState = nullptr;
    HistoryLogger* historyLogger = nullptr;
    PhSensor* phSensor = nullptr;
    Pushover* pushover = nullptr;
    String authUser;
    String authPass;
    bool isAP = false;
    
    std::function<void(const String& type, float customPH)> onCalibrate;
    std::function<void()> onConfigChange;
    std::function<void(int percent)> onOTAProgress;
    std::function<void(bool success)> onOTAComplete;
    
    bool authenticate(AsyncWebServerRequest* req);
    void setupRoutes();
};
