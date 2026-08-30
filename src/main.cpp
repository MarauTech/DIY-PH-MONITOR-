#include <Arduino.h>
#include "pins.h"
#include "defaults.h"
#include "sensor/ph_sensor.h"
#include "sensor/temperature_sensor.h"
#include "alarm/alarm_manager.h"
#include "alarm/buzzer.h"
#include "config/settings.h"
#include "data/history_logger.h"
#include "network/wifi_manager.h"
#include "network/pushover.h"
#include "network/mqtt_client.h"
#include "network/web_server.h"
#include "display/display.h"
#include "display/ui.h"

#if __has_include("secrets.h")
#include "secrets.h"
#endif

// Global objects
PhSensor phSensor;
TemperatureSensor tempSensor;
AlarmManager* alarmMgr = nullptr;
Buzzer buzzer;
HistoryLogger historyLogger;
WifiManager wifiMgr;
Pushover pushover;
MqttManager mqttMgr;
WebServer webServer(80);
SystemState sysState;

// In-RAM history for TFT chart
float phHistory[HISTORY_SIZE];
int historyIndex = 0;
int historyCount = 0;
float smoothedPH = 7.0f;

// Statistics
float minPH = 14.0f;
float maxPH = 0.0f;
float minTemp = 100.0f;
float maxTemp = -50.0f;
bool statsInitialized = false;

// Timers & flags
unsigned long lastSampleTime = 0;
unsigned long lastHistoryLogTime = 0;
bool otaRunning = false;
bool apModeActive = false;

String pendingCalType = "";
float pendingCalCustomPH = 7.0f;

void onCalibration(const String& type, float customPH) {
    pendingCalType = type;
    pendingCalCustomPH = customPH;
    phSensor.startCalibration();
}

void onConfigChange() {
    auto& cfg = Settings::instance().config();
    if (alarmMgr) {
        alarmMgr->setLimits(cfg.alarmLow, cfg.alarmHigh, cfg.hysteresis, cfg.alarmHoldSec * 1000);
    }
    buzzer.setMuted(cfg.buzzerMuted);
    
    if (cfg.mqttEnabled && strlen(cfg.mqttBroker) > 0) {
        mqttMgr.begin(cfg.mqttBroker, cfg.mqttPort, cfg.mqttUser, cfg.mqttPass);
        mqttMgr.setEnabled(true);
    } else {
        mqttMgr.setEnabled(false);
    }
}

void onOTAProgress(int percent) {
    otaRunning = true;
    UI::drawOTAProgress(percent);
}

void onOTAComplete(bool success) {
    if (success) {
        UI::drawOTASuccess();
        delay(1200);
        ESP.restart();
    }
}

void setup() {
    Serial.begin(115200);
    
    Settings::instance().load();
    auto& cfg = Settings::instance().config();

    Display::init();
    UI::drawBootAnimation();

    phSensor.begin();
    phSensor.setCalibrationParams(cfg.voltagePH4, cfg.voltagePH7, cfg.voltagePH9, cfg.ph4Value, cfg.ph7Value, cfg.ph9Value);

    tempSensor.begin();
    tempSensor.requestMeasurement();

    buzzer.begin(PIN_BUZZER);
    buzzer.setMuted(cfg.buzzerMuted);

    alarmMgr = new AlarmManager(cfg.alarmLow, cfg.alarmHigh, cfg.hysteresis, cfg.alarmHoldSec * 1000);
    historyLogger.begin();

    // Apply default WiFi from secrets.h (if defined) on first boot
#if defined(DEFAULT_WIFI_SSID) && defined(DEFAULT_WIFI_PASS)
    if (strlen(DEFAULT_WIFI_SSID) > 0 && strlen(cfg.wifiSSID) == 0) {
        strlcpy(cfg.wifiSSID, DEFAULT_WIFI_SSID, sizeof(cfg.wifiSSID));
        strlcpy(cfg.wifiPass, DEFAULT_WIFI_PASS, sizeof(cfg.wifiPass));
        Settings::instance().saveWifi();
        Serial.println("[CFG] Zapisano domyslne WiFi z secrets.h");
    }
#endif

    Serial.println("=== pH Monitor v2.0 ===");

    if (strlen(cfg.wifiSSID) > 0) {
        Serial.printf("[BOOT] Laczenie z WiFi: '%s'...\n", cfg.wifiSSID);
        
        Display::fillScreen(0x0000);
        Display::drawString(10, 10, "Laczenie z WiFi...", 0xFFFF, 0x0000, 2);
        Display::drawString(10, 40, cfg.wifiSSID, 0x07E0, 0x0000, 2);
        
        WiFi.disconnect(true);
        delay(100);
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.setAutoReconnect(true);
        WiFi.begin(cfg.wifiSSID, cfg.wifiPass);
        
        unsigned long wifiStart = millis();
        const unsigned long wifiTimeout = 20000;
        int dots = 0;
        while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < wifiTimeout) {
            delay(500);
            dots++;
            Serial.printf("  ... status=%d (czas: %lums)\n", WiFi.status(), millis() - wifiStart);
            char buf[40];
            snprintf(buf, sizeof(buf), "Proba %d / 40 ...", dots);
            Display::drawString(10, 70, buf, 0xFFFF, 0x0000, 1);
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            apModeActive = false;
            Serial.printf("[WiFi] POLACZONO! IP: %s  RSSI: %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
            
            Display::fillScreen(0x0000);
            Display::drawString(10, 10, "WiFi OK!", 0x07E0, 0x0000, 3);
            Display::drawString(10, 50, WiFi.localIP().toString().c_str(), 0xFFFF, 0x0000, 2);
            delay(2000);
            
            wifiMgr.setupMDNS(HOSTNAME);
            wifiMgr.setupNTP();
        } else {
            apModeActive = true;
            Serial.printf("[WiFi] BLAD! status=%d. Uruchamiam AP.\n", WiFi.status());
            
            Display::fillScreen(0x0000);
            Display::drawString(10, 10, "WiFi BLAD!", 0xF800, 0x0000, 2);
            char statusBuf[40];
            snprintf(statusBuf, sizeof(statusBuf), "Status: %d  SSID: %s", WiFi.status(), cfg.wifiSSID);
            Display::drawString(10, 40, statusBuf, 0xFFFF, 0x0000, 1);
            Display::drawString(10, 55, "Uruchamiam tryb AP...", 0xFFFF, 0x0000, 1);
            delay(3000);
            
            wifiMgr.startAP();
            UI::drawAPMode("PH-Monitor-Setup", wifiMgr.getIP().c_str());
        }
    } else {
        // No WiFi credentials at all — go straight to AP captive portal
        apModeActive = true;
        Serial.println("[BOOT] Brak danych WiFi. Uruchamiam tryb AP.");
        wifiMgr.startAP();
        UI::drawAPMode("PH-Monitor-Setup", wifiMgr.getIP().c_str());
    }

    webServer.setAuthCredentials(cfg.adminUser, cfg.adminPass);
    webServer.setSystemState(&sysState);
    webServer.setHistoryLogger(&historyLogger);
    webServer.setPhSensor(&phSensor);
    webServer.setPushover(&pushover);
    webServer.setCalibrationCallback(onCalibration);
    webServer.setConfigChangeCallback(onConfigChange);
    webServer.setOTAProgressCallback(onOTAProgress);
    webServer.setOTACompleteCallback(onOTAComplete);
    webServer.begin(apModeActive);

    if (cfg.mqttEnabled && strlen(cfg.mqttBroker) > 0 && !apModeActive) {
        mqttMgr.begin(cfg.mqttBroker, cfg.mqttPort, cfg.mqttUser, cfg.mqttPass);
        mqttMgr.setEnabled(true);
    } else {
        mqttMgr.setEnabled(false);
    }

    // Initial pH reading
    float initRaw = phSensor.readRaw();
    float initTemp = tempSensor.read();
    smoothedPH = phSensor.calculatePH(initRaw, initTemp);

    if (!apModeActive) {
        UI::drawStaticUI(cfg.deviceName);
        UI::drawWifi(wifiMgr.isConnected(), wifiMgr.getIP().c_str());
    }
}

void loop() {
    if (apModeActive) {
        wifiMgr.handleDNS();
        delay(10);
        return;
    }
    
    auto& cfg = Settings::instance().config();

    if (cfg.mqttEnabled) {
        mqttMgr.update(cfg.deviceName);
    }

    // Single owner of calibration state machine: loop()
    auto calState = phSensor.updateCalibration();
    if (pendingCalType.length() > 0) {
        if (calState == PhSensor::CalState::DONE) {
            float v = phSensor.getCalibrationVoltage();
            
            // Create temporary set of calibration points to validate before committing
            int32_t testV4 = cfg.voltagePH4;
            int32_t testV7 = cfg.voltagePH7;
            int32_t testV9 = cfg.voltagePH9;
            float testP4 = cfg.ph4Value;
            float testP7 = cfg.ph7Value;
            float testP9 = cfg.ph9Value;
            
            bool isPH4 = false;
            bool isPH7 = false;
            bool isPH9 = false;
            
            if (pendingCalType == "neutral" || pendingCalType == "7" || pendingCalType == "7.00") {
                testV7 = (int32_t)round(v);
                testP7 = 7.00f;
                isPH7 = true;
                // If pH4 or pH9 were not yet user-calibrated, adapt their default reference relative to new pH7 offset
                if (!cfg.calibratedPH4) {
                    testV4 = testV7 + (DEFAULT_VOLTAGE_PH4 - DEFAULT_VOLTAGE_PH7);
                }
                if (!cfg.calibratedPH9) {
                    testV9 = testV7 - (DEFAULT_VOLTAGE_PH7 - DEFAULT_VOLTAGE_PH9);
                }
            } else if (pendingCalType == "acid" || pendingCalType == "4" || pendingCalType == "4.01") {
                testV4 = (int32_t)round(v);
                testP4 = 4.01f;
                isPH4 = true;
            } else if (pendingCalType == "base" || pendingCalType == "9" || pendingCalType == "9.18") {
                testV9 = (int32_t)round(v);
                testP9 = 9.18f;
                isPH9 = true;
            } else if (pendingCalType == "custom") {
                if (pendingCalCustomPH >= 6.5f && pendingCalCustomPH <= 7.5f) {
                    testV7 = (int32_t)round(v);
                    testP7 = pendingCalCustomPH;
                    isPH7 = true;
                } else if (pendingCalCustomPH < 6.5f) {
                    testV4 = (int32_t)round(v);
                    testP4 = pendingCalCustomPH;
                    isPH4 = true;
                } else {
                    testV9 = (int32_t)round(v);
                    testP9 = pendingCalCustomPH;
                    isPH9 = true;
                }
            }
            
            // Perform validation
            String valReason = "";
            bool valid = PhSensor::validateCalibration(testV4, testV7, testV9, &valReason);
            if (valid) {
                cfg.voltagePH4 = testV4;
                cfg.voltagePH7 = testV7;
                cfg.voltagePH9 = testV9;
                cfg.ph4Value = testP4;
                cfg.ph7Value = testP7;
                cfg.ph9Value = testP9;
                
                if (isPH4) cfg.calibratedPH4 = true;
                if (isPH7) cfg.calibratedPH7 = true;
                if (isPH9) cfg.calibratedPH9 = true;
                cfg.calibrated = Settings::instance().isCalibrationComplete();
                
                Settings::instance().saveCalibration();
                phSensor.setCalibrationParams(cfg.voltagePH4, cfg.voltagePH7, cfg.voltagePH9, cfg.ph4Value, cfg.ph7Value, cfg.ph9Value);
                phSensor.setCalibrationDone();
                Serial.printf("[CALIB] Kalibracja punktu '%s' zakonczona sukcesem! (v=%d mV)\n", pendingCalType.c_str(), (int)round(v));
            } else {
                phSensor.setCalibrationFailed(valReason);
                Serial.printf("[CALIB] Odrzucono kalibracje: %s\n", valReason.c_str());
            }
            pendingCalType = "";
        } else if (calState == PhSensor::CalState::FAILED) {
            pendingCalType = "";
        }
    }

    unsigned long currentMillis = millis();

    if (currentMillis - lastSampleTime >= SAMPLE_INTERVAL_MS) {
        lastSampleTime = currentMillis;

        float temp = tempSensor.read();
        bool tempConn = tempSensor.isConnected();
        tempSensor.requestMeasurement();

        float rawVoltage = phSensor.readRaw();
        float calcPH = phSensor.calculatePH(rawVoltage, temp);
        smoothedPH = phSensor.applyEMA(smoothedPH, calcPH);

        if (!statsInitialized) {
            minPH = maxPH = smoothedPH;
            minTemp = maxTemp = tempConn ? temp : 25.0f;
            statsInitialized = true;
        } else {
            if (smoothedPH < minPH) minPH = smoothedPH;
            if (smoothedPH > maxPH) maxPH = smoothedPH;
            if (tempConn) {
                if (temp < minTemp) minTemp = temp;
                if (temp > maxTemp) maxTemp = temp;
            }
        }

        auto alarmState = alarmMgr->update(smoothedPH);
        if (alarmMgr->stateChanged()) {
            if (cfg.pushoverEnabled && strlen(cfg.pushoverUser) > 0 && strlen(cfg.pushoverToken) > 0) {
                char msg[96];
                if (alarmState == AlarmManager::AlarmState::ALARM_LOW) {
                    snprintf(msg, sizeof(msg), "ALARM! pH ponizej progu: %.2f (prog: %.2f)", smoothedPH, cfg.alarmLow);
                } else if (alarmState == AlarmManager::AlarmState::ALARM_HIGH) {
                    snprintf(msg, sizeof(msg), "ALARM! pH powyzej progu: %.2f (prog: %.2f)", smoothedPH, cfg.alarmHigh);
                } else {
                    snprintf(msg, sizeof(msg), "ALARM SKASOWANY. pH w normie: %.2f", smoothedPH);
                }
                pushover.send(msg, cfg.pushoverUser, cfg.pushoverToken);
            }
        }
        
        bool isAlarm = (alarmState != AlarmManager::AlarmState::NORMAL);
        buzzer.update(isAlarm);

        sysState.ph = smoothedPH;
        sysState.voltage = rawVoltage / 1000.0f; // Volts for UI
        sysState.temperature = temp;
        sysState.tempConnected = tempConn;
        sysState.alarmState = static_cast<uint8_t>(alarmState);
        sysState.wifiConnected = wifiMgr.isConnected();
        sysState.rssi = wifiMgr.getRSSI();
        sysState.ip = wifiMgr.getIP();
        sysState.ntpSynced = wifiMgr.isNTPSynced();
        sysState.ntpTime = wifiMgr.getTimeString();
        sysState.uptime = millis() / 1000;
        sysState.calibrated = Settings::instance().isCalibrationComplete();
        sysState.pushoverStatus = static_cast<uint8_t>(pushover.getLastStatus());
        sysState.buzzerMuted = buzzer.isMuted();
        sysState.minPH = minPH;
        sysState.maxPH = maxPH;
        sysState.minTemp = minTemp;
        sysState.maxTemp = maxTemp;
        sysState.statsInitialized = statsInitialized;

        phHistory[historyIndex] = smoothedPH;
        historyIndex = (historyIndex + 1) % HISTORY_SIZE;
        if (historyCount < HISTORY_SIZE) historyCount++;

        if (!otaRunning) {
            UI::drawPHValue(smoothedPH, cfg.alarmLow, cfg.alarmHigh);
            UI::drawVoltage(rawVoltage / 1000.0f);
            UI::drawTemperature(tempConn ? temp : -127.0f);
            UI::drawAlarm(isAlarm);
            UI::drawHistory(phHistory, historyCount, historyIndex, HISTORY_SIZE, cfg.alarmLow, cfg.alarmHigh);
            UI::drawWifi(sysState.wifiConnected, sysState.ip.c_str());
            UI::drawUptime(sysState.uptime);
        }

        if (mqttMgr.isConnected()) {
            mqttMgr.publishState(smoothedPH, tempConn ? temp : -127.0f, rawVoltage / 1000.0f, static_cast<uint8_t>(alarmState), wifiMgr.getRSSI());
        }
    }

    if (currentMillis - lastHistoryLogTime >= HISTORY_LOG_INTERVAL) {
        lastHistoryLogTime = currentMillis;
        HistoryRecord rec;
        rec.timestamp = sysState.ntpSynced ? time(nullptr) : (millis() / 1000); 
        rec.ph = smoothedPH;
        rec.temperature = tempSensor.read();
        rec.voltage = phSensor.getLastVoltage() / 1000.0f;
        rec.alarmState = static_cast<uint8_t>(alarmMgr->getState());
        historyLogger.logRecord(rec);
    }
}
