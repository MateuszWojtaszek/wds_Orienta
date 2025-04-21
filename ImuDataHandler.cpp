#include "ImuDataHandler.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QText2DEntity>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QSceneLoader>
#include <QtMath>
#include <QValueAxis>
SensorGraph::SensorGraph(const QString &title, int minY, int maxY, QWidget *parent)
    : QChartView(new QChart(), parent), sampleCount(1000) {
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

ImuDataHandler::ImuDataHandler(QWidget *parent) : QWidget(parent), sampleCount(100) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *currentDataButton = new QPushButton(tr("Current Data"), this);
    QPushButton *graphButton = new QPushButton(tr("Graph"), this);
    buttonLayout->addWidget(currentDataButton);
    buttonLayout->addWidget(graphButton);
    mainLayout->addLayout(buttonLayout);

    // Paski postępu
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

    addBarGroup("Accelerometer [mg]", accXBar, accYBar, accZBar, 4000);
    addBarGroup("Gyroscope [dps]", gyroXBar, gyroYBar, gyroZBar, 250);
    addBarGroup("Magnetometer [mG]", magXBar, magYBar, magZBar, 1600);

    // Wizualizacja: góra model, dół kompas
    QWidget *model3D = create3DView();
    QWidget *compass3D = createCompassView();

    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->addWidget(create3DView(), 3);
    rightLayout->addWidget(createCompassView(), 1);
    rightLayout->setSpacing(0);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    // Widok aktualnych danych
    QWidget *currentDataView = new QWidget(this);
    QHBoxLayout *currentLayout = new QHBoxLayout(currentDataView);
    currentLayout->addWidget(barWidget, 1);
    currentLayout->addWidget(rightPanel, 1);

    // Widok wykresów
    accGraph = new SensorGraph("Accelerometer [mg]", -4000, 4000, this);
    gyroGraph = new SensorGraph("Gyroscope [dps]", -250, 250, this);
    magGraph = new SensorGraph("Magnetometer [mG]", -1600, 1600, this);
    accGraph->setSampleCount(1000);
    gyroGraph->setSampleCount(1000);
    magGraph->setSampleCount(1000);
    QWidget *graphWidget = new QWidget(this);
    QVBoxLayout *graphLayout = new QVBoxLayout(graphWidget);
    graphLayout->addWidget(accGraph);
    graphLayout->addWidget(gyroGraph);
    graphLayout->addWidget(magGraph);

    QWidget *visualizationPanel = new QWidget(this);
    QVBoxLayout *visualizationLayout = new QVBoxLayout(visualizationPanel);
    visualizationLayout->setSpacing(0);
    visualizationLayout->setContentsMargins(0, 0, 0, 0);
    visualizationLayout->addWidget(create3DView(), 3);      // Model 3D płytki (góra)
    visualizationLayout->addWidget(createCompassView(), 1); // Kompas (dół)

    QWidget *graphView = new QWidget(this);
    QHBoxLayout *graphViewLayout = new QHBoxLayout(graphView);
    graphViewLayout->addWidget(graphWidget, 1);
    graphViewLayout->addWidget(visualizationPanel, 1); // prawa część: model + kompas

    stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(currentDataView);
    stackedWidget->addWidget(graphView);
    mainLayout->addWidget(stackedWidget);

    stackedWidget->setCurrentIndex(0);

    connect(currentDataButton, &QPushButton::clicked, this, &ImuDataHandler::showCurrentData);
    connect(graphButton, &QPushButton::clicked, this, &ImuDataHandler::showGraph);
}

QWidget* ImuDataHandler::create3DView() {
    auto *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    auto *container = QWidget::createWindowContainer(view);
    container->setMinimumSize(300, 300);

    auto *rootEntity = new Qt3DCore::QEntity();

    auto *camera = view->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(1.5, 1.5, 1.5));
    camera->setViewCenter(QVector3D(0, 0, 0));

    auto *controller = new Qt3DExtras::QOrbitCameraController(rootEntity);
    controller->setCamera(camera);

    auto *light = new Qt3DRender::QDirectionalLight();
    light->setWorldDirection(QVector3D(-1, -1, -1));
    auto *lightEntity = new Qt3DCore::QEntity(rootEntity);
    lightEntity->addComponent(light);

    auto *modelEntity = new Qt3DCore::QEntity(rootEntity);
    auto *loader = new Qt3DRender::QSceneLoader(modelEntity);
    loader->setSource(QUrl::fromLocalFile("/Users/mateuszwojtaszek/projekty/wds_Orienta/ESP32.dae"));
    modelEntity->addComponent(loader);

    // When model is loaded, apply transform to its root node
    connect(loader, &Qt3DRender::QSceneLoader::statusChanged, this, [=](Qt3DRender::QSceneLoader::Status status) {
        if (status == Qt3DRender::QSceneLoader::Ready) {
            const auto &childEntities = loader->entities();
            if (!childEntities.isEmpty()) {
                auto *sceneRoot = childEntities.first(); // assume the first root node
                boardTransform = new Qt3DCore::QTransform();
                sceneRoot->addComponent(boardTransform); // ✅ Attach transform to the loaded scene's root
            }
        }
    });

    view->setRootEntity(rootEntity);
    return container;
}

QWidget* ImuDataHandler::createCompassView() {
    auto *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    auto *container = QWidget::createWindowContainer(view);
    container->setMinimumSize(500, 500);

    auto *rootEntity = new Qt3DCore::QEntity();

    auto *camera = view->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0, 0, 45));
    camera->setViewCenter(QVector3D(0, 0, 0));

    auto *controller = new Qt3DExtras::QOrbitCameraController(rootEntity);
    controller->setCamera(camera);

    auto *light = new Qt3DRender::QDirectionalLight();
    light->setWorldDirection(QVector3D(-1, -1, -1));
    auto *lightEntity = new Qt3DCore::QEntity(rootEntity);
    lightEntity->addComponent(light);

    // Create the compass circle
    auto *ringEntity = new Qt3DCore::QEntity(rootEntity);
    auto *ringMesh = new Qt3DExtras::QTorusMesh();
    ringMesh->setRadius(6.5f);
    ringMesh->setMinorRadius(0.15f);
    auto *ringMaterial = new Qt3DExtras::QPhongMaterial();
    ringMaterial->setDiffuse(QColor("#44aaff"));
    ringEntity->addComponent(ringMesh);
    ringEntity->addComponent(ringMaterial);

    // Create the compass needle
    compassArrowEntity = new Qt3DCore::QEntity(rootEntity);
    auto *coneMesh = new Qt3DExtras::QConeMesh();
    coneMesh->setLength(3.8f);
    coneMesh->setTopRadius(0.0f);
    coneMesh->setBottomRadius(0.8f);
    auto *arrowMaterial = new Qt3DExtras::QPhongMaterial();
    arrowMaterial->setDiffuse(QColor("#ff0000"));
    compassTransform = new Qt3DCore::QTransform();
    compassTransform->setTranslation(QVector3D(0, 0, 0));
    compassArrowEntity->addComponent(coneMesh);
    compassArrowEntity->addComponent(arrowMaterial);
    compassArrowEntity->addComponent(compassTransform);

    // Add labels for N, E, S, W
    auto addLabel = [&](const QString &text, const QVector3D &pos) {
        auto *label = new Qt3DExtras::QText2DEntity(rootEntity);
        label->setFont(QFont("Arial", 2, QFont::Bold));
        label->setText(text);
        label->setHeight(5);
        label->setWidth(5);
        label->setColor(Qt::white);
        auto *t = new Qt3DCore::QTransform();
        t->setTranslation(pos);
        label->addComponent(t);
    };

    addLabel("N", QVector3D(-1, 4, 0));
    addLabel("E", QVector3D(7, -4, 0));
    addLabel("S", QVector3D(-1, -12, 5));
    addLabel("W", QVector3D(-9, -4, 0));

    view->setRootEntity(rootEntity);
    return container;
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

    float heading = std::atan2(mag[1], mag[0]) * 180.0f / M_PI;
    if (heading < 0) heading += 360.0f;

    if (compassTransform) {
        compassTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), -heading));
    }
}

void ImuDataHandler::setSampleCount(int samples) {
    sampleCount = samples;
    accGraph->setSampleCount(samples);
    gyroGraph->setSampleCount(samples);
    magGraph->setSampleCount(samples);
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

void ImuDataHandler::showCurrentData() {
    stackedWidget->setCurrentIndex(0);
}

void ImuDataHandler::showGraph() {
    stackedWidget->setCurrentIndex(1);
}
void ImuDataHandler::setRotation(float yaw, float pitch, float roll) {
    QQuaternion rotation = QQuaternion::fromEulerAngles(pitch, yaw, roll);

    if (compassTransform) {
        compassTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), -yaw)); // Still for compass only
    }

    if (boardTransform) {
        boardTransform->setRotation(rotation);  // ✅ Rotate the 3D board
        qDebug() << "Updated board model rotation: yaw=" << yaw << " pitch=" << pitch << " roll=" << roll;
    } else {
        qDebug() << "Board transform is null!";
    }
}

void ImuDataHandler::updateCompass(float heading) {
    // Example: emit signal to update compass widget
    qDebug() << "Simulated compass heading:" << heading;
    if (compassTransform) {
        compassTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), -heading));
    }
}