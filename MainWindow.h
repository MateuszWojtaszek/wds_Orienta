#pragma once

#include <QMainWindow>
#include <QStackedWidget> // Upewnij się, że jest dołączone
#include <QTimer>        // Upewnij się, że jest dołączone
#include <QVector3D>     // Upewnij się, że jest dołączone
#include <QString>       // Dla QString
#include <QVector>       // Dla QVector

// Forward declarations dla klas obsługujących dane i port
class ImuDataHandler;
class GPSDataHandler;
class SerialPortHandler;

/**
 * @brief Główne okno aplikacji wizualizującej dane sensoryczne.
 * @details Zarządza interfejsem użytkownika, przełączaniem między widokami
 * danych IMU i GPS, obsługą trybu symulacji oraz połączenia
 * przez port szeregowy.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT // Makro wymagane dla klas używających sygnałów i slotów

public:
    /**
     * @brief Konstruktor klasy MainWindow.
     * @param parent Wskaźnik na widżet nadrzędny (opcjonalny).
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy MainWindow.
     */
    ~MainWindow() override;

signals:
    /**
     * @brief Sygnał emitowany, gdy użytkownik chce przełączyć się na widok IMU.
     */
    void switchToIMU();

    /**
     * @brief Sygnał emitowany, gdy użytkownik chce przełączyć się na widok GPS.
     */
    void switchToGPS();

private slots:
    /**
     * @brief Slot ustawiający język angielski (placeholder).
     * @note Obecnie tylko wyświetla komunikat.
     */
    void setEnglishLanguage();

    /**
     * @brief Slot ustawiający język polski (placeholder).
     * @note Obecnie tylko wyświetla komunikat.
     */
    void setPolishLanguage();

    /**
     * @brief Slot przełączający tryb symulacji (włączony/wyłączony).
     * @details Zarządza timerem symulacji i stanem połączenia szeregowego.
     */
    void toggleSimulationMode();

    /**
     * @brief Slot otwierający dialog wyboru portu szeregowego i próbujący nawiązać połączenie.
     */
    void selectPort();

    /**
     * @brief Slot przełączający widok na ImuDataHandler.
     */
    void showIMUHandler();

    /**
     * @brief Slot przełączający widok na GPSDataHandler.
     */
    void showGPSHandler();

    /**
     * @brief Slot wywoływany przez timer symulacji w celu przetworzenia kolejnej linii danych.
     */
    void updateSimulationData();

    /**
     * @brief Slot odbierający i przetwarzający dane otrzymane z portu szeregowego.
     * @param data Wektor float zawierający dane z czujnika.
     */
    void handleSerialData(QVector<float> data);

    /**
     * @brief Slot do ustawiania ręcznej rotacji (nieużywany obecnie w MainWindow).
     * @param yaw Kąt odchylenia (Yaw).
     * @param pitch Kąt pochylenia (Pitch).
     * @param roll Kąt przechylenia (Roll).
     * @note Ten slot ustawia zmienną manualRotationEuler, ale sama zmienna nie jest używana
     * w logice klasy MainWindow. Może być używana przez ImuDataHandler.
     */
    void setManualRotation(float yaw, float pitch, float roll);

private:
    /**
     * @brief Tworzy i konfiguruje menu aplikacji (Sensor, Settings).
     */
    void createMenus();

    /**
     * @brief Ładuje dane symulacyjne z podanego pliku.
     * @param filePath Ścieżka do pliku z danymi symulacyjnymi.
     * @return true jeśli ładowanie powiodło się, false w przeciwnym razie.
     */
    bool loadSimulationData(const QString& filePath);

    /**
     * @brief Przetwarza surowy wektor danych IMU i aktualizuje ImuDataHandler.
     * @param data Wektor float zawierający dane z czujnika (oczekiwany rozmiar: EXPECTED_DATA_SIZE).
     */
    void processImuData(const QVector<float>& data);

    // --- Składowe klasy ---

    //!< Widżet przechowujący i przełączający widoki (IMU/GPS).
    QStackedWidget *stackedWidget;
    //!< Wskaźnik na obiekt obsługujący wizualizację danych IMU.
    ImuDataHandler *imuHandler;
    //!< Wskaźnik na obiekt obsługujący wizualizację danych GPS.
    GPSDataHandler *gpsHandler;
    //!< Wskaźnik na obiekt obsługujący komunikację przez port szeregowy.
    SerialPortHandler *serialHandler;

    //!< Timer używany do odtwarzania danych w trybie symulacji.
    QTimer *simulationTimer;
    //!< Przechowuje dane wczytane z pliku symulacji.
    QVector<QVector<float>> loadedData;
    //!< Indeks bieżącej linii danych w trybie symulacji.
    int currentDataIndex = 0;

    //!< Flaga wskazująca, czy tryb symulacji jest aktywny.
    bool simulationMode = false;
    //!< Flaga wskazująca, czy połączenie przez port szeregowy jest aktywne.
    bool serialConnected = false;
    //!< Przechowuje nazwę wybranego portu szeregowego.
    QString selectedPort;

    //!< Ręcznie ustawione kąty Eulera (Yaw, Pitch, Roll) - obecnie nieużywane w MainWindow.
    QVector3D manualRotationEuler = {0.0f, 0.0f, 0.0f};

    //!< Oczekiwana liczba wartości w pojedynczej linii danych (np. z pliku lub portu).
    static const int EXPECTED_DATA_SIZE = 12;
};