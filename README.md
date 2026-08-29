# Smart DIY pH Monitor & Logger v2.0 (ESP32)

[![PlatformIO Build](https://github.com/MarauTech/DIY-PH-MONITOR-/actions/workflows/build.yml/badge.svg)](https://github.com/MarauTech/DIY-PH-MONITOR-/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-red.svg)](https://www.espressif.com/)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-teal.svg)](https://www.arduino.cc/)

Profesjonalny, modularny i bezprzewodowy system ciągłego monitorowania odczynu pH oraz temperatury cieczy oparty na mikrokontrolerze **ESP32**, kolorowym wyświetlaczu **TFT ST7789 (320x240 px)** w układzie poziomym, nowoczesnym panelu webowym **SPA (LittleFS)**, integracji **Home Assistant (MQTT)** oraz bezpiecznych powiadomieniach push **Pushover (TLS z weryfikacją certyfikatu CA)**.

---

## 🌟 Główne Funkcje Firmware v2.0

### 🧪 Precyzyjny Pomiar pH i Zaawansowana Kalibracja
- **Skalibrowany przetwornik ADC:** Wykorzystanie fabrycznej kalibracji eFuse ESP32 (`analogReadMilliVolts`) zamiast uproszczonego przeliczania bitów.
- **Wielopunktowe próbkowanie z filtrami:** 32 próbki, algorytm sortowania przez wstawianie (Insertion Sort), odrzucenie 25% wartości skrajnych (IQR Trim) oraz filtr wykładniczy EMA ($\alpha = 0.05$).
- **Odcinkowa kalibracja 3-punktowa (Piecewise-Linear):** Osobne nachylenia charakterystyki dla zakresu kwaśnego (pH 4.01 ↔ 7.00) i zasadowego (pH 7.00 ↔ 9.18).
- **Obsługa własnego buforu:** Możliwość kalibracji dowolnym roztworem (np. pH 6.86).
- **Asynchroniczny test stabilności (Stability Control):** Maszyna stanów analizująca odchylenie standardowe ($\sigma \le 5\text{ mV}$) w oknie 5 sekund przed zatwierdzeniem punktu kalibracji.
- **Walidacja matematyczna:** Ochrona przed dzieleniem przez zero, kontrola minimalnych różnic napięć ($\Delta V \ge 100\text{ mV}$) i poprawności kierunku nachylenia.
- **Automatyczna Kompensacja Temperaturowa (ATC):** Równanie Nernsta zintegrowane z odczytem z czujnika DS18B20.

### 🛡️ Niezawodny System Alarmowy
- **Histereza:** Konfigurowalny margines wyjścia ze stanu alarmowego (domyślnie $0.10\text{ pH}$).
- **Czas potwierdzenia (Hold Time):** Eliminacja fałszywych alarmów przy chwilowych wahaniach (domyślnie $15\text{ s}$).
- **Trzystanowa maszyna stanów:** `NORMAL`, `ALARM_LOW`, `ALARM_HIGH`.
- **Wyciszany buzzer:** Opcjonalna sygnalizacja dźwiękowa z funkcją wyciszenia (Mute).

### 🌐 Nowoczesny Panel WWW (Single Page Application na LittleFS)
- **Czysty podział kodu:** Pliki `index.html`, `style.css` i `app.js` serwowane bezpośrednio z partycji LittleFS.
- **Responsywny interfejs Glassmorphic:** Tryb ciemny i jasny (Dark/Light mode z zapamiętywaniem w `localStorage`).
- **Wykres Canvas Live:** Rysowanie historii pomiarów z liniami progowymi, siatką i gradientami.
- **Eksport do CSV:** Bezpośrednie pobieranie historii pomiarów do pliku `.csv`.
- **Powiadomienia przeglądarkowe:** Obsługa HTML5 Web Notifications API.

### 🔒 Bezpieczeństwo i Architektura
- **Brak wycieków danych:** REST API nigdy nie zwraca wrażliwych tokenów Pushover ani haseł.
- **HTTP Basic Auth:** Ochrona paneli konfiguracyjnych i kalibracji konfigurowalnym loginem i hasłem administratora.
- **Prawdziwy TLS dla Pushover:** Pełna weryfikacja certyfikatu Root CA (**ISRG Root X1**) bez niebezpiecznego `setInsecure()`.
- **Asynchroniczność:** Zadania sieciowe i powiadomienia wykonywane w tle we FreeRTOS.

### 📡 Sieć, OTA, NTP i MQTT
- **WiFi Provisioning:** Automatyczny tryb Access Point (`PH-Monitor-Setup`, IP `192.168.4.1`) z Captive Portal przy braku skonfigurowanej sieci.
- **Aktualizacje OTA:** Bezprzewodowa aktualizacja firmware przez przeglądarkę (`/update`) z dedykowanym ekranem serwisowym na TFT.
- **NTP Time Sync:** Automatyczna synchronizacja czasu i strefy czasowej dla logów i wykresów.
- **Trwałe logowanie historii:** Zapis historii do pliku `/history.jsonl` na LittleFS z mechanizmem rotacji (do 24h z krokiem 1 minuty).
- **Home Assistant Auto-Discovery:** Publikacja encji i stanów przez protokół MQTT.

---

## 🔌 Schemat Połączeń (Wiring)

| Moduł / Funkcja | Pin ESP32 | Opis / Uwagi |
| :--- | :--- | :--- |
| **Wyświetlacz TFT ST7789** | | |
| VCC | 3.3V / 5V | Zasilanie wyświetlacza |
| GND | GND | Masa |
| SCLK (SCL) | **GPIO 18** | SPI Clock |
| MOSI (SDA) | **GPIO 23** | SPI Data |
| CS | **GPIO 5** | Chip Select |
| DC | **GPIO 17** | Data / Command |
| RST | **GPIO 16** | Reset |
| BL | **GPIO 4** | Podświetlenie ekranu (Backlight) |
| **Czujnik pH (PH-4502C)** | | |
| VCC | **5V / VIN** | Wymagane stabilne 5V dla wzmacniacza operacyjnego |
| GND | GND | Wspólna masa |
| PO | **GPIO 34** | Wyjście analogowe (przez dzielnik 2/3 np. 10kΩ / 20kΩ) |
| DO | **GPIO 35** | Opcjonalne cyfrowe wyjście alarmu modułu |
| **Termometr DS18B20** | | |
| VCC | 3.3V / 5V | Zasilanie |
| GND | GND | Masa |
| DATA | **GPIO 14** | Magistrala 1-Wire (wymagany rezystor pull-up 4.7kΩ do VCC) |
| **Buzzer (Opcjonalny)** | | |
| SIG | **GPIO -1** | Domyślnie wyłączony (`-1`). Zdefiniuj pin w `include/pins.h` |

---

## 📁 Struktura Projektu

```
DIY-PH-MONITOR/
├── .github/
│   └── workflows/
│       └── build.yml             # Automatyczny CI w GitHub Actions
├── data/                         # Pliki serwowane przez LittleFS
│   ├── app.js                    # Logika frontendu SPA, wykres Canvas, REST API
│   ├── index.html                # Interfejs WWW urządzenia
│   └── style.css                 # Style CSS (Dark / Light Theme)
├── include/
│   ├── defaults.h                # Wartości domyślne, wersja firmware, stałe
│   ├── pins.h                    # Mapowanie sprzętowe pinów ESP32
│   └── secrets.h.example         # Szablon sekretów / domyślnych danych admina
├── src/
│   ├── alarm/
│   │   ├── alarm_manager.cpp/.h  # Maszyna stanów alarmu z histerezą i debounce
│   │   └── buzzer.cpp/.h         # Sterownik buzzera z wyciszaniem
│   ├── config/
│   │   └── settings.cpp/.h       # Singleton konfiguracji NVS (Preferences)
│   ├── data/
│   │   └── history_logger.cpp/.h # Logger historii pomiarów LittleFS (JSONL + rotacja)
│   ├── display/
│   │   ├── display.cpp/.h        # Niskopoziomowy sterownik SPI ST7789
│   │   ├── font5x7.h             # Czcionka bitmapowa PROGMEM
│   │   └── ui.cpp/.h             # Renderowanie interfejsu TFT, wykresu i ekranu OTA
│   ├── network/
│   │   ├── mqtt_client.cpp/.h    # Klient MQTT + Home Assistant Discovery
│   │   ├── pushover.cpp/.h       # Asynchroniczne powiadomienia HTTPS TLS (ISRG Root X1)
│   │   ├── web_server.cpp/.h     # Asynchroniczny serwer WWW, REST API i OTA
│   │   └── wifi_manager.cpp/.h   # WiFi STA, AP Captive Portal, mDNS, NTP
│   ├── sensor/
│   │   ├── ph_sensor.cpp/.h      # Kalibracja 3-punktowa, pomiary ADC, ATC, EMA
│   │   └── temperature_sensor.cpp/.h # Obsługa 1-Wire DS18B20
│   └── main.cpp                  # Główny punkt wejścia i koordynator pętli zdarzeń
└── platformio.ini                # Konfiguracja PlatformIO, biblioteki, partycje
```

---

## 🚀 Kompilacja i Uruchomienie

Projekt został w pełni skonfigurowany pod środowisko **PlatformIO**.

### 1. Klonowanie repozytorium
```bash
git clone https://github.com/MarauTech/DIY-PH-MONITOR-.git
cd DIY-PH-MONITOR-
```

### 2. Budowa i wgranie firmware
```bash
# Kompilacja kodu firmware
pio run

# Wgranie firmware na podłączony ESP32
pio run --target upload
```

### 3. Wgranie plików interfejsu WWW (LittleFS)
```bash
# Zbudowanie i wgranie obrazu partycji LittleFS z katalogu data/
pio run --target uploadfs
```

---

## ⚙️ Pierwsze Uruchomienie i WiFi Provisioning

1. Przy pierwszym uruchomieniu (lub braku znanej sieci) ESP32 automatycznie uruchomi punkt dostępowy **`PH-Monitor-Setup`** (adres IP: `192.168.4.1`).
2. Na ekranie TFT wyświetlą się dane dostępowe AP.
3. Połącz się telefonem lub laptopem z siecią `PH-Monitor-Setup` — otworzy się Captive Portal (lub przejdź pod `http://192.168.4.1`).
4. Wpisz nazwę swojej domowej sieci WiFi oraz hasło i zapisz. Urządzenie zrestartuje się i połączy z Twoją siecią.
5. Aktualny adres IP oraz adres mDNS (`http://ph-monitor.local`) zostaną wyświetlone w stopce ekranu TFT.

Domyślne dane logowania do panelu administratora:
- **Użytkownik:** `admin`
- **Hasło:** `admin`
*(Zmień je niezwłocznie w sekcji „Bezpieczeństwo” panelu WWW).*

---

## 🧪 Procedura Kalibracji Sondy pH

### Krok 1: Wstępna regulacja sprzętowa (Potencjometr offsetu)
1. Zewrzyj styk środkowy gniazda BNC modułu PH-4502C z jego obudową (symulacja pH 7.00) lub zanurz sondę w roztworze buforowym **pH 7.00**.
2. Za pomocą potencjometru umieszczonego bliżej gniazda BNC ustaw napięcie na ekranie urządzenia jak najbliższe **2.500 V**.

### Krok 2: Kalibracja programowa 3-punktowa na WWW
1. **Punkt neutralny (pH 7.00):** Zanurz sondę w buforze pH 7.00, odczekaj 30 sekund i kliknij **Kalibruj pH 7.00**. Urządzenie wykona automatyczny test stabilności (5s) i zapisze punkt w pamięci flash.
2. **Punkt kwaśny (pH 4.01):** Przepłucz sondę wodą destylowaną, zanurz w buforze pH 4.01 i kliknij **Kalibruj pH 4.01**.
3. **Punkt zasadowy (pH 9.18):** Przepłucz sondę wodą destylowaną, zanurz w buforze pH 9.18 i kliknij **Kalibruj pH 9.18**.
4. *(Opcjonalnie)* Dla buforów o niestandardowym pH (np. pH 6.86) wpisz wartość w polu **Własny Bufor** i zatwierdź.

---

## 📡 REST API Dokumentacja

Wszystkie żądania zwracają nagłówki JSON i wspierają CORS.

| Metoda | Endpoint | Wymaga Auth | Opis |
| :--- | :--- | :---: | :--- |
| `GET` | `/api/status` | Nie | Zwraca aktualne odczyty, stan alarmu, RSSI, czas, diagnostykę i wersję |
| `GET` | `/api/config` | **Tak** | Zwraca bieżącą konfigurację urządzenia (bez haseł i tokenów) |
| `POST` | `/api/config` | **Tak** | Aktualizuje ustawienia progów, histerezy, pushover, MQTT, itp. |
| `POST` | `/api/calibrate` | **Tak** | Rozpoczyna procedurę kalibracji dla wybranego punktu |
| `GET` | `/api/calibrate/status` | Nie | Zwraca postęp i status stabilności kalibracji |
| `GET` | `/api/history` | Nie | Zwraca ostatnie rekordy historii pomiarów z LittleFS |
| `POST` | `/api/pushover/test` | **Tak** | Wysyła testowe powiadomienie push |
| `POST` | `/api/reset-stats` | **Tak** | Resetuje statystyki Min/Max odczytów |
| `GET` / `POST` | `/update` | **Tak** | Formularz i endpoint aktualizacji firmware OTA |

---

## 🏠 Integracja z Home Assistant (MQTT)

Po włączeniu obsługi MQTT w panelu WWW i podaniu adresu brokera, urządzenie automatycznie wysyła konfigurację **Home Assistant Auto-Discovery**. W systemie pojawią się sensory:
- `sensor.ph_monitor_ph` — odczyn pH wody
- `sensor.ph_monitor_temperature` — temperatura cieczy
- `sensor.ph_monitor_voltage` — napięcie wyjściowe z sondy
- `binary_sensor.ph_monitor_alarm` — stan alarmu progowego

---

## 👨‍💻 Autor i Licencja

Projekt rozwijany przez: **Wisnia** ([MarauTech](https://github.com/MarauTech))  
Licencja: **MIT** — szczegóły w pliku [LICENSE](LICENSE).
