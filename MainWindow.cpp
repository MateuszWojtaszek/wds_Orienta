#include "MainWindow.h"
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QDebug>
#include <QStackedWidget>

#include "ImuDataHandler.h"
#include "GpsDataHandler.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
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
    // TODO: Implement simulation mode toggle
    qDebug() << "Simulation mode toggled.";
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
        stackedWidget->setCurrentWidget(gpsHandler);
    } else {
        qDebug() << "GPS handler or stacked widget is null!";
    }
}