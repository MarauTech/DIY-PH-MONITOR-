#include "web_server.h"
#include <LittleFS.h>
#include <Update.h>
#include <AsyncJson.h>
#include "defaults.h"

WebServer::WebServer(uint16_t port) : server(port) {}

void WebServer::setAuthCredentials(const char* user, const char* pass) {
    if (user) authUser = user;
    if (pass) authPass = pass;
}

void WebServer::setSystemState(SystemState* state) {
    sysState = state;
}

void WebServer::setHistoryLogger(HistoryLogger* logger) {
    historyLogger = logger;
}

void WebServer::setPhSensor(PhSensor* sensor) {
    phSensor = sensor;
}

void WebServer::setPushover(Pushover* po) {
    pushover = po;
}

void WebServer::setCalibrationCallback(std::function<void(const String&, float)> cb) {
    onCalibrate = cb;
}

void WebServer::setConfigChangeCallback(std::function<void()> cb) {
    onConfigChange = cb;
}

void WebServer::setOTAProgressCallback(std::function<void(int)> cb) {
    onOTAProgress = cb;
}

void WebServer::setOTACompleteCallback(std::function<void(bool)> cb) {
    onOTAComplete = cb;
}

bool WebServer::authenticate(AsyncWebServerRequest* req) {
    if (authUser.length() > 0 && authPass.length() > 0) {
        if (!req->authenticate(authUser.c_str(), authPass.c_str())) {
            req->requestAuthentication();
            return false;
        }
    }
    return true;
}

void WebServer::begin(bool apMode) {
    isAP = apMode;
    setupRoutes();
    server.begin();
}

void WebServer::setupRoutes() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
        if (LittleFS.exists("/index.html")) {
            req->send(LittleFS, "/index.html", "text/html");
        } else {
            req->send(200, "text/html", "<h2>DIY pH Monitor</h2><p>Brak plikow w LittleFS. Wgraj pliki z katalogu data/ lub uzyj OTA.</p><p><a href='/update'>Aktualizacja OTA</a></p>");
        }
    });
    
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *req){
        if (LittleFS.exists("/style.css")) {
            req->send(LittleFS, "/style.css", "text/css");
        } else {
            req->send(404, "text/plain", "Not Found");
        }
    });
    
    server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest *req){
        if (LittleFS.exists("/app.js")) {
            req->send(LittleFS, "/app.js", "application/javascript");
        } else {
            req->send(404, "text/plain", "Not Found");
        }
    });
    
    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *req){
        if (!sysState) {
            req->send(500, "application/json", "{\"error\":\"Brak stanu systemu\"}");
            return;
        }
        AsyncResponseStream *res = req->beginResponseStream("application/json");
        JsonDocument doc;
        doc["ph"] = sysState->ph;
        doc["voltage"] = sysState->voltage;
        if (sysState->tempConnected && sysState->temperature > -50.0f) {
            doc["temperature"] = sysState->temperature;
        } else {
            doc["temperature"] = nullptr;
        }
        doc["tempConnected"] = sysState->tempConnected;
        doc["alarmState"] = sysState->alarmState;
        doc["wifiRSSI"] = sysState->rssi;
        doc["ip"] = sysState->ip;
        doc["uptime"] = sysState->uptime;
        doc["freeHeap"] = ESP.getFreeHeap();
        doc["firmwareVersion"] = FIRMWARE_VERSION;
        doc["ntpSynced"] = sysState->ntpSynced;
        doc["ntpTime"] = sysState->ntpTime;
        doc["calibrated"] = sysState->calibrated;
        doc["pushoverStatus"] = sysState->pushoverStatus;
        if (pushover) {
            doc["pushoverStatusText"] = pushover->getStatusText();
        }
        doc["buzzerMuted"] = sysState->buzzerMuted;
        doc["deviceName"] = Settings::instance().config().deviceName;
        
        if (sysState->statsInitialized) {
            doc["minPH"] = sysState->minPH;
            doc["maxPH"] = sysState->maxPH;
            if (sysState->tempConnected && sysState->minTemp > -50.0f) {
                doc["minTemp"] = sysState->minTemp;
                doc["maxTemp"] = sysState->maxTemp;
            }
        }
        
        auto& cfg = Settings::instance().config();
        doc["voltagePH4"] = cfg.voltagePH4;
        doc["voltagePH7"] = cfg.voltagePH7;
        doc["voltagePH9"] = cfg.voltagePH9;
        doc["ph4Value"] = cfg.ph4Value;
        doc["ph7Value"] = cfg.ph7Value;
        doc["ph9Value"] = cfg.ph9Value;
        
        serializeJson(doc, *res);
        req->send(res);
    });

    server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *req){
        if (!authenticate(req)) return;
        AsyncResponseStream *res = req->beginResponseStream("application/json");
        JsonDocument doc;
        auto& cfg = Settings::instance().config();
        doc["alarmLow"] = cfg.alarmLow;
        doc["alarmHigh"] = cfg.alarmHigh;
        doc["hysteresis"] = cfg.hysteresis;
        doc["alarmHoldSec"] = cfg.alarmHoldSec;
        doc["pushoverConfigured"] = (strlen(cfg.pushoverUser) > 0 && strlen(cfg.pushoverToken) > 0);
        doc["pushoverEnabled"] = cfg.pushoverEnabled;
        doc["mqttEnabled"] = cfg.mqttEnabled;
        doc["mqttBroker"] = cfg.mqttBroker;
        doc["mqttPort"] = cfg.mqttPort;
        doc["mqttUser"] = cfg.mqttUser;
        doc["deviceName"] = cfg.deviceName;
        doc["buzzerMuted"] = cfg.buzzerMuted;
        doc["adminUser"] = cfg.adminUser;
        // Never return pushover tokens or wifi passwords or admin password!
        serializeJson(doc, *res);
        req->send(res);
    });

    server.addHandler(new AsyncCallbackJsonWebHandler("/api/config", [this](AsyncWebServerRequest *req, JsonVariant &json) {
        if (!authenticate(req)) return;
        JsonObject doc = json.as<JsonObject>();
        auto& cfg = Settings::instance().config();
        
        if (!doc["alarmLow"].isNull() || !doc["alarmHigh"].isNull() || !doc["hysteresis"].isNull()) {
            float low = doc["alarmLow"] | cfg.alarmLow;
            float high = doc["alarmHigh"] | cfg.alarmHigh;
            float hyst = doc["hysteresis"] | cfg.hysteresis;
            if (!Settings::validateAlarmConfig(low, high, hyst)) {
                req->send(400, "application/json", "{\"error\":\"Nieprawidlowe progi alarmowe lub histereza (0 <= low < high <= 14, hyst > 0)\"}");
                return;
            }
            cfg.alarmLow = low;
            cfg.alarmHigh = high;
            cfg.hysteresis = hyst;
        }
        
        if (!doc["alarmHoldSec"].isNull()) {
            uint32_t hold = doc["alarmHoldSec"];
            if (hold < 1 || hold > 3600) {
                req->send(400, "application/json", "{\"error\":\"Czas potwierdzenia alarmu musi wynosic 1-3600s\"}");
                return;
            }
            cfg.alarmHoldSec = hold;
        }
        
        if (!doc["deviceName"].isNull()) {
            const char* name = doc["deviceName"];
            if (name && strlen(name) > 0) {
                strlcpy(cfg.deviceName, name, sizeof(cfg.deviceName));
            }
        }
        
        if (!doc["pushoverEnabled"].isNull()) {
            cfg.pushoverEnabled = doc["pushoverEnabled"];
        }
        
        if (!doc["pushoverUser"].isNull()) {
            const char* u = doc["pushoverUser"];
            if (u && strlen(u) > 0) {
                strlcpy(cfg.pushoverUser, u, sizeof(cfg.pushoverUser));
            }
        }
        
        if (!doc["pushoverToken"].isNull()) {
            const char* t = doc["pushoverToken"];
            if (t && strlen(t) > 0) {
                strlcpy(cfg.pushoverToken, t, sizeof(cfg.pushoverToken));
            }
        }
        
        if (!doc["mqttEnabled"].isNull()) {
            cfg.mqttEnabled = doc["mqttEnabled"];
        }
        if (!doc["mqttBroker"].isNull()) {
            strlcpy(cfg.mqttBroker, doc["mqttBroker"] | "", sizeof(cfg.mqttBroker));
        }
        if (!doc["mqttPort"].isNull()) {
            int port = doc["mqttPort"] | 1883;
            if (!Settings::validateMqttPort(port)) {
                req->send(400, "application/json", "{\"error\":\"Nieprawidlowy port MQTT\"}");
                return;
            }
            cfg.mqttPort = port;
        }
        if (!doc["mqttUser"].isNull()) {
            strlcpy(cfg.mqttUser, doc["mqttUser"] | "", sizeof(cfg.mqttUser));
        }
        if (!doc["mqttPass"].isNull()) {
            const char* p = doc["mqttPass"];
            if (p && strlen(p) > 0) {
                strlcpy(cfg.mqttPass, p, sizeof(cfg.mqttPass));
            }
        }
        
        if (!doc["buzzerMuted"].isNull()) {
            cfg.buzzerMuted = doc["buzzerMuted"];
        }
        
        if (!doc["adminUser"].isNull()) {
            const char* au = doc["adminUser"];
            if (au && strlen(au) >= 3) {
                strlcpy(cfg.adminUser, au, sizeof(cfg.adminUser));
            }
        }
        if (!doc["adminPass"].isNull()) {
            const char* ap = doc["adminPass"];
            if (ap && strlen(ap) >= 4) {
                strlcpy(cfg.adminPass, ap, sizeof(cfg.adminPass));
            }
        }
        
        Settings::instance().save();
        setAuthCredentials(cfg.adminUser, cfg.adminPass);
        
        if (onConfigChange) onConfigChange();
        req->send(200, "application/json", "{\"status\":\"ok\"}");
    }));

    server.addHandler(new AsyncCallbackJsonWebHandler("/api/calibrate", [this](AsyncWebServerRequest *req, JsonVariant &json) {
        if (!authenticate(req)) return;
        JsonObject doc = json.as<JsonObject>();
        String type = doc["type"] | "";
        float custom = doc["customPH"] | 7.0f;
        
        if (type == "reset") {
            auto& cfg = Settings::instance().config();
            cfg.voltagePH4 = DEFAULT_VOLTAGE_PH4;
            cfg.voltagePH7 = DEFAULT_VOLTAGE_PH7;
            cfg.voltagePH9 = DEFAULT_VOLTAGE_PH9;
            cfg.ph4Value = DEFAULT_PH4_VALUE;
            cfg.ph7Value = DEFAULT_PH7_VALUE;
            cfg.ph9Value = DEFAULT_PH9_VALUE;
            cfg.calibrated = false;
            Settings::instance().saveCalibration();
            if (phSensor) {
                phSensor->setCalibrationParams(cfg.voltagePH4, cfg.voltagePH7, cfg.voltagePH9, cfg.ph4Value, cfg.ph7Value, cfg.ph9Value);
            }
            req->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Kalibracja zresetowana do wartosci domyslnych\"}");
            return;
        }
        
        if (onCalibrate) {
            onCalibrate(type, custom);
            req->send(200, "application/json", "{\"status\":\"collecting\",\"message\":\"Trwa zbieranie i sprawdzanie stabilnosci probek (5s)...\"}");
        } else {
            req->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Brak modulu kalibracji\"}");
        }
    }));

    server.on("/api/calibrate/status", HTTP_GET, [this](AsyncWebServerRequest *req){
        JsonDocument doc;
        if (!phSensor) {
            doc["status"] = "idle";
        } else {
            auto state = phSensor->updateCalibration();
            if (state == PhSensor::CalState::COLLECTING) {
                doc["status"] = "collecting";
                doc["message"] = "Trwa zbieranie probek i test stabilnosci...";
            } else if (state == PhSensor::CalState::DONE) {
                doc["status"] = "done";
                doc["voltage"] = phSensor->getCalibrationVoltage();
                doc["stdDev"] = phSensor->getCalibrationStdDev();
                doc["message"] = "Pomiar stabilny. Zapisano punkt kalibracji.";
            } else if (state == PhSensor::CalState::FAILED) {
                doc["status"] = "failed";
                doc["error"] = "measurement_not_stable";
                doc["stdDev"] = phSensor->getCalibrationStdDev();
                doc["message"] = "Sygnal niestabilny (odchylenie > 5mV). Poczekaj na ustabilizowanie sondy i sprobuj ponownie.";
            } else {
                doc["status"] = "idle";
            }
        }
        AsyncResponseStream *res = req->beginResponseStream("application/json");
        serializeJson(doc, *res);
        req->send(res);
    });

    server.on("/api/history", HTTP_GET, [this](AsyncWebServerRequest *req){
        int limit = 100;
        if (req->hasParam("limit")) {
            limit = req->getParam("limit")->value().toInt();
            if (limit <= 0) limit = 100;
            if (limit > 1440) limit = 1440;
        }
        if (historyLogger && historyLogger->isAvailable()) {
            String json = historyLogger->getHistoryJSON(limit);
            req->send(200, "application/json", json);
        } else {
            req->send(200, "application/json", "[]");
        }
    });

    server.on("/api/pushover/test", HTTP_POST, [this](AsyncWebServerRequest *req){
        if (!authenticate(req)) return;
        auto& cfg = Settings::instance().config();
        if (strlen(cfg.pushoverUser) == 0 || strlen(cfg.pushoverToken) == 0) {
            req->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Pushover nie jest skonfigurowany\"}");
            return;
        }
        if (pushover) {
            char msg[96];
            snprintf(msg, sizeof(msg), "Test powiadomienia z urzadzenia %s v%s", cfg.deviceName, FIRMWARE_VERSION);
            pushover->send(msg, cfg.pushoverUser, cfg.pushoverToken);
            req->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Wyslano testowe powiadomienie\"}");
        } else {
            req->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Modul Pushover niedostepny\"}");
        }
    });
    
    server.on("/api/reset-stats", HTTP_POST, [this](AsyncWebServerRequest *req){
        if (!authenticate(req)) return;
        if (sysState) {
            sysState->statsInitialized = false;
        }
        req->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.addHandler(new AsyncCallbackJsonWebHandler("/api/wifi/setup", [](AsyncWebServerRequest *req, JsonVariant &json) {
        JsonObject doc = json.as<JsonObject>();
        const char* ssid = doc["ssid"];
        const char* pass = doc["password"];
        if (!ssid || strlen(ssid) == 0) {
            req->send(400, "application/json", "{\"error\":\"Nazwa sieci SSID jest wymagana\"}");
            return;
        }
        auto& cfg = Settings::instance().config();
        strlcpy(cfg.wifiSSID, ssid, sizeof(cfg.wifiSSID));
        strlcpy(cfg.wifiPass, pass ? pass : "", sizeof(cfg.wifiPass));
        Settings::instance().saveWifi();
        
        req->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Zapisano dane WiFi. Restartowanie urzadzenia...\"}");
        delay(1000);
        ESP.restart();
    }));

    server.on("/update", HTTP_GET, [this](AsyncWebServerRequest *req){
        if (!authenticate(req)) return;
        req->send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Aktualizacja OTA</title><style>body{font-family:sans-serif;background:#0b0e18;color:#dde1ef;display:flex;justify-content:center;align-items:center;min-height:100vh;margin:0}.box{background:#131726;padding:2rem;border-radius:12px;border:1px solid #1e2235;text-align:center}input[type=file]{margin:1rem 0;color:#fff}input[type=submit]{background:#2563eb;color:#fff;border:none;padding:0.6rem 1.5rem;border-radius:6px;cursor:pointer;font-weight:600}a{color:#60a5fa;text-decoration:none;display:inline-block;margin-top:1rem}</style></head><body><div class='box'><h2>Aktualizacja OTA</h2><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update' required><br><input type='submit' value='Rozpocznij aktualizacje'></form><a href='/'>&larr; Powrot do panelu</a></div></body></html>");
    });
    
    server.on("/update", HTTP_POST, [this](AsyncWebServerRequest *req){
        if (!authenticate(req)) return;
        bool shouldReboot = !Update.hasError();
        AsyncWebServerResponse *res = req->beginResponse(200, "text/plain", shouldReboot ? "OK" : "FAIL");
        res->addHeader("Connection", "close");
        req->send(res);
        if (onOTAComplete) onOTAComplete(shouldReboot);
    }, [this](AsyncWebServerRequest *req, String filename, size_t index, uint8_t *data, size_t len, bool final){
        if (!authenticate(req)) return;
        if (!index) {
            int cmd = (filename.indexOf("spiffs") > -1 || filename.indexOf("littlefs") > -1) ? U_SPIFFS : U_FLASH;
            if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
                Update.printError(Serial);
            }
        }
        if (!Update.hasError()) {
            if (Update.write(data, len) != len) {
                Update.printError(Serial);
            } else if (onOTAProgress) {
                size_t total = req->contentLength();
                int percent = total > 0 ? ((index + len) * 100 / total) : 0;
                onOTAProgress(percent);
            }
        }
        if (final) {
            if (!Update.end(true)) {
                Update.printError(Serial);
            }
        }
    });
}
