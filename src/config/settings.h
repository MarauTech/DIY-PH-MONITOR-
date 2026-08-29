#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "defaults.h"

struct Config {
    // Calibration (stored as millivolts)
    int32_t voltagePH4 = DEFAULT_VOLTAGE_PH4;
    int32_t voltagePH7 = DEFAULT_VOLTAGE_PH7;
    int32_t voltagePH9 = DEFAULT_VOLTAGE_PH9;
    float ph4Value = DEFAULT_PH4_VALUE;
    float ph7Value = DEFAULT_PH7_VALUE;
    float ph9Value = DEFAULT_PH9_VALUE;
    bool calibrated = false;
    
    // Alarm
    float alarmLow = DEFAULT_ALARM_LOW;
    float alarmHigh = DEFAULT_ALARM_HIGH;
    float hysteresis = DEFAULT_HYSTERESIS;
    uint32_t alarmHoldSec = DEFAULT_ALARM_HOLD_S;
    
    // WiFi
    char wifiSSID[33] = "";
    char wifiPass[65] = "";
    
    // Admin auth
    char adminUser[17] = "admin";
    char adminPass[33] = "admin";
    
    // Pushover
    bool pushoverEnabled = false;
    char pushoverUser[64] = "";
    char pushoverToken[64] = "";
    
    // MQTT
    bool mqttEnabled = false;
    char mqttBroker[64] = "";
    int mqttPort = DEFAULT_MQTT_PORT;
    char mqttUser[32] = "";
    char mqttPass[32] = "";
    
    // Device
    char deviceName[17] = "pH Monitor";
    bool buzzerMuted = false;
};

class Settings {
public:
    static Settings& instance();
    
    void load();
    void save();
    
    void saveCalibration();
    void saveAlarm();
    void saveWifi();
    void savePushover();
    void saveMqtt();
    void saveAdmin();
    
    void resetAll();
    
    Config& config();
    bool hasWifiCredentials();
    
    static bool validateAlarmConfig(float low, float high, float hyst);
    static bool validateMqttPort(int port);

private:
    Settings() = default;
    ~Settings() = default;
    
    // Prevent copying
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    Config _config;
    Preferences _prefs;
    const char* NAMESPACE = "ph-cfg";
};
