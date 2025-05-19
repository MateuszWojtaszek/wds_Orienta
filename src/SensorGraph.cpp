/**
 * @file SensorGraph.cpp
 * @brief Implementacja metod klasy SensorGraph.
 * @details Zawiera logikę inicjalizacji wykresu, dodawania danych,
 * ustawiania liczby próbek oraz zakresu osi Y, a także ponownego tłumaczenia UI.
 * @author Mateusz Wojtaszek (lub oryginalny autor, jeśli inny)
 * @date 2025-04-29 (lub aktualna data modyfikacji)
 */

#include "SensorGraph.h"

#include <QtCharts/QChart>      // Poprawiono ścieżkę
#include <QtCharts/QLineSeries> // Poprawiono ścieżkę
#include <QtCharts/QValueAxis>  // Poprawiono ścieżkę
#include <QtCharts/QLegend>     // Poprawiono ścieżkę
#include <QDebug>
#include <QStringList> // Dla nazw serii w legendzie
#include <QColor>
#include <QSizePolicy>
#include <QtMath> // Dla qMax
#include <QPainter> // Dla QPainter::Antialiasing


SensorGraph::SensorGraph(const QString &titleKey, int minY, int maxY, QWidget *parent)
    : QChartView(new QChart(), parent), // Inicjalizacja QChart w konstruktorze QChartView
      defaultSampleCount(1000),
      currentSampleIndex(0),
      baseTitleKey(titleKey) // Zapisz klucz tłumaczenia
{
    QChart *chartPtr = this->chart(); // Pobierz wskaźnik na QChart zarządzany przez QChartView
    if (!chartPtr) { // To nie powinno się zdarzyć, jeśli QChartView jest poprawnie skonstruowany
        qWarning() << "SensorGraph: Failed to get chart object!";
        return;
    }

    // Ustaw tytuł używając tr() od razu
    chartPtr->setTitle(tr(qPrintable(baseTitleKey)));

    QLegend *legend = chartPtr->legend();
    if (legend) {
        legend->setVisible(true);
        legend->setAlignment(Qt::AlignTop);
        legend->setMarkerShape(QLegend::MarkerShapeCircle);
    }

    QStringList axisNames = {"X", "Y", "Z"}; // Te nazwy serii nie są tłumaczone, są to etykiety danych
    QList<QColor> colors = {Qt::blue, Qt::red, Qt::green};

    for (int i = 0; i < 3; ++i) {
        auto *series = new QLineSeries(this); // Ustaw rodzica dla serii
        series->setName(axisNames[i]);
        series->setColor(colors[i]);
        chartPtr->addSeries(series);
        seriesList.append(series);
    }

    auto *xAxis = new QValueAxis(this); // Ustaw rodzica dla osi
    xAxis->setTitleText(tr("Sample Index")); // Tłumaczony tytuł osi
    xAxis->setTickCount(11);
    xAxis->setLabelFormat("%lld"); // Użyj "lld" dla qint64
    chartPtr->setAxisX(xAxis); // Użyj setAxisX bez drugiego argumentu, aby ustawić domyślną oś X

    auto *yAxis = new QValueAxis(this); // Ustaw rodzica dla osi
    yAxis->setTitleText(tr("Value")); // Tłumaczony tytuł osi
    yAxis->setRange(minY, maxY);
    chartPtr->setAxisY(yAxis); // Użyj setAxisY bez drugiego argumentu, aby ustawić domyślną oś Y

    // Przypisz osie do serii
    for (QLineSeries *series : seriesList) {
        chartPtr->setAxisX(xAxis, series);
        chartPtr->setAxisY(yAxis, series);
    }

    xAxis->setRange(0, defaultSampleCount > 0 ? defaultSampleCount - 1 : 0);

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    this->setRenderHint(QPainter::Antialiasing); // Ustaw antyaliasing dla QChartView
}

void SensorGraph::addData(const QVector<int> &axisValuesToAdd) {
    QChart *chartPtr = this->chart();
    if (axisValuesToAdd.size() != 3 || seriesList.size() != 3 || !chartPtr) {
        return;
    }

    for (int i = 0; i < 3; ++i) {
        QLineSeries *series = seriesList.at(i);
        if (!series) continue;

        series->append(currentSampleIndex, axisValuesToAdd[i]);

        if (series->count() > defaultSampleCount) {
            series->remove(0);
        }
    }

    const int xAxisUpdateFrequency = 10;
    bool isFirstSampleOrUpdateNeeded = (currentSampleIndex == 0 || currentSampleIndex % xAxisUpdateFrequency == 0);

    if (isFirstSampleOrUpdateNeeded) {
        if (auto *xAxis = qobject_cast<QValueAxis*>(chartPtr->axisX())) {
            qint64 lastIndex = currentSampleIndex;
            qint64 minX = (lastIndex >= defaultSampleCount) ? (lastIndex - defaultSampleCount + 1) : 0;
            qint64 maxX = lastIndex;

            if (minX > maxX) {
                 minX = qMax(0LL, maxX - defaultSampleCount + 1);
            }
            if (lastIndex < 1 && defaultSampleCount > 0) {
                 minX = 0;
                 maxX = defaultSampleCount -1;
            } else if (lastIndex < defaultSampleCount) {
                 minX = 0;
                 maxX = qMax(lastIndex, static_cast<qint64>(defaultSampleCount - 1));
            }
            xAxis->setRange(minX, maxX);
        }
    }
    currentSampleIndex++;
}

void SensorGraph::setSampleCount(int sampleCount) { // Zmieniono const int& na int
    defaultSampleCount = qMax(10, sampleCount);
    QChart *chartPtr = this->chart();

    for (QLineSeries *series : seriesList) {
        if (series) {
            while (series->count() > defaultSampleCount) {
                series->remove(0);
            }
        }
    }

    if (chartPtr) {
        if (auto *xAxis = qobject_cast<QValueAxis*>(chartPtr->axisX())) {
            qint64 minX = 0;
            qint64 maxX = 0;

            if (!seriesList.isEmpty() && seriesList.first() && seriesList.first()->count() > 0) {
                const auto &points = seriesList.first()->pointsVector();
                if (!points.isEmpty()) {
                    minX = static_cast<qint64>(points.first().x());
                    maxX = static_cast<qint64>(points.last().x());
                } else {
                     maxX = (defaultSampleCount > 0) ? (defaultSampleCount - 1) : 0;
                }
            } else {
                maxX = (defaultSampleCount > 0) ? (defaultSampleCount - 1) : 0;
            }

            if (maxX >= minX) {
                xAxis->setRange(minX, maxX);
            } else {
                 xAxis->setRange(0, defaultSampleCount > 0 ? defaultSampleCount - 1 : 0);
            }
        }
    }
}

void SensorGraph::setYRange(int minY, int maxY) {
    QChart *chartPtr = this->chart();
    if (chartPtr) {
        if (auto *yAxis = qobject_cast<QValueAxis*>(chartPtr->axisY())) {
            yAxis->setRange(minY, maxY);
        }
    }
}

/**
 * @brief Ponownie tłumaczy elementy UI specyficzne dla SensorGraph.
 * @details Aktualizuje tytuł wykresu oraz tytuły osi na podstawie
 * zapisanych kluczy tłumaczeń lub standardowych tekstów.
 */
void SensorGraph::retranslateUi() {
    if (auto *chartPtr = chart()) {
        chartPtr->setTitle(tr(qPrintable(baseTitleKey))); // Przetłumacz zapisany klucz tytułu

        if (auto *xAxis = qobject_cast<QValueAxis*>(chartPtr->axisX())) {
            xAxis->setTitleText(tr("Sample Index"));
        }
        if (auto *yAxis = qobject_cast<QValueAxis*>(chartPtr->axisY())) {
            yAxis->setTitleText(tr("Value"));
        }
        // Jeśli nazwy serii w legendzie również miałyby być tłumaczone,
        // musiałbyś je tutaj zaktualizować, np. przechowując klucze tłumaczeń dla nich.
        // Obecnie są stałe ("X", "Y", "Z").
    }
}