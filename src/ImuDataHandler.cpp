/**
 * @file ImuDataHandler.cpp
 * @brief Implementacja metod klasy ImuDataHandler.
 * @details Ten plik zawiera definicje funkcji składowych klasy `ImuDataHandler`,
 * które odpowiadają za logikę działania interfejsu użytkownika,
 * przetwarzanie danych IMU oraz konfigurację elementów wizualizacyjnych.
 * @author Mateusz Wojtaszek
 * @date 2025-04-19
 * @bug Brak znanych błędów.
 * @version 1.0
 */
#include "ImuDataHandler.h"
#include "SensorGraph.h"        // Wymagane dla wizualizacji wykresów
#include "Compass2DRenderer.h"  // Wymagane dla wizualizacji kompasu 2D

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QDebug>
#include <QFont>
#include <QUrl>
#include <QColor>
#include <QVector3D>
#include <QtMath>

// Inkluzje Qt3D dla modelu płytki
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DCore/QComponent>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QOrbitCameraController>


ImuDataHandler::ImuDataHandler(QWidget *parent)
    : QWidget(parent),
      currentSampleCount(1000), // Domyślna liczba próbek
      accXBar(nullptr), accYBar(nullptr), accZBar(nullptr),
      gyroXBar(nullptr), gyroYBar(nullptr), gyroZBar(nullptr),
      magXBar(nullptr), magYBar(nullptr), magZBar(nullptr),
      accGraph(nullptr), gyroGraph(nullptr), magGraph(nullptr),
      stackedWidget(nullptr),
      visualizationPanelWidget(nullptr),
      view3DContainerWidget(nullptr),
      m_compass2DRenderer(nullptr),
      boardTransform(nullptr),
      m_currentDataButton(nullptr),
      m_graphButton(nullptr),
      m_accGroupBox(nullptr),
      m_gyroGroupBox(nullptr),
      m_magGroupBox(nullptr) {
    setupMainLayout();
    setRange(0, 0); // Ustawienie domyślnych zakresów (funkcja obecnie ignoruje argumenty)
}

void ImuDataHandler::updateData(const QVector<int> &acc, const QVector<int> &gyro, const QVector<int> &mag) {
    if (acc.size() == 3) {
        if (accXBar) accXBar->setValue(acc[0]);
        if (accYBar) accYBar->setValue(acc[1]);
        if (accZBar) accZBar->setValue(acc[2]);
        if (accXBar) accXBar->setFormat(QString::number(acc[0]));
        if (accYBar) accYBar->setFormat(QString::number(acc[1]));
        if (accZBar) accZBar->setFormat(QString::number(acc[2]));
        if (accGraph) accGraph->addData(acc);
    } else if (!acc.isEmpty()) {
        // Log warning only if data was provided but was invalid
        qWarning() << "Accelerometer data size is not 3. Expected [X, Y, Z]. Received size:" << acc.size();
    }

    if (gyro.size() == 3) {
        if (gyroXBar) gyroXBar->setValue(gyro[0]);
        if (gyroYBar) gyroYBar->setValue(gyro[1]);
        if (gyroZBar) gyroZBar->setValue(gyro[2]);
        if (gyroXBar) gyroXBar->setFormat(QString::number(gyro[0]));
        if (gyroYBar) gyroYBar->setFormat(QString::number(gyro[1]));
        if (gyroZBar) gyroZBar->setFormat(QString::number(gyro[2]));
        if (gyroGraph) gyroGraph->addData(gyro);
    } else if (!gyro.isEmpty()) {
        qWarning() << "Gyroscope data size is not 3. Expected [X, Y, Z]. Received size:" << gyro.size();
    }

    if (mag.size() == 3) {
        if (magXBar) magXBar->setValue(mag[0]);
        if (magYBar) magYBar->setValue(mag[1]);
        if (magZBar) magZBar->setValue(mag[2]);
        if (magXBar) magXBar->setFormat(QString::number(mag[0]));
        if (magYBar) magYBar->setFormat(QString::number(mag[1]));
        if (magZBar) magZBar->setFormat(QString::number(mag[2]));
        if (magGraph) magGraph->addData(mag);
    } else if (!mag.isEmpty()) {
        qWarning() << "Magnetometer data size is not 3. Expected [X, Y, Z]. Received size:" << mag.size();
    }
}

void ImuDataHandler::setSampleCount(int samples) {
    currentSampleCount = qMax(10, samples); // Minimalna liczba próbek to 10
    if (accGraph) accGraph->setSampleCount(currentSampleCount);
    if (gyroGraph) gyroGraph->setSampleCount(currentSampleCount);
    if (magGraph) magGraph->setSampleCount(currentSampleCount);
}

void ImuDataHandler::setRange(int minVal, int maxVal) {
    // Aktualne zakresy są stałe i zdefiniowane w tej metodzie.
    // Argumenty minVal i maxVal są ignorowane.
    const int accRangeVal = 4000;
    const int gyroRangeVal = 250;
    const int magRangeVal = 1600;

    if (accXBar) accXBar->setRange(-accRangeVal, accRangeVal);
    if (accYBar) accYBar->setRange(-accRangeVal, accRangeVal);
    if (accZBar) accZBar->setRange(-accRangeVal, accRangeVal);

    if (gyroXBar) gyroXBar->setRange(-gyroRangeVal, gyroRangeVal);
    if (gyroYBar) gyroYBar->setRange(-gyroRangeVal, gyroRangeVal);
    if (gyroZBar) gyroZBar->setRange(-gyroRangeVal, gyroRangeVal);

    if (magXBar) magXBar->setRange(-magRangeVal, magRangeVal);
    if (magYBar) magYBar->setRange(-magRangeVal, magRangeVal);
    if (magZBar) magZBar->setRange(-magRangeVal, magRangeVal);

    if (accGraph) accGraph->setYRange(-accRangeVal, accRangeVal);
    if (gyroGraph) gyroGraph->setYRange(-gyroRangeVal, gyroRangeVal);
    if (magGraph) magGraph->setYRange(-magRangeVal, magRangeVal);

    Q_UNUSED(minVal); // Zaznaczenie, że parametr jest celowo nieużywany
    Q_UNUSED(maxVal); // Zaznaczenie, że parametr jest celowo nieużywany
}

void ImuDataHandler::setRotation(float yaw, float pitch, float roll) {
    // Konwersja kątów Eulera na kwaternion.
    // Uwaga: Qt używa kolejności (pitch, yaw, roll) dla osi (X, Y, Z) w fromEulerAngles,
    // co może być mylące. Jeśli 'yaw' ma być wokół osi Y, 'pitch' wokół X, 'roll' wokół Z,
    // to kolejność (pitch, yaw, roll) jest poprawna dla (X, Y, Z).
    // W dokumentacji funkcji jest: yaw (Z), pitch (X), roll (Y).
    // Jeśli to jest docelowy układ, to powinno być: QQuaternion::fromEulerAngles(pitch, roll, yaw);
    // Jednakże, jeśli kąty są już w układzie oczekiwanym przez model (np. Y-up),
    // to (pitch, yaw, roll) jako (kąt wokół X, kąt wokół Y, kąt wokół Z) jest częstą konwencją.
    // Aktualna implementacja: (pitch, yaw, roll) -> (obrót wokół X, obrót wokół Y, obrót wokół Z)
    // Jeśli yaw to Z, pitch to X, roll to Y, to trzeba dostosować:
    // QQuaternion rotation = QQuaternion::fromEulerAngles(pitch, roll, yaw); // (X, Y, Z)
    // Aktualnie używana (pitch, yaw, roll) w funkcji setRotation w klasie,
    // oznacza, że yaw jest drugim argumentem, więc odpowiada osi Y
    QQuaternion rotation = QQuaternion::fromEulerAngles(pitch, yaw, roll);
    if (boardTransform) {
        boardTransform->setRotation(rotation);
    } else {
        // qWarning() << "boardTransform is null in setRotation. Cannot set model rotation.";
        // Opcjonalnie: można dodać logowanie, jeśli transformacja nie istnieje.
    }
}

void ImuDataHandler::updateCompass(float heading) {
    if (m_compass2DRenderer) {
        m_compass2DRenderer->setHeading(heading);
    } else {
        qWarning() << "m_compass2DRenderer is null in updateCompass. Cannot set heading for 2D compass.";
    }
}

void ImuDataHandler::showCurrentData() {
    if (stackedWidget) stackedWidget->setCurrentIndex(0);
}

void ImuDataHandler::showGraph() {
    if (stackedWidget) stackedWidget->setCurrentIndex(1);
}

void ImuDataHandler::setupMainLayout() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QWidget *buttonPanel = createButtonPanel();
    QWidget *leftPanel = createLeftPanel();
    setupVisualizationPanel(); // Prawy panel (wizualizacje 3D/2D)

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->addWidget(leftPanel, 1); // Lewy panel zajmuje 1 część wagową
    if (visualizationPanelWidget) {
        contentLayout->addWidget(visualizationPanelWidget, 1); // Prawy panel zajmuje 1 część wagową
    } else {
        qWarning() << "Visualization panel widget is null after setupVisualizationPanel call!";
    }

    mainLayout->addWidget(buttonPanel);
    mainLayout->addLayout(contentLayout);

    if (stackedWidget) stackedWidget->setCurrentIndex(0); // Domyślnie pokaż dane bieżące
    setLayout(mainLayout);
}

QWidget *ImuDataHandler::createButtonPanel() {
    QWidget *buttonPanel = new QWidget(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonPanel);

    m_currentDataButton = new QPushButton(tr("Current Data"), buttonPanel);
    m_graphButton = new QPushButton(tr("Graph"), buttonPanel);

    buttonLayout->addWidget(m_currentDataButton);
    buttonLayout->addWidget(m_graphButton);

    connect(m_currentDataButton, &QPushButton::clicked, this, &ImuDataHandler::showCurrentData);
    connect(m_graphButton, &QPushButton::clicked, this, &ImuDataHandler::showGraph);

    buttonPanel->setLayout(buttonLayout);
    return buttonPanel;
}

QWidget *ImuDataHandler::createLeftPanel() {
    QWidget *leftPanelWidget = new QWidget(this);
    QVBoxLayout *leftPanelLayout = new QVBoxLayout(leftPanelWidget);
    leftPanelLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *barWidget = createBarDisplayWidget();
    QWidget *graphWidget = createGraphDisplayWidget();

    stackedWidget = new QStackedWidget(leftPanelWidget);
    stackedWidget->addWidget(barWidget);
    stackedWidget->addWidget(graphWidget);

    leftPanelLayout->addWidget(stackedWidget);
    leftPanelWidget->setLayout(leftPanelLayout);
    return leftPanelWidget;
}

QWidget *ImuDataHandler::createBarDisplayWidget() {
    QWidget *barWidget = new QWidget(this);
    QVBoxLayout *barLayout = new QVBoxLayout(barWidget);

    // Funkcja pomocnicza lambda do tworzenia grupy pasków postępu
    auto addBarGroup = [&](const QString &titleKey, QGroupBox *&groupBoxMemberRef, QProgressBar *&x, QProgressBar *&y,
                           QProgressBar *&z, int range) {
        groupBoxMemberRef = new QGroupBox(tr(qPrintable(titleKey)), barWidget);
        QVBoxLayout *groupLayout = new QVBoxLayout(groupBoxMemberRef);

        auto createBarInGroup = [&](const QString &axis, QProgressBar *&bar) {
            QWidget *rowWidget = new QWidget(groupBoxMemberRef);
            QHBoxLayout *row = new QHBoxLayout(rowWidget);
            row->setContentsMargins(0, 0, 0, 0);

            QLabel *axisLabel = new QLabel(axis, rowWidget);
            axisLabel->setFixedWidth(20);

            bar = new QProgressBar(rowWidget);
            bar->setRange(-range, range);
            bar->setValue(0);
            bar->setTextVisible(true);
            bar->setFormat(QString::number(0)); // Domyślny format, aktualizowany w updateData

            QWidget *labelsWidget = new QWidget(rowWidget);
            QVBoxLayout *barWithLabelsLayout = new QVBoxLayout(labelsWidget);
            barWithLabelsLayout->setContentsMargins(0, 0, 0, 0);
            barWithLabelsLayout->setSpacing(0);

            QHBoxLayout *rangeLabelsLayout = new QHBoxLayout();
            rangeLabelsLayout->addWidget(new QLabel(QString::number(-range), labelsWidget));
            rangeLabelsLayout->addStretch();
            rangeLabelsLayout->addWidget(new QLabel("0", labelsWidget));
            rangeLabelsLayout->addStretch();
            rangeLabelsLayout->addWidget(new QLabel(QString::number(range), labelsWidget));

            barWithLabelsLayout->addLayout(rangeLabelsLayout);
            barWithLabelsLayout->addWidget(bar);

            row->addWidget(axisLabel);
            row->addWidget(labelsWidget);
            groupLayout->addWidget(rowWidget);
        };

        createBarInGroup("X:", x);
        createBarInGroup("Y:", y);
        createBarInGroup("Z:", z);
        groupBoxMemberRef->setLayout(groupLayout);
        barLayout->addWidget(groupBoxMemberRef);
    };

    addBarGroup("Accelerometer [mg]", m_accGroupBox, accXBar, accYBar, accZBar, 4000);
    addBarGroup("Gyroscope [dps]", m_gyroGroupBox, gyroXBar, gyroYBar, gyroZBar, 250);
    addBarGroup("Magnetometer [mG]", m_magGroupBox, magXBar, magYBar, magZBar, 1600);

    barWidget->setLayout(barLayout);
    return barWidget;
}

QWidget *ImuDataHandler::createGraphDisplayWidget() {
    QWidget *graphWidget = new QWidget(this);
    QVBoxLayout *graphLayout = new QVBoxLayout(graphWidget);
    graphLayout->setContentsMargins(5, 5, 5, 5);

    // Tworzenie wykresów z odpowiednimi tytułami i zakresami
    accGraph = new SensorGraph(tr("Accelerometer [mg]"), -4000, 4000, graphWidget);
    gyroGraph = new SensorGraph(tr("Gyroscope [dps]"), -250, 250, graphWidget);
    magGraph = new SensorGraph(tr("Magnetometer [mG]"), -1600, 1600, graphWidget);

    // Ustawienie liczby próbek dla nowo utworzonych wykresów
    setSampleCount(this->currentSampleCount);

    graphLayout->addWidget(accGraph);
    graphLayout->addWidget(gyroGraph);
    graphLayout->addWidget(magGraph);

    graphWidget->setLayout(graphLayout);
    return graphWidget;
}

void ImuDataHandler::setupVisualizationPanel() {
    if (!view3DContainerWidget) {
        view3DContainerWidget = create3DView();
        if (view3DContainerWidget) {
            view3DContainerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }
    }

    if (!m_compass2DRenderer) {
        m_compass2DRenderer = new Compass2DRenderer(this);
        m_compass2DRenderer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        m_compass2DRenderer->setMinimumSize(150, 150); // Minimalny rozsądny rozmiar dla kompasu
    }

    if (!visualizationPanelWidget) {
        visualizationPanelWidget = new QWidget(this);
        QVBoxLayout *visualizationLayout = new QVBoxLayout(visualizationPanelWidget);
        visualizationLayout->setSpacing(5);
        visualizationLayout->setContentsMargins(0, 0, 0, 0);

        if (view3DContainerWidget) {
            visualizationLayout->addWidget(view3DContainerWidget, 2);
            // Widok 3D zajmuje więcej miejsca (stretch factor 2)
        }
        if (m_compass2DRenderer) {
            visualizationLayout->addWidget(m_compass2DRenderer, 1);
            // Kompas 2D zajmuje mniej miejsca (stretch factor 1)
        }
        visualizationPanelWidget->setLayout(visualizationLayout);
        visualizationPanelWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
}

QWidget *ImuDataHandler::create3DView() {
    Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f))); // Ciemnoszary kolor tła
    QWidget *container = QWidget::createWindowContainer(view, this);
    container->setFocusPolicy(Qt::StrongFocus);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
    setupCameraAndController3D(view, rootEntity);
    setupLighting3D(rootEntity);
    setupModelLoader3D(rootEntity);

    view->setRootEntity(rootEntity);
    return container;
}

void ImuDataHandler::setupCameraAndController3D(Qt3DExtras::Qt3DWindow *view, Qt3DCore::QEntity *rootEntity) {
    Qt3DRender::QCamera *camera = view->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(1.5f, 1.5f, 1.5f)); // Pozycja kamery nieco oddalona
    camera->setUpVector(QVector3D(0, 1, 0)); // Oś Y jako "góra"
    camera->setViewCenter(QVector3D(0, 0, 0)); // Kamera patrzy na środek sceny

    Qt3DExtras::QOrbitCameraController *controller = new Qt3DExtras::QOrbitCameraController(rootEntity);
    controller->setLinearSpeed(50.0f); // Dostosowanie prędkości kamery
    controller->setLookSpeed(180.0f);
    controller->setCamera(camera);
}

void ImuDataHandler::setupLighting3D(Qt3DCore::QEntity *rootEntity) {
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QDirectionalLight *light = new Qt3DRender::QDirectionalLight(lightEntity);
    light->setWorldDirection(QVector3D(-1, -1, -1)); // Kierunek światła
    light->setColor(Qt::white);
    light->setIntensity(1.0f);
    lightEntity->addComponent(light);
}

void ImuDataHandler::setupModelLoader3D(Qt3DCore::QEntity *rootEntity) {
    Qt3DCore::QEntity *modelEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QSceneLoader *loader = new Qt3DRender::QSceneLoader(modelEntity);

    connect(loader, &Qt3DRender::QSceneLoader::statusChanged, this,
            [this, loader, modelEntity](Qt3DRender::QSceneLoader::Status status) {
                if (status == Qt3DRender::QSceneLoader::Ready) {
                    const auto &childEntities = loader->entities();
                    if (!childEntities.isEmpty()) {
                        Qt3DCore::QEntity *sceneRootEntity = childEntities.first();
                        if (!sceneRootEntity) return;

                        if (!this->boardTransform) {
                            this->boardTransform = new Qt3DCore::QTransform();
                        }

                        bool alreadyHasTransform = false;
                        for (Qt3DCore::QComponent *comp: sceneRootEntity->components()) {
                            if (dynamic_cast<Qt3DCore::QTransform *>(comp) == this->boardTransform) {
                                alreadyHasTransform = true;
                                break;
                            }
                        }
                        if (!alreadyHasTransform) {
                            // Jeśli model ma własną transformację, można ją pobrać i używać,
                            // ale dodanie naszej transformacji daje pewniejszą kontrolę.
                            sceneRootEntity->addComponent(this->boardTransform);
                        }
                        // Przykładowe początkowe ustawienia transformacji, jeśli potrzebne
                        // this->boardTransform->setScale3D(QVector3D(0.05f, 0.05f, 0.05f));
                        // this->boardTransform->setRotationX(90);
                    } else {
                        qWarning() << "SceneLoader: No entities found after loading model from" << loader->source();
                    }
                } else if (status == Qt3DRender::QSceneLoader::Error) {
                    qWarning() << "Failed to load 3D model:" << loader->source();
                }
            });
    // lub użyj ścieżki zasobów Qt (qrc:/).
    loader->setSource(QUrl::fromLocalFile("/Users/mateuszwojtaszek/projekty/wds_Orienta/ESP32.dae"));
    //loader->setSource(QUrl("qrc:/models/ESP32.dae")); // Przykład użycia zasobów Qt

    modelEntity->addComponent(loader);
}


void ImuDataHandler::retranslateUi() {
    if (m_currentDataButton) {
        m_currentDataButton->setText(tr("Current Data"));
    }
    if (m_graphButton) {
        m_graphButton->setText(tr("Graph"));
    }

    if (m_accGroupBox) {
        m_accGroupBox->setTitle(tr("Accelerometer [mg]"));
    }
    if (m_gyroGroupBox) {
        m_gyroGroupBox->setTitle(tr("Gyroscope [dps]"));
    }
    if (m_magGroupBox) {
        m_magGroupBox->setTitle(tr("Magnetometer [mG]"));
    }

    // Zakładając, że SensorGraph ma metodę retranslateUi do aktualizacji swojego tytułu
    if (accGraph) accGraph->retranslateUi();
    if (gyroGraph) gyroGraph->retranslateUi();
    if (magGraph) magGraph->retranslateUi();

    // Compass2DRenderer nie przechowuje tekstów do tłumaczenia, więc pomijam.
}
