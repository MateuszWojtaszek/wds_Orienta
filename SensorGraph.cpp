/**
 * @file SensorGraph.cpp
 * @brief Implementacja metod klasy SensorGraph.
 */

#include "SensorGraph.h"

#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QLegend>
#include <QDebug>
#include <QStringList>
#include <QColor>
#include <QSizePolicy>
#include <QtMath> // Dla qMax

/*************************/
/**
 * @brief Konstruktor klasy SensorGraph.
 * @details Inicjalizuje wykres z podanym tytułem, zakresem osi Y i opcjonalnym rodzicem.
 * Tworzy wewnętrzny obiekt QChart, konfiguruje tytuł, legendę, osie (X i Y)
 * oraz tworzy trzy serie danych (domyślnie dla osi X, Y, Z) z odpowiednimi kolorami.
 * @param title Tytuł wykresu wyświetlany nad obszarem rysowania.
 * @param minY Minimalna wartość dla osi Y.
 * @param maxY Maksymalna wartość dla osi Y.
 * @param parent Wskaźnik na widget nadrzędny (opcjonalny).
 */
SensorGraph::SensorGraph(const QString &title, int minY, int maxY, QWidget *parent)
    : QChartView(new QChart(), parent), sampleCount(1000), currentSampleIndex(0)
{
    QChart *chartPtr = this->chart(); // Pobranie wskaźnika do obiektu QChart
    if (!chartPtr) {
        qWarning() << "Failed to get chart object!"; // Ostrzeżenie, jeśli nie udało się uzyskać obiektu wykresu
        return;
    }
    chartPtr->setTitle(title); // Ustawienie tytułu wykresu

    QLegend *legend = chartPtr->legend(); // Pobranie obiektu legendy
    if (legend) {
        legend->setVisible(true); // Ustawienie widoczności legendy
        legend->setAlignment(Qt::AlignTop); // Wyrównanie legendy do góry
        legend->setMarkerShape(QLegend::MarkerShapeCircle); // Ustawienie kształtu znacznika w legendzie
    }

    // Nazwy osi i kolory dla serii danych
    QStringList axisNames = {"X", "Y", "Z"};
    QList<QColor> colors = {Qt::blue, Qt::red, Qt::green};

    // Utworzenie i konfiguracja trzech serii liniowych
    for (int i = 0; i < 3; ++i) {
        auto *series = new QLineSeries(); // Utworzenie nowej serii
        series->setName(axisNames[i]); // Ustawienie nazwy serii (widocznej w legendzie)
        series->setColor(colors[i]); // Ustawienie koloru serii
        chartPtr->addSeries(series); // Dodanie serii do wykresu
        seriesList.append(series); // Dodanie serii do listy zarządzanej przez klasę
    }

    // Konfiguracja osi X (indeks próbki)
    auto xAxis = new QValueAxis();
    xAxis->setTitleText("Sample Index"); // Tytuł osi X
    xAxis->setTickCount(11); // Liczba głównych znaczników na osi
    xAxis->setLabelFormat("%lld"); // Format etykiet (liczby całkowite 64-bitowe)
    chartPtr->setAxisX(xAxis); // Ustawienie osi X dla wykresu

    // Konfiguracja osi Y (wartość)
    auto yAxis = new QValueAxis();
    yAxis->setTitleText("Value"); // Tytuł osi Y
    yAxis->setRange(minY, maxY); // Ustawienie początkowego zakresu osi Y
    chartPtr->setAxisY(yAxis); // Ustawienie osi Y dla wykresu

    // Przypisanie osi X i Y do każdej serii danych
    for (QLineSeries *series : seriesList) {
        if (series) {
            chartPtr->setAxisX(xAxis, series);
            chartPtr->setAxisY(yAxis, series);
        }
    }
    // Ustawienie początkowego zakresu osi X
    xAxis->setRange(0, sampleCount > 0 ? sampleCount - 1 : 0);

    // Ustawienie polityki rozmiaru widgetu
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
}

/*************************/
/**
 * @brief Dodaje nowy zestaw danych (punkt pomiarowy) do wykresu.
 * @details Funkcja przyjmuje wektor `data` z trzema wartościami. Każda wartość jest dodawana
 * do odpowiedniej serii (pierwsza do serii X, druga do Y, trzecia do Z)
 * z bieżącym indeksem próbki `currentSampleIndex` jako wartością X.
 * Jeśli liczba punktów w serii przekroczy `sampleCount`, najstarszy punkt jest usuwany.
 * Oś X jest aktualizowana co `xAxisUpdateFrequency` próbek lub przy pierwszej próbce,
 * aby pokazywać okno ostatnich `sampleCount` próbek.
 * @param data Wektor `QVector<int>` zawierający dokładnie trzy wartości danych do dodania.
 * Jeśli rozmiar wektora jest inny niż 3, funkcja nic nie robi.
 * @return void Funkcja nie zwraca wartości.
 */
void SensorGraph::addData(const QVector<int> &data) {
    QChart *chartPtr = this->chart();
    // Sprawdzenie warunków wstępnych: rozmiar danych, liczba serii, istnienie wykresu
    if (data.size() != 3 || seriesList.size() != 3 || !chartPtr) return;

    // Dodanie danych do każdej serii
    for (int i = 0; i < 3; ++i) {
        QLineSeries *series = seriesList.at(i); // Pobranie serii
        if (!series) continue; // Pomiń, jeśli seria nie istnieje
        series->append(currentSampleIndex, data[i]); // Dodanie punktu (indeks próbki, wartość)

        // Usunięcie najstarszego punktu, jeśli przekroczono limit `sampleCount`
        if (series->count() > sampleCount) {
            series->remove(0); // Usuń pierwszy (najstarszy) punkt
        }
    }

    // Okresowa aktualizacja zakresu osi X
    const int xAxisUpdateFrequency = 10; // Częstotliwość aktualizacji osi X
    bool isFirstSample = (currentSampleIndex == 1); // Sprawdzenie, czy to druga dodana próbka (indeks 1)

    if (currentSampleIndex % xAxisUpdateFrequency == 0 || isFirstSample) {
        if (auto *xAxis = qobject_cast<QValueAxis*>(chartPtr->axisX())) { // Bezpieczne rzutowanie na QValueAxis
            qint64 lastIndex = currentSampleIndex; // Ostatni dodany indeks
            // Obliczenie minimalnego indeksu X do wyświetlenia (przesuwne okno)
            qint64 minX = (lastIndex >= sampleCount) ? (lastIndex - sampleCount + 1) : 0;
            qint64 maxX = lastIndex; // Maksymalny indeks X to ostatni dodany

            // Korekta zakresu, jeśli minX > maxX (może się zdarzyć przy małej liczbie próbek)
             if (minX > maxX) {
                 minX = maxX > 0 ? maxX : 0;
             }
            // Korekta zakresu dla początkowych próbek, aby oś zaczynała się od 0
             if (lastIndex < 1) {
                 minX = 0;
                 maxX = sampleCount > 0 ? sampleCount -1 : 0; // Pokaż pełen zakres, jeśli nie ma danych
             }

            xAxis->setRange(minX, maxX); // Ustawienie nowego zakresu osi X
        }
    }

    currentSampleIndex++; // Inkrementacja indeksu próbki dla następnego wywołania
}

/*************************/
/**
 * @brief Ustawia maksymalną liczbę próbek wyświetlanych na wykresie dla każdej serii.
 * @details Aktualizuje wewnętrzną zmienną `sampleCount` (minimum 10). Następnie iteruje
 * przez wszystkie serie danych i usuwa najstarsze punkty, jeśli ich liczba
 * przekracza nowo ustawiony limit. Na koniec aktualizuje zakres osi X,
 * aby pasował do nowej liczby próbek lub aktualnych danych, jeśli istnieją.
 * @param count Nowa maksymalna liczba próbek do wyświetlenia. Wartość jest ograniczana od dołu do 10.
 * @return void Funkcja nie zwraca wartości.
 */
void SensorGraph::setSampleCount(int count) {
    sampleCount = qMax(10, count); // Ustawienie nowej liczby próbek (nie mniej niż 10)
    QChart *chartPtr = this->chart();

    // Usunięcie nadmiarowych punktów z każdej serii
    for (QLineSeries *series : seriesList) {
        if (series) {
            while (series->count() > sampleCount) {
                series->remove(0); // Usuwanie najstarszych punktów
            }
        }
    }

    // Aktualizacja zakresu osi X
    if (chartPtr) {
        if (auto *xAxis = qobject_cast<QValueAxis*>(chartPtr->axisX())) {
             qint64 minX = 0;
             qint64 maxX = 0;

             // Spróbuj ustalić zakres na podstawie istniejących punktów w pierwszej serii
             if(!seriesList.isEmpty() && seriesList.first() && seriesList.first()->count() > 0){
                 const auto& points = seriesList.first()->points();
                 if (!points.isEmpty()) {
                     // Pobierz X pierwszego i ostatniego punktu
                     minX = static_cast<qint64>(points.first().x());
                     maxX = static_cast<qint64>(points.last().x());
                 }
             } else {
                 // Jeśli brak punktów, ustaw domyślny zakres i zresetuj indeks próbki
                 maxX = (sampleCount > 0) ? (sampleCount - 1) : 0;
                 currentSampleIndex = 0; // Reset indeksu, jeśli czyścimy wykres przez zmianę sampleCount przy braku danych
             }

             // Ustaw zakres osi X, upewniając się, że minX <= maxX
             if (maxX >= minX) {
                 xAxis->setRange(minX, maxX);
             }
             else {
                 // Fallback na domyślny zakres, jeśli obliczenia dały niespójny wynik
                 xAxis->setRange(0, sampleCount > 0 ? sampleCount - 1 : 0);
             }
        }
    }
}

/*************************/
/**
 * @brief Ustawia zakres (minimalną i maksymalną wartość) dla osi Y.
 * @details Znajduje oś Y wykresu i ustawia jej zakres na podane wartości `minY` i `maxY`.
 * @param minY Nowa minimalna wartość dla osi Y.
 * @param maxY Nowa maksymalna wartość dla osi Y.
 * @return void Funkcja nie zwraca wartości.
 */
void SensorGraph::setYRange(int minY, int maxY) {
    QChart *chartPtr = this->chart();
    if (chartPtr) {
        // Pobranie osi Y (zakładając, że jest to QValueAxis)
        if (auto *yAxis = qobject_cast<QValueAxis*>(chartPtr->axisY())) {
            yAxis->setRange(minY, maxY); // Ustawienie nowego zakresu
        }
    }
}