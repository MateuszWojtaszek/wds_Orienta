#include <QApplication>
#include <QCoreApplication> // Dodano dla Qt::AA_EnableHighDpiScaling
#include <QtWebEngineQuick/QtWebEngineQuick> // dla QML
#include "MainWindow.h"

/**
 * @brief Główna funkcja aplikacji Orienta.
 *
 * @details Inicjalizuje aplikację Qt, ustawia jej podstawowe metadane,
 * tworzy i wyświetla główne okno aplikacji (MainWindow),
 * a następnie uruchamia główną pętlę zdarzeń Qt.
 * Opcjonalnie inicjalizuje QtWebEngineQuick dla wsparcia QML z WebEngine
 * oraz włącza skalowanie dla wysokich rozdzielczości DPI.
 *
 * @param argc Liczba argumentów wiersza poleceń.
 * @param argv Tablica wskaźników do argumentów wiersza poleceń.
 * @return Kod wyjścia aplikacji. Zazwyczaj 0 oznacza pomyślne zakończenie.
 * Wartość zwracana przez QApplication::exec().
 */
int main(int argc, char *argv[]) {

    // Włączenie skalowania dla wysokich rozdzielczości DPI (zalecane)
    // Należy to zrobić przed utworzeniem obiektu QApplication
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Opcjonalna inicjalizacja QtWebEngineQuick (jeśli jest używane intensywnie od startu)
    // W niektórych przypadkach może być wymagane wywołanie tego przed utworzeniem QApplication
    // Jeśli napotkasz problemy z WebEngine lub używasz go od razu, odkomentuj poniższą linię:
    // QtWebEngineQuick::initialize();

    // Utworzenie obiektu aplikacji Qt
    QApplication app(argc, argv);

    // Ustawienie nazwy aplikacji (widocznej np. w tytule okna lub menu)
    app.setApplicationName("Orienta");

    // Ustawienie nazwy organizacji (używanej np. do ścieżek zapisu ustawień)
    app.setOrganizationName("PWR");

    // Utworzenie instancji głównego okna aplikacji
    MainWindow w;

    // Wyświetlenie głównego okna
    w.show();

    // Uruchomienie głównej pętli zdarzeń aplikacji.
    // Program pozostanie w tej funkcji aż do zakończenia działania aplikacji (np. zamknięcia okna).
    return QApplication::exec();
}