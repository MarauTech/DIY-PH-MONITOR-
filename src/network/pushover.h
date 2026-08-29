#pragma once
#include <Arduino.h>

class Pushover {
public:
    enum class Status { NOT_CONFIGURED, OK, NO_WIFI, DNS_ERROR, TLS_ERROR, HTTP_4XX, HTTP_5XX, SENDING };
    
    void send(const char* message, const char* user, const char* token);
    Status getLastStatus() const;
    const char* getStatusText() const;
    bool isBusy() const;

private:
    Status lastStatus = Status::NOT_CONFIGURED;
    bool busy = false;
    char msgBuffer[256] = {0};
    char userKey[33] = {0};
    char apiToken[33] = {0};

    static void pushoverTask(void* parameter);
    void executeSend();
};
