#ifndef GPSDATAHANDLER_H
#define GPSDATAHANDLER_H

/**
 * @file GpsDataHandler.h
 * @brief Definicja klasy GPSDataHandler, służącej do zarządzania danymi GPS i ich wizualizacji na mapie.
 * @author Mateusz Wojtaszek
 * @date 2024-05-17
 * @bug Brak znanych błędów.
 */

#include <QWidget>
#include <QWebEngineView>

/**
 * @class GPSDataHandler
 * @brief Odpowiada za zarządzanie danymi GPS i ich wyświetlanie na interaktywnej mapie.
 *
 * @details
 * Klasa `GPSDataHandler` dziedziczy po `QWidget` i integruje `QWebEngineView`
 * w celu renderowania mapy opartej na OpenStreetMap z wykorzystaniem biblioteki Leaflet.
 * Umożliwia dynamiczną aktualizację pozycji markera na mapie, odzwierciedlając
 * bieżące współrzędne GPS. Logika mapy (HTML, CSS, JavaScript) jest osadzona
 * bezpośrednio w kodzie C++.
 *
 * @author Twoje Imię/Nazwa Zespołu (jeśli jesteś głównym autorem klasy)
 * @date 2024-05-17 - Utworzenie klasy.
 * @date 2024-05-18 - Dodano obsługę dynamicznej aktualizacji markera.
 *
 * @note
 * Do poprawnego działania wymagane jest połączenie z internetem w celu załadowania
 * kafelków mapy OpenStreetMap oraz biblioteki Leaflet.
 *
 * @warning
 * W przypadku braku dostępu do zasobów Leaflet (np. z powodu problemów z siecią lub
 * zmian w API unpkg), funkcjonalność mapy może być ograniczona lub niedostępna.
 *
 * @see SensorGraph (jeśli istnieje powiązana klasa do wyświetlania innych danych)
 * @see https://leafletjs.com/ (Oficjalna dokumentacja biblioteki Leaflet)
 *
 * @example GpsDataHandler_Usage.cpp
 * Poniżej znajduje się przykład ilustrujący podstawowe użycie klasy `GPSDataHandler`
 * w aplikacji Qt do wyświetlania mapy i aktualizowania pozycji markera.
 */
class GPSDataHandler : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy GPSDataHandler.
     *
     * @details
     * Inicjalizuje nowy widget `GPSDataHandler`. Tworzy wewnętrzny obiekt
     * `QWebEngineView`, ładuje do niego stronę HTML z mapą Leaflet i konfiguruje
     * niezbędne layouty. Mapa jest gotowa do interakcji po zakończeniu
     * konstruktora.
     * @param parent [in] Wskaźnik na widget nadrzędny. Domyślnie `nullptr`.
     */
    explicit GPSDataHandler(QWidget *parent = nullptr);

    /**
     * @brief Aktualizuje pozycję markera na wyświetlanej mapie.
     *
     * @details
     * Wywołuje funkcję JavaScript `updateMarker(lat, lon)` zdefiniowaną
     * w osadzonym kodzie HTML. Przekazuje nowe współrzędne geograficzne,
     * co powoduje przesunięcie markera oraz wycentrowanie mapy na nowej pozycji.
     * @param latitude [in] Nowa szerokość geograficzna (w stopniach dziesiętnych).
     * @param longitude [in] Nowa długość geograficzna (w stopniach dziesiętnych).
     *
     * @note Ta operacja jest asynchroniczna; JavaScript jest wykonywany w osobnym wątku silnika web.
     */
    void updateMarker(float latitude, float longitude);

private:
    QWebEngineView *mapView; //!< Wskaźnik na obiekt QWebEngineView, który renderuje mapę.
};

#endif // GPSDATAHANDLER_H