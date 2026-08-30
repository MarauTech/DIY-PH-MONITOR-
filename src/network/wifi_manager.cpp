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
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_AP);
    
    IPAddress apIP(192, 168, 4, 1);
    IPAddress netMsk(255, 255, 255, 0);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP("PH-Monitor-Setup");
    delay(200);
    
    dnsServer.start(53, "*", apIP);
    Serial.println("[WiFi] Tryb AP uruchomiony: PH-Monitor-Setup (192.168.4.1)");
}

bool WifiManager::connectSTA(const char* ssid, const char* pass, int timeoutMs) {
    apMode = false;
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid, pass);
    
    Serial.printf("[WiFi] Laczenie z siecia %s ...\n", ssid);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < (unsigned long)timeoutMs) {
        delay(200);
        Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WiFi] Polaczono! Adres IP: %s, RSSI: %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
        return true;
    } else {
        Serial.println("[WiFi] Nie udalo sie polaczyc z siecia. Uruchamiam AP.");
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
            Serial.printf("[mDNS] Dostepny pod http://%s.local\n", hostname);
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
    return now > 1704067200; // > 2024-01-01
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
