#include "buzzer.h"

Buzzer::Buzzer() : buzzerPin(-1), mutedState(false), lastToggleTime(0), isOn(false) {}

void Buzzer::begin(int8_t pin) {
    buzzerPin = pin;
    if (buzzerPin >= 0) {
        pinMode(buzzerPin, OUTPUT);
        digitalWrite(buzzerPin, LOW);
    }
}

void Buzzer::update(bool alarmActive) {
    if (buzzerPin < 0) return;
    
    if (!alarmActive || mutedState) {
        if (isOn) {
            digitalWrite(buzzerPin, LOW);
            isOn = false;
        }
        return;
    }
    
    uint32_t now = millis();
    if (now - lastToggleTime >= 500) {
        lastToggleTime = now;
        isOn = !isOn;
        digitalWrite(buzzerPin, isOn ? HIGH : LOW);
    }
}

void Buzzer::setMuted(bool muted) {
    mutedState = muted;
    if (muted && isOn && buzzerPin >= 0) {
        digitalWrite(buzzerPin, LOW);
        isOn = false;
    }
}

bool Buzzer::isMuted() const {
    return mutedState;
}
