#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

class MqttManager {
public:
    MqttManager();
    void begin(const char* broker, int port, const char* user, const char* pass);
    void update();
    bool isConnected();
    void publishState(float ph, float temp, float voltage, uint8_t alarmState);
    void publishHADiscovery(const char* deviceName);
    void setEnabled(bool enabled);

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
    
    void reconnect();
};
