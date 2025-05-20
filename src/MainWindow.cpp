/**
 * @file MainWindow.cpp
 * @brief Implementacja klasy MainWindow, głównego okna aplikacji wizualizatora sensorów.
 * @author Mateusz Wojtaszek
 * @date 2025-05-20 - Wersja 1.1: Drobne aktualizacje, głównie obsługa GPS z portu szeregowego.
 * @bug Brak znanych błędów.
 */

#include "MainWindow.h"
#include "ImuDataHandler.h"
#include "GpsDataHandler.h"
#include "SerialPortHandler.h"

#include <QApplication>
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
#include <QTimer>
#include <QVector>
#include <cmath>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Stałe ścieżek - rozważ użycie QSettings lub zasobów Qt
const QString SIMULATION_DATA_FILE_PATH_MW = "/Users/mateuszwojtaszek/projekty/wds_Orienta/simulation_data3.log";
const QString POLISH_TRANSLATION_FILE_MW = "/Users/mateuszwojtaszek/projekty/wds_Orienta/translations/wds_OrientaPL.qm";

constexpr int SIMULATION_TIMER_INTERVAL_MS_MW = 10; // ms

// Oczekiwana liczba wartości w pliku symulacyjnym (tylko IMU)
constexpr int EXPECTED_DATA_SIZE_SIM_FILE_MW = 12;

// Indeksy dla danych IMU (wspólne dla pliku symulacyjnego i części IMU z portu)
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

// Indeksy dla danych GPS w pełnej ramce 14-elementowej z portu szeregowego
constexpr int GPS_LAT_IDX_SERIAL_MW = 12;
constexpr int GPS_LON_IDX_SERIAL_MW = 13;

// Stałe dla symulacji GPS (gdy dane nie pochodzą z pliku/portu)
constexpr double BASE_LATITUDE_MW = 51.1079;  // Wrocław
constexpr double BASE_LONGITUDE_MW = 17.0595; // Wrocław
constexpr double GPS_OSCILLATION_AMPLITUDE_MW = 0.0001;
constexpr double GPS_OSCILLATION_SPEED_FACTOR_MW = 0.05;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          m_translator(nullptr),
                                          m_stackedWidget(new QStackedWidget(this)),
                                          m_imuHandler(new ImuDataHandler(this)),
                                          m_gpsHandler(new GPSDataHandler(this)),
                                          m_serialHandler(new SerialPortHandler(this)),
                                          m_simulationTimer(new QTimer(this)),
                                          m_currentDataIndex(0),
                                          m_simulationMode(false),
                                          m_serialConnected(false) {
    setWindowTitle(tr("Sensor Visualizer"));

    if (!loadSimulationData(SIMULATION_DATA_FILE_PATH_MW)) {
        QMessageBox::warning(this, tr("Simulation Data"),
                             tr("Could not load simulation data from: %1. Simulation mode may not work correctly.").arg(
                                 SIMULATION_DATA_FILE_PATH_MW));
    }

    m_stackedWidget->addWidget(m_imuHandler);
    m_stackedWidget->addWidget(m_gpsHandler);
    setCentralWidget(m_stackedWidget);
    m_stackedWidget->setCurrentWidget(m_imuHandler);

    createMenus();
    // showFullScreen(); // Odkomentuj, jeśli potrzebne

    connect(this, &MainWindow::switchToIMU, this, &MainWindow::showIMUHandler);
    connect(this, &MainWindow::switchToGPS, this, &MainWindow::showGPSHandler);
    connect(m_simulationTimer, &QTimer::timeout, this, &MainWindow::updateSimulationData);
    connect(m_serialHandler, &SerialPortHandler::newDataReceived, this, &MainWindow::handleSerialData);
}

MainWindow::~MainWindow() {
    if (m_translator) {
        qApp->removeTranslator(m_translator);
        // m_translator jest dzieckiem MainWindow, więc Qt go usunie
    }
}

void MainWindow::createMenus() {
    QMenuBar *menuBarPtr = menuBar();
    menuBarPtr->clear();

    QMenu *sensorMenu = menuBarPtr->addMenu(tr("Sensor"));
    QAction *imuAction = sensorMenu->addAction(tr("IMU View"));
    QAction *gpsAction = sensorMenu->addAction(tr("GPS View"));
    connect(imuAction, &QAction::triggered, this, &MainWindow::showIMUHandler);
    connect(gpsAction, &QAction::triggered, this, &MainWindow::showGPSHandler);

    QMenu *settingsMenu = menuBarPtr->addMenu(tr("Settings"));
    QMenu *languageMenu = settingsMenu->addMenu(tr("Language"));
    QAction *englishAction = languageMenu->addAction(tr("English"));
    QAction *polishAction = languageMenu->addAction(tr("Polish"));

    settingsMenu->addSeparator();
    QAction *simulationModeAction = settingsMenu->addAction(tr("Simulation Mode"));
    simulationModeAction->setCheckable(true);
    simulationModeAction->setChecked(m_simulationMode);
    simulationModeAction->setObjectName("simulationModeAction");

    QAction *selectPortAction = settingsMenu->addAction(tr("Select Serial Port"));

    connect(englishAction, &QAction::triggered, this, &MainWindow::setEnglishLanguage);
    connect(polishAction, &QAction::triggered, this, &MainWindow::setPolishLanguage);
    connect(simulationModeAction, &QAction::triggered, this, &MainWindow::toggleSimulationMode);
    connect(selectPortAction, &QAction::triggered, this, &MainWindow::selectPort);
}

void MainWindow::setEnglishLanguage() {
    if (m_translator) {
        qApp->removeTranslator(m_translator);
        m_translator->deleteLater();
        m_translator = nullptr;
    }
    retranslateApplicationUi();
    QMessageBox::information(this, tr("Language Change"), tr("Language successfully changed to English."));
}

void MainWindow::setPolishLanguage() {
    if (m_translator) {
        qApp->removeTranslator(m_translator);
        m_translator->deleteLater();
        m_translator = nullptr;
    }
    QTranslator *newTranslator = new QTranslator(this);
    if (newTranslator->load(POLISH_TRANSLATION_FILE_MW)) {
        qApp->installTranslator(newTranslator);
        m_translator = newTranslator;
        retranslateApplicationUi();
        QMessageBox::information(this, tr("Language Change"), tr("Language successfully changed to Polish."));
    } else {
        QMessageBox::warning(this, tr("Language Change Error"),
                             tr("Failed to load Polish translation from: %1. Please check file path and integrity.").
                             arg(POLISH_TRANSLATION_FILE_MW));
        delete newTranslator;
    }
}

void MainWindow::retranslateApplicationUi() {
    setWindowTitle(tr("Sensor Visualizer"));
    createMenus(); // Odtwarza menu z nowymi tłumaczeniami

    if (m_imuHandler) {
        m_imuHandler->retranslateUi();
    }
    // if (m_gpsHandler) m_gpsHandler->retranslateUi(); // Jeśli GPSHandler będzie miał taką metodę

    QList<QAction *> actions = menuBar()->findChildren<QAction *>("simulationModeAction");
    if (!actions.isEmpty()) {
        actions.first()->setChecked(m_simulationMode);
    }
}

bool MainWindow::loadSimulationData(const QString &pathToSimulationFile) {
    QFile file(pathToSimulationFile);
    if (!file.exists()) {
        qWarning() << "Simulation data file does not exist at path:" << pathToSimulationFile;
        return false;
    }
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        m_loadedData.clear();
        m_currentDataIndex = 0;
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith('#')) continue;

            QStringList values = line.split(',', Qt::SkipEmptyParts);
            if (values.size() == EXPECTED_DATA_SIZE_SIM_FILE_MW) { // Oczekuje 12 wartości
                QVector<float> dataFrame;
                dataFrame.reserve(EXPECTED_DATA_SIZE_SIM_FILE_MW);
                bool conversionOk = true;
                for (const QString &valStr: values) {
                    bool okFlag;
                    float floatVal = valStr.trimmed().toFloat(&okFlag);
                    if (!okFlag) {
                        qWarning() << "Conversion to float failed for value '" << valStr << "' in line:" << line;
                        conversionOk = false;
                        break;
                    }
                    dataFrame.append(floatVal);
                }
                if (conversionOk) {
                    m_loadedData.append(dataFrame);
                }
            } else {
                qWarning() << "Skipping line due to incorrect number of values. Expected:" << EXPECTED_DATA_SIZE_SIM_FILE_MW <<
                        "Got:" << values.size() << "Line:" << line;
            }
        }
        file.close();
        qInfo() << "Successfully loaded" << m_loadedData.size() << "data frames from" << pathToSimulationFile;
        return !m_loadedData.isEmpty();
    } else {
        qWarning() << "Failed to open simulation data file:" << pathToSimulationFile << "Error:" << file.errorString();
        return false;
    }
}

// Przetwarza tylko 12 wartości IMU
void MainWindow::processImuData(const QVector<float> &imuData) {
    if (imuData.size() != EXPECTED_DATA_SIZE_SIM_FILE_MW) { // Oczekuje 12 wartości
        qWarning() << "processImuData: Received IMU data with incorrect size. Expected:" << EXPECTED_DATA_SIZE_SIM_FILE_MW << "Got:"
                << imuData.size();
        return;
    }
    if (!m_imuHandler) {
        qWarning() << "processImuData: ImuDataHandler is null.";
        return;
    }

    QVector<int> gyro = { static_cast<int>(imuData[GYRO_X_IDX_MW]), static_cast<int>(imuData[GYRO_Y_IDX_MW]), static_cast<int>(imuData[GYRO_Z_IDX_MW]) };
    QVector<int> acc = { static_cast<int>(imuData[ACC_X_IDX_MW]), static_cast<int>(imuData[ACC_Y_IDX_MW]), static_cast<int>(imuData[ACC_Z_IDX_MW]) };
    QVector<int> mag = { static_cast<int>(imuData[MAG_X_IDX_MW]), static_cast<int>(imuData[MAG_Y_IDX_MW]), static_cast<int>(imuData[MAG_Z_IDX_MW]) };
    float roll = imuData[ROLL_IDX_MW];
    float pitch = imuData[PITCH_IDX_MW];
    float yaw = imuData[YAW_IDX_MW];

    m_imuHandler->updateData(acc, gyro, mag);
    m_imuHandler->setRotation(yaw, pitch, roll);

    if (std::abs(imuData[MAG_X_IDX_MW]) > 1e-6f || std::abs(imuData[MAG_Y_IDX_MW]) > 1e-6f) {
        float heading_rad = std::atan2(imuData[MAG_Y_IDX_MW], imuData[MAG_X_IDX_MW]);
        float heading_deg = heading_rad * 180.0f / static_cast<float>(M_PI);
        if (heading_deg < 0.0f) heading_deg += 360.0f;
        m_imuHandler->updateCompass(heading_deg);
    } else {
        m_imuHandler->updateCompass(0.0f);
    }
}

void MainWindow::handlePortConnectionAttempt(const QString &portName) {
    m_selectedPort = portName;
    if (m_serialConnected) {
        m_serialHandler->closePort();
        m_serialConnected = false;
        qInfo() << "Closed previously connected serial port.";
    }
    if (m_simulationMode) {
        m_simulationTimer->stop();
        m_simulationMode = false;
        qInfo() << "Simulation mode disabled due to serial port selection attempt.";
        QList<QAction *> actions = menuBar()->findChildren<QAction *>("simulationModeAction");
        if (!actions.isEmpty()) actions.first()->setChecked(false);
    }
    if (m_serialHandler->openPort(m_selectedPort)) {
        m_serialConnected = true;
        QMessageBox::information(this, tr("Serial Port Connected"), tr("Successfully connected to port: %1").arg(m_selectedPort));
        qInfo() << "Successfully connected to serial port:" << m_selectedPort;
    } else {
        QMessageBox::critical(this, tr("Serial Port Error"), tr("Failed to open port %1. Reason: %2").arg(m_selectedPort).arg(m_serialHandler->getLastError()));
        qWarning() << "Failed to open serial port:" << m_selectedPort << "Reason:" << m_serialHandler->getLastError();
        m_serialConnected = false;
    }
}

bool MainWindow::checkSimulationEndAndUpdateState() {
    if (m_currentDataIndex >= m_loadedData.size()) {
        m_simulationTimer->stop();
        QMessageBox::information(this, tr("Simulation Ended"), tr("End of simulation data reached. Disabling simulation mode."));
        qInfo() << "End of simulation data reached. Simulation mode disabled.";
        m_simulationMode = false;
        QList<QAction *> actions = menuBar()->findChildren<QAction *>("simulationModeAction");
        if (!actions.isEmpty()) actions.first()->setChecked(false);
        return true;
    }
    return false;
}

// Ta funkcja generuje dane GPS dla trybu symulacji, niezależnie od zawartości pliku
void MainWindow::updateSimulatedGPSMarker() {
    if (m_simulationMode && m_gpsHandler && !m_loadedData.isEmpty() && m_currentDataIndex < m_loadedData.size()) {
        double angleRad = static_cast<double>(m_currentDataIndex) * GPS_OSCILLATION_SPEED_FACTOR_MW;
        double latOffset = GPS_OSCILLATION_AMPLITUDE_MW * std::sin(angleRad);
        double lonOffset = GPS_OSCILLATION_AMPLITUDE_MW * std::cos(angleRad);
        double currentLatitude = BASE_LATITUDE_MW + latOffset;
        double currentLongitude = BASE_LONGITUDE_MW + lonOffset;
        m_gpsHandler->updateMarker(currentLatitude, currentLongitude);
    }
}

void MainWindow::toggleSimulationMode() {
    m_simulationMode = !m_simulationMode;
    m_currentDataIndex = 0;
    QAction *simAction = menuBar()->findChild<QAction *>("simulationModeAction");
    if (m_simulationMode) {
        if (m_serialConnected) {
            m_serialHandler->closePort();
            m_serialConnected = false;
            qInfo() << "Serial port closed due to enabling simulation mode.";
        }
        if (!m_loadedData.isEmpty()) {
            if (m_gpsHandler) m_gpsHandler->updateMarker(BASE_LATITUDE_MW, BASE_LONGITUDE_MW); // Ustaw GPS na start
            m_simulationTimer->start(SIMULATION_TIMER_INTERVAL_MS_MW);
            qInfo() << "Simulation mode enabled. Timer started.";
        } else {
            QMessageBox::warning(this, tr("Simulation Mode Warning"), tr("Simulation mode enabled, but no simulation data is loaded. Please load data first."));
            qInfo() << "Attempted to enable simulation mode, but no data loaded. Simulation mode remains disabled.";
            m_simulationMode = false;
        }
    } else {
        m_simulationTimer->stop();
        qInfo() << "Simulation mode disabled. Timer stopped.";
    }
    if (simAction) simAction->setChecked(m_simulationMode);
}

void MainWindow::selectPort() {
    QStringList portNames;
    const auto availablePortsInfo = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &portInfo: availablePortsInfo) portNames << portInfo.portName();
    if (portNames.isEmpty()) {
        QMessageBox::warning(this, tr("No Serial Ports"), tr("No serial ports were found on this system. Please check your hardware connections and drivers."));
        return;
    }
    bool ok;
    QString selectedPortName = QInputDialog::getItem(this, tr("Select Serial Port"), tr("Available serial ports:"), portNames, 0, false, &ok);
    if (ok && !selectedPortName.isEmpty()) {
        handlePortConnectionAttempt(selectedPortName);
    }
}

void MainWindow::showIMUHandler() {
    if (m_stackedWidget && m_imuHandler) {
        m_stackedWidget->setCurrentWidget(m_imuHandler);
        qDebug() << "View switched to IMU Handler.";
    }
}

void MainWindow::showGPSHandler() {
    if (m_stackedWidget && m_gpsHandler) {
        m_stackedWidget->setCurrentWidget(m_gpsHandler);
        qDebug() << "View switched to GPS Handler.";
    }
}

void MainWindow::updateSimulationData() {
    if (m_serialConnected || !m_simulationMode || m_loadedData.isEmpty()) {
        if (m_simulationTimer->isActive()) {
            m_simulationTimer->stop();
            qDebug() << "Simulation timer stopped due to invalid state (not in sim mode, serial connected, or no data).";
        }
        return;
    }
    if (checkSimulationEndAndUpdateState()) return;

    const QVector<float> &currentFrameData = m_loadedData[m_currentDataIndex]; // Dane IMU z pliku (12 wartości)
    processImuData(currentFrameData);     // Przetwórz dane IMU
    updateSimulatedGPSMarker();           // Generuj i zaktualizuj GPS dla symulacji

    m_currentDataIndex++;
}

void MainWindow::handleSerialData(const QVector<float> &dataFromSerial) {
    if (!m_serialConnected || m_simulationMode) {
        return; // Ignoruj, jeśli nie w trybie live lub symulacja aktywna
    }

    // Oczekujemy 14 wartości z portu szeregowego (12 IMU + 2 GPS)
    if (dataFromSerial.size() == EXPECTED_VALUE_COUNT_SERIAL) { // Użyj stałej z SerialPortHandler
        // Wyodrębnij pierwsze 12 wartości dla IMU
        QVector<float> imuData = dataFromSerial.mid(0, EXPECTED_DATA_SIZE_SIM_FILE_MW); // 12 wartości dla IMU
        processImuData(imuData); // Przetwórz dane IMU

        // Wyodrębnij dane GPS (indeksy 12 i 13 w ramce 14-elementowej)
        float latitude = dataFromSerial[GPS_LAT_IDX_SERIAL_MW];
        float longitude = dataFromSerial[GPS_LON_IDX_SERIAL_MW];

        if (m_gpsHandler) {
            m_gpsHandler->updateMarker(static_cast<double>(latitude), static_cast<double>(longitude));
            // qDebug() << "Live GPS Data Updated:" << latitude << longitude; // Opcjonalny log
        }
    } else {
        qWarning() << "handleSerialData: Received data with incorrect size from serial. Expected:"
                   << EXPECTED_VALUE_COUNT_SERIAL << "Got:" << dataFromSerial.size();
    }
}