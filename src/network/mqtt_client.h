#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

class MqttManager {
public:
    MqttManager();
    void begin(const char* broker, int port, const char* user, const char* pass);
    void update(const char* deviceName);
    bool isConnected();
    void publishState(float ph, float temp, float voltage, uint8_t alarmState, int32_t rssi);
    void publishHADiscovery(const char* deviceName);
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled; }

private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    bool enabled = false;
    unsigned long lastReconnectAttempt = 0;
    String brokerStr;
    int portNum = 1883;
    String userStr;
    String passStr;
    String clientIdStr;
    bool discoverySent = false;
    
    bool reconnect(const char* deviceName);
};
