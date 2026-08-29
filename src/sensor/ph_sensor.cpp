#include "ph_sensor.h"
#include "pins.h"
#include "defaults.h"
#include <math.h>

#ifndef PIN_PH_ANALOG
#define PIN_PH_ANALOG 34
#endif
#ifndef VOLTAGE_DIVIDER
#define VOLTAGE_DIVIDER 1.0f
#endif
#ifndef EMA_ALPHA
#define EMA_ALPHA 0.2f
#endif

PhSensor::PhSensor() : lastVoltage(0), voltagePH4(1500), voltagePH7(1000), voltagePH9(700),
                       ph4Value(4.01f), ph7Value(7.00f), ph9Value(9.18f),
                       calState(CalState::IDLE), calSampleCount(0), calResultVoltage(0), calResultStdDev(0) {}

void PhSensor::begin() {
    analogReadResolution(12);
    analogSetPinAttenuation(PIN_PH_ANALOG, ADC_11db);
}

float PhSensor::readRaw() {
    float samples[SAMPLE_COUNT];
    for (size_t i = 0; i < SAMPLE_COUNT; ++i) {
        samples[i] = analogReadMilliVolts(PIN_PH_ANALOG);
    }
    
    for (size_t i = 1; i < SAMPLE_COUNT; ++i) {
        float key = samples[i];
        int j = i - 1;
        while (j >= 0 && samples[j] > key) {
            samples[j + 1] = samples[j];
            j--;
        }
        samples[j + 1] = key;
    }
    
    size_t trim = SAMPLE_COUNT / 4;
    float sum = 0;
    for (size_t i = trim; i < SAMPLE_COUNT - trim; ++i) {
        sum += samples[i];
    }
    float avg = sum / (SAMPLE_COUNT - 2 * trim);
    lastVoltage = avg / VOLTAGE_DIVIDER;
    return lastVoltage;
}

float PhSensor::calculatePH(float voltageMV, float temperatureC) {
    float tempFactor = (temperatureC < -50.0f) ? 1.0f : (temperatureC + 273.15f) / 298.15f;
    float pH = 7.0f;
    
    if (voltageMV >= voltagePH7) {
        float slope = (float)(voltagePH4 - voltagePH7) / (ph7Value - ph4Value);
        if (slope != 0.0f) {
            pH = ph7Value - (voltageMV - voltagePH7) / (slope * tempFactor);
        }
    } else {
        float slope = (float)(voltagePH7 - voltagePH9) / (ph9Value - ph7Value);
        if (slope != 0.0f) {
            pH = ph7Value + (voltagePH7 - voltageMV) / (slope * tempFactor);
        }
    }
    
    if (pH < 0.0f) pH = 0.0f;
    if (pH > 14.0f) pH = 14.0f;
    return pH;
}

float PhSensor::applyEMA(float previous, float newValue) {
    return (EMA_ALPHA * newValue) + ((1.0f - EMA_ALPHA) * previous);
}

float PhSensor::getLastVoltage() const {
    return lastVoltage;
}

PhSensor::CalState PhSensor::startCalibration() {
    calState = CalState::COLLECTING;
    calStartTime = millis();
    calLastSampleTime = 0;
    calSampleCount = 0;
    calResultVoltage = 0;
    calResultStdDev = 0;
    return calState;
}

PhSensor::CalState PhSensor::updateCalibration() {
    if (calState != CalState::COLLECTING) return calState;
    
    uint32_t now = millis();
    
    if (now - calLastSampleTime >= CAL_SAMPLE_INTERVAL_MS) {
        calLastSampleTime = now;
        if (calSampleCount < CAL_MAX_SAMPLES) {
            calSamples[calSampleCount++] = readRaw();
        }
    }
    
    if (now - calStartTime >= CAL_STABILITY_PERIOD_MS) {
        if (calSampleCount == 0) {
            calState = CalState::FAILED;
            return calState;
        }
        
        float sum = 0;
        for (size_t i = 0; i < calSampleCount; ++i) {
            sum += calSamples[i];
        }
        float mean = sum / calSampleCount;
        
        float varianceSum = 0;
        for (size_t i = 0; i < calSampleCount; ++i) {
            varianceSum += (calSamples[i] - mean) * (calSamples[i] - mean);
        }
        
        calResultStdDev = sqrt(varianceSum / calSampleCount);
        calResultVoltage = mean;
        
        if (calResultStdDev > CAL_STABILITY_MAX_DEV_MV) {
            calState = CalState::FAILED;
        } else {
            calState = CalState::DONE;
        }
    }
    
    return calState;
}

float PhSensor::getCalibrationVoltage() const {
    return calResultVoltage;
}

float PhSensor::getCalibrationStdDev() const {
    return calResultStdDev;
}

bool PhSensor::validateCalibration(int32_t vPH4, int32_t vPH7, int32_t vPH9, String* reason) {
    if (vPH4 < CAL_MIN_VOLTAGE_MV || vPH4 > CAL_MAX_VOLTAGE_MV ||
        vPH7 < CAL_MIN_VOLTAGE_MV || vPH7 > CAL_MAX_VOLTAGE_MV ||
        vPH9 < CAL_MIN_VOLTAGE_MV || vPH9 > CAL_MAX_VOLTAGE_MV) {
        if (reason) *reason = "Napiecie poza zakresem";
        return false;
    }
    
    if (abs(vPH4 - vPH7) < CAL_MIN_POINT_DIFF_MV || abs(vPH7 - vPH9) < CAL_MIN_POINT_DIFF_MV) {
        if (reason) *reason = "Zbyt mala roznica miedzy punktami";
        return false;
    }
    
    if (vPH4 <= vPH7 || vPH7 <= vPH9) {
        if (reason) *reason = "Nieprawidlowe nachylenie, napiecia musza byc vPH4 > vPH7 > vPH9";
        return false;
    }
    
    return true;
}

void PhSensor::setCalibrationParams(int32_t vPH4, int32_t vPH7, int32_t vPH9, float p4, float p7, float p9) {
    voltagePH4 = vPH4;
    voltagePH7 = vPH7;
    voltagePH9 = vPH9;
    ph4Value = p4;
    ph7Value = p7;
    ph9Value = p9;
}
