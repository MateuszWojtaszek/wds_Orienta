//
// Created by Mateusz Wojtaszek on 31/03/2025.
//

#ifndef GPSDATAHANDLER_H
#define GPSDATAHANDLER_H

#include <QWidget>
#include <QWebEngineView>

/**
 * \class GPSDataHandler
 * \brief Widget for displaying GPS location using an embedded map view.
 */
class GPSDataHandler : public QWidget
{
    Q_OBJECT

public:
    /**
     * \brief Constructs the GPSDataHandler widget.
     * \param parent The parent widget, if any.
     */
    explicit GPSDataHandler(QWidget *parent = nullptr);

    /**
     * \brief Updates the marker position on the map.
     * \param latitude The latitude of the new position.
     * \param longitude The longitude of the new position.
     */
    void updateMarker(float latitude, float longitude);

private:
    QWebEngineView *mapView; ///< Web view to display the Leaflet map
};

#endif // GPSDATAHANDLER_H