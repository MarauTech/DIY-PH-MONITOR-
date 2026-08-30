#include "mqtt_client.h"
#include <ArduinoJson.h>

MqttManager::MqttManager() : mqttClient(wifiClient), enabled(false), discoverySent(false) {}

void MqttManager::begin(const char* broker, int port, const char* user, const char* pass) {
    brokerStr = broker ? broker : "";
    portNum = port > 0 ? port : 1883;
    userStr = user ? user : "";
    passStr = pass ? pass : "";
    discoverySent = false;
    
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char id[24];
    snprintf(id, sizeof(id), "phmonitor-%02x%02x%02x", mac[3], mac[4], mac[5]);
    clientIdStr = id;
    
    if (brokerStr.length() > 0) {
        mqttClient.setServer(brokerStr.c_str(), portNum);
        mqttClient.setBufferSize(512);
    }
}

void MqttManager::setEnabled(bool en) {
    enabled = en;
    discoverySent = false;
    if (!enabled && mqttClient.connected()) {
        mqttClient.disconnect();
    }
}

void MqttManager::update(const char* deviceName) {
    if (!enabled || brokerStr.length() == 0 || WiFi.status() != WL_CONNECTED) {
        if (mqttClient.connected()) {
            mqttClient.disconnect();
            discoverySent = false;
        }
        return;
    }
    
    if (!mqttClient.connected()) {
        discoverySent = false;
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 5000 || lastReconnectAttempt == 0) {
            lastReconnectAttempt = now;
            if (reconnect(deviceName)) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        mqttClient.loop();
    }
}

bool MqttManager::isConnected() {
    return enabled && mqttClient.connected();
}

bool MqttManager::reconnect(const char* deviceName) {
    if (brokerStr.length() == 0) return false;
    mqttClient.setServer(brokerStr.c_str(), portNum);
    
    const char* u = userStr.length() > 0 ? userStr.c_str() : NULL;
    const char* p = passStr.length() > 0 ? passStr.c_str() : NULL;
    
    Serial.printf("[MQTT] Laczenie z brokerem %s:%d (klient: %s)...\n", brokerStr.c_str(), portNum, clientIdStr.c_str());
    
    if (mqttClient.connect(clientIdStr.c_str(), u, p)) {
        Serial.println("[MQTT] Polaczono pomyślnie z brokerem!");
        publishHADiscovery(deviceName);
        return true;
    } else {
        Serial.printf("[MQTT] Blad polaczenia (rc=%d)\n", mqttClient.state());
        return false;
    }
}

void MqttManager::publishState(float ph, float temp, float voltage, uint8_t alarmState, int32_t rssi) {
    if (!enabled || !mqttClient.connected()) return;
    
    char payload[32];
    
    snprintf(payload, sizeof(payload), "%.2f", ph);
    mqttClient.publish("phmonitor/ph", payload);
    
    if (temp > -50.0f) {
        snprintf(payload, sizeof(payload), "%.1f", temp);
        mqttClient.publish("phmonitor/temperature", payload);
    }
    
    snprintf(payload, sizeof(payload), "%.3f", voltage);
    mqttClient.publish("phmonitor/voltage", payload);
    
    snprintf(payload, sizeof(payload), "%d", alarmState);
    mqttClient.publish("phmonitor/alarm", payload);
    
    snprintf(payload, sizeof(payload), "%d", (int)rssi);
    mqttClient.publish("phmonitor/rssi", payload);
    
    // Binary alarm state
    mqttClient.publish("phmonitor/alarm_state", alarmState > 0 ? "1" : "0");
    
    // Combined JSON status
    JsonDocument doc;
    doc["ph"] = ph;
    if (temp > -50.0f) doc["temperature"] = temp;
    doc["voltage"] = voltage;
    doc["alarm"] = alarmState;
    doc["rssi"] = rssi;
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    mqttClient.publish("phmonitor/status", jsonStr.c_str());
}

void MqttManager::publishHADiscovery(const char* deviceName) {
    if (!enabled || !mqttClient.connected()) return;
    
    const char* devName = (deviceName && strlen(deviceName) > 0) ? deviceName : "DIY pH Monitor";
    
    // 1. pH Sensor
    {
        JsonDocument doc;
        doc["name"] = "pH";
        doc["state_topic"] = "phmonitor/ph";
        doc["unit_of_measurement"] = "pH";
        doc["unique_id"] = clientIdStr + "_ph";
        doc["state_class"] = "measurement";
        
        JsonObject dev = doc["device"].to<JsonObject>();
        dev["identifiers"][0] = clientIdStr;
        dev["name"] = devName;
        dev["model"] = "DIY pH Monitor";
        dev["manufacturer"] = "MarauTech";
        
        String jsonStr;
        serializeJson(doc, jsonStr);
        String topic = "homeassistant/sensor/" + clientIdStr + "/ph/config";
        mqttClient.publish(topic.c_str(), jsonStr.c_str(), true);
    }
    
    // 2. Temperature Sensor
    {
        JsonDocument doc;
        doc["name"] = "Temperatura";
        doc["state_topic"] = "phmonitor/temperature";
        doc["unit_of_measurement"] = "°C";
        doc["device_class"] = "temperature";
        doc["state_class"] = "measurement";
        doc["unique_id"] = clientIdStr + "_temp";
        
        JsonObject dev = doc["device"].to<JsonObject>();
        dev["identifiers"][0] = clientIdStr;
        dev["name"] = devName;
        dev["model"] = "DIY pH Monitor";
        dev["manufacturer"] = "MarauTech";
        
        String jsonStr;
        serializeJson(doc, jsonStr);
        String topic = "homeassistant/sensor/" + clientIdStr + "/temp/config";
        mqttClient.publish(topic.c_str(), jsonStr.c_str(), true);
    }
    
    // 3. Voltage Sensor
    {
        JsonDocument doc;
        doc["name"] = "Napięcie Sondy";
        doc["state_topic"] = "phmonitor/voltage";
        doc["unit_of_measurement"] = "V";
        doc["device_class"] = "voltage";
        doc["state_class"] = "measurement";
        doc["unique_id"] = clientIdStr + "_voltage";
        
        JsonObject dev = doc["device"].to<JsonObject>();
        dev["identifiers"][0] = clientIdStr;
        dev["name"] = devName;
        dev["model"] = "DIY pH Monitor";
        dev["manufacturer"] = "MarauTech";
        
        String jsonStr;
        serializeJson(doc, jsonStr);
        String topic = "homeassistant/sensor/" + clientIdStr + "/voltage/config";
        mqttClient.publish(topic.c_str(), jsonStr.c_str(), true);
    }
    
    // 4. Binary Sensor Alarm
    {
        JsonDocument doc;
        doc["name"] = "Alarm pH";
        doc["state_topic"] = "phmonitor/alarm_state";
        doc["payload_on"] = "1";
        doc["payload_off"] = "0";
        doc["device_class"] = "problem";
        doc["unique_id"] = clientIdStr + "_alarm";
        
        JsonObject dev = doc["device"].to<JsonObject>();
        dev["identifiers"][0] = clientIdStr;
        dev["name"] = devName;
        dev["model"] = "DIY pH Monitor";
        dev["manufacturer"] = "MarauTech";
        
        String jsonStr;
        serializeJson(doc, jsonStr);
        String topic = "homeassistant/binary_sensor/" + clientIdStr + "/alarm/config";
        mqttClient.publish(topic.c_str(), jsonStr.c_str(), true);
    }
    
    // 5. Signal RSSI Sensor
    {
        JsonDocument doc;
        doc["name"] = "WiFi RSSI";
        doc["state_topic"] = "phmonitor/rssi";
        doc["unit_of_measurement"] = "dBm";
        doc["device_class"] = "signal_strength";
        doc["state_class"] = "measurement";
        doc["unique_id"] = clientIdStr + "_rssi";
        
        JsonObject dev = doc["device"].to<JsonObject>();
        dev["identifiers"][0] = clientIdStr;
        dev["name"] = devName;
        dev["model"] = "DIY pH Monitor";
        dev["manufacturer"] = "MarauTech";
        
        String jsonStr;
        serializeJson(doc, jsonStr);
        String topic = "homeassistant/sensor/" + clientIdStr + "/rssi/config";
        mqttClient.publish(topic.c_str(), jsonStr.c_str(), true);
    }
    
    discoverySent = true;
    Serial.println("[MQTT] Home Assistant Discovery opublikowane (5 encji, retain=true).");
}
