/**
* @file main.cpp
 * @brief Główny plik źródłowy aplikacji Orienta.
 * @details Zawiera punkt wejścia aplikacji (funkcję `main`), która jest
 * odpowiedzialna za inicjalizację środowiska Qt i uruchomienie głównego okna.
 */

#include <QApplication>
#include "MainWindow.h"

/**
 * @brief Główna funkcja aplikacji Orienta (punkt wejścia).
 *
 * @details Inicjalizuje obiekt QApplication, który zarządza zasobami aplikacji Qt.
 * Ustawia nazwę aplikacji. Tworzy instancję głównego okna aplikacji (`MainWindow`),
 * wyświetla je na ekranie (`w.show()`), a następnie uruchamia główną pętlę
 * zdarzeń Qt (`QApplication::exec()`), która obsługuje interakcje użytkownika
 * i inne zdarzenia systemowe aż do zamknięcia aplikacji.
 *
 * @param argc [in] Liczba argumentów przekazanych do aplikacji z wiersza poleceń.
 * @param argv [in] Tablica ciągów znaków (C-style strings) zawierająca argumenty wiersza poleceń.
 * Pierwszy argument (`argv[0]`) to zazwyczaj nazwa/ścieżka programu.
 * @return Kod wyjścia aplikacji przekazywany do systemu operacyjnego.
 * Jest to wartość zwracana przez `QApplication::exec()`.
 * Konwencjonalnie, 0 oznacza pomyślne zakończenie, wartości niezerowe wskazują na błędy.
 */
int main(int argc, char *argv[]) {
    // Inicjalizacja aplikacji Qt, przekazanie argumentów wiersza poleceń
    QApplication app(argc, argv);

    // Ustawienie nazwy aplikacji (może być używane np. w tytułach okien, ustawieniach)
    app.setApplicationName("Orienta");

    // Utworzenie instancji głównego okna aplikacji
    MainWindow w;

    // Wyświetlenie głównego okna
    w.show();

    // Uruchomienie głównej pętli zdarzeń aplikacji i zwrócenie jej kodu wyjścia
    return QApplication::exec();
}