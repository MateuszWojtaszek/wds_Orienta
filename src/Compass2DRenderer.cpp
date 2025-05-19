#include "Compass2DRenderer.h"
#include <QPainter>
#include <QtMath> // Dla qDegreesToRadians, qSin, qCos, fmod
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPointF>

Compass2DRenderer::Compass2DRenderer(QWidget *parent)
    : QWidget(parent),
      m_heading(0.0f),
      m_backgroundColor(QRgb(0x3B3B3B)),
      m_borderColor(Qt::darkGray),
      m_textColor(Qt::white),
      m_needleNorthColor(Qt::red),
      m_needleSouthColor(Qt::lightGray) // Jaśniejszy szary dla lepszego kontrastu
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(100, 100); // Zmniejszony minimalny rozmiar
    // setAttribute(Qt::WA_OpaquePaintEvent); // Może pomóc w wydajności, jeśli tło jest zawsze w pełni kryjące
}

void Compass2DRenderer::setHeading(float newHeading)
{
    float normalizedHeading = fmod(newHeading, 360.0f);
    if (normalizedHeading < 0) {
        normalizedHeading += 360.0f;
    }

    if (qFuzzyCompare(m_heading, normalizedHeading))
        return;

    m_heading = normalizedHeading;
    update();
}

QSize Compass2DRenderer::sizeHint() const
{
    return QSize(180, 180); // Nieco mniejszy preferowany rozmiar
}

QSize Compass2DRenderer::minimumSizeHint() const
{
    return QSize(80, 80);
}

void Compass2DRenderer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int side = qMin(width(), height());
    painter.translate(width() / 2.0, height() / 2.0);
    painter.scale(side / 200.0, side / 200.0); // Rysowanie w przestrzeni -100 do 100

    // Tarcza
    painter.setPen(QPen(m_borderColor, 2)); // Grubsza krawędź
    painter.setBrush(m_backgroundColor);
    painter.drawEllipse(QRectF(-98, -98, 196, 196));

    // Kreski co 30 stopni i co 10 stopni
    painter.setPen(m_borderColor);
    for (int i = 0; i < 360; i += 10) {
        painter.save();
        painter.rotate(i);
        if (i % 30 == 0) {
            painter.drawLine(0, -98, 0, -88); // Dłuższe kreski co 30 stopni
            if (i % 90 == 0) { // Jeszcze dłuższe dla głównych kierunków
                 painter.setPen(QPen(m_textColor, 2)); // Wyróżnienie głównych kierunków
                 painter.drawLine(0, -98, 0, -82);
                 painter.setPen(m_borderColor); // Powrót do domyślnego pena dla kresek
            }
        } else {
            painter.drawLine(0, -98, 0, -93); // Krótsze kreski co 10 stopni
        }
        painter.restore();
    }


    // Tekst (N, E, S, W)
    painter.setPen(m_textColor);
    QFont labelFont = font();
    labelFont.setPixelSize(qMax(10, side / 16)); // Nieco mniejsza, ale nadal skalowalna
    painter.setFont(labelFont);

    const int textOffset = 30; // Odstęp tekstu od krawędzi tarczy (w jednostkach -100 do 100)
    // Rysowanie tekstu na okręgu, obracając układ współrzędnych
    // N
    painter.save();
    painter.rotate(0); // Dla "N"
    painter.drawText(QRectF(-20, -98 + textOffset - labelFont.pixelSize()/2 , 40, labelFont.pixelSize()), Qt::AlignCenter, "N");
    painter.restore();
    // E
    painter.save();
    painter.rotate(90); // Dla "E"
    painter.drawText(QRectF(-20, -98 + textOffset - labelFont.pixelSize()/2 , 40, labelFont.pixelSize()), Qt::AlignCenter, "E");
    painter.restore();
    // S
    painter.save();
    painter.rotate(180); // Dla "S"
    painter.drawText(QRectF(-20, -98 + textOffset - labelFont.pixelSize()/2 , 40, labelFont.pixelSize()), Qt::AlignCenter, "S");
    painter.restore();
    // W
    painter.save();
    painter.rotate(270); // Dla "W"
    painter.drawText(QRectF(-20, -98 + textOffset - labelFont.pixelSize()/2 , 40, labelFont.pixelSize()), Qt::AlignCenter, "W");
    painter.restore();


    // Igła kompasu
    painter.save();
    painter.rotate(m_heading);

    // Kształt igły (Północ - czerwona, Południe - szara)
    QPolygonF needleNorthPoly;
    needleNorthPoly << QPointF(0, -85)      // Czupek
                    << QPointF(-8, -70)   // Lewa podstawa
                    << QPointF(0, -60)    // Punkt centralny (opcjonalnie, dla "grubości")
                    << QPointF(8, -70);   // Prawa podstawa

    QPolygonF needleSouthPoly;
    needleSouthPoly << QPointF(0, 85)       // Czupek
                    << QPointF(-8, 70)    // Lewa podstawa
                    << QPointF(0, 60)     // Punkt centralny
                    << QPointF(8, 70);    // Prawa podstawa

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_needleNorthColor);
    painter.drawConvexPolygon(needleNorthPoly);

    painter.setBrush(m_needleSouthColor);
    painter.drawConvexPolygon(needleSouthPoly);

    // Centralny okrąg
    painter.setBrush(Qt::darkRed); // Ciemniejszy niż igła
    painter.drawEllipse(QRectF(-6, -6, 12, 12));

    painter.restore();
}