# System Wizualizacji Danych Sensorycznych "Orienta"

## Wprowadzenie

Witaj w repozytorium systemu "Orienta"! Celem tego projektu jest dostarczenie interaktywnego interfejsu do wizualizacji danych pochodzących z różnych typów czujników, takich jak akcelerometry, żyroskopy, magnetometry (IMU - Inertial Measurement Unit) oraz moduły GPS. Aplikacja umożliwia monitorowanie odczytów w czasie rzeczywistym, analizę danych historycznych oraz graficzną reprezentację orientacji przestrzennej i położenia geograficznego.

---

## Główne Funkcjonalności

* Wizualizacja danych z **akcelerometru, żyroskopu i magnetometru** w formie numerycznej (paski postępu) oraz graficznej (wykresy czasowe).
* Renderowanie **orientacji obiektu w przestrzeni 3D** na podstawie otrzymanych danych.
* Wyświetlanie **kursu za pomocą kompasu 2D**.
* Prezentacja **pozycji GPS na interaktywnej mapie OpenStreetMap**.
* Interfejs użytkownika z możliwością **przełączania widoków** i **internacjonalizacji** (obsługa tłumaczeń).

---

## Podstawowe Użycie 

Aplikacja uruchamiana jest poprzez wykonanie skompilowanego pliku binarnego. Główne okno (`MainWindow`) integruje poszczególne moduły wizualizacyjne. Interakcja z danymi odbywa się poprzez graficzny interfejs użytkownika.

---

## Informacje Deweloperskie 🛠️

* **Autor:** Mateusz Wojtaszek
* **Data ostatniej aktualizacji dokumentacji:** 2025-05-19
* **Kod źródłowy:** [https://github.com/MateuszWojtaszek/wds_Orienta](https://github.com/MateuszWojtaszek/wds_Orienta)
* **Środowisko kompilacji:**
    * Qt w wersji 6.10.0
    * Kompilator Apple clang w wersji 17.0.0

---

## Dokumentacja 📄

Szczegółowa dokumentacja kodu została wygenerowana automatycznie za pomocą narzędzia **Doxygen**. Aby ją przejrzeć, otwórz plik `html/index.html` w przeglądarce internetowej po wygenerowaniu dokumentacji.

Dokumentacja została podzielona na następujące główne części:
* **Strona Główna:** Ogólny opis projektu.
* **Lista Klas:** Hierarchiczny spis wszystkich klas z krótkimi opisami.
* **Pliki:** Lista wszystkich plików źródłowych i nagłówkowych.
* **Przestrzenie Nazw:** (jeśli dotyczy) Opis zdefiniowanych przestrzeni nazw.

---

## Plik Główny Aplikacji (`main.cpp`)

* **Opis:** Główny plik źródłowy aplikacji "Orienta". Zawiera punkt wejścia aplikacji (funkcję `main`), która jest odpowiedzialna za inicjalizację środowiska Qt, ustawienie nazwy aplikacji oraz utworzenie i uruchomienie głównego okna.
* **Autor:** Mateusz Wojtaszek
* **Data utworzenia:** 2025-03-19
* **Błędy:** Brak znanych błędów.

Funkcja `main` inicjalizuje `QApplication`, ustawia nazwę aplikacji na "Orienta", tworzy instancję `MainWindow`, wyświetla ją i uruchamia pętlę zdarzeń Qt.
