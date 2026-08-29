#include "mqtt_client.h"
#include <ArduinoJson.h>

MqttManager::MqttManager() : mqttClient(wifiClient) {}

void MqttManager::begin(const char* broker, int port, const char* user, const char* pass) {
    brokerStr = broker ? broker : "";
    portNum = port;
    userStr = user ? user : "";
    passStr = pass ? pass : "";
    
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char id[20];
    snprintf(id, sizeof(id), "phmonitor-%02x%02x%02x", mac[3], mac[4], mac[5]);
    clientIdStr = id;
    
    if (brokerStr.length() > 0) {
        mqttClient.setServer(brokerStr.c_str(), portNum);
    }
}

void MqttManager::setEnabled(bool en) {
    enabled = en;
    if (!enabled && mqttClient.connected()) {
        mqttClient.disconnect();
    }
}

void MqttManager::update() {
    if (!enabled || brokerStr.length() == 0 || WiFi.status() != WL_CONNECTED) {
        return;
    }
    
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 10000 || lastReconnectAttempt == 0) {
            lastReconnectAttempt = now;
            reconnect();
        }
    } else {
        mqttClient.loop();
    }
}

bool MqttManager::isConnected() {
    return mqttClient.connected();
}

void MqttManager::reconnect() {
    if (mqttClient.connect(clientIdStr.c_str(), userStr.length() > 0 ? userStr.c_str() : NULL, passStr.length() > 0 ? passStr.c_str() : NULL)) {
        lastReconnectAttempt = 0;
    }
}

void MqttManager::publishState(float ph, float temp, float voltage, uint8_t alarmState) {
    if (!enabled || !mqttClient.connected()) return;
    
    char payload[32];
    
    snprintf(payload, sizeof(payload), "%.2f", ph);
    mqttClient.publish("phmonitor/ph", payload);
    
    snprintf(payload, sizeof(payload), "%.1f", temp);
    mqttClient.publish("phmonitor/temperature", payload);
    
    snprintf(payload, sizeof(payload), "%.3f", voltage);
    mqttClient.publish("phmonitor/voltage", payload);
    
    snprintf(payload, sizeof(payload), "%d", alarmState);
    mqttClient.publish("phmonitor/alarm", payload);
    
    StaticJsonDocument<200> doc;
    doc["ph"] = ph;
    doc["temperature"] = temp;
    doc["voltage"] = voltage;
    doc["alarm"] = alarmState;
    String jsonStr;
    serializeJson(doc, jsonStr);
    mqttClient.publish("phmonitor/status", jsonStr.c_str());
}

void MqttManager::publishHADiscovery(const char* deviceName) {
    if (!enabled || !mqttClient.connected()) return;
    
    StaticJsonDocument<512> doc;
    doc["name"] = String(deviceName) + " pH";
    doc["state_topic"] = "phmonitor/ph";
    doc["unit_of_measurement"] = "pH";
    doc["unique_id"] = clientIdStr + "_ph";
    
    JsonObject dev = doc.createNestedObject("device");
    dev["identifiers"][0] = clientIdStr;
    dev["name"] = deviceName;
    dev["model"] = "DIY pH Monitor";
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    
    String topic = String("homeassistant/sensor/") + clientIdStr + "/ph/config";
    mqttClient.publish(topic.c_str(), jsonStr.c_str(), true);
}
