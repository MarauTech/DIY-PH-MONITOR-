#include "wifi_manager.h"
#include <ESPmDNS.h>
#include <time.h>

#ifndef NTP_TZ
#define NTP_TZ "CET-1CEST,M3.5.0,M10.5.0/3"
#endif
#ifndef NTP_SERVER1
#define NTP_SERVER1 "pool.ntp.org"
#endif
#ifndef NTP_SERVER2
#define NTP_SERVER2 "time.nist.gov"
#endif

void WifiManager::begin(const char* ssid, const char* pass) {
    if (!ssid || ssid[0] == '\0') {
        startAP();
    } else {
        connectSTA(ssid, pass);
    }
}

void WifiManager::startAP() {
    apMode = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP("PH-Monitor-Setup", NULL);
    delay(500);
    dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
}

bool WifiManager::connectSTA(const char* ssid, const char* pass, int timeoutMs) {
    apMode = false;
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < (unsigned long)timeoutMs) {
        delay(100);
    }
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    } else {
        startAP();
        return false;
    }
}

bool WifiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool WifiManager::isAPMode() {
    return apMode;
}

String WifiManager::getIP() {
    if (apMode) return WiFi.softAPIP().toString();
    return WiFi.localIP().toString();
}

int32_t WifiManager::getRSSI() {
    if (apMode) return 0;
    return WiFi.RSSI();
}

void WifiManager::setupMDNS(const char* hostname) {
    if (!apMode && isConnected()) {
        if (MDNS.begin(hostname)) {
            MDNS.addService("http", "tcp", 80);
        }
    }
}

void WifiManager::setupNTP() {
    if (!apMode && isConnected()) {
        configTzTime(NTP_TZ, NTP_SERVER1, NTP_SERVER2);
    }
}

bool WifiManager::isNTPSynced() {
    time_t now;
    time(&now);
    // 2024-01-01 is roughly 1704067200
    return now > 1704067200;
}

String WifiManager::getTimeString() {
    if (!isNTPSynced()) {
        return String("--:--:--");
    }
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return String("--:--:--");
    }
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    return String(buffer);
}

void WifiManager::handleDNS() {
    if (apMode) {
        dnsServer.processNextRequest();
    }
}
