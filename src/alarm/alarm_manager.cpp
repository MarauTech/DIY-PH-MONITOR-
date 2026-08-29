#include "alarm_manager.h"

AlarmManager::AlarmManager(float lowLimit, float highLimit, float hysteresis, uint32_t holdTimeMs) 
    : limitLow(lowLimit), limitHigh(highLimit), hyst(hysteresis), holdTime(holdTimeMs),
      currentState(AlarmState::NORMAL), previousState(AlarmState::NORMAL), changed(false),
      crossingTime(0), isCrossing(false) {}
      
AlarmManager::AlarmState AlarmManager::update(float pH) {
    uint32_t now = millis();
    changed = false;
    previousState = currentState;

    if (currentState == AlarmState::NORMAL) {
        if (pH > limitHigh || pH < limitLow) {
            if (!isCrossing) {
                isCrossing = true;
                crossingTime = now;
            } else if (now - crossingTime >= holdTime) {
                currentState = (pH > limitHigh) ? AlarmState::ALARM_HIGH : AlarmState::ALARM_LOW;
                isCrossing = false;
                changed = true;
            }
        } else {
            isCrossing = false;
        }
    } else if (currentState == AlarmState::ALARM_HIGH) {
        if (pH < (limitHigh - hyst)) {
            currentState = AlarmState::NORMAL;
            changed = true;
        }
    } else if (currentState == AlarmState::ALARM_LOW) {
        if (pH > (limitLow + hyst)) {
            currentState = AlarmState::NORMAL;
            changed = true;
        }
    }
    
    return currentState;
}

bool AlarmManager::stateChanged() const {
    return changed;
}

AlarmManager::AlarmState AlarmManager::getState() const {
    return currentState;
}

void AlarmManager::setLimits(float low, float high, float hysteresis, uint32_t holdMs) {
    limitLow = low;
    limitHigh = high;
    hyst = hysteresis;
    holdTime = holdMs;
}
