# Smart pH Monitor & Logger (ESP32)

Profesjonalny, bezprzewodowy system monitorowania odczynu pH oraz temperatury cieczy oparty na mikrokontrolerze **ESP32**, z kolorowym wyświetlaczem **TFT ST7789 (320x240 px)** w układzie poziomym, nowoczesnym panelem webowym (Web Dashboard) oraz automatycznymi powiadomieniami push (**Pushover**).

Projekt został zaprojektowany z naciskiem na stabilność odczytów analogowych, prostotę kalibracji sondy bezpośrednio z poziomu przeglądarki oraz estetykę interfejsu graficznego.

---

## 🌟 Główne Funkcje Systemu

### 📺 Interfejs Urządzenia (TFT ST7789 320x240 px)
* **Układ Poziomy (Landscape):** Zoptymalizowany pod kątem czytelności.
* **Duży odczyt pH:** Dynamicznie zmieniający kolor w zależności od odczynu (kwaśny, neutralny, zasadowy).
* **Słowna kategoryzacja odczynu:** (np. *KWASNE*, *NEUTRALNE*, *ZASADOWE*).
* **Wykres Historii Live:** Renderowany w czasie rzeczywistym wykres ostatnich 60 pomiarów z liniami progów alarmowych i wartościami granicznymi (pH 0, 7, 14).
* **Panel informacyjny:** Aktualne napięcie z sondy (V), temperatura (°C) z czujnika DS18B20, status sieci Wi-Fi, przydzielony adres IP oraz czas pracy urządzenia (Uptime).
* **Sygnalizacja Alarmowa:** Wizualne ostrzeżenie na ekranie w przypadku przekroczenia zdefiniowanych progów pH.

### 🌐 Nowoczesny Web Dashboard (Panel WWW)
* **Responsywny Design:** Ciemny motyw w stylu *Glassmorphic*, dostosowany do telefonów, tabletów i komputerów.
* **Odświeżanie w czasie rzeczywistym:** Wykorzystanie REST API JSON (`/api`) odpytywanego asynchronicznie co 2 sekundy za pomocą technologii AJAX (brak przeładowywania strony).
* **Interaktywny wykres:** Renderowany na elemencie HTML5 Canvas z obsługą wysokiej gęstości pikseli (Retina/High-DPI).
* **Zdalna konfiguracja:** Możliwość zmiany progów alarmowych pH oraz danych do powiadomień Pushover bezpośrednio przez formularz na stronie.

### 🧪 Zaawansowana Kalibracja Sondy (Direct-Web)
* **Podgląd napięcia LIVE:** Widok dokładnego napięcia wyjściowego z czujnika (V) na żywo.
* **Dwuetapowa kalibracja buforowa:** 
  * Wzorzec neutralny: **pH 7.00**
  * Wzorzec kwaśny: **pH 4.01**
  * Wzorzec zasadowy: **pH 9.18**
* Zapisywanie parametrów kalibracyjnych (napięcia bazowego i nachylenia charakterystyki) bezpośrednio do pamięci nieulotnej ESP32 za pomocą jednego kliknięcia.

### 🔔 Powiadomienia Pushover
* Automatyczne wysyłanie powiadomień push na telefon komórkowy za pomocą serwisu **Pushover** przy wejściu w stan alarmowy i po powrocie pH do normy.
* Asynchroniczne wysyłanie żądań HTTPS za pomocą biblioteki `WiFiClientSecure` w dedykowanym zadaniu FreeRTOS (brak blokowania głównej pętli programu).

### 📈 Stabilizacja Pomiarów (Algorytmy Filtrujące)
* **Filtr Medianowy:** Sortowanie próbek metodą insertion-sort i odrzucenie 25% skrajnych wartości (odporność na nagłe szpilki napięcia).
* **Filtr EMA (Exponential Moving Average):** Wygładzanie powolnych zmian napięcia (współczynnik `0.05` skutecznie eliminujący szumy własne ESP32 przy włączonym Wi-Fi).

---

## 🔌 Schemat Połączeń (Wiring)

| Moduł | Pin ESP32 | Opis |
| :--- | :--- | :--- |
| **Wyświetlacz TFT ST7789** | | |
| VCC | 3.3V / 5V | Zasilanie wyświetlacza |
| GND | GND | Wspólna masa |
| SCLK | GPIO 18 | SPI Clock (SCL) |
| MOSI | GPIO 23 | SPI Data (SDA) |
| CS | GPIO 5 | Chip Select (CS) |
| DC | GPIO 17 | Data/Command (DC) |
| RST | GPIO 16 | Reset (RST) |
| BL | GPIO 4 | Podświetlenie (Backlight) |
| **Czujnik pH (np. PH-4502C)** | | |
| VCC | 5V / VIN | **Zalecane stabilne 5V** dla poprawnej pracy op-ampa |
| GND | GND | Wspólna masa |
| PO | GPIO 34 | Wejście analogowe (przez dzielnik rezystorowy 2/3 np. 10kΩ / 20kΩ) |
| DO (Opcjonalnie) | GPIO 35 | Cyfrowe wejście alarmu progowego z czujnika |
| **Czujnik temperatury DS18B20**| | |
| VCC | 3.3V / 5V | Zasilanie |
| GND | GND | Wspólna masa |
| DATA | GPIO 14 | Magistrala 1-Wire (wymaga rezystora podciągającego 4.7kΩ do VCC) |

---

## 🛠️ Technologie i Biblioteki

Projekt został zbudowany w środowisku **PlatformIO** z użyciem frameworka **Arduino** dla platformy **Espressif 32**.

Główne zależności (zdefiniowane w `platformio.ini`):
* **AsyncTCP & ESPAsyncWebServer** (autor: mathieucarbou) – wydajny, asynchroniczny serwer WWW obsługujący zapytania REST i statyczne pliki HTML.
* **DallasTemperature & OneWire** – obsługa czujnika temperatury DS18B20.
* **Preferences** (wbudowana w rdzeń ESP32) – obsługa zapisu i odczytu konfiguracji w pamięci Flash (NVS).
* **ESPmDNS** – obsługa protokołu lokalnego rozwiązywania nazw (`ph-monitor.local`).

---

## ⚙️ Pierwsze Uruchomienie i Konfiguracja

1. Skonfiguruj dane swojej sieci Wi-Fi w pliku `include/config.h`:
   ```cpp
   #define WIFI_SSID       "TwojaNazwaSieci"
   #define WIFI_PASSWORD   "TwojeHaslo"
   ```
2. Skompiluj projekt i wgraj oprogramowanie na ESP32 za pomocą PlatformIO:
   ```bash
   pio run --target upload
   ```
3. Odczytaj przydzielony adres IP z dolnej sekcji ekranu TFT urządzenia.
4. Otwórz przeglądarkę na komputerze lub smartfonie w tej samej sieci i wpisz odczytany adres IP (np. `http://192.168.1.150`).
5. Skonfiguruj progi alarmowe pH oraz klucze Pushover w dolnej sekcji strony i kliknij **Zapisz Ustawienia**.

---

## 🧪 Procedura Kalibracji Sondy

1. Przygotuj roztwory buforowe o znanym pH (np. pH 7.00, pH 4.01, pH 9.18).
2. Wyreguluj potencjometr sprzętowy na płytce czujnika pH (ten bliżej gniazda BNC), tak aby przy zanurzeniu sondy w buforze **pH 7.00** (lub po zwarciu pinu środkowego BNC z obudową) urządzenie wskazywało na ekranie napięcie jak najbliższe **2.500 V**.
3. **Kalibracja programowa na stronie WWW:**
   * Zanurz sondę w buforze **pH 7.00**, odczekaj na ustabilizowanie się odczytu napięcia na karcie kalibracji i kliknij **Kalibruj pH 7.00 (Neutralny)**.
   * Przepłucz sondę wodą destylowaną i umieść w buforze **pH 4.01**. Gdy odczyt napięcia przestanie się zmieniać, kliknij **Kalibruj pH 4.01 (Kwaśny)**. 
   * (Opcjonalnie) Powtórz proces dla bufora **pH 9.18** klikając **Kalibruj pH 9.18 (Zasadowy)**.
4. Parametry kalibracji zostaną natychmiast zapisane w pamięci flash i zastosowane do bieżących pomiarów.

---

## 👨‍💻 Autor

Projekt stworzony i rozwijany przez: **Wisnia**
