/**
 * @file MainWindow.h
 * @brief Deklaracja klasy MainWindow, głównego okna aplikacji do wizualizacji danych sensorycznych.
 * @details Plik ten zawiera definicję klasy MainWindow, która stanowi rdzeń interfejsu
 * użytkownika aplikacji. Odpowiada za zarządzanie widokami, obsługę trybów
 * pracy (symulacja, odczyt na żywo), inicjalizację komponentów obsługujących
 * dane z sensorów oraz komunikację.
 * @author Mateusz Wojtaszek
 * @date 2025-05-19
 * @version 1.0
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
 * @details MainWindow integruje różne komponenty aplikacji, takie jak:
 * - Wyświetlanie danych z IMU i GPS w dedykowanych widokach.
 * - Obsługę trybu symulacji, polegającego na odczycie danych z predefiniowanego pliku.
 * - Obsługę trybu pracy na żywo, z danymi odbieranymi przez port szeregowy.
 * - Możliwość zmiany języka interfejsu użytkownika.
 * - Zarządzanie obiektami `ImuDataHandler`, `GPSDataHandler` oraz `SerialPortHandler`.
 * @note Ścieżki do plików konfiguracyjnych (np. plik symulacji, pliki tłumaczeń)
 * są obecnie zdefiniowane w pliku implementacji (`.cpp`).
 * @example MainWindowUsage_PL.cpp
 * Poniżej znajduje się przykład podstawowego użycia (inicjalizacji) klasy MainWindow w funkcji main:
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy MainWindow.
     * @details Inicjalizuje główne okno, tworzy interfejs użytkownika (menu, widżety),
     * ładuje dane symulacyjne (jeśli dostępne) oraz konfiguruje połączenia sygnałów i slotów.
     * @param parent [in] Wskaźnik na widget nadrzędny, domyślnie nullptr.
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy MainWindow.
     * @details Zwalnia zasoby, w tym odinstalowuje translator języka, jeśli był załadowany.
     */
    ~MainWindow() override;

signals:
    /**
     * @brief Sygnał emitowany w celu przełączenia widoku na interfejs obsługi IMU.
     */
    void switchToIMU();

    /**
     * @brief Sygnał emitowany w celu przełączenia widoku na interfejs obsługi GPS.
     */
    void switchToGPS();

private slots:
    /**
     * @brief Ustawia język interfejsu aplikacji na angielski.
     * @details Usuwa aktualnie zainstalowany translator (jeśli istnieje) i odświeża
     * wszystkie teksty w interfejsie użytkownika.
     */
    void setEnglishLanguage();

    /**
     * @brief Ustawia język interfejsu aplikacji na polski.
     * @details Ładuje plik tłumaczeń dla języka polskiego, instaluje translator
     * i odświeża wszystkie teksty w interfejsie użytkownika.
     * @note Wymaga poprawnego pliku `.qm` z tłumaczeniami.
     */
    void setPolishLanguage();

    /**
     * @brief Przełącza tryb pracy aplikacji między symulacją a trybem na żywo.
     * @details Jeśli włączany jest tryb symulacji, rozpoczyna odtwarzanie danych z pliku.
     * Jeśli wyłączany, zatrzymuje symulację. Resetuje również indeks danych symulacyjnych.
     * Jeśli tryb symulacji jest włączany, a port szeregowy był aktywny, port jest zamykany.
     */
    void toggleSimulationMode();

    /**
     * @brief Otwiera dialog wyboru portu szeregowego i próbuje nawiązać połączenie.
     * @details Prezentuje użytkownikowi listę dostępnych portów szeregowych.
     * Po wyborze portu, jeśli aplikacja była w trybie symulacji, tryb ten jest wyłączany.
     * Następnie podejmowana jest próba otwarcia wybranego portu.
     */
    void selectPort();

    /**
     * @brief Wyświetla interfejs użytkownika do obsługi danych IMU.
     * @details Zmienia aktywny widżet w `QStackedWidget` na `ImuDataHandler`.
     */
    void showIMUHandler();

    /**
     * @brief Wyświetla interfejs użytkownika do obsługi danych GPS.
     * @details Zmienia aktywny widżet w `QStackedWidget` na `GPSDataHandler`.
     */
    void showGPSHandler();

    /**
     * @brief Aktualizuje dane w trybie symulacji.
     * @details Wywoływana przez `simulationTimer`. Odczytuje kolejną ramkę danych
     * z załadowanego pliku symulacyjnego, przetwarza ją i aktualizuje markery GPS.
     * Kończy symulację po przetworzeniu wszystkich danych.
     */
    void updateSimulationData();

    /**
     * @brief Przetwarza dane odebrane z portu szeregowego.
     * @details Wywoływana po otrzymaniu sygnału `newDataReceived` od `SerialPortHandler`.
     * Przekazuje odebrane dane do przetworzenia, o ile aplikacja nie jest w trybie symulacji.
     * @param dataFromSerial [in] Wektor sparsowanych wartości zmiennoprzecinkowych z portu szeregowego.
     */
    void handleSerialData(const QVector<float>& dataFromSerial);

private:
    /**
     * @brief Tworzy i konfiguruje paski menu aplikacji.
     * @details Inicjalizuje menu "Sensor", "Settings" (z podmenu "Language") oraz
     * akcje związane z przełączaniem widoków, trybem symulacji i wyborem portu.
     */
    void createMenus();

    /**
     * @brief Ponownie tłumaczy wszystkie elementy interfejsu użytkownika aplikacji.
     * @details Metoda ta jest wywoływana po zmianie języka. Odświeża tytuł okna,
     * teksty w menu oraz wywołuje metody `retranslateUi` dla podrzędnych handlerów danych.
     */
    void retranslateApplicationUi();

    /**
     * @brief Ładuje dane symulacyjne z podanego pliku.
     * @details Odczytuje dane z pliku tekstowego, gdzie każda linia reprezentuje ramkę danych
     * w formacie CSV. Parsuje wartości i przechowuje je wewnętrznie.
     * @param pathToSimulationFile [in] Ścieżka do pliku z danymi symulacyjnymi.
     * @return `true` jeśli dane zostały pomyślnie załadowane, `false` w przeciwnym wypadku.
     */
    bool loadSimulationData(const QString& pathToSimulationFile);

    /**
     * @brief Przetwarza pojedynczą ramkę danych IMU (akcelerometr, żyroskop, magnetometr, orientacja).
     * @details Rozdziela zagregowane dane na poszczególne komponenty i przekazuje je
     * do `ImuDataHandler` w celu wizualizacji.
     * @param acquiredData [in] Wektor zawierający kompletną ramkę danych IMU.
     */
    void processImuData(const QVector<float>& acquiredData);

    /**
     * @brief Obsługuje próbę nawiązania połączenia z wybranym portem szeregowym.
     * @details Zamyka ewentualnie istniejące połączenie lub tryb symulacji,
     * a następnie próbuje otworzyć nowy port. Informuje użytkownika o wyniku operacji.
     * @param portName [in] Nazwa portu szeregowego do połączenia.
     */
    void handlePortConnectionAttempt(const QString& portName);

    /**
     * @brief Sprawdza, czy dane symulacyjne dobiegły końca i aktualizuje stan aplikacji.
     * @details Jeśli wszystkie dane zostały odtworzone, zatrzymuje timer symulacji,
     * informuje użytkownika i wyłącza tryb symulacji.
     * @return `true` jeśli symulacja dobiegła końca, `false` w przeciwnym wypadku.
     */
    bool checkSimulationEndAndUpdateState();

    /**
     * @brief Aktualizuje pozycję markera GPS w trybie symulacji.
     * @details Generuje oscylującą pozycję GPS na podstawie bieżącego indeksu danych
     * symulacyjnych i przekazuje ją do `GPSDataHandler`.
     */
    void updateSimulatedGPSMarker();

    QTranslator* m_translator; ///< Wskaźnik na obiekt translatora języka Qt.

    QStackedWidget *m_stackedWidget; ///< Widżet do przełączania między widokami IMU i GPS.
    ImuDataHandler *m_imuHandler;    ///< Handler i wizualizator danych IMU.
    GPSDataHandler *m_gpsHandler;    ///< Handler i wizualizator danych GPS.
    SerialPortHandler *m_serialHandler; ///< Handler do komunikacji przez port szeregowy.
    QTimer *m_simulationTimer;       ///< Timer do odtwarzania danych w trybie symulacji.

    QVector<QVector<float>> m_loadedData; ///< Przechowuje dane załadowane z pliku symulacyjnego.
    int m_currentDataIndex;              ///< Indeks bieżącej ramki danych w trybie symulacji.

    bool m_simulationMode;  ///< Flaga wskazująca, czy aplikacja jest w trybie symulacji.
    bool m_serialConnected; ///< Flaga wskazująca, czy port szeregowy jest połączony.
    QString m_selectedPort; ///< Nazwa ostatnio wybranego lub połączonego portu szeregowego.
};

#endif // MAINWINDOW_H