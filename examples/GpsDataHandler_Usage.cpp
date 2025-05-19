/**
 * @file GpsDataHandler_Usage.cpp
 * @brief Przykład demonstrujący użycie klasy GPSDataHandler.
 * @example GpsDataHandler_Usage.cpp
 */

#include "GpsDataHandler.h"
#include <QApplication>
#include <QMainWindow>
#include <QTimer>    // Do symulacji aktualizacji pozycji GPS
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <random>    // Do generowania losowych współrzędnych

// Przykładowe współrzędne (Wrocław, Polska)
const float START_LAT = 51.1079f;
const float START_LON = 17.0385f;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Przykład GPSDataHandler z Mapą");

    QWidget *centralWidget = new QWidget(&mainWindow);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Etykieta wyświetlająca aktualne współrzędne
    QLabel *coordinatesLabel = new QLabel("Oczekiwanie na dane GPS...", centralWidget);
    coordinatesLabel->setAlignment(Qt::AlignCenter);
    QFont labelFont = coordinatesLabel->font();
    labelFont.setPointSize(10);
    coordinatesLabel->setFont(labelFont);

    // Utworzenie instancji GPSDataHandler
    GPSDataHandler *gpsMapWidget = new GPSDataHandler(centralWidget);
    // Ustawienie minimalnego rozmiaru dla mapy, aby była widoczna
    gpsMapWidget->setMinimumSize(400, 300);


    mainLayout->addWidget(coordinatesLabel);
    mainLayout->addWidget(gpsMapWidget, 1); // Mapa zajmuje dostępną przestrzeń
    centralWidget->setLayout(mainLayout);
    mainWindow.setCentralWidget(centralWidget);

    mainWindow.resize(600, 500); // Rozsądny rozmiar początkowy okna
    mainWindow.show();

    // Inicjalizacja mapy na pozycji startowej
    gpsMapWidget->updateMarker(START_LAT, START_LON);
    coordinatesLabel->setText(QString("Pozycja: %1, %2").arg(START_LAT, 0, 'f', 6).arg(START_LON, 0, 'f', 6));


    // Symulacja aktualizacji danych GPS co 2 sekundy
    QTimer *gpsUpdateTimer = new QTimer(gpsMapWidget);
    std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
    // Generowanie małych zmian wokół pozycji startowej
    std::uniform_real_distribution<float> lat_offset_dist(-0.001f, 0.001f);
    std::uniform_real_distribution<float> lon_offset_dist(-0.002f, 0.002f);

    static float currentLat = START_LAT;
    static float currentLon = START_LON;

    QObject::connect(gpsUpdateTimer, &QTimer::timeout, [=]() mutable {
        currentLat += lat_offset_dist(generator);
        currentLon += lon_offset_dist(generator);

        // Ograniczenie dryfowania od punktu startowego dla celów demonstracyjnych
        if (std::abs(currentLat - START_LAT) > 0.05f) currentLat = START_LAT;
        if (std::abs(currentLon - START_LON) > 0.1f) currentLon = START_LON;

        gpsMapWidget->updateMarker(currentLat, currentLon);
        coordinatesLabel->setText(QString("Aktualna pozycja: %1, %2")
                                     .arg(currentLat, 0, 'f', 6)
                                     .arg(currentLon, 0, 'f', 6));
    });

    gpsUpdateTimer->start(2000); // Aktualizuj co 2000 ms (2 sekundy)

    return app.exec();
}