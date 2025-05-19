/**
 * @file MainWindow.cpp
 * @brief Implementacja klasy MainWindow.
 */

#include "MainWindow.h"
#include "ImuDataHandler.h"    // Pełne definicje
#include "GpsDataHandler.h"
#include "SerialPortHandler.h"

#include <QApplication> // Dla qApp
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStackedWidget>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSerialPortInfo>
#include <QInputDialog>
#include <QTranslator>
#include <QTimer> // Dla QTimer
#include <QVector> // Dla QVector
#include <cmath> // Dla std::sin, std::cos, M_PI (jeśli używane, M_PI może wymagać _USE_MATH_DEFINES w Windows)

// Definicje stałych, jeśli nie są częścią klasy lub globalnie dostępne
// (Przeniesione z MainWindow.h do .cpp, aby uniknąć problemów z wielokrotną definicją,
//  lub zdefiniowane jako static constexpr w klasie, jeśli to C++17+)
const QString SIMULATION_DATA_FILE_PATH_MW = "/Users/mateuszwojtaszek/projekty/wds_Orienta/simulation_data3.log"; // Zmieniono nazwę, aby uniknąć konfliktu
const QString POLISH_TRANSLATION_FILE_MW = "/Users/mateuszwojtaszek/projekty/wds_Orienta/translations/wds_OrientaPL.qm";

constexpr int SIMULATION_TIMER_INTERVAL_MS_MW = 10;
constexpr int EXPECTED_DATA_SIZE_MW = 12;

constexpr int GYRO_X_IDX_MW = 0;
constexpr int GYRO_Y_IDX_MW = 1;
constexpr int GYRO_Z_IDX_MW = 2;
constexpr int ACC_X_IDX_MW = 3;
constexpr int ACC_Y_IDX_MW = 4;
constexpr int ACC_Z_IDX_MW = 5;
constexpr int MAG_X_IDX_MW = 6;
constexpr int MAG_Y_IDX_MW = 7;
constexpr int MAG_Z_IDX_MW = 8;
constexpr int ROLL_IDX_MW = 9;
constexpr int PITCH_IDX_MW = 10;
constexpr int YAW_IDX_MW = 11;

constexpr double BASE_LATITUDE_MW = 51.1079;
constexpr double BASE_LONGITUDE_MW = 17.0595;
constexpr double GPS_OSCILLATION_AMPLITUDE_MW = 0.0001;
constexpr double GPS_OSCILLATION_SPEED_FACTOR_MW = 0.05;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    translator(nullptr), // Inicjalizuj translator
    stackedWidget(new QStackedWidget(this)), // Twórz z rodzicem
    imuHandler(new ImuDataHandler(this)),    // Twórz z rodzicem
    gpsHandler(new GPSDataHandler(this)),    // Twórz z rodzicem
    serialHandler(new SerialPortHandler(this)), // Twórz z rodzicem
    simulationTimer(new QTimer(this)),       // Twórz z rodzicem
    currentDataIndex(0),
    simulationMode(false),
    serialConnected(false)
    // selectedPort inicjalizowany domyślnie (pusty QString)
{
    setWindowTitle(tr("Sensor Visualizer"));

    if (!loadSimulationData(SIMULATION_DATA_FILE_PATH_MW)) {
         QMessageBox::warning(this, tr("Simulation Data"), tr("Could not load simulation data from: %1").arg(SIMULATION_DATA_FILE_PATH_MW));
    }

    stackedWidget->addWidget(imuHandler);
    stackedWidget->addWidget(gpsHandler);

    setCentralWidget(stackedWidget);
    stackedWidget->setCurrentWidget(imuHandler); // Domyślny widok

    createMenus();

    showFullScreen(); // Jeśli potrzebne

    connect(this, &MainWindow::switchToIMU, this, &MainWindow::showIMUHandler);
    connect(this, &MainWindow::switchToGPS, this, &MainWindow::showGPSHandler);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::updateSimulationData);
    connect(serialHandler, &SerialPortHandler::newDataReceived, this, &MainWindow::handleSerialData);
}

MainWindow::~MainWindow()
{
    // Qt automatycznie usunie obiekty potomne (stackedWidget, imuHandler, gpsHandler,
    // serialHandler, simulationTimer, translator jeśli był dzieckiem 'this').
    // Jeśli translator nie był dzieckiem 'this' (co jest teraz poprawione),
    // musiałby być usunięty ręcznie.
    if (translator && translator->parent() != this) { // Dodatkowe zabezpieczenie, choć teraz powinien być dzieckiem
        qApp->removeTranslator(translator);
        delete translator;
    } else if (translator) { // Jeśli był dzieckiem, wystarczy odinstalować
         qApp->removeTranslator(translator);
    }
    // Nie ma potrzeby usuwać wskaźników na widgety zarządzane przez layouty lub jako dzieci.
}

void MainWindow::createMenus() {
    QMenuBar *bar = menuBar();
    bar->clear();

    QMenu *sensorMenu = bar->addMenu(tr("Sensor"));
    QAction *imuAction = sensorMenu->addAction(tr("IMU"));
    QAction *gpsAction = sensorMenu->addAction(tr("GPS"));
    connect(imuAction, &QAction::triggered, this, &MainWindow::switchToIMU);
    connect(gpsAction, &QAction::triggered, this, &MainWindow::switchToGPS);

    QMenu *settingsMenu = bar->addMenu(tr("Settings"));
    QMenu *languageMenu = settingsMenu->addMenu(tr("Language"));
    QAction *englishAction = languageMenu->addAction(tr("English"));
    QAction *polishAction = languageMenu->addAction(tr("Polish"));
    settingsMenu->addSeparator();
    QAction *simulationModeAction = settingsMenu->addAction(tr("Simulation Mode"));
    simulationModeAction->setCheckable(true);
    simulationModeAction->setChecked(simulationMode);
    simulationModeAction->setObjectName("simulationModeAction"); // Użyj objectName do identyfikacji
    QAction *selectPortAction = settingsMenu->addAction(tr("Select Port"));

    connect(englishAction, &QAction::triggered, this, &MainWindow::setEnglishLanguage);
    connect(polishAction, &QAction::triggered, this, &MainWindow::setPolishLanguage);
    connect(simulationModeAction, &QAction::triggered, this, &MainWindow::toggleSimulationMode);
    connect(selectPortAction, &QAction::triggered, this, &MainWindow::selectPort);
}

void MainWindow::setEnglishLanguage() {
    if (translator) {
        qApp->removeTranslator(translator);
        if (translator->parent() == this) { // Jeśli jest dzieckiem, pozwól Qt go usunąć
            translator->deleteLater();
        } else {
            delete translator; // Jeśli nie, usuń ręcznie
        }
        translator = nullptr;
    }
    retranslateApplicationUi(); // Odśwież UI
    QMessageBox::information(this, tr("Language Change"), tr("Language changed to English."));
}

void MainWindow::setPolishLanguage() {
    if (translator) { // Najpierw usuń stary translator
        qApp->removeTranslator(translator);
        if (translator->parent() == this) {
            translator->deleteLater();
        } else {
            delete translator;
        }
        translator = nullptr;
    }

    QTranslator* newTranslator = new QTranslator(this); // Utwórz jako dziecko MainWindow
    if (newTranslator->load(POLISH_TRANSLATION_FILE_MW)) {
        qApp->installTranslator(newTranslator);
        translator = newTranslator; // Przypisz wskaźnik, jeśli ładowanie się powiodło
        retranslateApplicationUi();
        QMessageBox::information(this, tr("Language Change"), tr("Language changed to Polish."));
    } else {
        QMessageBox::warning(this, tr("Language Change"), tr("Failed to load Polish translation from: %1").arg(POLISH_TRANSLATION_FILE_MW));
        delete newTranslator; // Usuń, jeśli ładowanie się nie powiodło
        // translator pozostaje nullptr, UI będzie używać języka źródłowego lub poprzedniego
    }
}

void MainWindow::retranslateApplicationUi() {
    setWindowTitle(tr("Sensor Visualizer"));
    createMenus(); // Odtworzy menu z nowymi tłumaczeniami

    if (imuHandler) {
        imuHandler->retranslateUi();
    }
    if (gpsHandler) {
        // gpsHandler->retranslateUi(); // Jeśli GPS handler ma taką metodę
    }
    // Przywróć stan zaznaczenia dla akcji "Simulation Mode"
    QList<QAction*> actions = menuBar()->findChildren<QAction*>();
    for(QAction* action : actions) {
        if(action->objectName() == "simulationModeAction") {
            action->setChecked(simulationMode);
            break;
        }
    }
}

bool MainWindow::loadSimulationData(const QString& pathToSimulationFile) { // Zmieniono nazwę parametru
    QFile file(pathToSimulationFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        loadedData.clear();
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList values = line.split(',', Qt::SkipEmptyParts);
            if (values.size() == EXPECTED_DATA_SIZE_MW) {
                QVector<float> dataToParse;
                dataToParse.reserve(EXPECTED_DATA_SIZE_MW);
                bool conversionOk = true;
                for (const QString &val : values) {
                    float fVal = val.toFloat(&conversionOk);
                    if (!conversionOk) {
                         qWarning() << "Failed to convert value to float in line:" << line;
                         dataToParse.clear();
                         break;
                    }
                    dataToParse.append(fVal);
                }
                if (!dataToParse.isEmpty()) {
                   loadedData.append(dataToParse);
                }
            } else if (!line.trimmed().isEmpty()) {
                 qWarning() << "Skipping line with incorrect number of values (" << values.size() << "):" << line;
            }
        }
        file.close();
        qInfo() << "Successfully loaded" << loadedData.size() << "lines of simulation data from" << pathToSimulationFile;
        return true;
    } else {
        qWarning() << "Failed to open simulation data file:" << pathToSimulationFile << "Error:" << file.errorString();
        return false;
    }
}

void MainWindow::processImuData(const QVector<float>& acquiredData) {
    if (acquiredData.size() != EXPECTED_DATA_SIZE_MW) {
        qWarning() << "Received data packet with incorrect size:" << acquiredData.size() << "Expected:" << EXPECTED_DATA_SIZE_MW;
        return;
    }

    QVector<int> gyro = { static_cast<int>(acquiredData[GYRO_X_IDX_MW]), static_cast<int>(acquiredData[GYRO_Y_IDX_MW]), static_cast<int>(acquiredData[GYRO_Z_IDX_MW]) };
    QVector<int> acc  = { static_cast<int>(acquiredData[ACC_X_IDX_MW]), static_cast<int>(acquiredData[ACC_Y_IDX_MW]), static_cast<int>(acquiredData[ACC_Z_IDX_MW]) };
    QVector<int> mag  = { static_cast<int>(acquiredData[MAG_X_IDX_MW]), static_cast<int>(acquiredData[MAG_Y_IDX_MW]), static_cast<int>(acquiredData[MAG_Z_IDX_MW]) };

    float roll = acquiredData[ROLL_IDX_MW];
    float pitch = acquiredData[PITCH_IDX_MW];
    float yaw = acquiredData[YAW_IDX_MW];

    if (imuHandler) {
        imuHandler->updateData(acc, gyro, mag);
        imuHandler->setRotation(yaw, pitch, roll);

        // Użyj std::abs do porównywania floatów z zerem, aby uniknąć problemów z precyzją
        if (std::abs(acquiredData[MAG_X_IDX_MW]) > 1e-6 || std::abs(acquiredData[MAG_Y_IDX_MW]) > 1e-6) {
            float heading_rad = std::atan2(acquiredData[MAG_Y_IDX_MW], acquiredData[MAG_X_IDX_MW]);
            float heading_deg = heading_rad * 180.0f / static_cast<float>(M_PI); // Użyj M_PI z <cmath>
            if (heading_deg < 0.0f) {
                heading_deg += 360.0f;
            }
            imuHandler->updateCompass(heading_deg);
        } else {
            imuHandler->updateCompass(0.0f);
        }
    } else {
        qWarning() << "processImuData called but imuHandler is null!";
    }
}

void MainWindow::handlePortConnectionAttempt(const QString& portName) {
    selectedPort = portName;

    if (serialConnected) {
        serialHandler->closePort();
        serialConnected = false;
    }

    if (simulationMode) {
        simulationTimer->stop();
        simulationMode = false;
        qInfo() << "Simulation mode disabled due to serial port selection.";
        // Odznacz akcję w menu
        QList<QAction*> actions = menuBar()->findChildren<QAction*>();
        for(QAction* action : actions) {
            if(action->objectName() == "simulationModeAction") {
                action->setChecked(false);
                break;
            }
        }
    }

    if (serialHandler->openPort(selectedPort)) {
        serialConnected = true;
        QMessageBox::information(this, tr("Connected"), tr("Successfully connected to %1").arg(portName));
        qInfo() << "Connected to serial port:" << selectedPort;
    } else {
        QMessageBox::critical(this, tr("Connection Error"), tr("Failed to open port %1. Reason: %2").arg(portName).arg(serialHandler->getLastError()));
        qWarning() << "Failed to open serial port:" << selectedPort << "Reason:" << serialHandler->getLastError();
        serialConnected = false;
    }
}

bool MainWindow::checkSimulationEndAndUpdateState() {
    if (currentDataIndex >= loadedData.size()) {
        simulationTimer->stop();
        QMessageBox::information(this, tr("Simulation Ended"), tr("End of simulation data reached."));
        qInfo() << "Simulation data playback finished.";
        simulationMode = false;

        QList<QAction*> actions = menuBar()->findChildren<QAction*>();
        for(QAction* action : actions) {
             if(action->objectName() == "simulationModeAction") {
                 action->setChecked(false);
                 break;
             }
        }
        return true;
    }
    return false;
}

void MainWindow::updateSimulatedGPSMarker() {
    if (simulationMode && gpsHandler) {
        double angleRad = static_cast<double>(currentDataIndex) * GPS_OSCILLATION_SPEED_FACTOR_MW;
        double latOffset = GPS_OSCILLATION_AMPLITUDE_MW * std::sin(angleRad);
        double lonOffset = GPS_OSCILLATION_AMPLITUDE_MW * std::cos(angleRad);

        double currentLatitude = BASE_LATITUDE_MW + latOffset;
        double currentLongitude = BASE_LONGITUDE_MW + lonOffset;

        gpsHandler->updateMarker(currentLatitude, currentLongitude);
    }
}

// --- Pozostałe sloty i metody ---
void MainWindow::toggleSimulationMode() {
    simulationMode = !simulationMode;
    currentDataIndex = 0; // Zresetuj indeks danych przy każdej zmianie trybu

    QAction* simAction = nullptr;
    QList<QAction*> actions = menuBar()->findChildren<QAction*>();
    for(QAction* action : actions) {
        if(action->objectName() == "simulationModeAction") {
            simAction = action;
            break;
        }
    }

    if (simulationMode) {
        if (serialConnected) { // Jeśli port jest połączony, zamknij go
            serialHandler->closePort();
            serialConnected = false;
            qInfo() << "Serial port closed due to enabling simulation mode.";
        }
        if (!loadedData.isEmpty()) {
            if (gpsHandler) { // Zaktualizuj pozycję GPS na startową
                 gpsHandler->updateMarker(BASE_LATITUDE_MW, BASE_LONGITUDE_MW);
            }
            simulationTimer->start(SIMULATION_TIMER_INTERVAL_MS_MW);
            qInfo() << "Simulation mode enabled. Timer started.";
        } else {
            QMessageBox::warning(this, tr("Simulation Mode"), tr("Simulation mode enabled, but no simulation data loaded."));
            qInfo() << "Simulation mode enabled, but no data loaded. Disabling simulation mode.";
            simulationMode = false; // Cofnij zmianę, jeśli nie ma danych
        }
    } else { // Wyłączanie trybu symulacji
        simulationTimer->stop();
        qInfo() << "Simulation mode disabled. Timer stopped.";
    }

    if (simAction) { // Zaktualizuj stan zaznaczenia w menu
        simAction->setChecked(simulationMode);
    }
}

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
        handlePortConnectionAttempt(port);
    }
}

void MainWindow::showIMUHandler() {
    if (stackedWidget && imuHandler) {
        stackedWidget->setCurrentWidget(imuHandler);
        qDebug() << "Switched view to IMU Handler";
    }
}

void MainWindow::showGPSHandler() {
     if (stackedWidget && gpsHandler) {
        stackedWidget->setCurrentWidget(gpsHandler);
        qDebug() << "Switched view to GPS Handler";
    }
}

void MainWindow::updateSimulationData() {
    if (serialConnected || !simulationMode || loadedData.isEmpty()) {
        simulationTimer->stop(); // Zatrzymaj, jeśli nie jesteśmy w trybie symulacji lub nie ma danych
        return;
    }

    if (checkSimulationEndAndUpdateState()) { // Sprawdź, czy symulacja się nie zakończyła
        return;
    }

    const QVector<float> &currentFrameData = loadedData[currentDataIndex];
    processImuData(currentFrameData);
    updateSimulatedGPSMarker();
    currentDataIndex++;
}

void MainWindow::handleSerialData(const QVector<float>& dataFromSerial) {
    if (!serialConnected || simulationMode) { // Przetwarzaj tylko, gdy połączony i nie w symulacji
        return;
    }
    processImuData(dataFromSerial);
}