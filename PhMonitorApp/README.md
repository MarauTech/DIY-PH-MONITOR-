# MarauTech pH Monitor — Aplikacja Mobilna Android

Natywna aplikacja na system Android (Kotlin, Jetpack Compose, Material 3) dedykowana do bezprzewodowego monitoringu, kalibracji i konfiguracji urządzenia **DIY pH Monitor** opartego na mikrokontrolerze ESP32.

---

## 📱 Najważniejsze Funkcje i Ekrany

### 1. Dashboard (Ekran Główny)
- **Hero Card pH**: Bardzo duży, czytelny odczyt aktualnej wartości pH wraz z napięciem przetwornika oraz animowaną odznaką stanu:
  - `NORMAL` (Szmaragdowy — parametry w normie)
  - `LOW pH` (Ciepły bursztyn / ostrzeżenie o kwasowości)
  - `HIGH pH` (Magenta / ostrzeżenie o zasadowości)
  - `OFFLINE` (Neutralny szary — brak łączności LAN)
- **Karty pomocnicze**: Temperatura z czujnika DS18B20 (°C), jakość sygnału WiFi (RSSI w dBm).
- **Stan urządzenia**: Uptime (czas pracy), wersja firmware, status synchronizacji czasu NTP oraz stan kalibracji.
- **Szybkie akcje**: Pull-to-refresh, ręczne odświeżanie oraz szybkie przejścia do kalibracji i historii.

### 2. Historia Pomiarów (Wykresy)
- Dedykowany, lekki wykres liniowy napisany w Jetpack Compose Canvas (zero WebView, zero ciężkich bibliotek).
- Przełączanie metryk: **Poziom pH**, **Temperatura**, **Napięcie sondy**.
- Filtrowanie zakresów czasowych: **1h**, **6h**, **12h**, **24h**, **Wszystko**.
- Kafelki podsumowania statystycznego: **Minimum**, **Maksimum**, **Średnia** oraz **Ostatni pomiar**.

### 3. Kalibracja Sondy
- Obsługa 3 standardowych buforów kalibracyjnych:
  - **pH 7.00** (neutralny)
  - **pH 4.01** (kwaśny)
  - **pH 9.18** (zasadowy)
- Obsługa **własnego bufora pH** (np. 6.86).
- Podgląd odczytu sondy na żywo (V i przeliczone pH).
- Pasek postępu i odpytywanie endpointu `/api/calibrate/status` z polskimi komunikatami błędów (`measurement_not_stable`, `invalid_voltage`, `points_too_close`, `invalid_slope`).
- Bezpieczny przycisk przywrócenia nastaw fabrycznych z potwierdzeniem.

### 4. Więcej (Centrum Konfiguracji)
- **Ustawienia Alarmów**: Konfiguracja dolnego i górnego progu pH, histerezy, czasu potwierdzenia alarmu (Hold Sec) oraz wyciszenia fizycznego buzzera.
- **Powiadomienia w tle (WorkManager)**: Włączanie monitorowania w tle, alerty LOW/HIGH oraz powiadomienie przy powrocie do normy.
- **Pushover**: Wprowadzanie klucza użytkownika, tokenu aplikacji oraz przycisk wysyłki testowego powiadomienia.
- **MQTT**: Konfiguracja brokera, portu (1883), loginu, hasła i integracji z Home Assistant MQTT Discovery.
- **Diagnostyka**: Szczegółowe parametry pamięci RAM (Free Heap), czasu NTP, sygnału RSSI z przyciskiem **"Kopiuj Diagnostykę"** do schowka (bez sekretów).
- **Połączenie**: Zmiana IP, portu, loginu i hasła administratora.

---

## 🔔 Powiadomienia w Tle (WorkManager)

Aplikacja łączy się bezpośrednio z ESP32 po sieci lokalnej LAN (HTTP), bez konieczności utrzymywania zewnętrznych serwerów w chmurze czy subskrypcji Firebase FCM.

- **Mechanizm**: Wykorzystano systemowy `WorkManager` (`DeviceMonitorWorker`), który w tle cyklicznie odpytuje endpoint `/api/status`.
- **Kanały Powiadomień**:
  - `Alarmy pH` (High Importance, dźwięk, wibracja)
  - `Status urządzenia` (Default Importance)
- **Ochrona przed spamem**: Powiadomienie jest wysyłane **wyłącznie przy zmianie stanu** (np. przejście z `NORMAL` do `LOW pH` lub `HIGH pH`, a także opcjonalnie przy powrocie z alarmu do `NORMAL`).
- **Autostart po restarcie telefonu**: Zaimplementowano `BootReceiver`, który automatycznie przywraca zadanie WorkManagera po ponownym uruchomieniu urządzenia.
- **Ograniczenia systemu Android**: Zgodnie z wytycznymi Androida dotyczącymi oszczędzania energii (Doze Mode), minimalny interwał okresowych zadań `WorkManager` wynosi 15 minut.

---

## 🖼️ Widżety na Ekran Główny (App Widgets)

Aplikacja dostarcza 3 estetyczne, dopracowane wizualnie widżety:
1. **Kompaktowy (1x1 / 2x1)**: Wartość pH, odznaka stanu (`NORMAL`/`LOW`/`HIGH`), godzina ostatniej aktualizacji.
2. **Średni (2x2 / 3x2)**: Nazwa urządzenia, pH, Temperatura, Odznaka stanu, Czas odczytu oraz przycisk odświeżenia.
3. **Pełny (4x2 / 4x3)**: Nazwa urządzenia, IP, pH, Temperatura, Napięcie sondy, Uptime, Odznaka stanu, Czas odczytu oraz szybkie przejście do aplikacji.

**Aktualizacja widżetów następuje automatycznie:**
- Po każdym odświeżeniu danych w otwartej aplikacji.
- Po każdym cyklu monitoringu w tle przez `WorkManager`.
- Po zmianie konfiguracji urządzenia.
- Po kliknięciu w widżet następuje natychmiastowe otwarcie Dashboardu.

---

## 🔒 Bezpieczeństwo i Komunikacja LAN

- **Cleartext HTTP**: Aplikacja komunikuje się w sieci LAN bezpośrednio z mikrokontrolerem ESP32 (`http://IP_ESP32`). W pliku `AndroidManifest.xml` włączono `android:usesCleartextTraffic="true"` dla bezpośrednich zapytań lokalnych.
- **Przechowywanie danych**:
  - Adres IP, port, ustawienia powiadomień i pamięć podręczna offline przechowywane są w `Jetpack DataStore`.
  - Hasło administratora przechowywane jest bezpiecznie za pomocą `EncryptedSharedPreferences` / `Android Keystore`.
  - Hasła MQTT i tokeny Pushover są czyszczone z pamięci formularza po zapisie i nigdy nie są eksponowane w logach.

---

## 🛠️ Budowanie Aplikacji (Kompilacja APK)

Projekt można otworzyć w **Android Studio** (Ladybug / Hedgehog / nowszym) lub zbudować bezpośrednio z wiersza poleceń:

### 1. Budowanie wersji Debug:
```bash
./gradlew assembleDebug
```
Wygenerowany plik APK:
`app/build/outputs/apk/debug/app-debug.apk`

### 2. Budowanie wersji Release:
```bash
./gradlew assembleRelease
```
Wygenerowany plik APK:
`app/build/outputs/apk/release/app-release.apk`

---

## 📋 Wymagania
- Android 8.0 (API Level 26) lub nowszy.
- Połączenie telefonu z tą samą siecią WiFi co monitor pH ESP32.
- Java JDK 17+.
