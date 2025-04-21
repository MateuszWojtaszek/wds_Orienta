/**
 * @file ImuDataHandler.cpp
 * @brief Implementacja klas SensorGraph i ImuDataHandler z użyciem klas QtCharts.
 * @author Mateusz Wojtaszek
 * @date 2025-03-31 (Zaktualizowano 2025-04-21)
 */

#include "ImuDataHandler.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QGroupBox> // Mimo że nie używany wprost, może być potrzebny dla zależności
#include <QDebug>
#include <QStringList>
#include <QTimer>
#include <QSizePolicy>

// Nagłówki Qt Charts
#include <QChart>
#include <QValueAxis>
#include <QLegend>
// QLineSeries i QChartView są już w ImuDataHandler.h

// Nagłówki Qt 3D
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCuboidMesh> // Dla testu kompasu
#include <Qt3DExtras/QText2DEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DCore/QComponent>

#include <QtMath>
#include <QQuaternion>

//=============================================================================
// Implementacja SensorGraph
//=============================================================================

SensorGraph::SensorGraph(const QString &title, int minY, int maxY, QWidget *parent)
    : QChartView(new QChart(), parent), sampleCount(1000), currentSampleIndex(0)
{
    QChart *chartPtr = this->chart();
    if (!chartPtr) { qWarning() << "Failed to get chart object!"; return; }
    chartPtr->setTitle(title);
    QLegend *legend = chartPtr->legend();
    if (legend) {
        legend->setVisible(true); legend->setAlignment(Qt::AlignTop);
        legend->setMarkerShape(QLegend::MarkerShapeCircle);
    }

    QStringList axisNames = {"X", "Y", "Z"};
    QList<QColor> colors = {Qt::blue, Qt::red, Qt::green};
    for (int i = 0; i < 3; ++i) {
        auto *series = new QLineSeries();
        series->setName(axisNames[i]); series->setColor(colors[i]);
        chartPtr->addSeries(series); seriesList.append(series);
    }

    auto xAxis = new QValueAxis();
    xAxis->setTitleText("Sample Index"); xAxis->setTickCount(11); xAxis->setLabelFormat("%lld");
    chartPtr->setAxisX(xAxis);

    auto yAxis = new QValueAxis();
    yAxis->setTitleText("Value"); yAxis->setRange(minY, maxY);
    chartPtr->setAxisY(yAxis);

    for (QLineSeries *series : seriesList) {
        if (series) { chartPtr->setAxisX(xAxis, series); chartPtr->setAxisY(yAxis, series); }
    }
    xAxis->setRange(0, sampleCount > 0 ? sampleCount - 1 : 0);

    // Ustawienie polityki rozmiaru dla kontroli wysokości wykresu
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
}

void SensorGraph::addData(const QVector<int> &data) {
    QChart *chartPtr = this->chart();
    // Podstawowe sprawdzenia na początku
    if (data.size() != 3 || seriesList.size() != 3 || !chartPtr) return;

    // Dodaj nowe punkty do każdej serii
    for (int i = 0; i < 3; ++i) {
        QLineSeries *series = seriesList.at(i);
        if (!series) continue;
        series->append(currentSampleIndex, data[i]); // Dodaj punkt z aktualnym indeksem

        // Usuń najstarszy punkt, jeśli seria przekracza limit
        // (Lepiej robić to przed dodaniem nowego, jeśli chcemy ścisły limit,
        // ale obecna logika też jest OK - seria chwilowo ma sampleCount+1 punktów)
        if (series->count() > sampleCount) {
            series->remove(0);
        }
    }

    // ***** POCZĄTEK OPTYMALIZACJI *****
    // Aktualizuj zakres osi X tylko co określoną liczbę próbek (np. co 10)
    // lub od razu na początku (gdy currentSampleIndex = 0, ale append jest po nim, więc index = 1)
    const int xAxisUpdateFrequency = 10; // Aktualizuj co 10 próbek
    bool isFirstSample = (currentSampleIndex == 1); // Sprawdź, czy to pierwszy dodany punkt

    if (currentSampleIndex % xAxisUpdateFrequency == 0 || isFirstSample) {
        if (auto *xAxis = qobject_cast<QValueAxis*>(chartPtr->axisX())) {
            // Oblicz minimalny X na podstawie bieżącego indeksu i liczby próbek w oknie
            // Używamy currentSampleIndex - 1, bo to jest indeks ostatnio dodanego punktu
            qint64 lastIndex = currentSampleIndex - 1;
            qint64 minX = (lastIndex >= sampleCount) ? (lastIndex - sampleCount + 1) : 0;
            qint64 maxX = lastIndex; // Ostatni dodany indeks

            // Upewnij się, że minX nie jest większy niż maxX
            if (minX > maxX) {
                 minX = maxX > 0 ? maxX : 0; // Jeśli max jest 0 lub mniej, min też powinien być 0
            }
            // Minimalny zakres, jeśli nie ma jeszcze danych lub jest ich mało
            if (maxX < 0) {
                minX = 0;
                maxX = sampleCount > 0 ? sampleCount -1 : 0; // Pokaż pusty zakres na początku
            }

            xAxis->setRange(minX, maxX);
        }
    }
    // ***** KONIEC OPTYMALIZACJI *****

    currentSampleIndex++; // Zwiększ globalny indeks próbki
}

void SensorGraph::setSampleCount(int count) {
    sampleCount = qMax(10, count);
    QChart *chartPtr = this->chart();

    for (QLineSeries *series : seriesList) {
        if (series) { while (series->count() > sampleCount) { series->remove(0); } }
    }

    if (chartPtr) {
        if (auto *xAxis = qobject_cast<QValueAxis*>(chartPtr->axisX())) {
             qint64 minX = 0; qint64 maxX = 0;
             if(!seriesList.isEmpty() && seriesList.first() && seriesList.first()->count() > 0){
                 const auto& points = seriesList.first()->points();
                 if (!points.isEmpty()) {
                     minX = static_cast<qint64>(points.first().x());
                     maxX = static_cast<qint64>(points.last().x());
                 }
             } else { maxX = (sampleCount > 0) ? (sampleCount - 1) : 0; }
             if (maxX >= minX) { xAxis->setRange(minX, maxX); }
             else { xAxis->setRange(0, sampleCount > 0 ? sampleCount - 1 : 0); }
        }
    }
}

void SensorGraph::setYRange(int minY, int maxY) {
    QChart *chartPtr = this->chart();
    if (chartPtr) {
        if (auto *yAxis = qobject_cast<QValueAxis*>(chartPtr->axisY())) {
            yAxis->setRange(minY, maxY);
        }
    }
}

//=============================================================================
// Implementacja ImuDataHandler
//=============================================================================

ImuDataHandler::ImuDataHandler(QWidget *parent)
    : QWidget(parent),
      currentSampleCount(1000)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *currentDataButton = new QPushButton(tr("Current Data"), this);
    QPushButton *graphButton = new QPushButton(tr("Graph"), this);
    buttonLayout->addWidget(currentDataButton); buttonLayout->addWidget(graphButton);

    QHBoxLayout *contentLayout = new QHBoxLayout();

    // --- Lewy Panel (z QStackedWidget) ---
    QWidget *leftPanelWidget = new QWidget();
    QVBoxLayout *leftPanelLayout = new QVBoxLayout(leftPanelWidget);
    leftPanelLayout->setContentsMargins(0,0,0,0);

    // -- Widok Pasków (Oryginalna implementacja) --
    QWidget *barWidget = new QWidget(); // Użyj this jako rodzica, jeśli ma być zarządzany przez ImuDataHandler
    QVBoxLayout *barLayout = new QVBoxLayout(barWidget);

    // Lambda DOKŁADNIE jak w Twojej "starej wersji"
    auto addBarGroup = [&](const QString &title, QProgressBar *&x, QProgressBar *&y, QProgressBar *&z, int range) {
        // Nie używamy QGroupBox
        barLayout->addWidget(new QLabel(title));
        auto createBar = [&](const QString &axis, QProgressBar *&bar) {
            QLabel *axisLabel = new QLabel(axis);
            // Ustawienie rodzica paska na 'this' (ImuDataHandler) może być problematyczne,
            // lepiej ustawić 'barWidget' lub zostawić bez rodzica (layout zarządza)
            bar = new QProgressBar(/*this*/); // Usunięto 'this'
            bar->setRange(-range, range);
            bar->setTextVisible(false); // Tak było w oryginale
            bar->setMinimumWidth(200); // Tak było w oryginale

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
            row->addLayout(barWithLabels); // Bez stretch factor
            barLayout->addLayout(row);
        };

        createBar("X:", x);
        createBar("Y:", y);
        createBar("Z:", z);
    };

    addBarGroup("Accelerometer [mg]", accXBar, accYBar, accZBar, 4000);
    addBarGroup("Gyroscope [dps]", gyroXBar, gyroYBar, gyroZBar, 250);
    addBarGroup("Magnetometer [mG]", magXBar, magYBar, magZBar, 1600);
    // barLayout->addStretch(); // Usunięto stretch, aby zachować oryginalny wygląd? Dostosuj w razie potrzeby.

    // -- Widok Wykresów --
    QWidget *graphWidget = new QWidget();
    QVBoxLayout *graphLayout = new QVBoxLayout(graphWidget);
    graphLayout->setContentsMargins(5, 5, 5, 5); // Zachowano marginesy dla wykresów
    accGraph = new SensorGraph("Accelerometer [mg]", -4000, 4000, this);
    gyroGraph = new SensorGraph("Gyroscope [dps]", -250, 250, this);
    magGraph = new SensorGraph("Magnetometer [mG]", -1600, 1600, this);
    setSampleCount(this->currentSampleCount);
    graphLayout->addWidget(accGraph); graphLayout->addWidget(gyroGraph); graphLayout->addWidget(magGraph);

    // StackedWidget dla lewego panelu
    stackedWidget = new QStackedWidget();
    stackedWidget->addWidget(barWidget);   // Indeks 0: Paski
    stackedWidget->addWidget(graphWidget); // Indeks 1: Wykresy
    leftPanelLayout->addWidget(stackedWidget);

    // --- Prawy Panel (Stały: Wizualizacja 3D + Kompas) ---
    setupVisualizationPanel(); // Utwórz prawy panel

    // --- Łączenie layoutów ---
    contentLayout->addWidget(leftPanelWidget, 1); // Lewy panel
    if (visualizationPanelWidget) {
        contentLayout->addWidget(visualizationPanelWidget, 1); // Prawy panel
    } else { qWarning() << "Visualization panel widget is null!"; }

    mainLayout->addLayout(buttonLayout); // Przyciski na górze
    mainLayout->addLayout(contentLayout); // Główna treść (lewy + prawy)

    stackedWidget->setCurrentIndex(0); // Pokaż widok pasków na starcie

    connect(currentDataButton, &QPushButton::clicked, this, &ImuDataHandler::showCurrentData);
    connect(graphButton, &QPushButton::clicked, this, &ImuDataHandler::showGraph);
}

/**
 * @brief Inicjalizuje wspólny panel wizualizacji (3D + Kompas).
 */
void ImuDataHandler::setupVisualizationPanel() {
    if (!view3DContainerWidget) { view3DContainerWidget = create3DView(); }
    if (!compassContainerWidget) { compassContainerWidget = createCompassView(); }
    if (!visualizationPanelWidget) {
        visualizationPanelWidget = new QWidget();
        QVBoxLayout *visualizationLayout = new QVBoxLayout(visualizationPanelWidget);
        visualizationLayout->setSpacing(0); visualizationLayout->setContentsMargins(0, 0, 0, 0);
        // Zachowano oryginalne proporcje 3:1 z Twojego kodu
        if (view3DContainerWidget) visualizationLayout->addWidget(view3DContainerWidget, 3);
        if (compassContainerWidget) visualizationLayout->addWidget(compassContainerWidget, 1);
    }
}

// createSensorBarGroup jest teraz lambdą w konstruktorze

/**
 * @brief Tworzy widżet kontenera dla widoku 3D płytki.
 */
QWidget* ImuDataHandler::create3DView() {
    auto *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    auto *container = QWidget::createWindowContainer(view);
    container->setMinimumSize(QSize(300, 200)); container->setFocusPolicy(Qt::StrongFocus);

    auto *rootEntity = new Qt3DCore::QEntity();
    auto *camera = view->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    // Zachowujemy oryginalną pozycję kamery z Twojego kodu
    camera->setPosition(QVector3D(1.5, 1.5, 1.5));
    camera->setUpVector(QVector3D(0, 1, 0)); camera->setViewCenter(QVector3D(0, 0, 0));

    auto *controller = new Qt3DExtras::QOrbitCameraController(rootEntity);
    controller->setCamera(camera); // Usunięto ustawienie prędkości, aby zachować oryginał

    auto *lightEntity = new Qt3DCore::QEntity(rootEntity);
    auto *light = new Qt3DRender::QDirectionalLight(lightEntity);
    light->setWorldDirection(QVector3D(-1, -1, -1)); // Oryginalny kierunek światła
    lightEntity->addComponent(light);

    auto *modelEntity = new Qt3DCore::QEntity(rootEntity);
    auto *loader = new Qt3DRender::QSceneLoader(modelEntity);

    connect(loader, &Qt3DRender::QSceneLoader::statusChanged, this,
            [this, loader, modelEntity](Qt3DRender::QSceneLoader::Status status) {
        if (status == Qt3DRender::QSceneLoader::Ready) {
            const auto &childEntities = loader->entities();
            if (!childEntities.isEmpty()) {
                Qt3DCore::QEntity *sceneRootEntity = childEntities.first();
                if (!sceneRootEntity) return;
                // Używamy this->boardTransform, inicjalizujemy jeśli null
                if (!this->boardTransform) { this->boardTransform = new Qt3DCore::QTransform(); qDebug() << "BoardTransform created."; }
                bool alreadyHasTransform = false;
                for(Qt3DCore::QComponent *comp : sceneRootEntity->components()){ if(comp == this->boardTransform) { alreadyHasTransform = true; break; } }
                if(!alreadyHasTransform) { qDebug() << "Attaching boardTransform."; sceneRootEntity->addComponent(this->boardTransform); }
            } else { qWarning() << "SceneLoader loaded successfully, but no entities found."; }
        } else if (status == Qt3DRender::QSceneLoader::Error) {
             qWarning() << "Failed to load model (status Error). Source:" << loader->source();
        }
    });

    loader->setSource(QUrl::fromLocalFile("/Users/mateuszwojtaszek/projekty/wds_Orienta/ESP32.dae")); // Popraw ścieżkę!
    modelEntity->addComponent(loader);
    view->setRootEntity(rootEntity);
    return container;
}

/**
 * @brief Tworzy widżet kontenera dla widoku kompasu (wersja testowa).
 */
QWidget* ImuDataHandler::createCompassView() {
    auto *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    auto *container = QWidget::createWindowContainer(view);
    // Zachowano oryginalny minimumSize
    container->setMinimumSize(QSize(500, 500));
    container->setFocusPolicy(Qt::NoFocus);

    auto *rootEntity = new Qt3DCore::QEntity();
    auto *camera = view->camera();
    // Zachowano oryginalną kamerę perspektywiczną
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0, 0, 45));
    camera->setUpVector(QVector3D(0, 1, 0)); camera->setViewCenter(QVector3D(0, 0, 0));

    // Zachowano OrbitController z Twojego kodu
    auto *controller = new Qt3DExtras::QOrbitCameraController(rootEntity);
    controller->setCamera(camera);

    auto *lightEntity = new Qt3DCore::QEntity(rootEntity);
    auto *light = new Qt3DRender::QDirectionalLight(lightEntity);
    light->setWorldDirection(QVector3D(-1, -1, -1)); // Oryginalny kierunek
    lightEntity->addComponent(light);

    // Tarcza kompasu (jak w oryginale)
    auto *ringEntity = new Qt3DCore::QEntity(rootEntity);
    auto *ringMesh = new Qt3DExtras::QTorusMesh();
    ringMesh->setRadius(6.5f); ringMesh->setMinorRadius(0.15f);
    auto *ringMaterial = new Qt3DExtras::QPhongMaterial();
    ringMaterial->setDiffuse(QColor("#44aaff"));
    ringEntity->addComponent(ringMesh); ringEntity->addComponent(ringMaterial);

    // Testowy Prostopadłościan zamiast stożka
    auto *arrowEntity = new Qt3DCore::QEntity(rootEntity);
    auto *cuboidMesh = new Qt3DExtras::QCuboidMesh();
    cuboidMesh->setXExtent(1.0f); cuboidMesh->setYExtent(6.0f); cuboidMesh->setZExtent(0.5f);
    auto *arrowMaterial = new Qt3DExtras::QPhongMaterial();
    arrowMaterial->setAmbient(Qt::darkYellow); arrowMaterial->setDiffuse(Qt::yellow);

    // Używamy this->compassTransform, inicjalizujemy jeśli null
    if (!this->compassTransform) {
        this->compassTransform = new Qt3DCore::QTransform();
        qDebug() << "CompassTransform created.";
        // Bez rotacji początkowej dla testu
    }
    arrowEntity->addComponent(cuboidMesh);
    arrowEntity->addComponent(arrowMaterial);
    arrowEntity->addComponent(this->compassTransform); // Używamy składowej klasy

    // Etykiety N, E, S, W (poprawiona logika dodawania, zachowano oryginalny wygląd)
    auto addLabel = [&](const QString &text, const QVector3D &pos) {
        auto *label = new Qt3DExtras::QText2DEntity(rootEntity);
        label->setFont(QFont("Arial", 2, QFont::Bold)); // Oryginalny rozmiar
        label->setText(text);
        label->setHeight(5); // Oryginalny rozmiar
        label->setWidth(5);  // Oryginalny rozmiar
        label->setColor(Qt::white);
        auto *t = new Qt3DCore::QTransform();
        t->setTranslation(pos);
        label->addComponent(t); // Dodaj komponent do encji etykiety
    };
    // Oryginalne pozycje etykiet
    addLabel("N", QVector3D(-1, 4, 0)); addLabel("E", QVector3D(7, -4, 0));
    addLabel("S", QVector3D(-1, -12, 5)); addLabel("W", QVector3D(-9, -4, 0));

    view->setRootEntity(rootEntity);
    return container;
}

/**
 * @brief Aktualizuje dane wyświetlane w UI.
 */
void ImuDataHandler::updateData(const QVector<int> &acc, const QVector<int> &gyro, const QVector<int> &mag) {
    // Paski - bez zmian
    if (acc.size() == 3) { if(accXBar) accXBar->setValue(acc[0]); if(accYBar) accYBar->setValue(acc[1]); if(accZBar) accZBar->setValue(acc[2]); }
    if (gyro.size() == 3) { if(gyroXBar) gyroXBar->setValue(gyro[0]); if(gyroYBar) gyroYBar->setValue(gyro[1]); if(gyroZBar) gyroZBar->setValue(gyro[2]); }
    if (mag.size() == 3) { if(magXBar) magXBar->setValue(mag[0]); if(magYBar) magYBar->setValue(mag[1]); if(magZBar) magZBar->setValue(mag[2]); }

    // Wykresy - aktualizacja danych
    if(accGraph && acc.size()==3) accGraph->addData(acc);
    if(gyroGraph && gyro.size()==3) gyroGraph->addData(gyro);
    if(magGraph && mag.size()==3) magGraph->addData(mag);

    // Usunięto aktualizację kompasu stąd
}

/**
 * @brief Ustawia liczbę próbek dla wykresów.
 */
void ImuDataHandler::setSampleCount(int samples) {
    currentSampleCount = qMax(10, samples); // Używamy nowej nazwy zmiennej
    if(accGraph) accGraph->setSampleCount(currentSampleCount);
    if(gyroGraph) gyroGraph->setSampleCount(currentSampleCount);
    if(magGraph) magGraph->setSampleCount(currentSampleCount);
}

/**
 * @brief Ustawia zakres wartości dla pasków i osi Y wykresów.
 */
void ImuDataHandler::setRange(int minVal, int maxVal) {
     // Zachowujemy oryginalne, sztywne wartości zakresów z Twojego kodu
     // Paski
     if(accXBar) accXBar->setRange(-4000, 4000); if(accYBar) accYBar->setRange(-4000, 4000); if(accZBar) accZBar->setRange(-4000, 4000);
     if(gyroXBar) gyroXBar->setRange(-250, 250); if(gyroYBar) gyroYBar->setRange(-250, 250); if(gyroZBar) gyroZBar->setRange(-250, 250);
     if(magXBar) magXBar->setRange(-1600, 1600); if(magYBar) magYBar->setRange(-1600, 1600); if(magZBar) magZBar->setRange(-1600, 1600);
     // Wykresy (Oś Y) - użyjemy tych samych zakresów
     if(accGraph) accGraph->setYRange(-4000, 4000);
     if(gyroGraph) gyroGraph->setYRange(-250, 250);
     if(magGraph) magGraph->setYRange(-1600, 1600);
     Q_UNUSED(minVal); // Oznacz jako nieużywane
     Q_UNUSED(maxVal);
}

/** @brief Przełącza na widok "Current Data". */
void ImuDataHandler::showCurrentData() { if(stackedWidget) stackedWidget->setCurrentIndex(0); }
/** @brief Przełącza na widok "Graph". */
void ImuDataHandler::showGraph() { if(stackedWidget) stackedWidget->setCurrentIndex(1); }

/** @brief Ustawia rotację modelu 3D płytki. */
void ImuDataHandler::setRotation(float yaw, float pitch, float roll) {
    QQuaternion rotation = QQuaternion::fromEulerAngles(pitch, yaw, roll);
    // Usunięto aktualizację kompasu stąd
    if (boardTransform) { boardTransform->setRotation(rotation); }
    else { /* qDebug() << "Board transform is null!"; */ } // Można odkomentować do debugowania
}

/** @brief Aktualizuje wskazanie kompasu (wersja testowa). */
void ImuDataHandler::updateCompass(float heading) {
    if (compassTransform) {
        // Rotacja dla testowego prostopadłościanu (wokół Z)
        QQuaternion headingRotation = QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), -heading);
        compassTransform->setRotation(headingRotation);
        // qDebug() << "Compass heading:" << heading << " Rotation:" << headingRotation; // Debug
    } else {
        // qDebug() << "Compass transform is null!";
    }
}