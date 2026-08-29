#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "defaults.h"

#ifndef HISTORY_FILE
#define HISTORY_FILE "/history.jsonl"
#endif

#ifndef HISTORY_MAX_RECORDS
#define HISTORY_MAX_RECORDS 1000
#endif

struct HistoryRecord {
    time_t timestamp;
    float ph;
    float temperature;
    float voltage;
    uint8_t alarmState;  // 0=normal, 1=low, 2=high
};

class HistoryLogger {
public:
    HistoryLogger();
    ~HistoryLogger();

    bool begin();
    void logRecord(const HistoryRecord& record);
    String getHistoryJSON(int maxRecords = 100);
    void clear();
    size_t getRecordCount();
    bool isAvailable();

private:
    void rotateHistory();
    
    bool _isMounted;
    size_t _recordCount;
    const char* _filename;
};
