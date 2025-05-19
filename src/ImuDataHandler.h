#ifndef IMUDATAHANDLER_H
#define IMUDATAHANDLER_H

#include <QWidget>
#include <QVector>
#include <QQuaternion> // Dla QQuaternion w setRotation

// Forward declarations
class QProgressBar;
class QPushButton;
class QStackedWidget;
class QGroupBox;
class SensorGraph;
class Compass2DRenderer;

namespace Qt3DCore {
    class QEntity;
    class QTransform;
}
namespace Qt3DRender {
    // QMaterial, QMesh mogą być potrzebne, jeśli QSceneLoader ich używa wewnętrznie
}
namespace Qt3DExtras {
    class Qt3DWindow;
}

/**
 * @file ImuDataHandler.h
 * @brief Definicja klasy ImuDataHandler, odpowiedzialnej za zarządzanie i wizualizację danych IMU.
 */

/**
 * @class ImuDataHandler
 * @brief Zarządza, przetwarza i wizualizuje dane z jednostki IMU.
 * @details Klasa `ImuDataHandler` jest centralnym komponentem interfejsu użytkownika
 * do obsługi danych z czujników IMU (akcelerometru, żyroskopu, magnetometru).
 * Odpowiada za:
 * - Odbieranie i aktualizowanie danych z czujników.
 * - Prezentację danych numerycznych za pomocą pasków postępu.
 * - Wizualizację danych historycznych na wykresach czasowych.
 * - Renderowanie orientacji urządzenia w przestrzeni 3D.
 * - Wyświetlanie kierunku kompasu 2D.
 * - Przełączanie między widokami danych bieżących i historycznych.
 * - Obsługę internacjonalizacji (zmiany języka interfejsu).
 *
 * @example ImuDataHandlerUsage_PL.cpp
 */
class ImuDataHandler : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy ImuDataHandler.
     * @details Inicjalizuje widget, ustawia domyślną liczbę próbek dla wykresów
     * oraz konfiguruje główny layout interfejsu użytkownika poprzez wywołanie `setupMainLayout()`.
     * @param parent Wskaźnik na widget nadrzędny. Domyślnie `nullptr`.
     */
    explicit ImuDataHandler(QWidget *parent = nullptr);

    /**
     * @brief Aktualizuje dane wyświetlane przez widget.
     * @details Przetwarza nowe odczyty z sensorów IMU i aktualizuje odpowiednie
     * elementy interfejsu: paski postępu (QProgressBar) oraz wykresy
     * czasowe (SensorGraph). Funkcja oczekuje, że każdy z wektorów
     * będzie zawierał trzy elementy (dla osi X, Y, Z).
     * @param acc Wektor danych z akcelerometru [X, Y, Z], jednostki: $mg$.
     * @param gyro Wektor danych z żyroskopu [X, Y, Z], jednostki: $dps$.
     * @param mag Wektor danych z magnetometru [X, Y, Z], jednostki: $mG$.
     * @warning Jeśli rozmiar któregokolwiek z wektorów wejściowych nie wynosi 3,
     * odpowiadające mu dane nie zostaną zaktualizowane dla pasków i wykresów.
     */
    void updateData(const QVector<int> &acc, const QVector<int> &gyro, const QVector<int> &mag);

    /**
     * @brief Ustawia liczbę próbek (historię) wyświetlanych na wykresach.
     * @details Definiuje, ile ostatnich punktów danych ma być przechowywanych
     * i renderowanych na każdym z wykresów (akcelerometr, żyroskop, magnetometr).
     * Wartość jest wewnętrznie ograniczana do minimum 10 próbek.
     * @param samples Liczba próbek do wyświetlenia. Wartość minimalna to 10.
     */
    void setSampleCount(int samples);

    /**
     * @brief Ustawia zakres wartości dla pasków postępu i osi Y wykresów.
     * @details Konfiguruje minimalne i maksymalne wartości dla osi Y wykresów
     * oraz zakresy dla pasków postępu, aby dostosować je do oczekiwanych
     * wartości z sensorów.
     * @note Obecnie funkcja ustawia predefiniowane, stałe zakresy dla każdego typu sensora:
     * - Akcelerometr: $[-4000, 4000]\ mg$
     * - Żyroskop: $[-250, 250]\ dps$
     * - Magnetometr: $[-1600, 1600]\ mG$
     * Przekazane argumenty `minVal` i `maxVal` są ignorowane.
     * @param minVal Minimalna wartość zakresu (obecnie ignorowana).
     * @param maxVal Maksymalna wartość zakresu (obecnie ignorowana).
     */
    void setRange(int minVal, int maxVal);

    /**
     * @brief Ustawia rotację modelu 3D reprezentującego urządzenie.
     * @details Konwertuje podane kąty Eulera (yaw, pitch, roll) na kwaternion
     * reprezentujący orientację, a następnie aplikuje tę transformację
     * do modelu 3D płytki (`boardTransform`) w scenie Qt3D.
     * Jeśli transformacja `boardTransform` nie została zainicjalizowana
     * (np. model 3D nie został jeszcze załadowany), operacja nie przyniesie efektu.
     * @param yaw Kąt odchylenia (obrót wokół osi Z globalnego układu współrzędnych) w stopniach.
     * @param pitch Kąt pochylenia (obrót wokół osi X globalnego układu współrzędnych) w stopniach.
     * @param roll Kąt przechylenia (obrót wokół osi Y globalnego układu współrzędnych) w stopniach.
     */
    void setRotation(float yaw, float pitch, float roll);

    /**
     * @brief Aktualizuje kierunek wskazywany przez kompas 2D.
     * @details Ustawia nowy kąt kursu na widgecie kompasu 2D (`m_compass2DRenderer`).
     * @param heading Kierunek w stopniach, gdzie $0^\circ$ oznacza Północ.
     */
    void updateCompass(float heading);

    /**
     * @brief Aktualizuje teksty interfejsu użytkownika po zmianie języka.
     * @details Odświeża wszystkie teksty etykiet, przycisków i tytułów grup,
     * które korzystają z mechanizmu tłumaczeń Qt (`tr(...)`).
     * Funkcja powinna być wywoływana po załadowaniu nowego pliku tłumaczeń
     * do aplikacji.
     */
    void retranslateUi();

public slots:
    /**
     * @brief Slot: Przełącza widok na zakładkę z aktualnymi danymi (paski postępu).
     * @details Aktywuje stronę w `QStackedWidget` (`stackedWidget`) o indeksie 0,
     * która zawiera widget z paskami postępu. Wywoływany zazwyczaj
     * po kliknięciu przycisku "Dane Bieżące" (lub jego odpowiednika).
     */
    void showCurrentData();

    /**
     * @brief Slot: Przełącza widok na zakładkę z wykresami danych historycznych.
     * @details Aktywuje stronę w `QStackedWidget` (`stackedWidget`) o indeksie 1,
     * która zawiera widget z wykresami. Wywoływany zazwyczaj
     * po kliknięciu przycisku "Wykres" (lub jego odpowiednika).
     */
    void showGraph();

private:
    /** @brief Inicjalizuje i konfiguruje główny layout widgetu. */
    void setupMainLayout();
    /** @brief Tworzy panel z przyciskami do przełączania widoków. @return Wskaźnik na utworzony widget. */
    QWidget* createButtonPanel();
    /** @brief Tworzy lewy panel zawierający `QStackedWidget` z widokami danych. @return Wskaźnik na utworzony widget. */
    QWidget* createLeftPanel();
    /** @brief Tworzy widget wyświetlający dane w formie pasków postępu. @return Wskaźnik na utworzony widget. */
    QWidget* createBarDisplayWidget();
    /** @brief Tworzy widget wyświetlający dane w formie wykresów. @return Wskaźnik na utworzony widget. */
    QWidget* createGraphDisplayWidget();
    /** @brief Inicjalizuje i konfiguruje panel wizualizacji (model 3D i kompas 2D). */
    void setupVisualizationPanel();
    /** @brief Tworzy i konfiguruje widok 3D dla modelu płytki. @return Wskaźnik na kontener widgetu Qt3DWindow. */
    QWidget *create3DView();
    /** @brief Konfiguruje kamerę i kontroler kamery dla sceny 3D.
     * @param view Wskaźnik na okno Qt3D.
     * @param rootEntity Wskaźnik na główną encję sceny.
     */
    void setupCameraAndController3D(Qt3DExtras::Qt3DWindow* view, Qt3DCore::QEntity* rootEntity);
    /** @brief Konfiguruje oświetlenie dla sceny 3D.
     * @param rootEntity Wskaźnik na główną encję sceny.
     */
    void setupLighting3D(Qt3DCore::QEntity* rootEntity);
    /** @brief Konfiguruje ładowanie modelu 3D do sceny.
     * @param rootEntity Wskaźnik na główną encję sceny.
     */
    void setupModelLoader3D(Qt3DCore::QEntity* rootEntity);

    // Paski postępu dla danych
    QProgressBar *accXBar, *accYBar, *accZBar;       //!< Paski postępu dla danych akcelerometru (X, Y, Z).
    QProgressBar *gyroXBar, *gyroYBar, *gyroZBar;     //!< Paski postępu dla danych żyroskopu (X, Y, Z).
    QProgressBar *magXBar, *magYBar, *magZBar;       //!< Paski postępu dla danych magnetometru (X, Y, Z).

    // Wykresy dla danych
    SensorGraph *accGraph;                          //!< Wykres dla danych akcelerometru.
    SensorGraph *gyroGraph;                         //!< Wykres dla danych żyroskopu.
    SensorGraph *magGraph;                          //!< Wykres dla danych magnetometru.

    QStackedWidget *stackedWidget;                  //!< Widget przełączający widoki danych bieżących i wykresów.
    int currentSampleCount;                         //!< Aktualna liczba próbek wyświetlanych na wykresach.

    QWidget* visualizationPanelWidget;              //!< Główny widget panelu wizualizacji (3D i kompas).
    QWidget* view3DContainerWidget;                 //!< Kontener dla widoku 3D modelu płytki.
    Compass2DRenderer* m_compass2DRenderer;         //!< Wskaźnik na widget renderujący kompas 2D.

    Qt3DCore::QTransform *boardTransform;           //!< Transformacja stosowana do modelu 3D płytki.

    // Wskaźniki do elementów UI dla łatwej aktualizacji w retranslateUi
    QPushButton *m_currentDataButton;               //!< Przycisk przełączający na widok danych bieżących.
    QPushButton *m_graphButton;                     //!< Przycisk przełączający na widok wykresów.
    QGroupBox *m_accGroupBox;                       //!< Grupa UI dla danych akcelerometru.
    QGroupBox *m_gyroGroupBox;                      //!< Grupa UI dla danych żyroskopu.
    QGroupBox *m_magGroupBox;                       //!< Grupa UI dla danych magnetometru.
};

#endif // IMUDATAHANDLER_H