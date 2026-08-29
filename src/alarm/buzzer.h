#pragma once
#include <Arduino.h>

class Buzzer {
public:
    Buzzer();
    void begin(int8_t pin);
    void update(bool alarmActive);
    void setMuted(bool muted);
    bool isMuted() const;

private:
    int8_t buzzerPin;
    bool mutedState;
    uint32_t lastToggleTime;
    bool isOn;
};
