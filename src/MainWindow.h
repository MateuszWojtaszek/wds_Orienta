/**
 * @file MainWindow.h
 * @brief Deklaracja klasy MainWindow - głównego okna aplikacji wizualizatora sensorów.
 * @author Mateusz Wojtaszek (lub oryginalny autor, jeśli inny)
 * @date 2025-04-29 (lub aktualna data modyfikacji)
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


class QStackedWidget;
class QTimer;
class QTranslator;
class QAction;

class ImuDataHandler;
class GPSDataHandler; // Założenie: GPSDataHandler.h istnieje i jest poprawny
class SerialPortHandler; // Założenie: SerialPortHandler.h istnieje i jest poprawny


/**
 * @brief Ścieżka do pliku z danymi symulacyjnymi.
 * @details Stała przechowująca domyślną ścieżkę do pliku .log, który zawiera dane
 * sensoryczne używane w trybie symulacji.
 */
// const QString SIMULATION_DATA_FILE_PATH = "/Users/mateuszwojtaszek/projekty/wds_Orienta/simulation_data3.log"; // Przeniesiono do .cpp lub jako składowa
// Podobnie inne stałe, jeśli nie są częścią publicznego API klasy

/**
 * @class MainWindow
 * @brief Główne okno aplikacji do wizualizacji danych sensorycznych.
 * @details Klasa MainWindow jest centralnym punktem aplikacji. Zarządza interfejsem użytkownika,
 * przełączaniem widoków między wizualizacją IMU a GPS, obsługą trybu symulacji
 * (odczyt danych z pliku) oraz trybu na żywo (odczyt danych z portu szeregowego).
 * Tworzy i zarządza obiektami odpowiedzialnymi za obsługę poszczególnych
 * sensorów (ImuDataHandler, GPSDataHandler) oraz komunikację (SerialPortHandler).
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy MainWindow.
     * @details Inicjalizuje główne okno aplikacji, tworzy i konfiguruje niezbędne komponenty.
     * @param[in] parent Wskaźnik na widget rodzica, domyślnie nullptr.
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy MainWindow.
     */
    ~MainWindow() override;

signals:
    void switchToIMU();
    void switchToGPS();

private slots:
    /**
     * @brief Ustawia język aplikacji na angielski.
     * @details Usuwa bieżący translator (jeśli istnieje) i odświeża interfejs użytkownika.
     */
    void setEnglishLanguage();

    /**
     * @brief Ustawia język aplikacji na polski.
     * @details Ładuje polski plik tłumaczeń, instaluje translator i odświeża interfejs użytkownika.
     */
    void setPolishLanguage();

    void toggleSimulationMode();
    void selectPort();
    void showIMUHandler();
    void showGPSHandler();
    void updateSimulationData();
    void handleSerialData(const QVector<float>& dataFromSerial);

private:
    void createMenus(); // Prywatna metoda pomocnicza, nie musi być slotem
    /**
     * @brief Ponownie tłumaczy wszystkie elementy interfejsu użytkownika aplikacji.
     * @details Ta metoda powinna być wywoływana po zmianie języka.
     * Odświeża tytuł okna, menu oraz interfejsy podrzędnych handlerów.
     */
    void retranslateApplicationUi();
    bool loadSimulationData(const QString& pathToSimulatioFile);
    void processImuData(const QVector<float>& acquiredData);
    void handlePortConnectionAttempt(const QString& portName);
    bool checkSimulationEndAndUpdateState();
    void updateSimulatedGPSMarker();

    QTranslator* translator;

    QStackedWidget *stackedWidget;
    ImuDataHandler *imuHandler;
    GPSDataHandler *gpsHandler;
    SerialPortHandler *serialHandler;
    QTimer *simulationTimer;

    QVector<QVector<float>> loadedData;
    int currentDataIndex;

    bool simulationMode;
    bool serialConnected;
    QString selectedPort;
};

#endif // MAINWINDOW_H