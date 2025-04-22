#include "GpsDataHandler.h"
#include <QVBoxLayout>
#include <QWebEngineView>
#include <QWebEnginePage>

/***************************************************************************/
/**
 * @brief Konstruktor klasy GPSDataHandler.
 * @details Inicjalizuje widget, tworzy widok mapy QWebEngineView,
 * ładuje do niego stronę HTML z mapą Leaflet i konfiguruje layout.
 * Kod HTML i JavaScript dla mapy Leaflet jest osadzony bezpośrednio
 * w kodzie źródłowym.
 * @param parent Wskaźnik na widget rodzica (domyślnie nullptr).
 * @return Nie zwraca wartości (konstruktor).
 */
GPSDataHandler::GPSDataHandler(QWidget *parent)
    : QWidget(parent) {

    mapView = new QWebEngineView(this);
    mapView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Kod HTML i JavaScript dla mapy Leaflet osadzony jako C++ Raw String Literal
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
                    background-color: lightgray; /* Dodano tło dla lepszej widoczności podczas ładowania */
                }
            </style>
        </head>
        <body>
            <div id="map"></div>
            <script>
                console.log("✅ Leaflet map loaded");

                // Inicjalizacja mapy Leaflet, początkowy widok ustawiony na [0, 0]
                var map = L.map('map').setView([0, 0], 15); // Początkowy zoom ustawiony na 15

                // Dodanie warstwy kafelków OpenStreetMap
                L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
                    maxZoom: 19,
                    attribution: '© OpenStreetMap contributors'
                }).addTo(map);

                // Utworzenie markera na początkowej pozycji [0, 0]
                var marker = L.marker([0, 0]).addTo(map);

                /**
                 * @brief Funkcja JavaScript do aktualizacji pozycji markera i widoku mapy.
                 * @param lat Nowa szerokość geograficzna.
                 * @param lon Nowa długość geograficzna.
                 */
                function updateMarker(lat, lon) {
                    console.log("📍 updateMarker:", lat, lon); // Logowanie do konsoli przeglądarki
                    marker.setLatLng([lat, lon]);          // Ustawienie nowej pozycji markera
                    map.setView([lat, lon], map.getZoom()); // Ustawienie widoku mapy na nową pozycję (z zachowaniem zooma)
                }

                // Udostępnienie funkcji updateMarker globalnie w oknie przeglądarki
                window.updateMarker = updateMarker;
            </script>
        </body>
        </html>
    )";

    // Ustawienie zawartości HTML w QWebEngineView
    mapView->setHtml(mapHtml);

    // Konfiguracja layoutu dla widgetu GPSDataHandler
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(mapView); // Dodanie widoku mapy do layoutu
    layout->setContentsMargins(0, 0, 0, 0); // Usunięcie marginesów layoutu
    setLayout(layout); // Ustawienie layoutu dla widgetu
}

/***************************************************************************/
/**
 * @brief Aktualizuje pozycję markera na mapie.
 * @details Wywołuje funkcję JavaScript `updateMarker` w załadowanej stronie HTML,
 * przekazując jej nowe współrzędne geograficzne. Ustawia również
 * widok mapy na nową pozycję. Metoda `page()->runJavaScript()`
 * jest używana do wykonania kodu JS w kontekście strony.
 * @param latitude Szerokość geograficzna (float) do ustawienia markera.
 * @param longitude Długość geograficzna (float) do ustawienia markera.
 * @return Nie zwraca wartości (void).
 */
void GPSDataHandler::updateMarker(float latitude, float longitude) {
    // Przygotowanie kodu JavaScript do wykonania
    QString jsCode = QString("updateMarker(%1, %2);").arg(latitude).arg(longitude);
    // Wykonanie kodu JavaScript na stronie załadowanej w mapView
    mapView->page()->runJavaScript(jsCode);
}