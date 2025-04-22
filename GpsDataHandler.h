#ifndef GPSDATAHANDLER_H
#define GPSDATAHANDLER_H

#include <QWidget>
#include <QWebEngineView>

/**
 * @class GPSDataHandler
 * @brief Klasa odpowiedzialna za zarządzanie danymi GPS i wyświetlanie ich na mapie.
 *
 * Ta klasa dziedziczy po QWidget i wykorzystuje QWebEngineView do wyświetlenia
 * interaktywnej mapy (Leaflet na OpenStreetMap) oraz do oznaczania
 * na niej pozycji za pomocą markera, którego położenie można aktualizować.
 */
class GPSDataHandler : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy GPSDataHandler.
     * @details Inicjalizuje widget, tworzy widok mapy QWebEngineView,
     * ładuje do niego stronę HTML z mapą Leaflet i konfiguruje layout.
     * @param parent Wskaźnik na widget rodzica (domyślnie nullptr).
     * @return Nie zwraca wartości (konstruktor).
     */
    explicit GPSDataHandler(QWidget *parent = nullptr);

    /**
     * @brief Aktualizuje pozycję markera na mapie.
     * @details Wywołuje funkcję JavaScript `updateMarker` w załadowanej stronie HTML,
     * przekazując jej nowe współrzędne geograficzne. Ustawia również
     * widok mapy na nową pozycję.
     * @param latitude Szerokość geograficzna (float).
     * @param longitude Długość geograficzna (float).
     * @return Nie zwraca wartości (void).
     */
    void updateMarker(float latitude, float longitude);

private:
    /// Wskaźnik na obiekt QWebEngineView, który wyświetla mapę.
    QWebEngineView *mapView;
};

#endif // GPSDATAHANDLER_H