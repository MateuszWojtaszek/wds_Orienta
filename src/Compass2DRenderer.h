#ifndef COMPASS2DRENDERER_H
#define COMPASS2DRENDERER_H

/**
 * @file Compass2DRenderer.h
 * @brief Definicja klasy Compass2DRenderer, widgetu Qt renderującego kompas 2D.
 * @author Mateusz Wojtaszek
 * @date 2024-05-19
 * @bug Brak znanych błędów.
 * @version 1.0.0
 */

#include <QWidget>
#include <QColor> // Dla typów kolorów używanych jako składowe prywatne

/**
 * @class Compass2DRenderer
 * @brief Widget Qt wyświetlający graficzną reprezentację kompasu 2D.
 *
 * @details
 * Klasa `Compass2DRenderer` dziedziczy po `QWidget` i jest odpowiedzialna za rysowanie
 * tarczy kompasu, oznaczeń kierunków geograficznych (N, E, S, W) oraz igły wskazującej
 * aktualny kurs. Kurs można ustawiać dynamicznie za pomocą metody `setHeading()`.
 * Wygląd kompasu (kolory) jest zdefiniowany wewnętrznie. Widget dba o poprawne
 * skalowanie i odświeżanie swojego wyglądu.
 *
 * @date 2024-05-19 - Utworzenie podstawowej wersji klasy z rysowaniem tarczy i igły.
 * - Dodano normalizację kursu w `setHeading()`.
 * @date 2024-05-20 - Poprawiono skalowanie tekstu i kresek na tarczy.
 * - Zoptymalizowano rysowanie głównych kierunków.
 *
 * @note
 * Widget ten jest przeznaczony do wizualizacji danych o orientacji, np. z sensorów IMU lub GPS.
 * Kolory elementów kompasu są obecnie stałe, zdefiniowane w konstruktorze.
 *
 * @see ImuDataHandler (jeśli ta klasa używa Compass2DRenderer)
 * @see SensorGraph (jeśli istnieje powiązana klasa do wyświetlania innych danych)
 *
 * @example Compass2DRenderer_Usage.cpp
 * Poniżej znajduje się przykład ilustrujący podstawowe użycie klasy `Compass2DRenderer`
 * w aplikacji Qt.
 */
class Compass2DRenderer : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy Compass2DRenderer.
     *
     * @details
     * Inicjalizuje widget kompasu, ustawia domyślne wartości kolorów
     * oraz początkowy kurs na Północ. Konfiguruje również
     * politykę rozmiaru i minimalny rozmiar widgetu.
     * @param parent [in] Wskaźnik na widget nadrzędny. Domyślnie `nullptr`.
     */
    explicit Compass2DRenderer(QWidget *parent = nullptr);

    /**
     * @brief Ustawia aktualny kurs (azymut) wskazywany przez kompas.
     *
     * @details
     * Nowy kurs jest normalizowany do zakresu $[0, 360)$ stopni.
     * Jeśli podany kurs różni się od aktualnego, widget zostanie
     * odświeżony w celu pokazania nowej orientacji igły.
     * @param newHeading [in] Nowy kurs w stopniach
     */
    void setHeading(float newHeading);

    /**
     * @brief Zwraca preferowany rozmiar widgetu kompasu.
     *
     * @details
     * Ta metoda jest reimplementacją `QWidget::sizeHint()`. Zwraca
     * predefiniowany, rozsądny rozmiar dla kompasu.
     * @return Preferowany rozmiar jako `QSize`.
     */
    QSize sizeHint() const override;

    /**
     * @brief Zwraca minimalny rozsądny rozmiar widgetu kompasu.
     *
     * @details
     * Ta metoda jest reimplementacją `QWidget::minimumSizeHint()`. Zwraca
     * najmniejszy rozmiar, przy którym kompas jest jeszcze czytelny.
     * @return Minimalny preferowany rozmiar jako `QSize`.
     */
    QSize minimumSizeHint() const override;

protected:
    /**
     * @brief Obsługuje zdarzenie rysowania widgetu.
     *
     * @details
     * Ta metoda jest reimplementacją `QWidget::paintEvent()`. Jest wywoływana
     * automatycznie, gdy widget wymaga odświeżenia. Rysuje wszystkie
     * elementy kompasu: tarczę, oznaczenia i igłę.
     * @param event [in] Wskaźnik na obiekt zdarzenia `QPaintEvent` (nieużywany bezpośrednio).
     */
    void paintEvent(QPaintEvent *event) override;

private:
    float m_heading; //!< Aktualny kurs (azymut) w stopniach, znormalizowany do $[0, 360)$.
    QColor m_backgroundColor; //!< Kolor tła tarczy kompasu.
    QColor m_borderColor; //!< Kolor obramowania tarczy i głównych kresek.
    QColor m_textColor; //!< Kolor tekstu oznaczeń kierunków (N, E, S, W).
    QColor m_needleNorthColor; //!< Kolor północnej części igły kompasu (zazwyczaj czerwony).
    QColor m_needleSouthColor; //!< Kolor południowej części igły kompasu.
};

#endif // COMPASS2DRENDERER_H
