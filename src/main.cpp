/**
 * @mainpage System Wizualizacji Danych Sensorycznych "Orienta"
 *
 * @section intro_sec Wprowadzenie
 * Witaj w dokumentacji systemu "Orienta". Celem tego projektu jest dostarczenie
 * interaktywnego interfejsu do wizualizacji danych pochodzących z różnych typów
 * czujników, takich jak akcelerometry, żyroskopy, magnetometry inaczej IMU (Inertial Measurement Unit)
 * oraz moduły GPS. Aplikacja umożliwia monitorowanie odczytów w czasie rzeczywistym,
 * analizę danych historycznych oraz graficzną reprezentację orientacji
 * przestrzennej i położenia geograficznego.
 *
 * @section features_sec Główne Funkcjonalności
 * - Wizualizacja danych z akcelerometru, żyroskopu i magnetometru w formie numerycznej (paski postępu) oraz graficznej (wykresy czasowe).
 * - Renderowanie orientacji obiektu w przestrzeni 3D na podstawie otrzymanych danych.
 * - Wyświetlanie kursu za pomocą kompasu 2D.
 * - Prezentacja pozycji GPS na interaktywnej mapie OpenStreetMap.
 * - Interfejs użytkownika z możliwością przełączania widoków i internacjonalizacji (obsługa tłumaczeń).
 *
 * @section structure_sec Struktura Dokumentacji
 * Dokumentacja została podzielona na następujące główne części:
 * - **Strona Główna:** (ta strona) Ogólny opis projektu.
 * - **Lista Klas:** Hierarchiczny spis wszystkich klas z krótkimi opisami.
 * - **Pliki:** Lista wszystkich plików źródłowych i nagłówkowych.
 * - **Przestrzenie Nazw:** (jeśli dotyczy) Opis zdefiniowanych przestrzeni nazw.
 *
 * Szczegółowy opis każdej klasy, jej metod i składowych znajduje się na dedykowanych stronach.
 * Zachęcamy do korzystania z paska wyszukiwania oraz drzewa nawigacyjnego.
 *
 * @section usage_sec Podstawowe Użycie
 * Aplikacja uruchamiana jest poprzez wykonanie skompilowanego pliku binarnego.
 * Główne okno (`MainWindow`) integruje poszczególne moduły wizualizacyjne.
 * Interakcja z danymi odbywa się poprzez graficzny interfejs użytkownika.
 *
 * @section development_sec Informacje Deweloperskie
 * @author Mateusz Wojtaszek
 * @date 2025-05-19
 * Kod źródłowy dostępny jest w repozytorium https://github.com/MateuszWojtaszek/wds_Orienta
 * Do kompilacji projektu wykorzystano Qt w wersji 6.10.0 oraz kompilator Apple clang w wersji 17.0.0
 *
 * @note
 * Ta dokumentacja została wygenerowana automatycznie za pomocą narzędzia Doxygen.
 */

/**
 * @file main.cpp
 * @brief Główny plik źródłowy aplikacji "Orienta".
 * @details Zawiera punkt wejścia aplikacji (funkcję `main`), która jest
 * odpowiedzialna za inicjalizację środowiska Qt, ustawienie nazwy aplikacji
 * oraz utworzenie i uruchomienie głównego okna.
 * @author Mateusz Wojtaszek
 * @date 2025-03-19
 * @bug Brak znanych błędów.
 */

#include <QApplication>
#include "MainWindow.h" // Dołączenie definicji klasy głównego okna

/**
 * @brief Główna funkcja aplikacji "Orienta" (punkt wejścia).
 *
 * @details
 * Funkcja ta inicjalizuje obiekt `QApplication`, który zarządza zasobami
 * i główną pętlą zdarzeń aplikacji Qt. Ustawia globalną nazwę aplikacji,
 * która może być wykorzystywana np. w tytułach okien systemowych lub
 * przy zapisywaniu ustawień. Następnie tworzy instancję głównego okna
 * aplikacji (`MainWindow`), wyświetla je na ekranie użytkownika
 * i uruchamia pętlę zdarzeń poprzez wywołanie `QApplication::exec()`.
 * Pętla ta obsługuje interakcje użytkownika, sygnały systemowe i inne
 * zdarzenia aż do momentu zamknięcia aplikacji.
 *
 * @param argc [in] Liczba argumentów przekazanych do aplikacji z wiersza poleceń.
 * Jest to standardowy argument funkcji `main` w C/C++.
 * @param argv [in] Tablica wskaźników do ciągów znaków (C-style strings)
 * reprezentujących argumenty wiersza poleceń. Pierwszy element
 * (`argv[0]`) to zazwyczaj nazwa lub ścieżka do wykonywalnego
 * pliku programu.
 * @return Kod wyjścia aplikacji, który jest przekazywany do systemu operacyjnego.
 * Jest to wartość zwracana przez `QApplication::exec()`. Zgodnie z
 * konwencją, wartość 0 oznacza pomyślne zakończenie działania programu,
 * podczas gdy wartości niezerowe mogą sygnalizować wystąpienie błędów.
 *
 * @see MainWindow
 * @see QApplication
 */
int main(int argc, char *argv[]) {
    // Inicjalizacja obiektu aplikacji Qt, przekazanie argumentów wiersza poleceń.
    // Obiekt `app` musi istnieć przez cały czas działania aplikacji.
    QApplication app(argc, argv);

    // Ustawienie nazwy aplikacji. Może być używane przez Qt wewnętrznie
    // oraz przez system operacyjny (np. do identyfikacji procesu).
    app.setApplicationName("Orienta");
    // Opcjonalnie można też ustawić wersję aplikacji i nazwę organizacji:
    // app.setApplicationVersion("1.0.0");
    // app.setOrganizationName("Wydział Fotoniki Elektroniki i Mikrosystemów W12N");

    // Utworzenie instancji głównego okna aplikacji.
    // Główne okno jest sercem interfejsu użytkownika.
    MainWindow w;

    // Wyświetlenie głównego okna. Od tego momentu okno staje się widoczne dla użytkownika.
    w.show();

    // Uruchomienie głównej pętli zdarzeń aplikacji.
    // Funkcja ta blokuje wykonanie do momentu zamknięcia aplikacji
    // (np. przez zamknięcie ostatniego okna lub wywołanie QApplication::quit()).
    // Zwraca kod zakończenia.
    return QApplication::exec();
}
