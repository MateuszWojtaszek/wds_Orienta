//
// Created by Mateusz Wojtaszek on 31/03/2025.
//

#include "ImuDataHandler.h"
#include <QLabel>
#include <QValueAxis>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QDirectionalLight>

SensorGraph::SensorGraph(const QString &title, int minY, int maxY, QWidget *parent)
    : QChartView(new QChart(), parent), sampleCount(100) {
    chart()->setTitle(title);
    chart()->legend()->setVisible(true);
    chart()->legend()->setAlignment(Qt::AlignTop);

    for (int i = 0; i < 3; ++i) {
        auto *series = new QLineSeries();
        switch (i) {
            case 0: series->setName("X"); break;
            case 1: series->setName("Y"); break;
            case 2: series->setName("Z"); break;
        }
        chart()->addSeries(series);
        seriesList.append(series);
    }

    auto xAxis = new QValueAxis();
    xAxis->setTitleText("Sample");
    chart()->setAxisX(xAxis);

    auto yAxis = new QValueAxis();
    yAxis->setTitleText("Value");
    yAxis->setRange(minY, maxY);
    chart()->setAxisY(yAxis);

    for (auto *series : seriesList) {
        chart()->setAxisX(xAxis, series);
        chart()->setAxisY(yAxis, series);
    }
}

void SensorGraph::addData(const QVector<int> &data) {
    for (int i = 0; i < seriesList.size(); ++i) {
        auto *series = seriesList[i];
        QVector<QPointF> points = series->points().toVector();
        points.append(QPointF(points.size(), data[i]));
        if (points.size() > sampleCount) {
            points.remove(0);
        }
        series->replace(points);
    }
}

void SensorGraph::setSampleCount(int count) {
    sampleCount = count;
}

QWidget* ImuDataHandler::create3DView() {
    auto *view = new Qt3DExtras::Qt3DWindow();
    if (auto *fg = qobject_cast<Qt3DExtras::QForwardRenderer *>(view->activeFrameGraph())) {
        fg->setClearColor(QColor(QRgb(0x4d4d4f)));
    }

    auto *container = QWidget::createWindowContainer(view);
    container->setMinimumSize(300, 300);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *rootEntity = new Qt3DCore::QEntity();

    // Camera setup
    Qt3DRender::QCamera *camera = view->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(45, 45, 45));
    camera->setViewCenter(QVector3D(0, 0, 0));
    camera->setUpVector(QVector3D(0, 1, 0));

    // Orbit camera controller
    auto *camController = new Qt3DExtras::QOrbitCameraController(rootEntity);
    camController->setLinearSpeed(50.0f);
    camController->setLookSpeed(180.0f);
    camController->setCamera(camera);

    // Directional light
    auto *lightEntity = new Qt3DCore::QEntity(rootEntity);
    auto *light = new Qt3DRender::QDirectionalLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1.0f);
    light->setWorldDirection(QVector3D(-1.0f, -1.0f, -1.0f));
    lightEntity->addComponent(light);

    // Ambient light
    auto *ambientEntity = new Qt3DCore::QEntity(rootEntity);
    //light->setIntensity(1.3f);  // mocniejsze światło
    // Load model (.obj)
    auto *modelEntity = new Qt3DCore::QEntity(rootEntity);
    auto *loader = new Qt3DRender::QSceneLoader(modelEntity);
    loader->setSource(QUrl::fromLocalFile("/Users/mateuszwojtaszek/projekty/wds_Orienta/Freenove ESP32-S3-WROOM v84 (Meshed).obj"));
    modelEntity->addComponent(loader);

    view->setRootEntity(rootEntity);
    return container;
}

ImuDataHandler::ImuDataHandler(QWidget *parent) : QWidget(parent), sampleCount(100) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *currentDataButton = new QPushButton(tr("Current Data"), this);
    QPushButton *graphButton = new QPushButton(tr("Graph"), this);
    buttonLayout->addWidget(currentDataButton);
    buttonLayout->addWidget(graphButton);
    mainLayout->addLayout(buttonLayout);

    visualizationCurrent = create3DView();
    visualizationGraph = create3DView();

    QWidget *barWidget = new QWidget(this);
    QVBoxLayout *barLayout = new QVBoxLayout(barWidget);

    auto addBarGroup = [&](const QString &title, QProgressBar *&x, QProgressBar *&y, QProgressBar *&z, int range) {
        barLayout->addWidget(new QLabel(title));
        auto createBar = [&](const QString &axis, QProgressBar *&bar) {
            QLabel *axisLabel = new QLabel(axis);
            bar = new QProgressBar(this);
            bar->setRange(-range, range);
            bar->setTextVisible(false);
            bar->setMinimumWidth(200);

            QHBoxLayout *rangeLabels = new QHBoxLayout();
            rangeLabels->addWidget(new QLabel(QString::number(-range)));
            rangeLabels->addStretch();
            rangeLabels->addWidget(new QLabel("0"));
            rangeLabels->addStretch();
            rangeLabels->addWidget(new QLabel(QString::number(range)));

            QVBoxLayout *barWithLabels = new QVBoxLayout();
            barWithLabels->addLayout(rangeLabels);
            barWithLabels->addWidget(bar);

            QHBoxLayout *row = new QHBoxLayout();
            row->addWidget(axisLabel);
            row->addLayout(barWithLabels);
            barLayout->addLayout(row);
        };

        createBar("X:", x);
        createBar("Y:", y);
        createBar("Z:", z);
    };

    addBarGroup("Accelerometer [mg]", accXBar, accYBar, accZBar, 2000);
    addBarGroup("Gyroscope [dps]", gyroXBar, gyroYBar, gyroZBar, 2000);
    addBarGroup("Magnetometer [0.1 G]", magXBar, magYBar, magZBar, 1600);

    accGraph = new SensorGraph("Accelerometer [mg]", -2000, 2000, this);
    gyroGraph = new SensorGraph("Gyroscope [dps]", -2000, 2000, this);
    magGraph = new SensorGraph("Magnetometer [0.1 G]", -1600, 1600, this);

    QWidget *graphWidget = new QWidget(this);
    QVBoxLayout *graphLayout = new QVBoxLayout(graphWidget);
    graphLayout->addWidget(accGraph);
    graphLayout->addWidget(gyroGraph);
    graphLayout->addWidget(magGraph);

    stackedWidget = new QStackedWidget(this);

    QWidget *currentDataView = new QWidget(this);
    QHBoxLayout *currentLayout = new QHBoxLayout(currentDataView);
    currentLayout->addWidget(barWidget, 1);
    currentLayout->addWidget(visualizationCurrent, 1);
    stackedWidget->addWidget(currentDataView);

    QWidget *graphView = new QWidget(this);
    QHBoxLayout *graphLayoutMain = new QHBoxLayout(graphView);
    graphLayoutMain->addWidget(graphWidget, 1);
    graphLayoutMain->addWidget(visualizationGraph, 1);
    stackedWidget->addWidget(graphView);

    mainLayout->addWidget(stackedWidget);
    stackedWidget->setCurrentIndex(0);

    connect(currentDataButton, &QPushButton::clicked, this, &ImuDataHandler::showCurrentData);
    connect(graphButton, &QPushButton::clicked, this, &ImuDataHandler::showGraph);
}

void ImuDataHandler::setRange(int, int) {
    accXBar->setRange(-2000, 2000);
    accYBar->setRange(-2000, 2000);
    accZBar->setRange(-2000, 2000);
    gyroXBar->setRange(-2000, 2000);
    gyroYBar->setRange(-2000, 2000);
    gyroZBar->setRange(-2000, 2000);
    magXBar->setRange(-1600, 1600);
    magYBar->setRange(-1600, 1600);
    magZBar->setRange(-1600, 1600);
}

void ImuDataHandler::updateData(const QVector<int> &acc, const QVector<int> &gyro, const QVector<int> &mag) {
    accXBar->setValue(acc[0]);
    accYBar->setValue(acc[1]);
    accZBar->setValue(acc[2]);
    gyroXBar->setValue(gyro[0]);
    gyroYBar->setValue(gyro[1]);
    gyroZBar->setValue(gyro[2]);
    magXBar->setValue(mag[0]);
    magYBar->setValue(mag[1]);
    magZBar->setValue(mag[2]);

    accGraph->addData(acc);
    gyroGraph->addData(gyro);
    magGraph->addData(mag);
}

void ImuDataHandler::setSampleCount(int samples) {
    sampleCount = samples;
    accGraph->setSampleCount(samples);
    gyroGraph->setSampleCount(samples);
    magGraph->setSampleCount(samples);
}

void ImuDataHandler::showCurrentData() {
    stackedWidget->setCurrentIndex(0);
}

void ImuDataHandler::showGraph() {
    stackedWidget->setCurrentIndex(1);
}