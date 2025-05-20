/**
 * @file MainWindow.h
 * @brief Deklaracja klasy MainWindow, głównego okna aplikacji do wizualizacji danych sensorycznych.
 * @details Plik ten zawiera definicję klasy MainWindow, która stanowi rdzeń interfejsu
 * użytkownika aplikacji. Odpowiada za zarządzanie widokami, obsługę trybów
 * pracy (symulacja, odczyt na żywo z IMU i GPS), inicjalizację komponentów obsługujących
 * dane z sensorów oraz komunikację.
 * @author Mateusz Wojtaszek
 * - 2025-05-20 - Wersja 1.1: Drobne aktualizacje w związku z obsługą GPS z portu.
 * @bug Brak znanych błędów.
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// Deklaracje wyprzedzające dla klas Qt
class QStackedWidget;
class QTimer;
class QTranslator;
class QAction;

// Deklaracje wyprzedzające dla klas projektu
class ImuDataHandler;
class GPSDataHandler;
class SerialPortHandler;

/**
 * @class MainWindow
 * @brief Główne okno aplikacji, zarządzające interfejsem i logiką wizualizatora danych sensorycznych.
 * @author Mateusz Wojtaszek // Zakładając, że jest to nowy kod lub znacząca modyfikacja [cite: 8, 10]
 *
 * @details MainWindow integruje różne komponenty aplikacji, takie jak:
 * - Wyświetlanie danych z IMU i GPS w dedykowanych widokach.
 * - Obsługę trybu symulacji (dane IMU z pliku, GPS generowany).
 * - Obsługę trybu pracy na żywo (dane IMU i GPS odbierane przez port szeregowy).
 * - Możliwość zmiany języka interfejsu użytkownika. [cite: 28]
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

signals:
    void switchToIMU();
    void switchToGPS();

private slots:
    void setEnglishLanguage();
    void setPolishLanguage();
    void toggleSimulationMode();
    void selectPort();
    void showIMUHandler();
    void showGPSHandler();
    void updateSimulationData();
    /**
     * @brief Przetwarza dane odebrane z portu szeregowego (12 wartości IMU + 2 wartości GPS).
     * @author Mateusz Wojtaszek // Zakładając, że jest to nowy kod lub znacząca modyfikacja [cite: 8, 10]
     *
     * @details Wywoływana po otrzymaniu sygnału `newDataReceived` od `SerialPortHandler`.
     * Dzieli dane na część IMU i GPS, przekazując je do odpowiednich handlerów.
     * Działa tylko, gdy aplikacja nie jest w trybie symulacji i port jest połączony. [cite: 28]
     * @param dataFromSerial [in] Wektor 14 sparsowanych wartości (12 IMU + 2 GPS) z portu szeregowego. [cite: 33]
     */
    void handleSerialData(const QVector<float> &dataFromSerial);

private:
    void createMenus();
    void retranslateApplicationUi();
    bool loadSimulationData(const QString &pathToSimulationFile);
    /**
     * @brief Przetwarza pojedynczą ramkę danych IMU (12 wartości).
     * @author Mateusz Wojtaszek // Zakładając, że jest to nowy kod lub znacząca modyfikacja [cite: 8, 10]
     *
     * @details Używana głównie dla danych z pliku symulacyjnego oraz części IMU z danych live. [cite: 28]
     * @param imuData [in] Wektor zawierający 12 wartości danych IMU. [cite: 33]
     */
    void processImuData(const QVector<float> &imuData);
    void handlePortConnectionAttempt(const QString &portName);
    bool checkSimulationEndAndUpdateState();
    void updateSimulatedGPSMarker(); // Dla generowania GPS w trybie symulacji

    QTranslator *m_translator;
    QStackedWidget *m_stackedWidget;
    ImuDataHandler *m_imuHandler;
    GPSDataHandler *m_gpsHandler;
    SerialPortHandler *m_serialHandler;
    QTimer *m_simulationTimer;

    QVector<QVector<float> > m_loadedData; // Dla danych symulacyjnych (12 wartości IMU)
    int m_currentDataIndex;

    bool m_simulationMode;
    bool m_serialConnected;
    QString m_selectedPort;
    /**
     * @var EXPECTED_VALUE_COUNT_SERIAL
     * @brief Definiuje oczekiwaną liczbę wartości w ładunku CSV (12 IMU + 2 GPS = 14). [cite: 5, 17]
     */
    const int EXPECTED_VALUE_COUNT_SERIAL = 14;
};

#endif // MAINWINDOW_H