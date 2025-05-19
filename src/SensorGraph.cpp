/**
 * @file SensorGraph.cpp
 * @brief Implementacja metod klasy SensorGraph.
 * @details Zawiera logikę inicjalizacji wykresu, dodawania danych,
 * ustawiania liczby próbek oraz zakresu osi Y, a także ponownego tłumaczenia UI.
 * @author Mateusz Wojtaszek
 * @date 2025-05-19
 */

#include "SensorGraph.h"

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegend>
#include <QDebug>
#include <QStringList>
#include <QColor>
#include <QSizePolicy>
#include <QtMath> // Dla qMax
#include <QPainter>


SensorGraph::SensorGraph(const QString &titleKey, int minY, int maxY, QWidget *parent)
    : QChartView(new QChart(), parent), // Inicjalizacja QChart bezpośrednio
      m_maxSampleCount(1000),           // Domyślna liczba próbek
      m_currentSampleIndex(0),
      m_baseTitleKey(titleKey)
{
    QChart *chartPtr = this->chart(); // Pobierz wskaźnik na QChart

    chartPtr->setTitle(tr(qPrintable(m_baseTitleKey))); // Użyj tr() dla tytułu

    QLegend *legend = chartPtr->legend();
    legend->setVisible(true);
    legend->setAlignment(Qt::AlignTop);
    legend->setMarkerShape(QLegend::MarkerShapeCircle);

    // Nazwy serii są stałe i nie podlegają tłumaczeniu w tym kontekście,
    // reprezentują kanały danych.
    QStringList seriesNames = {"X", "Y", "Z"};
    QList<QColor> seriesColors = {Qt::blue, Qt::red, Qt::green};

    for (int i = 0; i < 3; ++i) {
        auto *series = new QLineSeries(this); // Ustawienie rodzica dla serii
        series->setName(seriesNames[i]);
        series->setColor(seriesColors[i]);
        chartPtr->addSeries(series);
        m_seriesList.append(series);
    }

    auto *axisX = new QValueAxis(this); // Ustawienie rodzica dla osi
    axisX->setTitleText(tr("Indeks próbki"));
    axisX->setTickCount(11); // Przykładowa liczba ticków
    axisX->setLabelFormat("%lld");
    chartPtr->setAxisX(axisX);

    auto *axisY = new QValueAxis(this); // Ustawienie rodzica dla osi
    axisY->setTitleText(tr("Wartość"));
    axisY->setRange(minY, maxY);
    chartPtr->setAxisY(axisY);

    // Przypisanie osi do serii
    for (QLineSeries *series : m_seriesList) {
        chartPtr->setAxisX(axisX, series);
        chartPtr->setAxisY(axisY, series);
    }

    axisX->setRange(0, m_maxSampleCount > 0 ? m_maxSampleCount - 1 : 0);

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    this->setRenderHint(QPainter::Antialiasing);
}

void SensorGraph::addData(const QVector<int> &axisValuesToAdd) {
    QChart *chartPtr = this->chart();
    if (axisValuesToAdd.size() != 3 || m_seriesList.size() != 3 || !chartPtr) {
        // Można dodać qWarning, jeśli oczekiwane jest logowanie takich sytuacji
        return;
    }

    for (int i = 0; i < 3; ++i) {
        QLineSeries *series = m_seriesList.at(i);
        if (!series) continue;

        series->append(m_currentSampleIndex, axisValuesToAdd[i]);

        // Usuń najstarszy punkt, jeśli przekroczono limit
        if (series->count() > m_maxSampleCount) {
            series->remove(0);
        }
    }

    // Aktualizacja zakresu osi X
    // Aktualizuj co pewną liczbę próbek lub przy pierwszej próbce, aby zoptymalizować wydajność
    const int xAxisUpdateFrequency = 10; // Aktualizuj rzadziej dla większej liczby danych
    bool needsUpdate = (m_currentSampleIndex == 0 || m_currentSampleIndex % xAxisUpdateFrequency == 0);

    if (needsUpdate) {
        if (auto *axisX = qobject_cast<QValueAxis*>(chartPtr->axisX())) {
            qint64 minX = 0;
            qint64 maxX = m_currentSampleIndex;

            if (m_currentSampleIndex >= m_maxSampleCount) {
                minX = m_currentSampleIndex - m_maxSampleCount + 1;
            } else {
                minX = 0;
                maxX = qMax(m_currentSampleIndex, static_cast<qint64>(m_maxSampleCount -1) ); // Upewnij się, że początkowy zakres jest poprawny
            }
            axisX->setRange(minX, maxX);
        }
    }
    m_currentSampleIndex++;
}

void SensorGraph::setSampleCount(int sampleCount) {
    m_maxSampleCount = qMax(10, sampleCount); // Minimalna liczba próbek to 10
    QChart *chartPtr = this->chart();

    // Dostosuj istniejące serie do nowej liczby próbek
    for (QLineSeries *series : m_seriesList) {
        if (series) {
            while (series->count() > m_maxSampleCount) {
                series->remove(0); // Usuwaj najstarsze próbki
            }
        }
    }

    // Zaktualizuj zakres osi X
    if (chartPtr) {
        if (auto *axisX = qobject_cast<QValueAxis*>(chartPtr->axisX())) {
            qint64 minX = 0;
            qint64 maxX = m_maxSampleCount > 0 ? m_maxSampleCount - 1 : 0; // Domyślny zakres

            if (m_currentSampleIndex > 0) { // Jeśli są już jakieś dane
                 maxX = m_currentSampleIndex;
                 if (m_currentSampleIndex >= m_maxSampleCount) {
                    minX = m_currentSampleIndex - m_maxSampleCount + 1;
                 }
            }
            axisX->setRange(minX, maxX);
        }
    }
}

void SensorGraph::setYRange(int minY, int maxY) {
    QChart *chartPtr = this->chart();
    if (chartPtr) {
        if (auto *axisY = qobject_cast<QValueAxis*>(chartPtr->axisY())) {
            if (minY >= maxY) {
                qWarning() << "SensorGraph::setYRange: minY musi być mniejsze niż maxY.";
                return;
            }
            axisY->setRange(minY, maxY);
        }
    }
}

void SensorGraph::retranslateUi() {
    if (auto *chartPtr = chart()) {
        chartPtr->setTitle(tr(qPrintable(m_baseTitleKey)));

        if (auto *axisX = qobject_cast<QValueAxis*>(chartPtr->axisX())) {
            axisX->setTitleText(tr("Indeks próbki"));
        }
        if (auto *axisY = qobject_cast<QValueAxis*>(chartPtr->axisY())) {
            axisY->setTitleText(tr("Wartość"));
        }
        // Nazwy serii ("X", "Y", "Z") są obecnie stałe i nie są tłumaczone.
        // Jeśli miałyby być tłumaczone, należałoby zaimplementować ich aktualizację tutaj.
    }
}