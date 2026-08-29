#include "temperature_sensor.h"
#include "pins.h"

#ifndef PIN_DS18B20
#define PIN_DS18B20 4
#endif

TemperatureSensor::TemperatureSensor() : oneWire(PIN_DS18B20), sensors(&oneWire), connected(false) {}

void TemperatureSensor::begin() {
    sensors.begin();
    sensors.setWaitForConversion(false);
    connected = (sensors.getDeviceCount() > 0);
}

void TemperatureSensor::requestMeasurement() {
    if (connected) {
        sensors.requestTemperatures();
    }
}

float TemperatureSensor::read() {
    if (!connected) return -127.0f;
    return sensors.getTempCByIndex(0);
}

bool TemperatureSensor::isConnected() {
    return connected;
}
