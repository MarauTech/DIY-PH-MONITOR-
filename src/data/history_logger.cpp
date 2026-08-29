#include "history_logger.h"

HistoryLogger::HistoryLogger() : _isMounted(false), _recordCount(0), _filename(HISTORY_FILE) {
}

HistoryLogger::~HistoryLogger() {
}

bool HistoryLogger::begin() {
    // Attempt to mount LittleFS, format if it fails
    if (LittleFS.begin(true)) {
        _isMounted = true;
        _recordCount = getRecordCount();
        return true;
    }
    return false;
}

bool HistoryLogger::isAvailable() {
    return _isMounted;
}

void HistoryLogger::logRecord(const HistoryRecord& record) {
    if (!_isMounted) return;

    if (_recordCount >= HISTORY_MAX_RECORDS) {
        rotateHistory();
    }

    File f = LittleFS.open(_filename, FILE_APPEND);
    if (!f) return;

    // Use StaticJsonDocument for lightweight allocation (ArduinoJson 6/7 compatible)
    // In ArduinoJson 7 it ignores the size template argument but remains valid syntax.
#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    StaticJsonDocument<256> doc;
#endif

    doc["t"] = record.timestamp;
    
    // Format to 2 decimal places using math rounding to avoid String allocs
    doc["p"] = round(record.ph * 100.0) / 100.0;
    doc["te"] = round(record.temperature * 10.0) / 10.0;
    doc["v"] = round(record.voltage * 1000.0) / 1000.0;
    doc["a"] = record.alarmState;

    String line;
    serializeJson(doc, line);
    line += "\n";

    if (f.print(line)) {
        _recordCount++;
    }
    f.close();
}

String HistoryLogger::getHistoryJSON(int maxRecords) {
    if (!_isMounted) return "[]";

    File f = LittleFS.open(_filename, FILE_READ);
    if (!f) return "[]";

    size_t totalLines = _recordCount;
    size_t skipLines = 0;
    
    if (totalLines > (size_t)maxRecords) {
        skipLines = totalLines - maxRecords;
    }

    String result = "[";
    size_t currentLine = 0;
    bool firstRecord = true;

    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (line.length() > 0) {
            if (currentLine >= skipLines) {
                if (!firstRecord) {
                    result += ",";
                }
                result += line;
                firstRecord = false;
            }
            currentLine++;
        }
    }
    
    f.close();
    result += "]";
    return result;
}

void HistoryLogger::clear() {
    if (!_isMounted) return;
    if (LittleFS.exists(_filename)) {
        LittleFS.remove(_filename);
    }
    _recordCount = 0;
}

size_t HistoryLogger::getRecordCount() {
    if (!_isMounted) return 0;
    File f = LittleFS.open(_filename, FILE_READ);
    if (!f) return 0;

    size_t count = 0;
    while (f.available()) {
        if (f.read() == '\n') {
            count++;
        }
    }
    f.close();
    return count;
}

void HistoryLogger::rotateHistory() {
    if (!_isMounted) return;
    
    File src = LittleFS.open(_filename, FILE_READ);
    if (!src) return;

    size_t linesToSkip = _recordCount / 4; // Skip first 25%
    size_t currentLine = 0;

    String tempFile = String(_filename) + ".tmp";
    File dst = LittleFS.open(tempFile, FILE_WRITE);
    if (!dst) {
        src.close();
        return;
    }

    while (src.available()) {
        String line = src.readStringUntil('\n');
        if (line.length() > 0) {
            if (currentLine >= linesToSkip) {
                dst.print(line);
                dst.print('\n');
            }
            currentLine++;
        }
    }

    src.close();
    dst.close();

    LittleFS.remove(_filename);
    LittleFS.rename(tempFile, _filename);
    
    _recordCount = currentLine - linesToSkip;
}
