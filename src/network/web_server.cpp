#include "web_server.h"
#include <LittleFS.h>
#include <Update.h>
#include <AsyncJson.h>
#include "defaults.h"

static const char WIFI_SETUP_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="pl">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>pH Monitor — Konfiguracja WiFi</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,-apple-system,sans-serif;background:#0b0e18;color:#dde1ef;display:flex;justify-content:center;align-items:center;min-height:100vh;padding:1rem}
.card{background:#131726;border:1px solid #1e2235;border-radius:16px;padding:2rem;max-width:420px;width:100%;box-shadow:0 10px 25px rgba(0,0,0,0.5)}
h1{font-size:1.4rem;color:#4ade80;margin-bottom:0.4rem;text-align:center}
p.sub{font-size:0.85rem;color:#718096;text-align:center;margin-bottom:1.5rem}
.form-group{margin-bottom:1.2rem}
label{display:block;font-size:0.8rem;color:#a0aec0;margin-bottom:0.4rem;font-weight:600}
input[type="text"],input[type="password"]{width:100%;background:#0b0e18;border:1px solid #2d3748;border-radius:8px;padding:0.7rem;color:#fff;font-size:0.95rem}
input:focus{border-color:#3b82f6;outline:none}
.btn{width:100%;background:#2563eb;color:#fff;border:none;border-radius:8px;padding:0.8rem;font-size:1rem;font-weight:600;cursor:pointer;transition:background 0.2s;margin-top:0.5rem}
.btn:hover{background:#1d4ed8}
.ota-box{margin-top:1.5rem;padding-top:1.2rem;border-top:1px solid #1e2235;text-align:center;font-size:0.8rem;color:#718096}
.ota-box a{color:#60a5fa;text-decoration:none;font-weight:600}
</style>
</head>
<body>
<div class="card">
  <h1>pH Monitor v2.0</h1>
  <p class="sub">Wprowadź dane sieci Wi-Fi, aby połączyć urządzenie</p>
  <form action="/save-wifi" method="POST">
    <div class="form-group">
      <label for="ssid">Nazwa sieci Wi-Fi (SSID)</label>
      <input type="text" id="ssid" name="ssid" placeholder="Nazwa Twojej sieci domowej" required>
    </div>
    <div class="form-group">
      <label for="password">Hasło Wi-Fi</label>
      <input type="password" id="password" name="password" placeholder="Hasło do sieci">
    </div>
    <button type="submit" class="btn">Zapisz i Połącz</button>
  </form>
  <div class="ota-box">
    <p>Chcesz wgrać pliki strony WWW lub nowy firmware?</p>
    <p><a href="/update">&rarr; Przejdź do Aktualizacji OTA (/update)</a></p>
  </div>
</div>
</body>
</html>
)rawhtml";

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
    // Captive Portal redirects
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *req){ req->redirect("http://192.168.4.1/"); });
    server.on("/gen_204", HTTP_GET, [](AsyncWebServerRequest *req){ req->redirect("http://192.168.4.1/"); });
    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *req){ req->redirect("http://192.168.4.1/"); });
    server.on("/canonical.html", HTTP_GET, [](AsyncWebServerRequest *req){ req->redirect("http://192.168.4.1/"); });
    server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *req){ req->redirect("http://192.168.4.1/"); });
    server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *req){ req->redirect("http://192.168.4.1/"); });

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *req){
        if (isAP || !LittleFS.exists("/index.html")) {
            req->send(200, "text/html", WIFI_SETUP_HTML);
        } else {
            req->send(LittleFS, "/index.html", "text/html");
        }
    });
    
    server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *req){
        req->send(200, "text/html", WIFI_SETUP_HTML);
    });

    server.on("/save-wifi", HTTP_POST, [](AsyncWebServerRequest *req){
        String ssid = "";
        String pass = "";
        if (req->hasParam("ssid", true)) ssid = req->getParam("ssid", true)->value();
        if (req->hasParam("password", true)) pass = req->getParam("password", true)->value();
        
        if (ssid.length() == 0) {
            req->send(400, "text/html", "<p>Blad: Nazwa sieci SSID jest wymagana. <a href='/'>Wroc</a></p>");
            return;
        }
        
        auto& cfg = Settings::instance().config();
        strlcpy(cfg.wifiSSID, ssid.c_str(), sizeof(cfg.wifiSSID));
        strlcpy(cfg.wifiPass, pass.c_str(), sizeof(cfg.wifiPass));
        Settings::instance().saveWifi();
        
        req->send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Restart...</title><style>body{font-family:sans-serif;background:#0b0e18;color:#dde1ef;text-align:center;padding:3rem}.box{background:#131726;padding:2rem;border-radius:12px;display:inline-block;border:1px solid #1e2235}h2{color:#4ade80}</style></head><body><div class='box'><h2>Zapisano konfiguracje WiFi!</h2><p>Urzadzenie restartuje sie i laczy z siecia: <b>" + ssid + "</b></p><p>Odczytaj przydzielony adres IP z ekranu urzadzenia.</p></div></body></html>");
        
        delay(1500);
        ESP.restart();
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
        doc["wifiSSID"] = cfg.wifiSSID;
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
        
        if (!doc["wifiSSID"].isNull()) {
            const char* s = doc["wifiSSID"];
            if (s && strlen(s) > 0) {
                strlcpy(cfg.wifiSSID, s, sizeof(cfg.wifiSSID));
            }
        }
        if (!doc["wifiPass"].isNull()) {
            const char* wp = doc["wifiPass"];
            if (wp && strlen(wp) > 0) {
                strlcpy(cfg.wifiPass, wp, sizeof(cfg.wifiPass));
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

    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *req){
        req->send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Aktualizacja OTA</title><style>body{font-family:sans-serif;background:#0b0e18;color:#dde1ef;display:flex;justify-content:center;align-items:center;min-height:100vh;margin:0}.box{background:#131726;padding:2rem;border-radius:12px;border:1px solid #1e2235;text-align:center;max-width:400px}input[type=file]{margin:1rem 0;color:#fff}input[type=submit]{background:#2563eb;color:#fff;border:none;padding:0.6rem 1.5rem;border-radius:6px;cursor:pointer;font-weight:600}a{color:#60a5fa;text-decoration:none;display:inline-block;margin-top:1rem}</style></head><body><div class='box'><h2>Aktualizacja OTA</h2><p style='color:#718096;font-size:0.85rem'>Wybierz plik <b>firmware.bin</b> lub <b>littlefs.bin</b></p><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update' required><br><input type='submit' value='Rozpocznij aktualizacje'></form><a href='/'>&larr; Powrot</a></div></body></html>");
    });
    
    server.on("/update", HTTP_POST, [this](AsyncWebServerRequest *req){
        bool shouldReboot = !Update.hasError();
        AsyncWebServerResponse *res = req->beginResponse(200, "text/plain", shouldReboot ? "OK" : "FAIL");
        res->addHeader("Connection", "close");
        req->send(res);
        if (onOTAComplete) onOTAComplete(shouldReboot);
    }, [this](AsyncWebServerRequest *req, String filename, size_t index, uint8_t *data, size_t len, bool final){
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

    server.onNotFound([this](AsyncWebServerRequest *req){
        if (isAP) {
            req->redirect("http://192.168.4.1/");
        } else {
            req->send(404, "text/plain", "Nie znaleziono (404)");
        }
    });
}
