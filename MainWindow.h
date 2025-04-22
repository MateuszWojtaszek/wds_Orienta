#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>
#include <QVector3D>
#include <QString>
#include <QVector>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <QInputDialog>
#include <QSerialPortInfo> // Dodano brakujący include
#include <cmath>           // Dodano brakujący include dla std::atan2, std::sin, std::cos, M_PI

// Forward declarations
class ImuDataHandler;
class GPSDataHandler;
class SerialPortHandler;

#include "ImuDataHandler.h" // Zakładając, że te pliki istnieją
#include "GpsDataHandler.h"   // Zakładając, że te pliki istnieją
#include "SerialPortHandler.h"// Zakładając, że te pliki istnieją


/**
 * @brief Ścieżka do pliku z danymi symulacyjnymi.
 * @details Stała przechowująca domyślną ścieżkę do pliku .log, który zawiera dane
 * sensoryczne używane w trybie symulacji.
 */
const QString SIMULATION_DATA_FILE_PATH = "/Users/mateuszwojtaszek/projekty/wds_Orienta/simulation_data3.log";

/**
 * @brief Interwał timera symulacji w milisekundach.
 * @details Określa, jak często (w ms) timer symulacji będzie wywoływał
 * aktualizację danych w trybie symulacji.
 */
constexpr int SIMULATION_TIMER_INTERVAL_MS = 10;

/**
 * @brief Oczekiwana liczba wartości w pojedynczej linii danych.
 * @details Definiuje, ile oddzielonych przecinkami wartości float powinno znajdować się
 * w każdej linii pliku symulacyjnego lub w pakiecie danych z portu szeregowego.
 */
static constexpr int EXPECTED_DATA_SIZE = 12;

// Indeksy poszczególnych danych w wektorze
constexpr int GYRO_X_IDX = 0; ///< Indeks danych żyroskopu dla osi X.
constexpr int GYRO_Y_IDX = 1; ///< Indeks danych żyroskopu dla osi Y.
constexpr int GYRO_Z_IDX = 2; ///< Indeks danych żyroskopu dla osi Z.
constexpr int ACC_X_IDX = 3;  ///< Indeks danych akcelerometru dla osi X.
constexpr int ACC_Y_IDX = 4;  ///< Indeks danych akcelerometru dla osi Y.
constexpr int ACC_Z_IDX = 5;  ///< Indeks danych akcelerometru dla osi Z.
constexpr int MAG_X_IDX = 6;  ///< Indeks danych magnetometru dla osi X.
constexpr int MAG_Y_IDX = 7;  ///< Indeks danych magnetometru dla osi Y.
constexpr int MAG_Z_IDX = 8;  ///< Indeks danych magnetometru dla osi Z.
constexpr int ROLL_IDX = 9;   ///< Indeks danych kąta przechyłu (Roll).
constexpr int PITCH_IDX = 10; ///< Indeks danych kąta pochylenia (Pitch).
constexpr int YAW_IDX = 11;   ///< Indeks danych kąta odchylenia (Yaw).

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
     * @details Inicjalizuje główne okno aplikacji, tworzy i konfiguruje niezbędne komponenty:
     * widgety interfejsu użytkownika (QStackedWidget), handlery danych
     * (ImuDataHandler, GPSDataHandler), handler portu szeregowego (SerialPortHandler),
     * timer symulacji. Ładuje dane symulacyjne, tworzy menu, ustawia połączenia
     * sygnałów i slotów oraz wyświetla okno na pełnym ekranie.
     * @param parent Wskaźnik na widget rodzica, domyślnie nullptr.
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy MainWindow.
     * @details Domyślny destruktor. Qt zajmuje się zwolnieniem pamięci obiektów potomnych
     * dzięki mechanizmowi rodzic-dziecko.
     */
    ~MainWindow() override;

signals:
    /**
     * @brief Sygnał emitowany w celu przełączenia widoku na wizualizację IMU.
     * @details Ten sygnał jest wywoływany, gdy użytkownik wybierze opcję IMU w menu,
     * informując aplikację o konieczności pokazania widżetu ImuDataHandler.
     */
    void switchToIMU();

    /**
     * @brief Sygnał emitowany w celu przełączenia widoku na wizualizację GPS.
     * @details Ten sygnał jest wywoływany, gdy użytkownik wybierze opcję GPS w menu,
     * informując aplikację o konieczności pokazania widżetu GPSDataHandler.
     */
    void switchToGPS();

private slots:
    /**
     * @brief Slot do ustawienia języka angielskiego (placeholder).
     * @details Wywoływany po wybraniu opcji "English" w menu języka. Aktualnie
     * wyświetla jedynie komunikat informacyjny o zmianie języka.
     * W przyszłości powinien implementować faktyczną zmianę języka interfejsu.
     */
    void setEnglishLanguage();

    /**
     * @brief Slot do ustawienia języka polskiego (placeholder).
     * @details Wywoływany po wybraniu opcji "Polish" w menu języka. Aktualnie
     * wyświetla jedynie komunikat informacyjny o zmianie języka.
     * W przyszłości powinien implementować faktyczną zmianę języka interfejsu.
     */
    void setPolishLanguage();

    /**
     * @brief Przełącza tryb działania aplikacji między symulacją a odczytem z portu szeregowego.
     * @details Aktywowany przez akcję "Simulation Mode" w menu. Jeśli tryb symulacji jest
     * włączany, zamyka port szeregowy (jeśli był otwarty), uruchamia timer symulacji
     * (jeśli dane symulacyjne są załadowane) i aktualizuje stan znacznika GPS.
     * Jeśli tryb symulacji jest wyłączany, zatrzymuje timer. Aktualizuje również
     * stan zaznaczenia odpowiedniej akcji w menu.
     */
    void toggleSimulationMode();

    /**
     * @brief Otwiera okno dialogowe do wyboru portu szeregowego.
     * @details Wyświetla listę dostępnych portów szeregowych w systemie i pozwala
     * użytkownikowi wybrać jeden z nich. Po dokonaniu wyboru, próbuje
     * nawiązać połączenie za pomocą `handlePortConnectionAttempt`.
     */
    void selectPort();

    /**
     * @brief Wyświetla widżet obsługujący dane IMU.
     * @details Slot podłączony do sygnału `switchToIMU` lub akcji menu. Ustawia
     * `imuHandler` jako bieżący widżet w `stackedWidget`.
     */
    void showIMUHandler();

    /**
     * @brief Wyświetla widżet obsługujący dane GPS.
     * @details Slot podłączony do sygnału `switchToGPS` lub akcji menu. Ustawia
     * `gpsHandler` jako bieżący widżet w `stackedWidget`.
     */
    void showGPSHandler();

    /**
     * @brief Aktualizuje dane w trybie symulacji.
     * @details Slot wywoływany cyklicznie przez `simulationTimer`. Sprawdza, czy symulacja
     * powinna być kontynuowana. Jeśli tak, pobiera kolejny zestaw danych
     * z `loadedData`, przetwarza go za pomocą `processImuData`, aktualizuje
     * symulowany znacznik GPS i inkrementuje indeks bieżących danych.
     * Obsługuje również zakończenie symulacji.
     */
    void updateSimulationData();

    /**
     * @brief Obsługuje nowe dane otrzymane z portu szeregowego.
     * @details Slot połączony z sygnałem `newDataReceived` obiektu `SerialPortHandler`.
     * Sprawdza, czy połączenie szeregowe jest aktywne i czy aplikacja nie jest
     * w trybie symulacji. Jeśli warunki są spełnione, przekazuje otrzymane dane
     * do przetworzenia przez funkcję `processImuData`.
     * @param data Wektor `float` zawierający dane sensoryczne odebrane przez port szeregowy.
     */
    void handleSerialData(QVector<float> data);

private:
    /**
     * @brief Tworzy i konfiguruje paski menu aplikacji.
     * @details Inicjalizuje główny pasek menu (`QMenuBar`) oraz dodaje do niego
     * menu "Sensor" (z akcjami IMU, GPS) i "Settings" (z podmenu "Language",
     * akcją "Simulation Mode" i "Select Port"). Łączy akcje menu
     * z odpowiednimi slotami.
     */
    void createMenus();

    /**
     * @brief Ładuje dane symulacyjne z podanego pliku.
     * @details Otwiera plik tekstowy pod wskazaną ścieżką, czyta go linia po linii,
     * dzieli każdą linię na wartości liczbowe (oczekując formatu CSV),
     * konwertuje je na `float` i zapisuje w wektorze `loadedData`.
     * Sprawdza poprawność formatu danych (oczekiwana liczba wartości).
     * @param filePath Ścieżka do pliku z danymi symulacyjnymi.
     * @return `true` jeśli ładowanie danych powiodło się, `false` w przeciwnym razie.
     */
    bool loadSimulationData(const QString& filePath);

    /**
     * @brief Przetwarza surowy wektor danych sensorycznych.
     * @details Funkcja przyjmuje wektor danych (z pliku symulacyjnego lub portu szeregowego),
     * sprawdza jego rozmiar, a następnie ekstrahuje wartości dla akcelerometru,
     * żyroskopu, magnetometru oraz kątów Roll, Pitch, Yaw. Przekazuje te dane
     * do `imuHandler` w celu aktualizacji wizualizacji 3D i wskaźników.
     * Oblicza również kurs (heading) na podstawie danych magnetometru
     * i aktualizuje kompas w `imuHandler`.
     * @param data Wektor `float` zawierający kompletny zestaw danych z sensorów
     * (Gyro X,Y,Z, Acc X,Y,Z, Mag X,Y,Z, Roll, Pitch, Yaw).
     */
    void processImuData(const QVector<float>& data);

    /**
     * @brief Obsługuje próbę nawiązania połączenia z wybranym portem szeregowym.
     * @details Zapisuje nazwę wybranego portu. Jeśli istnieje aktywne połączenie szeregowe,
     * zamyka je. Jeśli aktywny jest tryb symulacji, wyłącza go. Następnie próbuje
     * otworzyć wybrany port za pomocą `serialHandler`. Aktualizuje flagę
     * `serialConnected` i wyświetla odpowiedni komunikat (sukces lub błąd).
     * @param portName Nazwa portu szeregowego do połączenia (np. "COM3", "/dev/ttyACM0").
     */
    void handlePortConnectionAttempt(const QString& portName);

    /**
     * @brief Sprawdza, czy osiągnięto koniec danych symulacyjnych i aktualizuje stan.
     * @details Porównuje bieżący indeks danych (`currentDataIndex`) z rozmiarem
     * załadowanych danych (`loadedData.size()`). Jeśli indeks jest większy lub równy,
     * oznacza to koniec symulacji. W takim przypadku zatrzymuje timer, wyświetla
     * komunikat, wyłącza tryb symulacji (ustawia `simulationMode` na `false`)
     * i odznacza odpowiednią akcję w menu.
     * @return `true` jeśli symulacja dobiegła końca, `false` w przeciwnym razie.
     */
    bool checkSimulationEndAndUpdateState();

    /**
     * @brief Aktualizuje pozycję markera GPS w trybie symulacji.
     * @details Jeśli aplikacja jest w trybie symulacji i `gpsHandler` istnieje,
     * oblicza nową pozycję GPS na podstawie `baseLatitude`, `baseLongitude`,
     * `currentDataIndex` oraz parametrów oscylacji (`gpsOscillationAmplitude`,
     * `gpsOscillationSpeedFactor`), używając funkcji sinus i cosinus do
     * stworzenia ruchu kołowego/eliptycznego. Następnie wywołuje
     * `gpsHandler->updateMarker` z nowymi współrzędnymi.
     */
    void updateSimulatedGPSMarker();

    // Wskaźniki na kluczowe komponenty
    QStackedWidget *stackedWidget; ///< Widget zarządzający przełączaniem widoków (IMU/GPS).
    ImuDataHandler *imuHandler;   ///< Handler i wizualizator danych IMU.
    GPSDataHandler *gpsHandler;   ///< Handler i wizualizator danych GPS.

    SerialPortHandler *serialHandler; ///< Handler do komunikacji przez port szeregowy.
    QTimer *simulationTimer;        ///< Timer używany do cyklicznego odczytu danych w trybie symulacji.
    QVector<QVector<float>> loadedData; ///< Wektor przechowujący dane załadowane z pliku symulacyjnego.
    int currentDataIndex = 0;        ///< Indeks wskazujący na aktualnie przetwarzany wiersz danych symulacyjnych.

    // Flagi stanu i konfiguracja
    bool simulationMode = false;     ///< Flaga wskazująca, czy aplikacja jest w trybie symulacji.
    bool serialConnected = false;    ///< Flaga wskazująca, czy port szeregowy jest połączony.
    QString selectedPort;            ///< Nazwa aktualnie wybranego lub połączonego portu szeregowego.

    // Zmienne używane do symulacji GPS
    QVector3D manualRotationEuler = {0.0f, 0.0f, 0.0f}; ///< (Nie używane w obecnym kodzie) Potencjalnie do ręcznej rotacji modelu.
    double baseLatitude = 51.1079;   ///< Bazowa szerokość geograficzna dla symulacji GPS.
    double baseLongitude = 17.0595;  ///< Bazowa długość geograficzna dla symulacji GPS.
    double gpsOscillationAmplitude = 0.0001; ///< Amplituda oscylacji pozycji GPS w symulacji (w stopniach).
    double gpsOscillationSpeedFactor = 0.05; ///< Współczynnik prędkości oscylacji pozycji GPS w symulacji.
};

#endif // MAINWINDOW_H