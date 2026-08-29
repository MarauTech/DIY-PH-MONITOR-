#include "pushover.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

static const char PUSHOVER_ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogZiUwwg7CRNDApS3bBs7kODrk0KO/YFPcA+A7P/9ak0sTH
P3k/EfVeIG1Hrcj3r0USjIMj7DygUhZCfEbkHADPBMOAhcfoVMy32DVB14cRSAFh
nhMGJAEOBFoAqfUj3pDacuNLC2TRHQbp7IHsFUt0QGVkA7NsHlSjAY2Bj2VhzJrs
6sDWJzEgVA5dPaj+VLq/IuAoMBgpT+M9r2TOk37B5K3FiBhzIHAHG2ZqAVPAGKLI
6jHMdl4GxM0P4hPbBHJnFi1bcMhqAb3S0oLMSwMnB07FGCkkC3cE0jLe0l0N1aq+
tKLEHYbga5Nh4vAzRNtMEvDUzOQ3JNH5lfCE3MUYh8KEJVrB3zM2xmISmMi/gMdZ
SRKsqgGKiKxhIXQQARkzFPQ5UyOBIAbfQbJ9GkPfSQx7GfcGIlHALKOIpMQ+Z6ng
1V6F35B9atRC6nr5GsdmjVIJleTwcnDB0sw30a9k7QIh1pCzW+JZBVwp/7c/cGd4
-----END CERTIFICATE-----
)EOF";

String urlEncode(const char* msg) {
    String encodedMsg = "";
    while (*msg) {
        char c = *msg;
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encodedMsg += c;
        } else if (c == ' ') {
            encodedMsg += '+';
        } else {
            char buf[4];
            snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
            encodedMsg += buf;
        }
        msg++;
    }
    return encodedMsg;
}

void Pushover::send(const char* message, const char* user, const char* token) {
    if (busy) return;
    if (!message || !user || !token || strlen(user) == 0 || strlen(token) == 0) {
        lastStatus = Status::NOT_CONFIGURED;
        return;
    }
    
    strncpy(msgBuffer, message, sizeof(msgBuffer) - 1);
    strncpy(userKey, user, sizeof(userKey) - 1);
    strncpy(apiToken, token, sizeof(apiToken) - 1);
    
    busy = true;
    lastStatus = Status::SENDING;
    
    xTaskCreate(pushoverTask, "pushover_task", 4096, this, 1, NULL);
}

void Pushover::pushoverTask(void* parameter) {
    Pushover* instance = static_cast<Pushover*>(parameter);
    instance->executeSend();
    instance->busy = false;
    vTaskDelete(NULL);
}

void Pushover::executeSend() {
    if (WiFi.status() != WL_CONNECTED) {
        lastStatus = Status::NO_WIFI;
        return;
    }

    WiFiClientSecure client;
    client.setCACert(PUSHOVER_ROOT_CA);
    
    if (!client.connect("api.pushover.net", 443)) {
        lastStatus = Status::TLS_ERROR;
        return;
    }
    
    String postData = "token=" + String(apiToken) + "&user=" + String(userKey) + "&message=" + urlEncode(msgBuffer);
    
    client.println("POST /1/messages.json HTTP/1.1");
    client.println("Host: api.pushover.net");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.print(postData);
    
    unsigned long timeout = millis();
    while (client.connected() && !client.available()) {
        if (millis() - timeout > 10000) {
            client.stop();
            lastStatus = Status::TLS_ERROR;
            return;
        }
        delay(10);
    }
    
    if (client.available()) {
        String line = client.readStringUntil('\n');
        if (line.indexOf("200 OK") != -1) {
            lastStatus = Status::OK;
        } else if (line.indexOf("4") != -1) {
            lastStatus = Status::HTTP_4XX;
        } else if (line.indexOf("5") != -1) {
            lastStatus = Status::HTTP_5XX;
        } else {
            lastStatus = Status::HTTP_5XX; // Default generic error
        }
    }
    client.stop();
}

Pushover::Status Pushover::getLastStatus() const {
    return lastStatus;
}

bool Pushover::isBusy() const {
    return busy;
}

const char* Pushover::getStatusText() const {
    switch(lastStatus) {
        case Status::NOT_CONFIGURED: return "Nie skonfigurowano";
        case Status::OK: return "Wysłano pomyślnie";
        case Status::NO_WIFI: return "Brak WiFi";
        case Status::DNS_ERROR: return "Błąd DNS";
        case Status::TLS_ERROR: return "Błąd połączenia TLS";
        case Status::HTTP_4XX: return "Błąd 4xx (złe dane)";
        case Status::HTTP_5XX: return "Błąd serwera (5xx)";
        case Status::SENDING: return "Wysyłanie...";
        default: return "Nieznany błąd";
    }
}
