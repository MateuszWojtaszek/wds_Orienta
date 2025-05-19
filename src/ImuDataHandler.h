#ifndef IMUDATAHANDLER_H
#define IMUDATAHANDLER_H

#include <QWidget>
#include <QVector>
#include <QQuaternion> // Zawarte dla typu QQuaternion w setRotation

// Forward declarations
class QProgressBar;
class QPushButton;
class QStackedWidget;
class QGroupBox;
class SensorGraph;
class Compass2DRenderer; // Dodano forward declaration dla kompasu 2D

// Forward declarations dla klas Qt3D (tylko dla modelu 3D płytki)
namespace Qt3DCore {
    class QEntity;
    class QTransform;
}
namespace Qt3DRender {
    // QMaterial, QMesh mogą być potrzebne, jeśli QSceneLoader ich używa wewnętrznie
}
namespace Qt3DExtras {
    class Qt3DWindow;
    // QPhongMaterial nie jest już potrzebny, jeśli model płytki go nie używa bezpośrednio
}


/**
 * @class ImuDataHandler
 * @brief Zarządza i wizualizuje dane z jednostki IMU (akcelerometr, żyroskop, magnetometr).
 * @details Klasa ta integruje odbieranie danych sensorów, ich prezentację
 * w formie numerycznej (paski postępu) i graficznej (wykresy czasowe),
 * a także renderowanie sceny 3D pokazującej orientację urządzenia
 * oraz kierunek wskazywany przez kompas 2D. Jest to główny widget
 * odpowiedzialny za interfejs związany z danymi IMU.
 */
class ImuDataHandler : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy ImuDataHandler.
     * @details Inicjalizuje obiekt, ustawia domyślną liczbę próbek dla wykresów
     * i wywołuje `setupMainLayout` w celu skonfigurowania całego interfejsu użytkownika.
     * @param parent [in] Wskaźnik na widget nadrzędny (opcjonalny, domyślnie nullptr).
     */
    explicit ImuDataHandler(QWidget *parent = nullptr);

    /**
     * @brief Aktualizuje dane wyświetlane przez widget (paski postępu i wykresy).
     * @details Odbiera najnowsze odczyty z sensorów IMU i przekazuje je do odpowiednich
     * elementów interfejsu: aktualizuje wartości na paskach postępu (QProgressBar)
     * oraz dodaje nowe punkty danych do wykresów czasowych (SensorGraph).
     * Sprawdza rozmiar wektorów wejściowych (oczekiwany rozmiar to 3).
     * @param acc [in] Wektor danych z akcelerometru [X, Y, Z] (jednostki: mg).
     * @param gyro [in] Wektor danych z żyroskopu [X, Y, Z] (jednostki: dps).
     * @param mag [in] Wektor danych z magnetometru [X, Y, Z] (jednostki: mG).
     */
    void updateData(const QVector<int> &acc, const QVector<int> &gyro, const QVector<int> &mag);

    /**
     * @brief Ustawia liczbę próbek (historię) wyświetlanych na wykresach.
     * @details Definiuje, ile ostatnich punktów danych ma być przechowywanych
     * i wyświetlanych na każdym z wykresów (akcelerometr, żyroskop, magnetometr).
     * Wartość jest wewnętrznie ograniczana do minimum 10 próbek.
     * Aktualizuje ustawienia dla wszystkich obiektów SensorGraph.
     * @param samples [in] Liczba próbek do wyświetlenia (minimum 10).
     */
    void setSampleCount(int samples);

    /**
     * @brief Ustawia zakres wartości dla pasków postępu i osi Y wykresów.
     * @details Konfiguruje minimalne i maksymalne wartości wyświetlane na osi Y wykresów
     * oraz zakresy dla pasków postępu, aby dopasować je do oczekiwanych
     * wartości z sensorów.
     * @note Aktualnie funkcja ustawia predefiniowane, sztywne zakresy dla każdego typu sensora,
     * ignorując przekazane parametry `minVal` i `maxVal`.
     * Zakresy: Akcelerometr (-4000, 4000), Żyroskop (-250, 250), Magnetometr (-1600, 1600).
     * @param minVal [in] Minimalna wartość zakresu (obecnie ignorowana).
     * @param maxVal [in] Maksymalna wartość zakresu (obecnie ignorowana).
     */
    void setRange(int minVal, int maxVal);

    /**
     * @brief Ustawia rotację modelu 3D reprezentującego urządzenie na podstawie kątów Eulera.
     * @details Konwertuje podane kąty odchylenia (yaw), pochylenia (pitch) i przechylenia (roll)
     * na kwaternion reprezentujący orientację, a następnie stosuje tę transformację
     * do modelu 3D płytki (`boardTransform`) w scenie Qt3D. Jeśli transformacja
     * `boardTransform` nie została jeszcze zainicjalizowana (np. model się nie załadował),
     * operacja nie ma efektu.
     * @param yaw [in] Kąt odchylenia (obrót wokół osi Z) w stopniach.
     * @param pitch [in] Kąt pochylenia (obrót wokół osi X) w stopniach.
     * @param roll [in] Kąt przechylenia (obrót wokół osi Y) w stopniach.
     */
    void setRotation(float yaw, float pitch, float roll);

    /**
     * @brief Aktualizuje kierunek wskazywany przez kompas 2D.
     * @details Ustawia nowy kurs na widgecie kompasu 2D.
     * @param heading [in] Kierunek w stopniach (0 = Północ).
     */
    void updateCompass(float heading);
    /**
     * @brief Aktualizuje napisy interfejsu użytkownika w klasie ImuDataHandler po zmianie języka.
     * @details Ta funkcja odświeża teksty wszystkich elementów interfejsu, które używają `tr(...)`,
     * takich jak przyciski wyboru widoku ("Current Data", "Graph") oraz tytuły grup pasków i wykresów.
     * Powinna być wywoływana po załadowaniu nowego tłumaczenia.
     */
    void retranslateUi();

public slots:
    /**
     * @brief Slot: Przełącza widok w lewym panelu na zakładkę z aktualnymi danymi (paski postępu).
     * @details Aktywuje stronę w QStackedWidget (`stackedWidget`) o indeksie 0,
     * która zawiera widget z paskami postępu (wynik `createBarDisplayWidget`).
     * Wywoływany po kliknięciu przycisku "Current Data".
     */
    void showCurrentData();

    /**
     * @brief Slot: Przełącza widok w lewym panelu na zakładkę z wykresami danych historycznych.
     * @details Aktywuje stronę w QStackedWidget (`stackedWidget`) o indeksie 1,
     * która zawiera widget z wykresami (wynik `createGraphDisplayWidget`).
     * Wywoływany po kliknięciu przycisku "Graph".
     */
    void showGraph();

private:
    void setupMainLayout();
    QWidget* createButtonPanel();
    QWidget* createLeftPanel();
    QWidget* createBarDisplayWidget();
    QWidget* createGraphDisplayWidget();
    void setupVisualizationPanel();
    QWidget *create3DView(); // Dla modelu płytki 3D
    void setupCameraAndController3D(Qt3DExtras::Qt3DWindow* view, Qt3DCore::QEntity* rootEntity); // Dla modelu płytki 3D
    void setupLighting3D(Qt3DCore::QEntity* rootEntity); // Dla modelu płytki 3D
    void setupModelLoader3D(Qt3DCore::QEntity* rootEntity); // Dla modelu płytki 3D

    // Paski postępu dla danych
    QProgressBar *accXBar, *accYBar, *accZBar;
    QProgressBar *gyroXBar, *gyroYBar, *gyroZBar;
    QProgressBar *magXBar, *magYBar, *magZBar;

    // Wykresy dla danych
    SensorGraph *accGraph;
    SensorGraph *gyroGraph;
    SensorGraph *magGraph;

    QStackedWidget *stackedWidget;
    int currentSampleCount;

    QWidget* visualizationPanelWidget;
    QWidget* view3DContainerWidget; // Kontener dla widoku 3D płytki
    Compass2DRenderer* m_compass2DRenderer; // Wskaźnik do kompasu 2D

    Qt3DCore::QTransform *boardTransform; // Transformacja dla modelu 3D płytki

    // Wskaźniki do UI elementów dla łatwej aktualizacji w retranslateUi
    QPushButton *m_currentDataButton;
    QPushButton *m_graphButton;
    QGroupBox *m_accGroupBox;
    QGroupBox *m_gyroGroupBox;
    QGroupBox *m_magGroupBox;
};

#endif // IMUDATAHANDLER_H