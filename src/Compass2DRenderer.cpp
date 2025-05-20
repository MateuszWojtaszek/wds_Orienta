/**
 * @file Compass2DRenderer.cpp
 * @brief Implementacja metod klasy Compass2DRenderer.
 * @details Ten plik zawiera logikę konstruktora, ustawiania kursu oraz
 * algorytm rysowania poszczególnych elementów kompasu w metodzie `paintEvent`.
 */

#include "Compass2DRenderer.h"
#include <QPainter>
#include <QtMath> // Dla qDegreesToRadians, qSin, qCos, fmod
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPointF> // Dla QPointF używanego w QPolygonF

Compass2DRenderer::Compass2DRenderer(QWidget *parent)
    : QWidget(parent),
      m_heading(0.0f), // Domyślnie na północ
      m_backgroundColor(QRgb(0x3B3B3B)), // Ciemnoszary
      m_borderColor(Qt::darkGray), // Ciemniejszy szary dla krawędzi
      m_textColor(Qt::white), // Biały tekst dla kontrastu
      m_needleNorthColor(Qt::red), // Czerwona część północna igły
      m_needleSouthColor(Qt::lightGray) // Jasnoszara część południowa igły
{
    // Ustawienie polityki rozmiaru, aby widget mógł się rozciągać
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // Ustawienie minimalnego rozmiaru, aby kompas był zawsze czytelny
    setMinimumSize(100, 100);
    // Można rozważyć Qt::WA_OpaquePaintEvent dla optymalizacji, jeśli tło jest zawsze w pełni kryjące
    // setAttribute(Qt::WA_OpaquePaintEvent);
}

void Compass2DRenderer::setHeading(float newHeading) {
    // Normalizacja kursu do zakresu [0, 360)
    float normalizedHeading = fmod(newHeading, 360.0f);
    if (normalizedHeading < 0) {
        normalizedHeading += 360.0f;
    }

    // Aktualizuj i przerysuj tylko jeśli wartość faktycznie się zmieniła
    // (qFuzzyCompare dla uniknięcia problemów z precyzją float)
    if (qFuzzyCompare(m_heading, normalizedHeading))
        return;

    m_heading = normalizedHeading;
    update(); // Żądanie odświeżenia widgetu
}

QSize Compass2DRenderer::sizeHint() const {
    // Sugerowany, preferowany rozmiar widgetu
    return QSize(180, 180);
}

QSize Compass2DRenderer::minimumSizeHint() const {
    // Minimalny rozsądny rozmiar widgetu
    return QSize(80, 80);
}

void Compass2DRenderer::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event); // Zaznaczenie, że parametr event nie jest używany w tej funkcji
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // Włączenie antyaliasingu dla gładszych krawędzi

    int side = qMin(width(), height()); // Użyj mniejszego wymiaru jako bazę dla skalowania
    painter.translate(width() / 2.0, height() / 2.0); // Przesunięcie środka układu współrzędnych do środka widgetu
    // Skalowanie, aby rysować w wirtualnej przestrzeni o boku 200 jednostek (od -100 do 100)
    painter.scale(side / 200.0, side / 200.0);

    // Rysowanie tarczy kompasu
    painter.setPen(QPen(m_borderColor, 2)); // Ustawienie grubszego pióra dla krawędzi tarczy
    painter.setBrush(m_backgroundColor); // Ustawienie koloru wypełnienia tarczy
    painter.drawEllipse(QRectF(-98, -98, 196, 196)); // Tarcza nieco mniejsza niż 100, aby krawędź była widoczna

    // Rysowanie kresek (podziałki) na tarczy
    painter.setPen(m_borderColor); // Domyślny kolor dla kresek
    for (int i = 0; i < 360; i += 10) {
        // Kreski co 10 stopni
        painter.save(); // Zapisanie aktualnego stanu transformacji
        painter.rotate(i); // Obrót układu współrzędnych
        if (i % 90 == 0) {
            // Główne kierunki (N, E, S, W) - najdłuższe kreski
            painter.setPen(QPen(m_textColor, 2)); // Wyróżnienie głównych kierunków
            painter.drawLine(0, -98, 0, -82);
            painter.setPen(m_borderColor); // Przywrócenie domyślnego pióra
        } else if (i % 30 == 0) {
            // Kreski co 30 stopni - średniej długości
            painter.drawLine(0, -98, 0, -88);
        } else {
            // Kreski co 10 stopni - najkrótsze
            painter.drawLine(0, -98, 0, -93);
        }
        painter.restore(); // Przywrócenie poprzedniego stanu transformacji
    }

    // Rysowanie oznaczeń tekstowych (N, E, S, W)
    painter.setPen(m_textColor);
    QFont labelFont = font(); // Pobranie domyślnej czcionki widgetu
    // Dynamiczne skalowanie rozmiaru czcionki, z minimum 10px
    labelFont.setPixelSize(qMax(10, qRound(side / 16.0)));
    painter.setFont(labelFont);

    const int textRadius = 70; // Promień, na którym umieszczone będą teksty (w jednostkach -100 do 100)
    // N
    painter.drawText(QRectF(-15, -textRadius - labelFont.pixelSize() / 2, 30, labelFont.pixelSize()), Qt::AlignCenter,
                     "N");
    // E
    painter.drawText(QRectF(textRadius - labelFont.pixelSize() / 2, -15, labelFont.pixelSize(), 30), Qt::AlignCenter,
                     "E");
    // S
    painter.drawText(QRectF(-15, textRadius - labelFont.pixelSize() / 2, 30, labelFont.pixelSize()), Qt::AlignCenter,
                     "S");
    // W
    painter.drawText(QRectF(-textRadius - labelFont.pixelSize() / 2, -15, labelFont.pixelSize(), 30), Qt::AlignCenter,
                     "W");


    // Rysowanie igły kompasu
    painter.save(); // Zapisanie stanu przed obrotem dla igły
    painter.rotate(m_heading); // Obrót układu współrzędnych zgodnie z aktualnym kursem

    // Definicja kształtu północnej części igły
    QPolygonF needleNorthPoly;
    needleNorthPoly << QPointF(0, -80) // Wierzchołek
            << QPointF(-7, -65) // Lewa podstawa
            << QPointF(0, -55) // Środek podstawy (dla kształtu)
            << QPointF(7, -65); // Prawa podstawa

    // Definicja kształtu południowej części igły
    QPolygonF needleSouthPoly;
    needleSouthPoly << QPointF(0, 80) // Wierzchołek
            << QPointF(-7, 65) // Lewa podstawa
            << QPointF(0, 55) // Środek podstawy
            << QPointF(7, 65); // Prawa podstawa

    painter.setPen(Qt::NoPen); // Brak krawędzi dla igły
    painter.setBrush(m_needleNorthColor); // Kolor wypełnienia północnej części
    painter.drawConvexPolygon(needleNorthPoly);

    painter.setBrush(m_needleSouthColor); // Kolor wypełnienia południowej części
    painter.drawConvexPolygon(needleSouthPoly);

    // Centralny okrąg na igle (oś obrotu)
    painter.setBrush(m_needleNorthColor.darker(150)); // Ciemniejszy niż igła, np. ciemnoczerwony
    painter.drawEllipse(QRectF(-6, -6, 12, 12));

    painter.restore(); // Przywrócenie stanu sprzed rysowania igły
}
