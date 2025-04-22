#include <QApplication>
#include <QCoreApplication> // Dodano dla Qt::AA_EnableHighDpiScaling
#include <QtWebEngineQuick/QtWebEngineQuick> // dla QML
#include "MainWindow.h"

/**
 * @brief Główna funkcja aplikacji Orienta.
 *
 * @details Inicjalizuje aplikację Qt, inicjalizuje jej ustawienia,
 * tworzy i wyświetla główne okno aplikacji (MainWindow),
 * a następnie uruchamia główną pętlę zdarzeń Qt.
 *
 * @param argc Liczba argumentów wiersza poleceń.
 * @param argv Tablica wskaźników do argumentów wiersza poleceń.
 * @return Kod wyjścia aplikacji. Zazwyczaj 0 oznacza pomyślne zakończenie.
 * Wartość zwracana przez QApplication::exec().
 */
int main(int argc, char *argv[]) {
    // Utworzenie obiektu aplikacji Qt
    QApplication app(argc, argv);

    // Ustawienie nazwy aplikacji
    app.setApplicationName("Orienta");

    // Utworzenie instancji głównego okna aplikacji
    MainWindow w;

    // Wyświetlenie głównego okna
    w.show();

    // Uruchomienie głównej pętli zdarzeń aplikacji.
    // Program pozostanie w tej funkcji aż do zakończenia działania aplikacji (np. zamknięcia okna).
    return QApplication::exec();
}