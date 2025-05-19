#ifndef COMPASS2DRENDERER_H
#define COMPASS2DRENDERER_H

#include <QWidget>
#include <QColor>

class Compass2DRenderer : public QWidget
{
    Q_OBJECT

public:
    explicit Compass2DRenderer(QWidget *parent = nullptr);

    /**
     * @brief Ustawia aktualny kurs (azymut) wskazywany przez kompas.
     * @param newHeading Nowy kurs w stopniach (0 = Północ, 90 = Wschód, itd.).
     */
    void setHeading(float newHeading);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    float m_heading; // Aktualny kurs w stopniach
    QColor m_backgroundColor;
    QColor m_borderColor;
    QColor m_textColor;
    QColor m_needleNorthColor;
    QColor m_needleSouthColor;
};

#endif // COMPASS2DRENDERER_H