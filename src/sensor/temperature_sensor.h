#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class TemperatureSensor {
public:
    TemperatureSensor();
    void begin();
    void requestMeasurement();
    float read();
    bool isConnected();

private:
    OneWire oneWire;
    DallasTemperature sensors;
    bool connected;
};
