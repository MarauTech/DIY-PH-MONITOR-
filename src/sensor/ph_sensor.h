#pragma once
#include <Arduino.h>
#include "defaults.h"

class PhSensor {
public:
    enum class CalState { IDLE, COLLECTING, DONE, FAILED };

    PhSensor();
    
    void begin();
    float readRaw();
    float calculatePH(float voltageMV, float temperatureC);
    float applyEMA(float previous, float newValue);
    float getLastVoltage() const;

    CalState startCalibration();
    CalState updateCalibration();
    
    CalState getCalibrationState() const;
    float getCalibrationVoltage() const;
    float getCalibrationStdDev() const;
    int getCalibrationProgress() const;
    String getCalibrationError() const;
    
    void setCalibrationDone();
    void setCalibrationFailed(const String& err);

    static bool validateCalibration(int32_t vPH4, int32_t vPH7, int32_t vPH9, String* reason = nullptr);

    void setCalibrationParams(int32_t vPH4, int32_t vPH7, int32_t vPH9, float p4, float p7, float p9);

private:
    float lastVoltage;
    
    int32_t voltagePH4;
    int32_t voltagePH7;
    int32_t voltagePH9;
    float ph4Value;
    float ph7Value;
    float ph9Value;

    CalState calState;
    uint32_t calStartTime;
    uint32_t calLastSampleTime;
    String calErrorStr;
    
    static constexpr uint32_t CAL_SAMPLE_INTERVAL_MS = 50; 
    static constexpr size_t CAL_MAX_SAMPLES = CAL_STABILITY_PERIOD_MS / CAL_SAMPLE_INTERVAL_MS;
    
    float calSamples[CAL_MAX_SAMPLES];
    size_t calSampleCount;

    float calResultVoltage;
    float calResultStdDev;
};
