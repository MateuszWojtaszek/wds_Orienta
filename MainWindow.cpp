#include "MainWindow.h"
#include "ImuDataHandler.h"
#include "GpsDataHandler.h"
#include "SerialPortHandler.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>     // Potrzebne dla layoutów, jeśli są używane bezpośrednio
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QInputDialog>
#include <QSerialPortInfo> // Potrzebne dla QSerialPortInfo
#include <cmath>           // Dla std::atan2, M_PI

/**
 * @file MainWindow.cpp
 * @brief Implementacja klasy MainWindow.
 */

// --- Stałe ---
//! Ścieżka do pliku z danymi symulacji. UWAGA: Hardkodowana ścieżka jest złą praktyką!
const QString SIMULATION_DATA_FILE_PATH = "/Users/mateuszwojtaszek/projekty/wds_Orienta/simulation_data2.log";
//! Interwał timera symulacji w milisekundach.
const int SIMULATION_TIMER_INTERVAL_MS = 10;

// --- Indeksy danych w wektorze (dla czytelności) ---
// Można by użyć enuma, ale proste const int też poprawia czytelność
const int GYRO_X_IDX = 0;
const int GYRO_Y_IDX = 1;
const int GYRO_Z_IDX = 2;
const int ACC_X_IDX = 3;
const int ACC_Y_IDX = 4;
const int ACC_Z_IDX = 5;
const int MAG_X_IDX = 6;
const int MAG_Y_IDX = 7;
const int MAG_Z_IDX = 8;
const int ROLL_IDX = 9;
const int PITCH_IDX = 10;
const int YAW_IDX = 11;

/**
 * @brief Konstruktor klasy MainWindow.
 * @details Inicjalizuje UI, ładuje dane symulacji, tworzy obiekty handlerów,
 * konfiguruje połączenia sygnał-slot.
 * @param parent Wskaźnik na widżet nadrzędny.
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    simulationTimer(new QTimer(this)), // Inicjalizacja timera
    serialHandler(new SerialPortHandler(this)) // Inicjalizacja handlera portu
{
    setWindowTitle(tr("Sensor Visualizer"));

    // Spróbuj załadować dane symulacji
    if (!loadSimulationData(SIMULATION_DATA_FILE_PATH)) {
        // Ostrzeżenie już jest w loadSimulationData, można dodać QMessageBox jeśli krytyczne
         QMessageBox::warning(this, tr("Simulation Data"), tr("Could not load simulation data from: %1").arg(SIMULATION_DATA_FILE_PATH));
    }

    // Utworzenie widżetów podrzędnych (handlerów)
    imuHandler = new ImuDataHandler(this);
    gpsHandler = new GPSDataHandler(this);

    // Utworzenie i konfiguracja QStackedWidget
    stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(imuHandler); // Indeks 0
    stackedWidget->addWidget(gpsHandler); // Indeks 1

    setCentralWidget(stackedWidget); // Ustawienie stackedWidget jako centralnego widżetu
    stackedWidget->setCurrentWidget(imuHandler); // Domyślnie pokaż widok IMU

    createMenus(); // Utworzenie menu aplikacji

    showFullScreen(); // Pokaż okno na pełnym ekranie

    // Konfiguracja połączeń sygnał-slot
    connect(this, &MainWindow::switchToIMU, this, &MainWindow::showIMUHandler);
    connect(this, &MainWindow::switchToGPS, this, &MainWindow::showGPSHandler);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::updateSimulationData);
    connect(serialHandler, &SerialPortHandler::newDataReceived, this, &MainWindow::handleSerialData);
    // Należy dodać connect dla setManualRotation, jeśli jest używany (np. z jakiegoś suwaka)
    // connect(someSliderWidget, &QSlider::valueChanged, this, [this](int value){ /* ... przelicz na kąt ... */ setManualRotation(yaw, pitch, roll); });
}

/**
 * @brief Destruktor klasy MainWindow.
 * @details Domyślny destruktor, Qt zajmie się usunięciem obiektów potomnych.
 */
MainWindow::~MainWindow() = default;

/**
 * @brief Ładuje dane symulacyjne z podanego pliku.
 * @details Odczytuje plik linia po linii, dzieli wartości oddzielone przecinkami,
 * parsuje je na float i dodaje do wektora `loadedData`.
 * @param filePath Ścieżka do pliku z danymi symulacyjnymi.
 * @return true jeśli ładowanie powiodło się (plik istnieje i został otwarty),
 * false w przeciwnym razie.
 */
bool MainWindow::loadSimulationData(const QString& filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        loadedData.clear(); // Wyczyść stare dane, jeśli są
        while (!in.atEnd()) {
            QString line = in.readLine();
            // Użycie split z nowszym Qt::SkipEmptyParts jest zalecane
            QStringList values = line.split(',', Qt::SkipEmptyParts);
             // Używamy stałej EXPECTED_DATA_SIZE
            if (values.size() == EXPECTED_DATA_SIZE) {
                QVector<float> parsedLine;
                parsedLine.reserve(EXPECTED_DATA_SIZE); // Optymalizacja - rezerwacja miejsca
                bool conversionOk = true;
                for (const QString &val : values) {
                    float fVal = val.toFloat(&conversionOk);
                    if (!conversionOk) {
                         qWarning() << "Failed to convert value to float in line:" << line;
                         parsedLine.clear(); // Odrzuć całą linię jeśli jest błąd konwersji
                         break;
                    }
                    parsedLine.append(fVal);
                }
                if (!parsedLine.isEmpty()) { // Dodaj tylko jeśli konwersja była ok dla całej linii
                   loadedData.append(parsedLine);
                }
            } else if (!line.trimmed().isEmpty()) { // Opcjonalne ostrzeżenie o złym formacie linii
                 qWarning() << "Skipping line with incorrect number of values (" << values.size() << "):" << line;
            }
        }
        file.close();
        qInfo() << "Successfully loaded" << loadedData.size() << "lines of simulation data from" << filePath;
        return true;
    } else {
        qWarning() << "Failed to open simulation data file:" << filePath << "Error:" << file.errorString();
        return false;
    }
}

/**
 * @brief Tworzy i konfiguruje menu aplikacji.
 * @details Dodaje menu "Sensor" (z akcjami IMU, GPS) oraz "Settings"
 * (z podmenu Language, akcją Simulation Mode, Select Port).
 * Łączy akcje menu z odpowiednimi slotami.
 */
void MainWindow::createMenus() {
    QMenuBar *bar = menuBar(); // Pobierz pasek menu

    // --- Menu Sensor ---
    QMenu *sensorMenu = bar->addMenu(tr("Sensor"));
    QAction *imuAction = sensorMenu->addAction(tr("IMU"));
    QAction *gpsAction = sensorMenu->addAction(tr("GPS"));

    connect(imuAction, &QAction::triggered, this, &MainWindow::switchToIMU);
    connect(gpsAction, &QAction::triggered, this, &MainWindow::switchToGPS);

    // --- Menu Settings ---
    QMenu *settingsMenu = bar->addMenu(tr("Settings"));

    // Podmenu Language
    QMenu *languageMenu = settingsMenu->addMenu(tr("Language"));
    QAction *englishAction = languageMenu->addAction(tr("English"));
    QAction *polishAction = languageMenu->addAction(tr("Polish"));
    // TODO: Implement actual language switching using QTranslator

    settingsMenu->addSeparator(); // Separator dla lepszej organizacji

    // Akcja Simulation Mode
    QAction *simulationModeAction = settingsMenu->addAction(tr("Simulation Mode"));
    simulationModeAction->setCheckable(true); // Ustaw akcję jako przełączalną
    simulationModeAction->setChecked(simulationMode); // Ustaw początkowy stan

    // Akcja Select Port
    QAction *selectPortAction = settingsMenu->addAction(tr("Select Port"));

    // Połączenia dla Settings
    connect(englishAction, &QAction::triggered, this, &MainWindow::setEnglishLanguage);
    connect(polishAction, &QAction::triggered, this, &MainWindow::setPolishLanguage);
    connect(simulationModeAction, &QAction::triggered, this, &MainWindow::toggleSimulationMode);
    connect(selectPortAction, &QAction::triggered, this, &MainWindow::selectPort);
}

/**
 * @brief Slot ustawiający język angielski (placeholder).
 */
void MainWindow::setEnglishLanguage() {
    // TODO: Implement using QTranslator
    QMessageBox::information(this, tr("Language Change"), tr("Language changed to English (placeholder)."));
}

/**
 * @brief Slot ustawiający język polski (placeholder).
 */
void MainWindow::setPolishLanguage() {
    // TODO: Implement using QTranslator
    QMessageBox::information(this, tr("Language Change"), tr("Language changed to Polish (placeholder)."));
}

/**
 * @brief Slot przełączający tryb symulacji.
 * @details Włącza/wyłącza timer symulacji. Jeśli symulacja jest włączana,
 * zamyka port szeregowy. Resetuje indeks danych symulacji.
 */
void MainWindow::toggleSimulationMode() {
    simulationMode = !simulationMode; // Przełącz stan
    currentDataIndex = 0; // Zresetuj indeks przy każdej zmianie trybu

    if (simulationMode) {
        if (serialConnected) {
            serialHandler->closePort(); // Zamknij port, jeśli był otwarty
            serialConnected = false;
            qInfo() << "Serial port closed due to enabling simulation mode.";
        }
        if (!loadedData.isEmpty()) {
             simulationTimer->start(SIMULATION_TIMER_INTERVAL_MS); // Uruchom timer symulacji
             qInfo() << "Simulation mode enabled. Timer started.";
        } else {
             QMessageBox::warning(this, tr("Simulation Mode"), tr("Simulation mode enabled, but no simulation data loaded."));
             qInfo() << "Simulation mode enabled, but no data loaded.";
             simulationMode = false; // Nie można włączyć bez danych
             // Zaktualizuj stan checkmarka w menu
                QList<QAction*> actions = menuBar()->findChildren<QAction*>();
                for(QAction* action : actions) {
                    if(action->text() == tr("Simulation Mode")) {
                         action->setChecked(false);
                         break;
                    }
                }
        }
    } else {
        simulationTimer->stop(); // Zatrzymaj timer symulacji
        qInfo() << "Simulation mode disabled. Timer stopped.";
        // Nie otwieramy automatycznie portu - użytkownik musi go wybrać ręcznie
    }

    // Wyświetl informację użytkownikowi (opcjonalnie)
    // QMessageBox::information(this, tr("Simulation Mode"),
    //                           simulationMode ? tr("Simulation mode enabled.") : tr("Simulation mode disabled."));
}

/**
 * @brief Slot do wyboru i otwarcia portu szeregowego.
 * @details Wyświetla listę dostępnych portów, pozwala użytkownikowi wybrać jeden.
 * Jeśli wybór jest poprawny, próbuje otworzyć port. Jeśli się powiedzie,
 * zatrzymuje symulację i ustawia flagę `serialConnected`.
 */
void MainWindow::selectPort() {
    QStringList ports;
    const auto availablePorts = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : availablePorts) {
        ports << info.portName();
    }

    if (ports.isEmpty()) {
        QMessageBox::warning(this, tr("No Ports"), tr("No serial ports available. Check connections and permissions."));
        return;
    }

    bool ok;
    QString port = QInputDialog::getItem(this, tr("Select Port"), tr("Available Ports:"), ports, 0, false, &ok);

    if (ok && !port.isEmpty()) {
        selectedPort = port;
        // Zamknij poprzedni port, jeśli był otwarty
        if (serialConnected) {
            serialHandler->closePort();
            serialConnected = false;
        }
        // Zatrzymaj symulację, jeśli działała
        if (simulationMode) {
             simulationTimer->stop();
             simulationMode = false;
             qInfo() << "Simulation mode disabled due to serial port selection.";
             // Zaktualizuj stan checkmarka w menu
             QList<QAction*> actions = menuBar()->findChildren<QAction*>();
             for(QAction* action : actions) {
                 if(action->text() == tr("Simulation Mode")) {
                     action->setChecked(false);
                     break;
                 }
             }
        }

        // Spróbuj otworzyć nowy port
        if (serialHandler->openPort(selectedPort)) {
            serialConnected = true;
            QMessageBox::information(this, tr("Connected"), tr("Successfully connected to %1").arg(port));
            qInfo() << "Connected to serial port:" << selectedPort;
        } else {
            QMessageBox::critical(this, tr("Connection Error"), tr("Failed to open port %1. Reason: %2").arg(port).arg(serialHandler->getLastError()));
            qWarning() << "Failed to open serial port:" << selectedPort;
            serialConnected = false; // Upewnij się, że flaga jest false
        }
    }
}

/**
 * @brief Slot timera symulacji - przetwarza kolejną linię danych.
 * @details Pobiera kolejną linię z `loadedData`, przetwarza ją za pomocą
 * `processImuData`. Zatrzymuje timer po przetworzeniu wszystkich danych.
 */
void MainWindow::updateSimulationData() {
    // Sprawdź, czy nie jesteśmy w trybie połączenia szeregowego
    if (serialConnected || !simulationMode) {
        simulationTimer->stop(); // Na wszelki wypadek zatrzymaj timer, jeśli coś jest nie tak ze stanem
        return;
    }

    if (loadedData.isEmpty()) {
        qWarning() << "Simulation timer fired, but no simulation data is loaded.";
        simulationTimer->stop();
        return;
    }

    if (currentDataIndex >= loadedData.size()) {
        simulationTimer->stop();
        QMessageBox::information(this, tr("Simulation Ended"), tr("End of simulation data reached."));
        qInfo() << "Simulation data playback finished.";
        simulationMode = false; // Zakończ tryb symulacji
        // Zaktualizuj stan checkmarka w menu
        QList<QAction*> actions = menuBar()->findChildren<QAction*>();
        for(QAction* action : actions) {
            if(action->text() == tr("Simulation Mode")) {
                 action->setChecked(false);
                 break;
            }
        }
        return;
    }

    // Pobierz i przetwórz dane
    const QVector<float> &data = loadedData[currentDataIndex++];
    processImuData(data);
}

/**
 * @brief Slot odbierający dane z portu szeregowego.
 * @details Przetwarza otrzymany wektor danych za pomocą `processImuData`.
 * @param data Wektor float z danymi z portu szeregowego.
 */
void MainWindow::handleSerialData(QVector<float> data) {
    // Sprawdź, czy jesteśmy w trybie połączenia szeregowego
    if (!serialConnected || simulationMode) {
        return; // Ignoruj dane, jeśli nie oczekujemy ich z portu
    }
    processImuData(data);
}

/**
 * @brief Przetwarza surowy wektor danych IMU.
 * @details Sprawdza rozmiar wektora, ekstrahuje wartości acc, gyro, mag, roll, pitch, yaw,
 * konwertuje na odpowiednie typy (int dla raw data, float dla kątów),
 * oblicza kurs kompasu i aktualizuje `ImuDataHandler`.
 * @param data Wektor float zawierający dane (oczekiwany rozmiar: EXPECTED_DATA_SIZE).
 */
void MainWindow::processImuData(const QVector<float>& data) {
    // Podstawowa walidacja rozmiaru danych
    if (data.size() != EXPECTED_DATA_SIZE) {
        qWarning() << "Received data packet with incorrect size:" << data.size() << "Expected:" << EXPECTED_DATA_SIZE;
        return;
    }

    // Ekstrakcja i konwersja danych
    // Używamy stałych indeksów dla czytelności
    QVector<int> gyro = { static_cast<int>(data[GYRO_X_IDX]), static_cast<int>(data[GYRO_Y_IDX]), static_cast<int>(data[GYRO_Z_IDX]) };
    QVector<int> acc  = { static_cast<int>(data[ACC_X_IDX]), static_cast<int>(data[ACC_Y_IDX]), static_cast<int>(data[ACC_Z_IDX]) };
    QVector<int> mag  = { static_cast<int>(data[MAG_X_IDX]), static_cast<int>(data[MAG_Y_IDX]), static_cast<int>(data[MAG_Z_IDX]) };

    // Kąty są już jako float
    float roll = data[ROLL_IDX];
    float pitch = data[PITCH_IDX];
    float yaw = data[YAW_IDX];

    // Aktualizacja handlera IMU
    if (imuHandler) {
        imuHandler->updateData(acc, gyro, mag); // Przekaż surowe dane
        imuHandler->setRotation(yaw, pitch, roll); // Przekaż obliczone kąty

        // Obliczenie i aktualizacja kursu kompasu (jeśli potrzebne w ImuHandler)
        // Uwaga: Kurs z samego magnetometru jest wrażliwy na przechyły!
        // Lepsze wyniki daje fuzja sensorów (np. filtr Madgwicka/Mahony'ego),
        // która prawdopodobnie oblicza yaw, pitch, roll. Sprawdź, czy setRotation
        // nie ustawia już orientacji w 3D, z której można wyznaczyć kurs.
        // Poniższy kod oblicza kurs tylko w płaszczyźnie XY magnetometru.

        // ***** POPRAWIONY KOD *****
        // Używamy indeksów 0 (dla X) i 1 (dla Y) dla lokalnego wektora 'mag'
        if (mag.size() >= 2 && (mag[0] != 0 || mag[1] != 0)) { // Dodatkowe sprawdzenie rozmiaru dla pewności
            // Używamy osi X (indeks 0) i Y (indeks 1) magnetometru z wektora 'mag'
            float heading = std::atan2(static_cast<float>(mag[1]), static_cast<float>(mag[0])) * 180.0f / M_PI;
            // Normalizacja do zakresu 0-360 stopni
            if (heading < 0) {
                heading += 360.0f;
            }
            imuHandler->updateCompass(heading); // Aktualizuj kompas w handlerze
        } else {
            // Jeśli mag[0] i mag[1] są oba 0, lub jeśli wektor 'mag' jest za krótki (co nie powinno się zdarzyć przy obecnej logice)
            imuHandler->updateCompass(0.0f); // Lub wartość wskazującą błąd/brak danych
        }

    } else {
        qWarning() << "processImuData called but imuHandler is null!";
    }

     // Aktualizacja handlera GPS - dane IMU nie są dla niego, chyba że do fuzji?
     // Tutaj nie ma danych GPS, więc nie wywołujemy gpsHandler->update...
}


/**
 * @brief Slot przełączający widok na ImuDataHandler.
 * @details Ustawia `imuHandler` jako bieżący widżet w `stackedWidget`.
 */
void MainWindow::showIMUHandler() {
    if (stackedWidget && imuHandler) {
        stackedWidget->setCurrentWidget(imuHandler);
        qDebug() << "Switched view to IMU Handler";
    }
}

/**
 * @brief Slot przełączający widok na GPSDataHandler.
 * @details Ustawia `gpsHandler` jako bieżący widżet w `stackedWidget`.
 * @note Obecnie ustawia też statyczny marker w Warszawie przy każdym przełączeniu.
 * W realnej aplikacji dane GPS powinny przychodzić dynamicznie.
 */
void MainWindow::showGPSHandler() {
    if (stackedWidget && gpsHandler) {
        // TODO: Aktualizacja markera powinna pochodzić z danych GPS, nie być statyczna.
        // Ta linia powoduje ustawienie markera na W-wa za każdym razem, gdy POKAŻESZ widok GPS.
        // gpsHandler->updateMarker(52.2297, 21.0122); // Warszawa - przykład
        stackedWidget->setCurrentWidget(gpsHandler);
        qDebug() << "Switched view to GPS Handler";
    }
}

/**
 * @brief Slot do ustawiania ręcznej rotacji (nieużywany obecnie w MainWindow).
 * @details Zapisuje podane kąty Eulera w składowej `manualRotationEuler`.
 * @param yaw Kąt odchylenia (Yaw).
 * @param pitch Kąt pochylenia (Pitch).
 * @param roll Kąt przechylenia (Roll).
 */
void MainWindow::setManualRotation(float yaw, float pitch, float roll) {
    // Ten slot i zmienna manualRotationEuler nie są aktywnie używane w MainWindow.
    // Zakładam, że mogą być potrzebne dla ImuDataHandler do jakiegoś trybu manualnego.
    manualRotationEuler = QVector3D(yaw, pitch, roll);
    qDebug() << "Manual rotation set (Yaw, Pitch, Roll):" << yaw << pitch << roll;
    // Jeśli ImuDataHandler ma używać tej wartości, potrzebny byłby mechanizm
    // przekazania jej, np. wywołanie metody w imuHandler:
    // if(imuHandler) { imuHandler->applyManualRotation(manualRotationEuler); }
}