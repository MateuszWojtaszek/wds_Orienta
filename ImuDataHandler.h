#ifndef IMUDATAHANDLER_H
#define IMUDATAHANDLER_H

#include <QWidget>
#include <QVector>
#include <QQuaternion>

// Forward declarations
class QProgressBar;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QStackedWidget;
class QLabel;
class QGroupBox;
class QWidget;
class QDebug;
class SensorGraph;

namespace Qt3DExtras {
    class Qt3DWindow;
    class QOrbitCameraController;
    class QPhongMaterial;
    class QTorusMesh;
    class QConeMesh;
    class QCuboidMesh;
    class QText2DEntity;
}
namespace Qt3DRender {
    class QCamera;
    class QSceneLoader;
    class QDirectionalLight;
}
namespace Qt3DCore {
    class QEntity;
    class QTransform;
    class QComponent;
}


/**
 * @brief Klasa ImuDataHandler zarządza i wizualizuje dane z jednostki IMU.
 *
 * Odpowiada za odbieranie danych z akcelerometru, żyroskopu i magnetometru,
 * wyświetlanie ich w postaci pasków postępu, wykresów, a także wizualizację
 * orientacji 3D urządzenia i kierunku kompasu.
 */
class ImuDataHandler : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy ImuDataHandler.
     * @param parent Wskaźnik na widget nadrzędny (opcjonalny).
     */
    explicit ImuDataHandler(QWidget *parent = nullptr);

    /**
     * @brief Aktualizuje dane wyświetlane przez widget.
     * @param acc Wektor danych z akcelerometru [X, Y, Z].
     * @param gyro Wektor danych z żyroskopu [X, Y, Z].
     * @param mag Wektor danych z magnetometru [X, Y, Z].
     */
    void updateData(const QVector<int> &acc, const QVector<int> &gyro, const QVector<int> &mag);

    /**
     * @brief Ustawia liczbę próbek wyświetlanych na wykresach.
     * @param samples Liczba próbek do wyświetlenia.
     */
    void setSampleCount(int samples);

    /**
     * @brief Ustawia zakres wartości dla pasków postępu i wykresów.
     * UWAGA: Aktualnie funkcja ustawia sztywne zakresy zdefiniowane wewnątrz, ignorując parametry.
     * @param minVal Minimalna wartość zakresu (obecnie nieużywana).
     * @param maxVal Maksymalna wartość zakresu (obecnie nieużywana).
     */
    void setRange(int minVal, int maxVal);

    /**
     * @brief Ustawia rotację modelu 3D na podstawie kątów Eulera.
     * @param yaw Kąt odchylenia (obrót wokół osi Z).
     * @param pitch Kąt pochylenia (obrót wokół osi X).
     * @param roll Kąt przechylenia (obrót wokół osi Y).
     */
    void setRotation(float yaw, float pitch, float roll);

    /**
     * @brief Aktualizuje kierunek wskazywany przez strzałkę kompasu.
     * @param heading Kierunek w stopniach (0 = Północ).
     */
    void updateCompass(float heading);

private slots:
    /**
     * @brief Slot przełączający widok na zakładkę z aktualnymi danymi (paski postępu).
     */
    void showCurrentData();

    /**
     * @brief Slot przełączający widok na zakładkę z wykresami.
     */
    void showGraph();

private:
    /**
     * @brief Konfiguruje główny układ interfejsu użytkownika.
     */
    void setupMainLayout();

    /**
     * @brief Tworzy panel z przyciskami do przełączania widoków.
     * @return Wskaźnik na utworzony widget panelu przycisków.
     */
    QWidget* createButtonPanel();

    /**
     * @brief Tworzy lewy panel zawierający przełączane widoki danych (paski/wykresy).
     * @return Wskaźnik na utworzony widget lewego panelu.
     */
    QWidget* createLeftPanel();

    /**
     * @brief Tworzy widget wyświetlający dane w postaci pasków postępu.
     * @return Wskaźnik na utworzony widget z paskami postępu.
     */
    QWidget* createBarDisplayWidget();

    /**
     * @brief Tworzy widget wyświetlający dane w postaci wykresów.
     * @return Wskaźnik na utworzony widget z wykresami.
     */
    QWidget* createGraphDisplayWidget();

    /**
     * @brief Konfiguruje panel wizualizacji 3D (model i kompas).
     */
    void setupVisualizationPanel();

    /**
     * @brief Tworzy widok 3D do wyświetlania modelu urządzenia.
     * @return Wskaźnik na kontener QWidget zawierający okno Qt3DWindow dla modelu 3D.
     */
    QWidget *create3DView();

    /**
     * @brief Konfiguruje kamerę i kontroler dla widoku modelu 3D.
     * @param view Wskaźnik na okno Qt3DWindow dla modelu 3D.
     * @param rootEntity Wskaźnik na główny byt sceny 3D.
     */
    void setupCameraAndController3D(Qt3DExtras::Qt3DWindow* view, Qt3DCore::QEntity* rootEntity);

    /**
     * @brief Konfiguruje oświetlenie dla sceny modelu 3D.
     * @param rootEntity Wskaźnik na główny byt sceny 3D.
     */
    void setupLighting3D(Qt3DCore::QEntity* rootEntity);

    /**
     * @brief Konfiguruje ładowanie modelu 3D (np. z pliku .dae).
     * @param rootEntity Wskaźnik na główny byt sceny 3D.
     */
    void setupModelLoader3D(Qt3DCore::QEntity* rootEntity);

    /**
     * @brief Tworzy widok 3D dla kompasu.
     * @return Wskaźnik na kontener QWidget zawierający okno Qt3DWindow dla kompasu.
     */
    QWidget *createCompassView();

    /**
     * @brief Konfiguruje kamerę i kontroler dla widoku kompasu.
     * @param view Wskaźnik na okno Qt3DWindow dla kompasu.
     * @param rootEntity Wskaźnik na główny byt sceny kompasu.
     */
    void setupCameraAndControllerCompass(Qt3DExtras::Qt3DWindow* view, Qt3DCore::QEntity* rootEntity);

    /**
     * @brief Konfiguruje oświetlenie dla sceny kompasu.
     * @param rootEntity Wskaźnik na główny byt sceny kompasu.
     */
    void setupLightingCompass(Qt3DCore::QEntity* rootEntity);

    /**
     * @brief Tworzy pierścień kompasu jako element sceny 3D.
     * @param parentEntity Wskaźnik na byt nadrzędny, do którego zostanie dodany pierścień.
     * @return Wskaźnik na utworzony byt pierścienia kompasu.
     */
    Qt3DCore::QEntity* createCompassRing(Qt3DCore::QEntity* parentEntity);

    /**
     * @brief Tworzy strzałkę kompasu jako element sceny 3D.
     * @param parentEntity Wskaźnik na byt nadrzędny, do którego zostanie dodana strzałka.
     * @return Wskaźnik na utworzony byt strzałki kompasu.
     */
    Qt3DCore::QEntity* createCompassArrow(Qt3DCore::QEntity* parentEntity);

    /**
     * @brief Dodaje etykiety kierunków (N, E, S, W) do sceny kompasu.
     * @param parentEntity Wskaźnik na byt nadrzędny, do którego zostaną dodane etykiety.
     */
    void addCompassLabels(Qt3DCore::QEntity* parentEntity);


    // Paski postępu dla danych
    QProgressBar *accXBar = nullptr, *accYBar = nullptr, *accZBar = nullptr; ///< Paski dla akcelerometru X, Y, Z
    QProgressBar *gyroXBar = nullptr, *gyroYBar = nullptr, *gyroZBar = nullptr; ///< Paski dla żyroskopu X, Y, Z
    QProgressBar *magXBar = nullptr, *magYBar = nullptr, *magZBar = nullptr; ///< Paski dla magnetometru X, Y, Z

    // Wykresy dla danych
    SensorGraph *accGraph = nullptr; ///< Wykres dla akcelerometru
    SensorGraph *gyroGraph = nullptr; ///< Wykres dla żyroskopu
    SensorGraph *magGraph = nullptr; ///< Wykres dla magnetometru

    // Widget do przełączania widoków (paski/wykresy)
    QStackedWidget *stackedWidget = nullptr; ///< Widget zarządzający widokami danych
    int currentSampleCount = 1000; ///< Aktualna liczba próbek na wykresach

    // Panele wizualizacji
    QWidget* visualizationPanelWidget = nullptr; ///< Główny widget panelu wizualizacji (3D + kompas)
    QWidget* view3DContainerWidget = nullptr; ///< Kontener dla widoku modelu 3D
    QWidget* compassContainerWidget = nullptr; ///< Kontener dla widoku kompasu

    // Transformacje 3D
    Qt3DCore::QTransform *compassTransform = nullptr; ///< Transformacja dla strzałki kompasu
    Qt3DCore::QTransform *boardTransform = nullptr; ///< Transformacja dla modelu 3D płytki
};

#endif // IMUDATAHANDLER_H