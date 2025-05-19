/**
 * @file SensorGraph.h
 * @brief Definiuje klasę SensorGraph do wyświetlania danych z czujników na wykresie. [cite: 17]
 * @details Ten plik zawiera deklarację klasy SensorGraph, która rozszerza QChartView
 * i jest przeznaczona do wizualizacji danych telemetrycznych (np. z akcelerometru, żyroskopu)
 * w czasie rzeczywistym. Klasa obsługuje dynamiczne dodawanie danych dla trzech
 * osobnych kanałów (np. osie X, Y, Z), automatyczne przesuwanie osi czasu
 * oraz zarządzanie liczbą wyświetlanych próbek.
 * @author Mateusz Wojtaszek
 * @date 2025-05-19
 * @version 1.0
 */

#ifndef SENSORGRAPH_H
#define SENSORGRAPH_H

#include <QtCharts/QChartView>
#include <QList>
#include <QVector>
#include <QString>

// Forward declarations klas Qt
QT_BEGIN_NAMESPACE
class QLineSeries;
class QValueAxis;
QT_END_NAMESPACE

/**
 * @class SensorGraph
 * @brief Wizualizuje dane z czujników w formie dynamicznego wykresu liniowego.
 * @details Dziedziczy po QChartView, umożliwiając rysowanie do trzech serii danych
 * (np. dla osi X, Y, Z) w czasie rzeczywistym. Wykres automatycznie dostosowuje
 * zakres osi X, aby wyświetlać najnowsze dane, usuwając najstarsze próbki
 * po przekroczeniu zdefiniowanego limitu. Zapewnia również metody konfiguracji
 * zakresu osi Y oraz maksymalnej liczby próbek.
 * @note Wymaga modułu Qt Charts.
 * @example SensorGraphUsage_PL.cpp
 * Poniżej znajduje się przykład użycia klasy SensorGraph:
 */
class SensorGraph : public QChartView {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy SensorGraph.
     * @details Inicjalizuje wykres, tworzy trzy serie danych (dla kanałów X, Y, Z),
     * konfiguruje osie (oś X jako indeks próbki, oś Y jako wartość), ustawia legendę
     * oraz tytuł wykresu.
     * @param titleKey [in] Klucz tłumaczenia dla tytułu wykresu (np. "AccelerometerData").
     * @param minY [in] Początkowa minimalna wartość dla osi Y.
     * @param maxY [in] Początkowa maksymalna wartość dla osi Y.
     * @param parent [in] Opcjonalny wskaźnik na widget nadrzędny. Domyślnie nullptr.
     */
    explicit SensorGraph(const QString &titleKey, int minY, int maxY, QWidget *parent = nullptr);

    /**
     * @brief Dodaje nowy zestaw punktów danych (X, Y, Z) do wykresu.
     * @details Każda wartość z wektora `axisValuesToAdd` jest dodawana do odpowiedniej
     * serii danych (pierwsza wartość do pierwszej serii, itd.) wraz z bieżącym
     * indeksem próbki. Jeśli liczba punktów w serii przekroczy zdefiniowany limit,
     * najstarszy punkt jest usuwany. Oś X jest automatycznie aktualizowana.
     * @param axisValuesToAdd [in] Wektor zawierający 3 wartości całkowite dla kolejnych serii.
     * @note Dane nie zostaną dodane, jeśli wektor nie zawiera dokładnie 3 wartości.
     */
    void addData(const QVector<int> &axisValuesToAdd);

    /**
     * @brief Ustawia maksymalną liczbę próbek wyświetlanych jednocześnie na wykresie.
     * @details Definiuje szerokość "okna" danych widocznych na osi X. Minimalna
     * dozwolona wartość to 10. Zmiana tej wartości powoduje usunięcie nadmiarowych
     * starych próbek i dostosowanie zakresu osi X. [cite: 27]
     * @param sampleCount [in] Nowa maksymalna liczba widocznych próbek (minimum 10).
     */
    void setSampleCount(int sampleCount);

    /**
     * @brief Ustawia zakres (minimum i maksimum) dla osi pionowej (Y).
     * @details Umożliwia dostosowanie widocznego zakresu wartości na osi Y.
     * @param minY [in] Nowa minimalna wartość dla osi Y.
     * @param maxY [in] Nowa maksymalna wartość dla osi Y.
     */
    void setYRange(int minY, int maxY);

    /**
     * @brief Ponownie tłumaczy teksty interfejsu użytkownika wykresu.
     * @details Aktualizuje tytuł wykresu oraz etykiety osi na podstawie
     * przechowywanych kluczy tłumaczeń lub domyślnych wartości,
     * używając mechanizmu `tr()`.
     */
    void retranslateUi();

private:
    QList<QLineSeries *> m_seriesList;      ///< Lista wskaźników na trzy serie danych (X, Y, Z).
    int m_maxSampleCount;                   ///< Maksymalna liczba wyświetlanych punktów na serii.
    qint64 m_currentSampleIndex;            ///< Bieżący indeks próbki (wartość na osi X).
    QString m_baseTitleKey;                 ///< Klucz tłumaczenia dla głównego tytułu wykresu.
};

#endif // SENSORGRAPH_H