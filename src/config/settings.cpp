#include "settings.h"
#include <math.h>
#include <string.h>

Settings& Settings::instance() {
    static Settings _instance;
    return _instance;
}

void Settings::load() {
    _prefs.begin(NAMESPACE, true);
    
    _config.voltagePH4 = _prefs.getInt("vPh4", DEFAULT_VOLTAGE_PH4);
    _config.voltagePH7 = _prefs.getInt("vPh7", DEFAULT_VOLTAGE_PH7);
    _config.voltagePH9 = _prefs.getInt("vPh9", DEFAULT_VOLTAGE_PH9);
    _config.ph4Value = _prefs.getFloat("ph4Val", DEFAULT_PH4_VALUE);
    _config.ph7Value = _prefs.getFloat("ph7Val", DEFAULT_PH7_VALUE);
    _config.ph9Value = _prefs.getFloat("ph9Val", DEFAULT_PH9_VALUE);
    _config.calibrated = _prefs.getBool("calib", false);
    
    _config.alarmLow = _prefs.getFloat("alLow", DEFAULT_ALARM_LOW);
    _config.alarmHigh = _prefs.getFloat("alHigh", DEFAULT_ALARM_HIGH);
    _config.hysteresis = _prefs.getFloat("alHyst", DEFAULT_HYSTERESIS);
    _config.alarmHoldSec = _prefs.getUInt("alHold", DEFAULT_ALARM_HOLD_S);
    
    _prefs.getString("wifiSsid", "").toCharArray(_config.wifiSSID, sizeof(_config.wifiSSID));
    _prefs.getString("wifiPass", "").toCharArray(_config.wifiPass, sizeof(_config.wifiPass));
    
    _prefs.getString("adUser", "admin").toCharArray(_config.adminUser, sizeof(_config.adminUser));
    _prefs.getString("adPass", "admin").toCharArray(_config.adminPass, sizeof(_config.adminPass));
    
    _config.pushoverEnabled = _prefs.getBool("poEna", false);
    _prefs.getString("poUser", "").toCharArray(_config.pushoverUser, sizeof(_config.pushoverUser));
    _prefs.getString("poToken", "").toCharArray(_config.pushoverToken, sizeof(_config.pushoverToken));
    
    _config.mqttEnabled = _prefs.getBool("mqEna", false);
    _prefs.getString("mqBrok", "").toCharArray(_config.mqttBroker, sizeof(_config.mqttBroker));
    _config.mqttPort = _prefs.getInt("mqPort", DEFAULT_MQTT_PORT);
    _prefs.getString("mqUser", "").toCharArray(_config.mqttUser, sizeof(_config.mqttUser));
    _prefs.getString("mqPass", "").toCharArray(_config.mqttPass, sizeof(_config.mqttPass));
    
    _prefs.getString("devName", "pH Monitor").toCharArray(_config.deviceName, sizeof(_config.deviceName));
    _config.buzzerMuted = _prefs.getBool("buzMute", false);
    
    _prefs.end();
}

void Settings::save() {
    _prefs.begin(NAMESPACE, false);
    
    _prefs.putInt("vPh4", _config.voltagePH4);
    _prefs.putInt("vPh7", _config.voltagePH7);
    _prefs.putInt("vPh9", _config.voltagePH9);
    _prefs.putFloat("ph4Val", _config.ph4Value);
    _prefs.putFloat("ph7Val", _config.ph7Value);
    _prefs.putFloat("ph9Val", _config.ph9Value);
    _prefs.putBool("calib", _config.calibrated);
    
    _prefs.putFloat("alLow", _config.alarmLow);
    _prefs.putFloat("alHigh", _config.alarmHigh);
    _prefs.putFloat("alHyst", _config.hysteresis);
    _prefs.putUInt("alHold", _config.alarmHoldSec);
    
    _prefs.putString("wifiSsid", _config.wifiSSID);
    _prefs.putString("wifiPass", _config.wifiPass);
    
    _prefs.putString("adUser", _config.adminUser);
    _prefs.putString("adPass", _config.adminPass);
    
    _prefs.putBool("poEna", _config.pushoverEnabled);
    _prefs.putString("poUser", _config.pushoverUser);
    _prefs.putString("poToken", _config.pushoverToken);
    
    _prefs.putBool("mqEna", _config.mqttEnabled);
    _prefs.putString("mqBrok", _config.mqttBroker);
    _prefs.putInt("mqPort", _config.mqttPort);
    _prefs.putString("mqUser", _config.mqttUser);
    _prefs.putString("mqPass", _config.mqttPass);
    
    _prefs.putString("devName", _config.deviceName);
    _prefs.putBool("buzMute", _config.buzzerMuted);
    
    _prefs.end();
}

void Settings::saveCalibration() {
    _prefs.begin(NAMESPACE, false);
    _prefs.putInt("vPh4", _config.voltagePH4);
    _prefs.putInt("vPh7", _config.voltagePH7);
    _prefs.putInt("vPh9", _config.voltagePH9);
    _prefs.putFloat("ph4Val", _config.ph4Value);
    _prefs.putFloat("ph7Val", _config.ph7Value);
    _prefs.putFloat("ph9Val", _config.ph9Value);
    _prefs.putBool("calib", _config.calibrated);
    _prefs.end();
}

void Settings::saveAlarm() {
    _prefs.begin(NAMESPACE, false);
    _prefs.putFloat("alLow", _config.alarmLow);
    _prefs.putFloat("alHigh", _config.alarmHigh);
    _prefs.putFloat("alHyst", _config.hysteresis);
    _prefs.putUInt("alHold", _config.alarmHoldSec);
    _prefs.end();
}

void Settings::saveWifi() {
    _prefs.begin(NAMESPACE, false);
    _prefs.putString("wifiSsid", _config.wifiSSID);
    _prefs.putString("wifiPass", _config.wifiPass);
    _prefs.end();
}

void Settings::savePushover() {
    _prefs.begin(NAMESPACE, false);
    _prefs.putBool("poEna", _config.pushoverEnabled);
    _prefs.putString("poUser", _config.pushoverUser);
    _prefs.putString("poToken", _config.pushoverToken);
    _prefs.end();
}

void Settings::saveMqtt() {
    _prefs.begin(NAMESPACE, false);
    _prefs.putBool("mqEna", _config.mqttEnabled);
    _prefs.putString("mqBrok", _config.mqttBroker);
    _prefs.putInt("mqPort", _config.mqttPort);
    _prefs.putString("mqUser", _config.mqttUser);
    _prefs.putString("mqPass", _config.mqttPass);
    _prefs.end();
}

void Settings::saveAdmin() {
    _prefs.begin(NAMESPACE, false);
    _prefs.putString("adUser", _config.adminUser);
    _prefs.putString("adPass", _config.adminPass);
    _prefs.end();
}

void Settings::resetAll() {
    _prefs.begin(NAMESPACE, false);
    _prefs.clear();
    _prefs.end();
    
    // Reset to defaults
    _config = Config();
}

Config& Settings::config() {
    return _config;
}

bool Settings::hasWifiCredentials() {
    return strlen(_config.wifiSSID) > 0;
}

bool Settings::validateAlarmConfig(float low, float high, float hyst) {
    if (isnan(low) || isnan(high) || isnan(hyst)) return false;
    if (isinf(low) || isinf(high) || isinf(hyst)) return false;
    if (low < 0.0f || high > 14.0f) return false;
    if (low >= high) return false;
    if (hyst <= 0.0f || hyst >= (high - low) / 2.0f) return false;
    return true;
}

bool Settings::validateMqttPort(int port) {
    return port > 0 && port <= 65535;
}
