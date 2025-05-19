/**
 * @file ImuDataHandler.cpp
 * @brief Implementacja metod klasy ImuDataHandler.
 */
#include "ImuDataHandler.h"
#include "SensorGraph.h"
#include "Compass2DRenderer.h" // Dodano include dla kompasu 2D

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
// QPainter nie jest bezpośrednio używany w tym pliku

// Qt3D Includes - potrzebne dla modelu płytki
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DCore/QComponent> // Może być potrzebny dla boardTransform
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QSceneLoader>
// #include <Qt3DRender/QMaterial> // Prawdopodobnie niepotrzebne, jeśli model DAE ma własne materiały
// #include <Qt3DRender/QMesh> // Prawdopodobnie niepotrzebne
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QOrbitCameraController>
// Usunięto Qt3DExtras, które były tylko dla kompasu 3D:
// #include <Qt3DExtras/QPhongMaterial>
// #include <Qt3DExtras/QTorusMesh>
// #include <Qt3DExtras/QCuboidMesh>
// #include <Qt3DExtras/QConeMesh>
// #include <Qt3DExtras/QText2DEntity>


ImuDataHandler::ImuDataHandler(QWidget *parent)
    : QWidget(parent),
      currentSampleCount(1000), // Domyślna liczba próbek
      accXBar(nullptr), accYBar(nullptr), accZBar(nullptr),
      gyroXBar(nullptr), gyroYBar(nullptr), gyroZBar(nullptr),
      magXBar(nullptr), magYBar(nullptr), magZBar(nullptr),
      accGraph(nullptr), gyroGraph(nullptr), magGraph(nullptr),
      stackedWidget(nullptr),
      visualizationPanelWidget(nullptr),
      view3DContainerWidget(nullptr), // Inicjalizacja kontenera widoku 3D
      m_compass2DRenderer(nullptr), // Inicjalizacja wskaźnika kompasu 2D
      boardTransform(nullptr), // Inicjalizacja transformacji modelu płytki
      m_currentDataButton(nullptr),
      m_graphButton(nullptr),
      m_accGroupBox(nullptr),
      m_gyroGroupBox(nullptr),
      m_magGroupBox(nullptr)
{
    setupMainLayout();
    setRange(0, 0); // Ustawienie domyślnych zakresów dla pasków/wykresów
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
    }
    if (gyro.size() == 3) {
        if (gyroXBar) gyroXBar->setValue(gyro[0]);
        if (gyroYBar) gyroYBar->setValue(gyro[1]);
        if (gyroZBar) gyroZBar->setValue(gyro[2]);
        if (gyroXBar) gyroXBar->setFormat(QString::number(gyro[0]));
        if (gyroYBar) gyroYBar->setFormat(QString::number(gyro[1]));
        if (gyroZBar) gyroZBar->setFormat(QString::number(gyro[2]));
        if (gyroGraph) gyroGraph->addData(gyro);
    }
    if (mag.size() == 3) {
        if (magXBar) magXBar->setValue(mag[0]);
        if (magYBar) magYBar->setValue(mag[1]);
        if (magZBar) magZBar->setValue(mag[2]);
        if (magXBar) magXBar->setFormat(QString::number(mag[0]));
        if (magYBar) magYBar->setFormat(QString::number(mag[1]));
        if (magZBar) magZBar->setFormat(QString::number(mag[2]));
        if (magGraph) magGraph->addData(mag);
    }
}

void ImuDataHandler::setSampleCount(int samples) {
    currentSampleCount = qMax(10, samples); // Ograniczenie minimalnej liczby próbek
    if (accGraph) accGraph->setSampleCount(currentSampleCount);
    if (gyroGraph) gyroGraph->setSampleCount(currentSampleCount);
    if (magGraph) magGraph->setSampleCount(currentSampleCount);
}

void ImuDataHandler::setRange(int minVal, int maxVal) {
    // Ustawienie zakresów dla pasków postępu
    if (accXBar) accXBar->setRange(-4000, 4000);
    if (accYBar) accYBar->setRange(-4000, 4000);
    if (accZBar) accZBar->setRange(-4000, 4000);
    if (gyroXBar) gyroXBar->setRange(-250, 250);
    if (gyroYBar) gyroYBar->setRange(-250, 250);
    if (gyroZBar) gyroZBar->setRange(-250, 250);
    if (magXBar) magXBar->setRange(-1600, 1600);
    if (magYBar) magYBar->setRange(-1600, 1600);
    if (magZBar) magZBar->setRange(-1600, 1600);

    // Ustawienie zakresów dla wykresów
    if (accGraph) accGraph->setYRange(-4000, 4000);
    if (gyroGraph) gyroGraph->setYRange(-250, 250);
    if (magGraph) magGraph->setYRange(-1600, 1600);

    Q_UNUSED(minVal); // Parametry minVal i maxVal są obecnie ignorowane
    Q_UNUSED(maxVal);
}

void ImuDataHandler::setRotation(float yaw, float pitch, float roll) {
    QQuaternion rotation = QQuaternion::fromEulerAngles(pitch, yaw, roll);
    if (boardTransform) {
        boardTransform->setRotation(rotation);
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
    QVBoxLayout *mainLayout = new QVBoxLayout(this); // Główny layout pionowy

    QWidget *buttonPanel = createButtonPanel(); // Panel z przyciskami wyboru widoku
    QWidget *leftPanel = createLeftPanel();     // Lewy panel (dane bieżące/wykresy)
    setupVisualizationPanel();                  // Prawy panel (wizualizacje 3D/2D)

    QHBoxLayout *contentLayout = new QHBoxLayout(); // Layout dla lewego i prawego panelu
    contentLayout->addWidget(leftPanel, 1);         // Lewy panel zajmuje 1 część
    if (visualizationPanelWidget) {
        contentLayout->addWidget(visualizationPanelWidget, 1); // Prawy panel zajmuje 1 część
    } else {
        qWarning() << "Visualization panel widget is null after setup!";
    }

    mainLayout->addWidget(buttonPanel);  // Dodanie panelu przycisków na górze
    mainLayout->addLayout(contentLayout); // Dodanie paneli treści

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
    leftPanelLayout->setContentsMargins(0, 0, 0, 0); // Usunięcie marginesów

    QWidget *barWidget = createBarDisplayWidget();   // Widget z paskami postępu
    QWidget *graphWidget = createGraphDisplayWidget(); // Widget z wykresami

    stackedWidget = new QStackedWidget(leftPanelWidget); // Widget do przełączania widoków
    stackedWidget->addWidget(barWidget);
    stackedWidget->addWidget(graphWidget);

    leftPanelLayout->addWidget(stackedWidget);
    leftPanelWidget->setLayout(leftPanelLayout);
    return leftPanelWidget;
}

QWidget *ImuDataHandler::createBarDisplayWidget() {
    QWidget *barWidget = new QWidget(this);
    QVBoxLayout *barLayout = new QVBoxLayout(barWidget);

    // Funkcja pomocnicza do tworzenia grupy pasków postępu
    auto addBarGroup = [&](const QString &titleKey, QGroupBox *&groupBoxMemberRef, QProgressBar *&x, QProgressBar *&y, QProgressBar *&z, int range) {
        groupBoxMemberRef = new QGroupBox(tr(qPrintable(titleKey)), barWidget);
        QVBoxLayout *groupLayout = new QVBoxLayout(groupBoxMemberRef);

        auto createBarInGroup = [&](const QString &axis, QProgressBar *&bar) {
            QWidget* rowWidget = new QWidget(groupBoxMemberRef);
            QHBoxLayout *row = new QHBoxLayout(rowWidget);
            row->setContentsMargins(0,0,0,0);

            QLabel *axisLabel = new QLabel(axis, rowWidget);
            axisLabel->setFixedWidth(20); // Stała szerokość etykiety osi

            bar = new QProgressBar(rowWidget);
            bar->setRange(-range, range);
            bar->setValue(0);
            bar->setTextVisible(true); // Pokaż tekst na pasku
            // Domyślny format dla paska, aktualizowany w updateData
            bar->setFormat(QString::number(0));


            QWidget* labelsWidget = new QWidget(rowWidget);
            QVBoxLayout *barWithLabelsLayout = new QVBoxLayout(labelsWidget);
            barWithLabelsLayout->setContentsMargins(0,0,0,0);
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
            row->addWidget(labelsWidget); // Dodanie widgetu z paskiem i etykietami zakresu
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
    graphLayout->setContentsMargins(5, 5, 5, 5); // Niewielkie marginesy wewnętrzne

    accGraph = new SensorGraph(tr("Accelerometer [mg]"), -4000, 4000, graphWidget);
    gyroGraph = new SensorGraph(tr("Gyroscope [dps]"), -250, 250, graphWidget);
    magGraph  = new SensorGraph(tr("Magnetometer [mG]"), -1600, 1600, graphWidget);

    setSampleCount(this->currentSampleCount); // Ustawienie liczby próbek dla nowo utworzonych wykresów

    graphLayout->addWidget(accGraph);
    graphLayout->addWidget(gyroGraph);
    graphLayout->addWidget(magGraph);

    graphWidget->setLayout(graphLayout);
    return graphWidget;
}

void ImuDataHandler::setupVisualizationPanel() {
    // Tworzenie kontenera dla widoku 3D płytki, jeśli jeszcze nie istnieje
    if (!view3DContainerWidget) {
        view3DContainerWidget = create3DView();
        if(view3DContainerWidget) {
            view3DContainerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }
    }

    // Tworzenie widgetu kompasu 2D, jeśli jeszcze nie istnieje
    if (!m_compass2DRenderer) {
        m_compass2DRenderer = new Compass2DRenderer(this);
        m_compass2DRenderer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        // Można ustawić preferowany lub minimalny rozmiar, np.
        m_compass2DRenderer->setMinimumSize(150, 150);
        // Compass2DRenderer używa sizeHint, więc to powinno wystarczyć
    }

    // Tworzenie głównego widgetu panelu wizualizacji, jeśli jeszcze nie istnieje
    if (!visualizationPanelWidget) {
        visualizationPanelWidget = new QWidget(this);
        QVBoxLayout *visualizationLayout = new QVBoxLayout(visualizationPanelWidget);
        visualizationLayout->setSpacing(5); // Odstęp między elementami
        visualizationLayout->setContentsMargins(0, 0, 0, 0);

        if (view3DContainerWidget) {
            // Widok 3D płytki zajmuje więcej miejsca
            visualizationLayout->addWidget(view3DContainerWidget, 2); // Stretch factor 2
        }
        if (m_compass2DRenderer) {
            // Kompas 2D zajmuje mniej miejsca
            visualizationLayout->addWidget(m_compass2DRenderer, 1); // Stretch factor 1
        }
        visualizationPanelWidget->setLayout(visualizationLayout);
        visualizationPanelWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
}

QWidget *ImuDataHandler::create3DView() {
    auto *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f))); // Ciemnoszary kolor tła
    auto *container = QWidget::createWindowContainer(view, this);
    container->setFocusPolicy(Qt::StrongFocus); // Umożliwia interakcję z kamerą
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    auto *rootEntity = new Qt3DCore::QEntity();
    setupCameraAndController3D(view, rootEntity); // Konfiguracja kamery i kontrolera
    setupLighting3D(rootEntity);                  // Konfiguracja oświetlenia sceny
    setupModelLoader3D(rootEntity);               // Ładowanie modelu 3D

    view->setRootEntity(rootEntity);
    return container;
}

void ImuDataHandler::setupCameraAndController3D(Qt3DExtras::Qt3DWindow *view, Qt3DCore::QEntity *rootEntity) {
    auto *camera = view->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f); // Ustawienia projekcji
    camera->setPosition(QVector3D(1.5f, 1.5f, 1.5f)); // Pozycja kamery
    camera->setUpVector(QVector3D(0, 1, 0));          // Orientacja "góry" kamery
    camera->setViewCenter(QVector3D(0, 0, 0));        // Punkt, na który patrzy kamera

    auto *controller = new Qt3DExtras::QOrbitCameraController(rootEntity); // Kontroler kamery orbitalnej
    controller->setCamera(camera);
}

void ImuDataHandler::setupLighting3D(Qt3DCore::QEntity *rootEntity) {
    auto *lightEntity = new Qt3DCore::QEntity(rootEntity); // Encja dla światła
    auto *light = new Qt3DRender::QDirectionalLight(lightEntity); // Światło kierunkowe
    light->setWorldDirection(QVector3D(-1, -1, -1)); // Kierunek padania światła
    light->setColor(Qt::white);
    light->setIntensity(1.0f);
    lightEntity->addComponent(light);
}

void ImuDataHandler::setupModelLoader3D(Qt3DCore::QEntity *rootEntity) {
    auto *modelEntity = new Qt3DCore::QEntity(rootEntity); // Encja dla modelu
    auto *loader = new Qt3DRender::QSceneLoader(modelEntity);  // Komponent do ładowania sceny/modelu

    // Połączenie sygnału statusChanged, aby przetworzyć załadowany model
    connect(loader, &Qt3DRender::QSceneLoader::statusChanged, this,
            [this, loader, modelEntity](Qt3DRender::QSceneLoader::Status status) {
                if (status == Qt3DRender::QSceneLoader::Ready) {
                    const auto &childEntities = loader->entities(); // Pobranie encji ze sceny
                    if (!childEntities.isEmpty()) {
                        Qt3DCore::QEntity *sceneRootEntity = childEntities.first();
                        if (!sceneRootEntity) return;

                        // Utworzenie i przypisanie transformacji do głównej encji modelu,
                        // jeśli jeszcze nie została przypisana.
                        if (!this->boardTransform) {
                            this->boardTransform = new Qt3DCore::QTransform();
                        }
                        // Sprawdzenie, czy komponent transformacji już istnieje
                        bool alreadyHasTransform = false;
                        for(Qt3DCore::QComponent *comp : sceneRootEntity->components()){
                            if(dynamic_cast<Qt3DCore::QTransform*>(comp) == this->boardTransform){
                                alreadyHasTransform = true;
                                break;
                            }
                        }
                        if(!alreadyHasTransform){
                             // Jeśli model ma już swoją transformację, można ją pobrać
                             // Qt3DCore::QTransform *existingTransform = sceneRootEntity->property("transform").value<Qt3DCore::QTransform*>();
                             // if (existingTransform) { this->boardTransform = existingTransform; }
                             // else { sceneRootEntity->addComponent(this->boardTransform); }
                             // Bezpieczniej jest dodać naszą, jeśli chcemy nią sterować z zewnątrz.
                             // Jeśli chcemy zachować oryginalną transformację modelu i na niej operować,
                             // trzeba by ją znaleźć i przypisać do boardTransform.
                             // Na razie zakładamy, że dodajemy nową lub przejmujemy kontrolę.
                            sceneRootEntity->addComponent(this->boardTransform);
                        }
                         // Przeskalowanie i ewentualne początkowe obrócenie modelu, jeśli potrzeba
                         // this->boardTransform->setScale3D(QVector3D(0.1f, 0.1f, 0.1f)); // Przykład skalowania
                         // this->boardTransform->setRotationX(90); // Przykład obrotu
                    } else {
                        qWarning() << "SceneLoader: No entities found after loading model from" << loader->source();
                    }
                } else if (status == Qt3DRender::QSceneLoader::Error) {
                     qWarning() << "Failed to load model:" << loader->source();
                }
            });

    // Ustawienie źródła modelu (ścieżka do pliku .dae, .gltf, .obj itp.)
    // PAMIĘTAJ o zmianie ścieżki na poprawną dla Twojego systemu!
    loader->setSource(QUrl::fromLocalFile("/Users/mateuszwojtaszek/projekty/wds_Orienta/ESP32.dae"));
    // loader->setSource(QUrl("qrc:/models/ESP32.dae")); // Jeśli używasz zasobów Qt

    modelEntity->addComponent(loader); // Dodanie komponentu ładowarki do encji modelu
}


void ImuDataHandler::retranslateUi() {
    if (m_currentDataButton) {
        m_currentDataButton->setText(tr("Current Data"));
    }
    if (m_graphButton) {
        m_graphButton->setText(tr("Graph"));
    }

    // Tłumaczenie tytułów GroupBoxów dla pasków postępu
    if (m_accGroupBox) {
        m_accGroupBox->setTitle(tr("Accelerometer [mg]"));
    }
    if (m_gyroGroupBox) {
        m_gyroGroupBox->setTitle(tr("Gyroscope [dps]"));
    }
    if (m_magGroupBox) {
        m_magGroupBox->setTitle(tr("Magnetometer [mG]"));
    }

    // Tłumaczenie tytułów wykresów (jeśli SensorGraph ma taką metodę)
    if (accGraph) accGraph->retranslateUi(); // Zakładając, że SensorGraph::retranslateUi() istnieje
    if (gyroGraph) gyroGraph->retranslateUi();
    if (magGraph) magGraph->retranslateUi();

    // Compass2DRenderer nie ma metody retranslateUi, jego teksty (N,E,S,W) są stałe
    // i nie są ustawiane przez tr(). Jeśli w przyszłości Compass2DRenderer
    // będzie miał tłumaczone elementy, trzeba będzie dodać jego retranslację.
}