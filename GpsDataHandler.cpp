#include "GPSDataHandler.h"
#include <QVBoxLayout>

GPSDataHandler::GPSDataHandler(QWidget *parent)
    : QWidget(parent)
{
    mapView = new QWebEngineView(this);
    mapView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString mapHtml = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>OpenStreetMap</title>
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.3/dist/leaflet.css" />
            <script src="https://unpkg.com/leaflet@1.9.3/dist/leaflet.js"></script>
            <style>
                html, body, #map {
                    height: 100%;
                    margin: 0;
                    padding: 0;
                }
            </style>
        </head>
        <body>
            <div id="map"></div>
            <script>
                var map = L.map('map').setView([0, 0], 15);
                L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
                    maxZoom: 19,
                    attribution: '© OpenStreetMap contributors'
                }).addTo(map);
                var marker = L.marker([0, 0]).addTo(map);
                function updateMarker(lat, lon) {
                    marker.setLatLng([lat, lon]);
                    map.setView([lat, lon], map.getZoom());
                }
                window.updateMarker = updateMarker;
            </script>
        </body>
        </html>
    )";

    mapView->setHtml(mapHtml);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(mapView);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void GPSDataHandler::updateMarker(float latitude, float longitude)
{
    QString jsCode = QString("updateMarker(%1, %2);").arg(latitude).arg(longitude);
    mapView->page()->runJavaScript(jsCode);
}