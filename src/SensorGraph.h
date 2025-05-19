/**
 * @file SensorGraph.h
 * @brief Definicja klasy SensorGraph do wyświetlania danych z czujników na wykresie.
 * @details Rozszerza QChartView i służy do wizualizacji danych w czasie rzeczywistym.
 * Obsługuje trzy kanały (np. X, Y, Z) jako osobne serie danych.
 * @author Mateusz Wojtaszek (lub oryginalny autor, jeśli inny)
 * @date 2025-04-29 (lub aktualna data modyfikacji)
 */

#ifndef SENSORGRAPH_H
#define SENSORGRAPH_H

#include <QtCharts/QChartView> // Poprawiono ścieżkę do QChartView
#include <QList>
#include <QVector>
#include <QString>

// Forward declarations klas Qt
QT_BEGIN_NAMESPACE // Używane dla Qt Charts
class QLineSeries;
class QValueAxis;
// QWidget jest już znany przez QChartView
// class QChart; // QChart jest częścią QChartView
QT_END_NAMESPACE


/**
 * @class SensorGraph
 * @brief Klasa odpowiadająca za wizualizację danych z czujników w formie wykresu liniowego.
 * @details Dziedziczy po QChartView i umożliwia dynamiczne rysowanie danych z trzech
 * kanałów (np. osie X, Y, Z akcelerometru) w czasie rzeczywistym. Wykres
 * automatycznie przesuwa się wraz z napływem nowych danych, a najstarsze
 * punkty są usuwane po przekroczeniu zdefiniowanego limitu próbek.
 * @note Wykorzystuje bibliotekę Qt Charts.
 */
class SensorGraph : public QChartView { // QChartView jest już w namespace QtCharts
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy SensorGraph.
     * @details Inicjalizuje obiekt QChart, tworzy i konfiguruje trzy serie danych
     * (dla osi X, Y, Z), ustawia osie wykresu (czasową X i wartości Y),
     * konfiguruje legendę oraz ustawia tytuł wykresu na podstawie klucza `titleKey`.
     * Definiuje również początkowy zakres osi Y oraz domyślną liczbę widocznych próbek.
     *
     * @param[in] titleKey Klucz tłumaczenia dla tytułu wyświetlanego nad wykresem (np. "Accelerometer [mg]").
     * @param[in] minY Minimalna wartość początkowa dla osi Y.
     * @param[in] maxY Maksymalna wartość początkowa dla osi Y.
     * @param[in] parent Opcjonalny wskaźnik na widget nadrzędny w hierarchii Qt. Domyślnie nullptr.
     */
    explicit SensorGraph(const QString &titleKey, int minY, int maxY, QWidget *parent = nullptr);

    /**
     * @brief Dodaje nowy punkt danych (zestaw wartości X, Y, Z) do wykresu.
     * @details Oczekuje wektora `QVector<int>` zawierającego dokładnie 3 wartości,
     * które są dodawane do odpowiednich serii danych (X, Y, Z) z bieżącym
     * indeksem próbki jako współrzędną X. Jeśli liczba punktów w serii
     * przekroczy `defaultSampleCount`, najstarszy punkt jest usuwany.
     * Zakres osi X jest okresowo aktualizowany, aby przesuwać "okno" widoku.
     * @param[in] axisValuesToAdd Wektor zawierający 3 liczby całkowite (wartość dla serii X, Y, Z).
     * @note Jeśli rozmiar wektora `axisValuesToAdd` jest inny niż 3, dane nie zostaną dodane.
     */
    void addData(const QVector<int> &axisValuesToAdd);

    /**
     * @brief Ustawia maksymalną liczbę próbek widocznych jednocześnie na wykresie.
     * @details Określa rozmiar "okna czasowego" wykresu. Po dodaniu nowych danych,
     * jeśli liczba punktów przekracza ten limit, najstarsze punkty są usuwane
     * z każdej serii. Minimalna dozwolona wartość to 10. Zakres osi X jest
     * automatycznie dostosowywany do nowej liczby próbek.
     * @param[in] sampleCount Nowa maksymalna liczba widocznych próbek (min. 10).
     * Zmieniono typ na `int` dla prostoty, `const int&` jest rzadko potrzebne dla typów podstawowych.
     */
    void setSampleCount(int sampleCount);

    /**
     * @brief Ustawia zakres (minimum i maksimum) dla osi pionowej (Y).
     * @details Pozwala na dynamiczną zmianę widocznego zakresu wartości na osi Y,
     * np. w celu dostosowania do innego zakresu danych z czujnika.
     * @param[in] minY Nowa minimalna wartość dla osi Y.
     * @param[in] maxY Nowa maksymalna wartość dla osi Y.
     */
    void setYRange(int minY, int maxY);

    /**
     * @brief Ponownie tłumaczy elementy UI specyficzne dla SensorGraph.
     * @details Głównie aktualizuje tytuł wykresu oraz tytuły osi na podstawie
     * zapisanych kluczy tłumaczeń.
     */
    void retranslateUi();

private:
    QList<QLineSeries *> seriesList; ///< Lista wskaźników na serie danych (X, Y, Z).
    int defaultSampleCount;          ///< Maksymalna liczba punktów na serii.
    qint64 currentSampleIndex;       ///< Bieżący indeks próbki (oś X).
    QString baseTitleKey;            ///< Klucz tłumaczenia dla głównego tytułu wykresu.
    // Klucze dla tytułów osi, jeśli mają być dynamicznie tłumaczone i nie są stałe
    // QString xAxisTitleKey;
    // QString yAxisTitleKey;
};

#endif // SENSORGRAPH_H