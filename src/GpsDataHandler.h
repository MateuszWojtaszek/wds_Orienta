#ifndef GPSDATAHANDLER_H
#define GPSDATAHANDLER_H

/**
 * @file GpsDataHandler.h
 * @brief Definicja klasy GPSDataHandler do zarządzania danymi GPS i wyświetlania mapy.
 */

#include <QWidget>
#include <QWebEngineView> // Forward declaration nie wystarczy, bo używamy wskaźnika jako składowej

// Forward declaration dla QWebEngineView, jeśli nie includujemy go tutaj
// class QWebEngineView;

/**
 * @class GPSDataHandler
 * @brief Klasa odpowiedzialna za zarządzanie danymi GPS i wyświetlanie ich na mapie.
 *
 * @details Ta klasa dziedziczy po QWidget i wykorzystuje QWebEngineView do wyświetlenia
 * interaktywnej mapy (Leaflet na OpenStreetMap) oraz do oznaczania
 * na niej pozycji za pomocą markera, którego położenie można aktualizować.
 * Mapa i jej logika są osadzone jako kod HTML/JavaScript bezpośrednio w klasie.
 */
class GPSDataHandler : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy GPSDataHandler.
     * @details Inicjalizuje widget, tworzy widok mapy QWebEngineView,
     * ładuje do niego stronę HTML z mapą Leaflet i konfiguruje layout.
     * @param parent [in] Wskaźnik na widget rodzica (domyślnie nullptr).
     */
    explicit GPSDataHandler(QWidget *parent = nullptr);

    /**
     * @brief Aktualizuje pozycję markera na mapie.
     * @details Wywołuje funkcję JavaScript `updateMarker` w załadowanej stronie HTML,
     * przekazując jej nowe współrzędne geograficzne. Ustawia również
     * widok mapy na nową pozycję.
     * @param latitude [in] Szerokość geograficzna (float).
     * @param longitude [in] Długość geograficzna (float).
     * @return Nie zwraca wartości (void).
     */
    void updateMarker(float latitude, float longitude);

private:
    /// Wskaźnik na obiekt QWebEngineView, który wyświetla mapę.
    QWebEngineView *mapView;
};

#endif // GPSDATAHANDLER_H