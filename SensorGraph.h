/**
 * @file SensorGraph.h
 * @brief Definiuje klasę SensorGraph służącą do wyświetlania danych z czujników na wykresie.
 */

#ifndef SENSORGRAPH_H
#define SENSORGRAPH_H

#include <QChartView>
#include <QList>
#include <QVector>
#include <QString>

// Forward declarations
class QLineSeries;
class QValueAxis;
class QWidget;
class QChart;

/**
 * @brief Klasa SensorGraph dziedzicząca po QChartView, przeznaczona do wyświetlania danych
 * z czujników (np. 3-osiowych) w czasie rzeczywistym jako wykres liniowy.
 * @details Zarządza seriami danych dla różnych kanałów, konfiguruje osie wykresu
 * i aktualizuje widok w miarę napływania nowych danych.
 */
class SensorGraph : public QChartView {
    Q_OBJECT // Makro wymagane dla klas Qt używających sygnałów i slotów

public:
    /**
     * @brief Konstruktor klasy SensorGraph.
     * @details Inicjalizuje wykres z podanym tytułem, zakresem osi Y i opcjonalnym rodzicem.
     * Tworzy trzy serie danych (domyślnie dla osi X, Y, Z).
     * @param title Tytuł wykresu wyświetlany nad obszarem rysowania.
     * @param minY Minimalna wartość dla osi Y.
     * @param maxY Maksymalna wartość dla osi Y.
     * @param parent Wskaźnik na widget nadrzędny (opcjonalny).
     */
    explicit SensorGraph(const QString &title, int minY, int maxY, QWidget *parent = nullptr);

    /**
     * @brief Dodaje nowy zestaw danych (punkt pomiarowy) do wykresu.
     * @details Oczekuje wektora z trzema wartościami całkowitymi, które są dodawane
     * do odpowiednich serii danych (X, Y, Z). Aktualizuje oś X, aby pokazywać
     * najnowszy zakres próbek.
     * @param data Wektor `QVector<int>` zawierający trzy wartości danych do dodania.
     * @return void Funkcja nie zwraca wartości.
     */
    void addData(const QVector<int> &data);

    /**
     * @brief Ustawia maksymalną liczbę próbek wyświetlanych na wykresie dla każdej serii.
     * @details Określa, ile ostatnich punktów danych ma być widocznych. Starsze punkty
     * są usuwane, gdy dodawane są nowe, przekraczające ten limit.
     * @param count Nowa maksymalna liczba próbek (minimum 10).
     * @return void Funkcja nie zwraca wartości.
     */
    void setSampleCount(int count);

    /**
     * @brief Ustawia zakres (minimalną i maksymalną wartość) dla osi Y.
     * @details Definiuje widoczny zakres wartości na pionowej osi wykresu.
     * @param minY Nowa minimalna wartość dla osi Y.
     * @param maxY Nowa maksymalna wartość dla osi Y.
     * @return void Funkcja nie zwraca wartości.
     */
    void setYRange(int minY, int maxY);

private:
    /**
     * @brief Lista przechowująca wskaźniki do obiektów QLineSeries, reprezentujących
     * poszczególne kanały danych (np. X, Y, Z).
     */
    QList<QLineSeries *> seriesList;

    /**
     * @brief Maksymalna liczba punktów danych wyświetlanych jednocześnie na wykresie
     * dla każdej serii. Domyślnie 1000.
     */
    int sampleCount = 1000;

    /**
     * @brief Bieżący indeks próbki, używany jako wartość na osi X dla nowo dodawanych punktów.
     * Inkrementowany po każdym dodaniu danych.
     */
    qint64 currentSampleIndex = 0;
};

#endif // SENSORGRAPH_H