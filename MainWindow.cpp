#include "MainWindow.h"
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QDebug>
#include <QStackedWidget>
#include <random> // Include this header for random number generation

#include "ImuDataHandler.h"
#include "GpsDataHandler.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), simulationTimer(new QTimer(this))
{
    setWindowTitle(tr("Main Window"));

    // Utworzenie widgetów
    stackedWidget = new QStackedWidget(this);
    imuHandler = new ImuDataHandler(this);
    gpsHandler = new GPSDataHandler(this);

    // Dodanie widżetów do stosu
    stackedWidget->addWidget(imuHandler);
    stackedWidget->addWidget(gpsHandler);

    setCentralWidget(stackedWidget);
    stackedWidget->setCurrentWidget(imuHandler); // domyślnie IMU

    createMenus();
    showFullScreen();

    connect(this, &MainWindow::switchToIMU, this, &MainWindow::showIMUHandler);
    connect(this, &MainWindow::switchToGPS, this, &MainWindow::showGPSHandler);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::updateSimulationData);

}

MainWindow::~MainWindow() = default;

void MainWindow::createMenus() {
    QMenuBar *bar = this->menuBar();

    QMenu *sensorMenu = bar->addMenu(tr("Sensor"));
    QAction *imuAction = sensorMenu->addAction(tr("IMU"));
    QAction *gpsAction = sensorMenu->addAction(tr("GPS"));

    connect(imuAction, &QAction::triggered, this, [this]() { emit switchToIMU(); });
    connect(gpsAction, &QAction::triggered, this, [this]() { emit switchToGPS(); });

    QMenu *settingsMenu = bar->addMenu(tr("Settings"));
    QMenu *languageMenu = settingsMenu->addMenu(tr("Language"));
    QAction *englishAction = languageMenu->addAction(tr("English"));
    QAction *polishAction = languageMenu->addAction(tr("Polish"));

    QAction *simulationModeAction = settingsMenu->addAction(tr("Simulation Mode"));
    simulationModeAction->setCheckable(true);

    QAction *selectPortAction = settingsMenu->addAction(tr("Select Port"));

    connect(englishAction, &QAction::triggered, this, &MainWindow::setEnglishLanguage);
    connect(polishAction, &QAction::triggered, this, &MainWindow::setPolishLanguage);
    connect(simulationModeAction, &QAction::triggered, this, &MainWindow::toggleSimulationMode);
    connect(selectPortAction, &QAction::triggered, this, &MainWindow::selectPort);
}

void MainWindow::setEnglishLanguage() {
    QMessageBox::information(this, tr("Language Change"), tr("Language changed to English."));
}

void MainWindow::setPolishLanguage() {
    QMessageBox::information(this, tr("Language Change"), tr("Language changed to Polish."));
}

void MainWindow::toggleSimulationMode() {
    simulationMode = !simulationMode;
    if (simulationMode) {
        simulationTimer->start(100); // Update every 100 ms
    } else {
        simulationTimer->stop();
    }
    QMessageBox::information(this, tr("Simulation Mode"), simulationMode ? tr("Simulation mode enabled.") : tr("Simulation mode disabled."));
}
void MainWindow::updateSimulationData() {
    static float t = 0;
    static float yaw = 0;
    static float pitch = 0;
    static float roll = 0;

    t += 0.1f;
    yaw += 1.0f;
    pitch += 0.5f;
    roll += 0.8f;

    if (yaw > 360.0f) yaw -= 360.0f;
    if (pitch > 360.0f) pitch -= 360.0f;
    if (roll > 360.0f) roll -= 360.0f;

    // === 1. Apply rotation to 3D board ===
    imuHandler->setRotation(yaw, pitch, roll);

    // === 2. Build rotation matrix from Euler angles ===
    QQuaternion rotation = QQuaternion::fromEulerAngles(pitch, yaw, roll);
    QMatrix4x4 rotMatrix;
    rotMatrix.rotate(rotation);

    // === 3. Simulate accelerometer data (gravity in world frame) ===
    QVector3D gravityWorld(0, 0, -1000); // in mg
    QVector3D accSensor = rotMatrix.inverted() * gravityWorld;

    // === 4. Simulate magnetometer data (north = +X in world frame) ===
    QVector3D magWorld(600, 0, 0); // in mG
    QVector3D magSensor = rotMatrix.inverted() * magWorld;

    // === 5. Simulate gyroscope data ===
    QVector<int> gyro = {
        static_cast<int>(yaw * 10),
        static_cast<int>(pitch * 10),
        static_cast<int>(roll * 10)
    };

    QVector<int> acc = {
        static_cast<int>(accSensor.x()),
        static_cast<int>(accSensor.y()),
        static_cast<int>(accSensor.z())
    };

    QVector<int> mag = {
        static_cast<int>(magSensor.x()),
        static_cast<int>(magSensor.y()),
        static_cast<int>(magSensor.z())
    };

    imuHandler->updateData(acc, gyro, mag);

    // === 6. Compass heading from magnetometer ===
    float heading = std::atan2(mag[1], mag[0]) * 180.0f / M_PI;
    if (heading < 0) heading += 360.0f;
    imuHandler->updateCompass(heading);

    // === 7. Simulate GPS offset ===
    double offsetLat = 0.00005 * sin(t);
    double offsetLon = 0.00005 * cos(t);
    gpsHandler->updateMarker(52.2297 + offsetLat, 21.0122 + offsetLon);
}
void MainWindow::setManualRotation(float yaw, float pitch, float roll) {
    manualRotationEuler = QVector3D(yaw, pitch, roll);
}
void MainWindow::selectPort() {
    // TODO: Implement port selection dialog
    qDebug() << "Select port triggered.";
}

void MainWindow::showIMUHandler() {
    qDebug() << "Switching to IMU handler.";
    if (stackedWidget && imuHandler) {
        stackedWidget->setCurrentWidget(imuHandler);
    } else {
        qDebug() << "IMU handler or stacked widget is null!";
    }
}

void MainWindow::showGPSHandler() {
    qDebug() << "Switching to GPS handler.";
    if (stackedWidget && gpsHandler) {
        gpsHandler->updateMarker(52.2297, 21.0122); // Warszawa
        stackedWidget->setCurrentWidget(gpsHandler);
    } else {
        qDebug() << "GPS handler or stacked widget is null!";
    }
}