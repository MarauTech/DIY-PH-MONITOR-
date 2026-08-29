#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>

class WifiManager {
public:
    void begin(const char* ssid, const char* pass);
    void startAP();
    bool connectSTA(const char* ssid, const char* pass, int timeoutMs = 15000);
    bool isConnected();
    bool isAPMode();
    String getIP();
    int32_t getRSSI();
    void setupMDNS(const char* hostname);
    void setupNTP();
    bool isNTPSynced();
    String getTimeString();
    void handleDNS();

private:
    bool apMode = false;
    DNSServer dnsServer;
};
