/**
* @file MainWindowUsage_PL.cpp
 * @brief Główny plik aplikacji demonstrujący inicjalizację i uruchomienie MainWindow.
 * @details Tworzy instancję QApplication oraz MainWindow, a następnie wyświetla
 * główne okno i uruchamia pętlę zdarzeń aplikacji.
 */

#include "MainWindow.h" // Zakładając, że MainWindow.h jest w ścieżce include
#include <QApplication>
#include <QTranslator> // Dla przykładu ładowania tłumaczeń
#include <QDebug>      // Dla logowania

// Przykładowa funkcja main
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Opcjonalne: Ustawienie informacji o aplikacji dla QSettings itp.
    QCoreApplication::setOrganizationName("MojaOrganizacja");
    QCoreApplication::setApplicationName("SensorVisualizer");
    QCoreApplication::setApplicationVersion("1.0");

    // Przykład dynamicznego ładowania tłumaczeń przy starcie (opcjonalne,
    // jeśli MainWindow ma obsługiwać zmianę języka w locie)
    // QTranslator appTranslator;
    // if (appTranslator.load(":/translations/wds_Orienta_" + QLocale::system().name() + ".qm")) {
    //     app.installTranslator(&appTranslator);
    //     qDebug() << "Loaded system language translation:" << QLocale::system().name();
    // } else {
    //     qDebug() << "Could not load system language translation, using default (English/source).";
    // }


    MainWindow mainWindow; // Utworzenie głównego okna
    mainWindow.show();    // Wyświetlenie głównego okna
    // mainWindow.showFullScreen(); // Aby uruchomić w trybie pełnoekranowym

    return app.exec(); // Uruchomienie pętli zdarzeń Qt
}