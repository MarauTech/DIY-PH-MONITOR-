#pragma once
#include <Arduino.h>

class AlarmManager {
public:
    enum class AlarmState { NORMAL, ALARM_LOW, ALARM_HIGH };

    AlarmManager(float lowLimit, float highLimit, float hysteresis, uint32_t holdTimeMs);

    AlarmState update(float pH);
    bool stateChanged() const;
    AlarmState getState() const;
    void setLimits(float low, float high, float hysteresis, uint32_t holdMs);

private:
    float limitLow;
    float limitHigh;
    float hyst;
    uint32_t holdTime;

    AlarmState currentState;
    AlarmState previousState;
    bool changed;

    uint32_t crossingTime;
    bool isCrossing;
};
