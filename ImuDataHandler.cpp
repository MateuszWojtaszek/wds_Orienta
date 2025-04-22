#include "ImuDataHandler.h"
#include "SensorGraph.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QWidget>
#include <QDebug>
#include <QFont>
#include <QUrl>
#include <QColor>
#include <QString>
#include <QVector3D>
#include <QtMath>
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
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QText2DEntity>

/***************************************************************************/
/**
 * @brief Konstruktor klasy ImuDataHandler.
 * Inicjalizuje UI poprzez wywołanie setupMainLayout.
 * @param parent Wskaźnik na widget nadrzędny (opcjonalny).
 */
ImuDataHandler::ImuDataHandler(QWidget *parent)
    : QWidget(parent),
      currentSampleCount(1000) // Inicjalizacja domyślnej liczby próbek
{
    setupMainLayout();
}

/***************************************************************************/
/**
 * @brief Aktualizuje dane wyświetlane przez widget.
 * Odświeża wartości pasków postępu i dodaje nowe punkty do wykresów
 * na podstawie otrzymanych danych z sensorów IMU.
 * @param acc Wektor danych z akcelerometru [X, Y, Z].
 * @param gyro Wektor danych z żyroskopu [X, Y, Z].
 * @param mag Wektor danych z magnetometru [X, Y, Z].
 */
void ImuDataHandler::updateData(const QVector<int> &acc, const QVector<int> &gyro, const QVector<int> &mag) {
    // Aktualizacja pasków i wykresu akcelerometru
    if (acc.size() == 3) {
        if(accXBar) { accXBar->setValue(acc[0]); accXBar->setFormat(QString::number(acc[0])); }
        if(accYBar) { accYBar->setValue(acc[1]); accYBar->setFormat(QString::number(acc[1])); }
        if(accZBar) { accZBar->setValue(acc[2]); accZBar->setFormat(QString::number(acc[2])); }
        if(accGraph) accGraph->addData(acc);
    }
    // Aktualizacja pasków i wykresu żyroskopu
    if (gyro.size() == 3) {
        if(gyroXBar) { gyroXBar->setValue(gyro[0]); gyroXBar->setFormat(QString::number(gyro[0])); }
        if(gyroYBar) { gyroYBar->setValue(gyro[1]); gyroYBar->setFormat(QString::number(gyro[1])); }
        if(gyroZBar) { gyroZBar->setValue(gyro[2]); gyroZBar->setFormat(QString::number(gyro[2])); }
         if(gyroGraph) gyroGraph->addData(gyro);
    }
    // Aktualizacja pasków i wykresu magnetometru
    if (mag.size() == 3) {
        if(magXBar) { magXBar->setValue(mag[0]); magXBar->setFormat(QString::number(mag[0])); }
        if(magYBar) { magYBar->setValue(mag[1]); magYBar->setFormat(QString::number(mag[1])); }
        if(magZBar) { magZBar->setValue(mag[2]); magZBar->setFormat(QString::number(mag[2])); }
        if(magGraph) magGraph->addData(mag);
    }
}

/***************************************************************************/
/**
 * @brief Ustawia liczbę próbek wyświetlanych na wykresach.
 * Aktualizuje liczbę próbek dla każdego z wykresów (akcelerometr, żyroskop, magnetometr).
 * Minimalna liczba próbek to 10.
 * @param samples Liczba próbek do wyświetlenia.
 */
void ImuDataHandler::setSampleCount(int samples) {
    currentSampleCount = qMax(10, samples); // Upewnij się, że liczba próbek nie jest mniejsza niż 10
    if(accGraph) accGraph->setSampleCount(currentSampleCount);
    if(gyroGraph) gyroGraph->setSampleCount(currentSampleCount);
    if(magGraph) magGraph->setSampleCount(currentSampleCount);
}

/***************************************************************************/
/**
 * @brief Ustawia zakres wartości dla pasków postępu i wykresów.
 * UWAGA: Aktualnie funkcja ustawia sztywne zakresy zdefiniowane wewnątrz.
 * Parametry minVal i maxVal są ignorowane. Ustawia predefiniowane zakresy
 * dla akcelerometru (-4000, 4000), żyroskopu (-250, 250) i magnetometru (-1600, 1600).
 * @param minVal Minimalna wartość zakresu (obecnie nieużywana).
 * @param maxVal Maksymalna wartość zakresu (obecnie nieużywana).
 */
void ImuDataHandler::setRange(int minVal, int maxVal) {
     // Ustawianie zakresów dla pasków postępu (hardcoded)
     if(accXBar) accXBar->setRange(-4000, 4000); if(accYBar) accYBar->setRange(-4000, 4000); if(accZBar) accZBar->setRange(-4000, 4000);
     if(gyroXBar) gyroXBar->setRange(-250, 250); if(gyroYBar) gyroYBar->setRange(-250, 250); if(gyroZBar) gyroZBar->setRange(-250, 250);
     if(magXBar) magXBar->setRange(-1600, 1600); if(magYBar) magYBar->setRange(-1600, 1600); if(magZBar) magZBar->setRange(-1600, 1600);

     // Ustawianie zakresów dla wykresów (hardcoded)
     if(accGraph) accGraph->setYRange(-4000, 4000);
     if(gyroGraph) gyroGraph->setYRange(-250, 250);
     if(magGraph) magGraph->setYRange(-1600, 1600);

     // Oznaczenie parametrów jako nieużywane, aby uniknąć ostrzeżeń kompilatora
     Q_UNUSED(minVal);
     Q_UNUSED(maxVal);
}

/***************************************************************************/
/**
 * @brief Ustawia rotację modelu 3D na podstawie kątów Eulera.
 * Tworzy kwaternion rotacji z podanych kątów i stosuje go do transformacji
 * modelu 3D (`boardTransform`).
 * @param yaw Kąt odchylenia (obrót wokół osi Z).
 * @param pitch Kąt pochylenia (obrót wokół osi X).
 * @param roll Kąt przechylenia (obrót wokół osi Y).
 */
void ImuDataHandler::setRotation(float yaw, float pitch, float roll) {
    // Tworzenie kwaternionu z kątów Eulera (pitch, yaw, roll)
    QQuaternion rotation = QQuaternion::fromEulerAngles(pitch, yaw, roll);
    // Zastosowanie rotacji do transformacji modelu, jeśli istnieje
    if (boardTransform) {
        boardTransform->setRotation(rotation);
    }
    else {
        // Opcjonalnie: logowanie, jeśli transformacja nie została jeszcze utworzona
        // qDebug() << "boardTransform is null, cannot set rotation yet.";
     }
}

/***************************************************************************/
/**
 * @brief Aktualizuje kierunek wskazywany przez strzałkę kompasu.
 * Tworzy kwaternion rotacji wokół osi Z na podstawie podanego kierunku
 * i stosuje go do transformacji strzałki kompasu (`compassTransform`).
 * @param heading Kierunek w stopniach (0 = Północ, obrót przeciwny do ruchu wskazówek zegara).
 */
void ImuDataHandler::updateCompass(float heading) {
    // Zastosowanie rotacji do transformacji kompasu, jeśli istnieje
    if (compassTransform) {
        // Tworzenie kwaternionu rotacji wokół osi Z (0,0,1) o kąt -heading
        // (ujemny, aby dopasować kierunek obrotu)
        QQuaternion headingRotation = QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), -heading);
        compassTransform->setRotation(headingRotation);
    } else {
        // Opcjonalnie: logowanie, jeśli transformacja nie została jeszcze utworzona
        // qDebug() << "compassTransform is null, cannot update compass heading.";
    }
}

/***************************************************************************/
/**
 * @brief Slot przełączający widok na zakładkę z aktualnymi danymi (paski postępu).
 * Ustawia aktywny indeks w QStackedWidget na 0.
 */
void ImuDataHandler::showCurrentData() {
    if(stackedWidget) stackedWidget->setCurrentIndex(0); // Pokaż widget z paskami (indeks 0)
}

/***************************************************************************/
/**
 * @brief Slot przełączający widok na zakładkę z wykresami.
 * Ustawia aktywny indeks w QStackedWidget na 1.
 */
void ImuDataHandler::showGraph() {
    if(stackedWidget) stackedWidget->setCurrentIndex(1); // Pokaż widget z wykresami (indeks 1)
}

/***************************************************************************/
/**
 * @brief Konfiguruje główny układ interfejsu użytkownika.
 * Tworzy panel przycisków, lewy panel z danymi (paski/wykresy),
 * panel wizualizacji (3D/kompas) i układa je w głównym oknie widgetu.
 */
void ImuDataHandler::setupMainLayout() {
    // Główny layout pionowy
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Utworzenie poszczególnych paneli
    QWidget *buttonPanel = createButtonPanel(); // Panel z przyciskami "Current Data" / "Graph"
    QWidget *leftPanel = createLeftPanel();     // Panel z QStackedWidget (paski/wykresy)

    setupVisualizationPanel(); // Konfiguracja panelu z widokami 3D

    // Layout poziomy dla treści (lewy panel + panel wizualizacji)
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->addWidget(leftPanel, 1); // Lewy panel zajmuje dostępną przestrzeń (stretch factor 1)
    if (visualizationPanelWidget) {
        contentLayout->addWidget(visualizationPanelWidget, 1); // Panel wizualizacji zajmuje dostępną przestrzeń (stretch factor 1)
    } else {
        qWarning() << "Visualization panel widget is null!"; // Ostrzeżenie, jeśli panel wizualizacji nie został utworzony
    }

    // Dodanie paneli do głównego layoutu
    mainLayout->addWidget(buttonPanel);  // Przyciski na górze
    mainLayout->addLayout(contentLayout); // Reszta treści poniżej

    // Ustawienie domyślnego widoku na paski postępu
    if (stackedWidget) stackedWidget->setCurrentIndex(0);
}

/***************************************************************************/
/**
 * @brief Tworzy panel z przyciskami do przełączania widoków ("Current Data", "Graph").
 * Łączy sygnały `clicked` przycisków z odpowiednimi slotami `showCurrentData` i `showGraph`.
 * @return Wskaźnik na utworzony widget panelu przycisków.
 */
QWidget* ImuDataHandler::createButtonPanel() {
    QWidget *buttonPanel = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonPanel);

    // Tworzenie przycisków
    QPushButton *currentDataButton = new QPushButton(tr("Current Data"));
    QPushButton *graphButton = new QPushButton(tr("Graph"));

    // Dodawanie przycisków do layoutu
    buttonLayout->addWidget(currentDataButton);
    buttonLayout->addWidget(graphButton);

    // Łączenie sygnałów `clicked` ze slotami
    connect(currentDataButton, &QPushButton::clicked, this, &ImuDataHandler::showCurrentData);
    connect(graphButton, &QPushButton::clicked, this, &ImuDataHandler::showGraph);

    return buttonPanel;
}

/***************************************************************************/
/**
 * @brief Tworzy lewy panel zawierający QStackedWidget.
 * QStackedWidget umożliwia przełączanie między widokiem pasków postępu
 * a widokiem wykresów.
 * @return Wskaźnik na utworzony widget lewego panelu.
 */
QWidget* ImuDataHandler::createLeftPanel() {
    QWidget *leftPanelWidget = new QWidget();
    QVBoxLayout *leftPanelLayout = new QVBoxLayout(leftPanelWidget);
    leftPanelLayout->setContentsMargins(0,0,0,0); // Usunięcie marginesów wewnętrznych

    // Utworzenie widgetów dla poszczególnych widoków
    QWidget *barWidget = createBarDisplayWidget();   // Widok z paskami postępu
    QWidget *graphWidget = createGraphDisplayWidget(); // Widok z wykresami

    // Utworzenie QStackedWidget i dodanie do niego widoków
    stackedWidget = new QStackedWidget();
    stackedWidget->addWidget(barWidget);   // Indeks 0
    stackedWidget->addWidget(graphWidget); // Indeks 1

    // Dodanie QStackedWidget do layoutu lewego panelu
    leftPanelLayout->addWidget(stackedWidget);

    return leftPanelWidget;
}

/***************************************************************************/
/**
 * @brief Tworzy widget wyświetlający dane IMU w postaci pasków postępu.
 * Grupuje paski dla akcelerometru, żyroskopu i magnetometru w osobnych QGroupBox.
 * @return Wskaźnik na utworzony widget z paskami postępu.
 */
QWidget* ImuDataHandler::createBarDisplayWidget() {
    QWidget *barWidget = new QWidget();
    QVBoxLayout *barLayout = new QVBoxLayout(barWidget);

    // Funkcja pomocnicza do tworzenia grupy pasków dla jednego sensora
    auto addBarGroup = [&](const QString &title, QProgressBar *&x, QProgressBar *&y, QProgressBar *&z, int range) {
        QGroupBox* groupBox = new QGroupBox(title); // Grupa dla sensora
        QVBoxLayout* groupLayout = new QVBoxLayout(groupBox); // Layout wewnątrz grupy

        // Funkcja pomocnicza do tworzenia pojedynczego paska (X, Y lub Z)
        auto createBar = [&](const QString &axis, QProgressBar *&bar) {
            QLabel *axisLabel = new QLabel(axis); // Etykieta osi (X:, Y:, Z:)
            axisLabel->setFixedWidth(20); // Stała szerokość etykiety
            bar = new QProgressBar(); // Tworzenie paska postępu
            bar->setRange(-range, range); // Ustawienie zakresu
            bar->setValue(0); // Wartość początkowa
            bar->setTextVisible(true); // Wyświetlanie tekstu na pasku
            // Format wyświetlania tekstu (początkowo pokazujący zakres)
            bar->setFormat(QString("%1 / %2").arg(QString::number(range)).arg(QString::number(-range)));

            // Dodanie etykiet z zakresem (-range, 0, range) pod paskiem
            QHBoxLayout *rangeLabels = new QHBoxLayout();
            rangeLabels->addWidget(new QLabel(QString::number(-range)));
            rangeLabels->addStretch(); // Rozciągliwa przestrzeń
            rangeLabels->addWidget(new QLabel("0"));
            rangeLabels->addStretch(); // Rozciągliwa przestrzeń
            rangeLabels->addWidget(new QLabel(QString::number(range)));

            // Layout pionowy dla paska i etykiet zakresu
            QVBoxLayout *barWithLabels = new QVBoxLayout();
            barWithLabels->addLayout(rangeLabels);
            barWithLabels->addWidget(bar);
            barWithLabels->setSpacing(0); // Brak odstępu między etykietami a paskiem

            // Layout poziomy dla etykiety osi i paska z etykietami zakresu
            QHBoxLayout *row = new QHBoxLayout();
            row->addWidget(axisLabel);
            row->addLayout(barWithLabels);
            groupLayout->addLayout(row); // Dodanie wiersza do layoutu grupy
        };

        // Utworzenie pasków dla osi X, Y, Z
        createBar("X:", x);
        createBar("Y:", y);
        createBar("Z:", z);
        barLayout->addWidget(groupBox); // Dodanie grupy do głównego layoutu pasków
    };

    // Utworzenie grup pasków dla każdego sensora
    addBarGroup("Accelerometer [mg]", accXBar, accYBar, accZBar, 4000);
    addBarGroup("Gyroscope [dps]", gyroXBar, gyroYBar, gyroZBar, 250);
    addBarGroup("Magnetometer [mG]", magXBar, magYBar, magZBar, 1600);

    return barWidget;
}

/***************************************************************************/
/**
 * @brief Tworzy widget wyświetlający dane IMU w postaci wykresów.
 * Inicjalizuje obiekty SensorGraph dla akcelerometru, żyroskopu i magnetometru.
 * @return Wskaźnik na utworzony widget z wykresami.
 */
QWidget* ImuDataHandler::createGraphDisplayWidget() {
    QWidget *graphWidget = new QWidget();
    QVBoxLayout *graphLayout = new QVBoxLayout(graphWidget);
    graphLayout->setContentsMargins(5, 5, 5, 5); // Dodanie małych marginesów

    // Tworzenie instancji SensorGraph dla każdego sensora
    accGraph = new SensorGraph("Accelerometer [mg]", -4000, 4000, this);
    gyroGraph = new SensorGraph("Gyroscope [dps]", -250, 250, this);
    magGraph = new SensorGraph("Magnetometer [mG]", -1600, 1600, this);

    // Ustawienie początkowej liczby próbek na wykresach
    setSampleCount(this->currentSampleCount);

    // Dodanie wykresów do layoutu
    graphLayout->addWidget(accGraph);
    graphLayout->addWidget(gyroGraph);
    graphLayout->addWidget(magGraph);

    return graphWidget;
}

/***************************************************************************/
/**
 * @brief Konfiguruje panel wizualizacji, tworząc widoki 3D i kompasu.
 * Jeśli widoki nie zostały jeszcze utworzone, wywołuje odpowiednie funkcje
 * `create3DView` i `createCompassView`. Następnie umieszcza je w głównym
 * widgecie panelu wizualizacji (`visualizationPanelWidget`).
 */
void ImuDataHandler::setupVisualizationPanel() {
    // Utworzenie kontenera dla widoku 3D, jeśli jeszcze nie istnieje
    if (!view3DContainerWidget) {
        view3DContainerWidget = create3DView();
    }
    // Utworzenie kontenera dla widoku kompasu, jeśli jeszcze nie istnieje
    if (!compassContainerWidget) {
        compassContainerWidget = createCompassView();
    }

    // Utworzenie głównego widgetu panelu wizualizacji, jeśli jeszcze nie istnieje
    if (!visualizationPanelWidget) {
        visualizationPanelWidget = new QWidget();
        QVBoxLayout *visualizationLayout = new QVBoxLayout(visualizationPanelWidget);
        visualizationLayout->setSpacing(0); // Brak odstępów między widokami
        visualizationLayout->setContentsMargins(0, 0, 0, 0); // Brak marginesów
        // Dodanie utworzonych kontenerów do layoutu
        if (view3DContainerWidget) visualizationLayout->addWidget(view3DContainerWidget);
        if (compassContainerWidget) visualizationLayout->addWidget(compassContainerWidget);
    }
}

/***************************************************************************/
/**
 * @brief Tworzy widok 3D do wyświetlania modelu urządzenia.
 * Inicjalizuje okno Qt3DWindow, ustawia jego tło, tworzy główny byt sceny,
 * konfiguruje kamerę, oświetlenie i ładowanie modelu.
 * @return Wskaźnik na kontener QWidget (`QWidget::createWindowContainer`)
 * zawierający okno Qt3DWindow dla modelu 3D.
 */
QWidget* ImuDataHandler::create3DView() {
    // Tworzenie okna Qt3D
    auto *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f))); // Ustawienie koloru tła (ciemnoszary)

    // Tworzenie kontenera QWidget dla okna Qt3D, aby można było je umieścić w layoucie Qt Widgets
    auto *container = QWidget::createWindowContainer(view);
    container->setMinimumSize(QSize(300, 200)); // Minimalny rozmiar widoku 3D
    container->setFocusPolicy(Qt::StrongFocus); // Umożliwia interakcję z widokiem (np. kontrolerem kamery)

    // Tworzenie głównego bytu (root entity) dla sceny 3D
    auto *rootEntity = new Qt3DCore::QEntity();

    // Konfiguracja kamery, kontrolera, oświetlenia i modelu
    setupCameraAndController3D(view, rootEntity);
    setupLighting3D(rootEntity);
    setupModelLoader3D(rootEntity); // Konfiguracja ładowania modelu

    // Ustawienie głównego bytu dla okna widoku
    view->setRootEntity(rootEntity);
    return container; // Zwrócenie kontenera QWidget
}

/***************************************************************************/
/**
 * @brief Konfiguruje kamerę i kontroler orbity dla widoku modelu 3D.
 * Ustawia projekcję perspektywiczną, pozycję i kierunek kamery oraz
 * tworzy kontroler QOrbitCameraController umożliwiający obracanie widoku myszą.
 * @param view Wskaźnik na okno Qt3DWindow dla modelu 3D.
 * @param rootEntity Wskaźnik na główny byt sceny 3D, do którego zostanie dodany kontroler.
 */
void ImuDataHandler::setupCameraAndController3D(Qt3DExtras::Qt3DWindow* view, Qt3DCore::QEntity* rootEntity) {
    // Pobranie obiektu kamery z widoku
    auto *camera = view->camera();
    // Ustawienie projekcji perspektywicznej (FOV, aspect ratio, near/far plane)
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    // Ustawienie pozycji kamery
    camera->setPosition(QVector3D(1.5, 1.5, 1.5));
    // Ustawienie wektora "góry" dla kamery (oś Y)
    camera->setUpVector(QVector3D(0, 1, 0));
    // Ustawienie punktu, na który patrzy kamera (środek sceny)
    camera->setViewCenter(QVector3D(0, 0, 0));

    // Tworzenie kontrolera kamery typu "orbita"
    auto *controller = new Qt3DExtras::QOrbitCameraController(rootEntity);
    // Powiązanie kontrolera z kamerą
    controller->setCamera(camera);
}

/***************************************************************************/
/**
 * @brief Konfiguruje oświetlenie kierunkowe dla sceny modelu 3D.
 * Dodaje do sceny byt reprezentujący źródło światła kierunkowego.
 * @param rootEntity Wskaźnik na główny byt sceny 3D, do którego zostanie dodane światło.
 */
void ImuDataHandler::setupLighting3D(Qt3DCore::QEntity* rootEntity) {
    // Tworzenie bytu dla światła
    auto *lightEntity = new Qt3DCore::QEntity(rootEntity);
    // Tworzenie komponentu światła kierunkowego
    auto *light = new Qt3DRender::QDirectionalLight(lightEntity);
    // Ustawienie kierunku padania światła
    light->setWorldDirection(QVector3D(-1, -1, -1));
    // Dodanie komponentu światła do bytu światła
    lightEntity->addComponent(light);
}

/***************************************************************************/
/**
 * @brief Konfiguruje ładowanie modelu 3D z pliku (np. DAE).
 * Tworzy byt dla modelu i komponent QSceneLoader. Łączy sygnał statusu ładowania,
 * aby po pomyślnym załadowaniu przypisać transformację `boardTransform` do
 * głównego bytu załadowanej sceny.
 * @param rootEntity Wskaźnik na główny byt sceny 3D, do którego zostanie dodany model.
 */
void ImuDataHandler::setupModelLoader3D(Qt3DCore::QEntity* rootEntity) {
    // Tworzenie bytu, który będzie przechowywał ładowany model
    auto *modelEntity = new Qt3DCore::QEntity(rootEntity);
    // Tworzenie komponentu do ładowania sceny 3D
    auto *loader = new Qt3DRender::QSceneLoader(modelEntity);

    // Połączenie sygnału zmiany statusu ładowarki
    connect(loader, &Qt3DRender::QSceneLoader::statusChanged, this,
            [this, loader, modelEntity](Qt3DRender::QSceneLoader::Status status) {
        // Sprawdzenie, czy ładowanie zakończyło się sukcesem (Ready)
        if (status == Qt3DRender::QSceneLoader::Ready) {
            // Pobranie listy bytów załadowanych przez loader
            const auto &childEntities = loader->entities();
            if (!childEntities.isEmpty()) {
                // Pobranie pierwszego (zazwyczaj głównego) bytu załadowanej sceny
                Qt3DCore::QEntity *sceneRootEntity = childEntities.first();
                if (!sceneRootEntity) return; // Zabezpieczenie, jeśli byt jest nullem

                // Utworzenie transformacji dla płytki, jeśli jeszcze nie istnieje
                if (!this->boardTransform) {
                    this->boardTransform = new Qt3DCore::QTransform();
                    qDebug() << "BoardTransform created.";
                }

                // Sprawdzenie, czy transformacja nie jest już dodana (na wszelki wypadek)
                bool alreadyHasTransform = false;
                for(Qt3DCore::QComponent *comp : sceneRootEntity->components()) {
                    if(comp == this->boardTransform) {
                        alreadyHasTransform = true;
                        break;
                    }
                }
                // Dodanie transformacji do głównego bytu załadowanej sceny, jeśli jeszcze jej nie ma
                if(!alreadyHasTransform) {
                    qDebug() << "Attaching boardTransform.";
                    sceneRootEntity->addComponent(this->boardTransform);
                }
            } else {
                qWarning() << "SceneLoader loaded successfully, but no entities found.";
            }
        // Sprawdzenie, czy wystąpił błąd podczas ładowania
        } else if (status == Qt3DRender::QSceneLoader::Error) {
             qWarning() << "Failed to load model (status Error). Source:" << loader->source();
        }
    });

    // Ustawienie źródła modelu 3D (ścieżka do pliku)
    // WAŻNE: Upewnij się, że ta ścieżka jest poprawna w Twoim systemie!
    loader->setSource(QUrl::fromLocalFile("/Users/mateuszwojtaszek/projekty/wds_Orienta/ESP32.dae"));
    // Dodanie komponentu loadera do bytu modelu
    modelEntity->addComponent(loader);
}

/***************************************************************************/
/**
 * @brief Tworzy widok 3D dla kompasu.
 * Inicjalizuje okno Qt3DWindow, ustawia tło, tworzy główny byt sceny,
 * konfiguruje kamerę, oświetlenie oraz tworzy elementy wizualne kompasu
 * (pierścień, strzałkę, etykiety).
 * @return Wskaźnik na kontener QWidget (`QWidget::createWindowContainer`)
 * zawierający okno Qt3DWindow dla kompasu.
 */
QWidget* ImuDataHandler::createCompassView() {
    // Tworzenie okna Qt3D
    auto *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f))); // Ustawienie koloru tła

    // Tworzenie kontenera QWidget dla okna Qt3D
    auto *container = QWidget::createWindowContainer(view);
    container->setMinimumSize(QSize(500, 500)); // Minimalny rozmiar widoku kompasu
    container->setFocusPolicy(Qt::NoFocus); // Wyłączenie fokusu, aby nie przechwytywał zdarzeń myszy przeznaczonych dla modelu 3D

    // Tworzenie głównego bytu sceny kompasu
    auto *rootEntity = new Qt3DCore::QEntity();

    // Konfiguracja kamery, kontrolera i oświetlenia dla kompasu
    setupCameraAndControllerCompass(view, rootEntity);
    setupLightingCompass(rootEntity);

    // Tworzenie elementów graficznych kompasu
    createCompassRing(rootEntity);  // Pierścień kompasu
    createCompassArrow(rootEntity); // Strzałka kompasu
    addCompassLabels(rootEntity);   // Etykiety kierunków

    // Ustawienie głównego bytu dla okna widoku kompasu
    view->setRootEntity(rootEntity);
    return container; // Zwrócenie kontenera QWidget
}

/***************************************************************************/
/**
 * @brief Konfiguruje kamerę i kontroler orbity dla widoku kompasu.
 * Ustawia kamerę w widoku z góry (nad kompasem) i tworzy kontroler
 * QOrbitCameraController (choć interakcja z nim jest wyłączona przez NoFocus).
 * @param view Wskaźnik na okno Qt3DWindow dla kompasu.
 * @param rootEntity Wskaźnik na główny byt sceny kompasu.
 */
void ImuDataHandler::setupCameraAndControllerCompass(Qt3DExtras::Qt3DWindow* view, Qt3DCore::QEntity* rootEntity) {
    // Pobranie obiektu kamery
    auto *camera = view->camera();
    // Ustawienie projekcji perspektywicznej
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    // Ustawienie pozycji kamery - wysoko nad sceną (patrzy w dół)
    camera->setPosition(QVector3D(0, 0, 45));
    // Ustawienie wektora "góry" (oś Y)
    camera->setUpVector(QVector3D(0, 1, 0));
    // Ustawienie punktu centralnego widoku (środek kompasu)
    camera->setViewCenter(QVector3D(0, 0, 0));

    // Tworzenie kontrolera kamery (mimo NoFocus, jest potrzebny dla struktury sceny)
    auto *controller = new Qt3DExtras::QOrbitCameraController(rootEntity);
    controller->setCamera(camera);
}

/***************************************************************************/
/**
 * @brief Konfiguruje oświetlenie kierunkowe dla sceny kompasu.
 * Dodaje do sceny byt reprezentujący źródło światła kierunkowego.
 * @param rootEntity Wskaźnik na główny byt sceny kompasu.
 */
void ImuDataHandler::setupLightingCompass(Qt3DCore::QEntity* rootEntity) {
    // Tworzenie bytu dla światła
    auto *lightEntity = new Qt3DCore::QEntity(rootEntity);
    // Tworzenie komponentu światła kierunkowego
    auto *light = new Qt3DRender::QDirectionalLight(lightEntity);
    // Ustawienie kierunku padania światła
    light->setWorldDirection(QVector3D(-1, -1, -1));
    // Dodanie komponentu światła do bytu
    lightEntity->addComponent(light);
}

/***************************************************************************/
/**
 * @brief Tworzy pierścień kompasu jako element sceny 3D.
 * Wykorzystuje siatkę QTorusMesh i materiał QPhongMaterial.
 * @param parentEntity Wskaźnik na byt nadrzędny, do którego zostanie dodany pierścień.
 * @return Wskaźnik na utworzony byt pierścienia kompasu.
 */
Qt3DCore::QEntity* ImuDataHandler::createCompassRing(Qt3DCore::QEntity* parentEntity) {
    // Tworzenie bytu dla pierścienia
    auto *ringEntity = new Qt3DCore::QEntity(parentEntity);
    // Tworzenie siatki w kształcie torusa (pierścienia)
    auto *ringMesh = new Qt3DExtras::QTorusMesh();
    ringMesh->setRadius(6.5f); // Główny promień torusa
    ringMesh->setMinorRadius(0.15f); // Promień przekroju torusa
    // Tworzenie materiału dla pierścienia
    auto *ringMaterial = new Qt3DExtras::QPhongMaterial();
    ringMaterial->setDiffuse(QColor("#44aaff")); // Kolor materiału (jasnoniebieski)
    // Dodanie komponentów siatki i materiału do bytu pierścienia
    ringEntity->addComponent(ringMesh);
    ringEntity->addComponent(ringMaterial);
    return ringEntity;
}

/***************************************************************************/
/**
 * @brief Tworzy strzałkę kompasu jako element sceny 3D.
 * Wykorzystuje siatkę QCuboidMesh (prostopadłościan) i materiał QPhongMaterial.
 * Przypisuje do strzałki transformację `compassTransform`, która będzie używana
 * do obracania strzałki zgodnie z kierunkiem.
 * @param parentEntity Wskaźnik na byt nadrzędny, do którego zostanie dodana strzałka.
 * @return Wskaźnik na utworzony byt strzałki kompasu.
 */
Qt3DCore::QEntity* ImuDataHandler::createCompassArrow(Qt3DCore::QEntity* parentEntity) {
    // Tworzenie bytu dla strzałki
    auto *arrowEntity = new Qt3DCore::QEntity(parentEntity);
    // Tworzenie siatki w kształcie prostopadłościanu (wydłużonego, aby przypominał strzałkę)
    auto *cuboidMesh = new Qt3DExtras::QCuboidMesh();
    cuboidMesh->setXExtent(1.0f); // Szerokość
    cuboidMesh->setYExtent(6.0f); // Długość (wysokość w osi Y)
    cuboidMesh->setZExtent(0.5f); // Grubość
    // Tworzenie materiału dla strzałki
    auto *arrowMaterial = new Qt3DExtras::QPhongMaterial();
    arrowMaterial->setAmbient(Qt::darkYellow); // Kolor otoczenia
    arrowMaterial->setDiffuse(Qt::yellow);     // Główny kolor

    // Utworzenie transformacji dla kompasu, jeśli jeszcze nie istnieje
    if (!this->compassTransform) {
        this->compassTransform = new Qt3DCore::QTransform();
        qDebug() << "CompassTransform created.";
    }
    // Dodanie komponentów siatki, materiału i transformacji do bytu strzałki
    arrowEntity->addComponent(cuboidMesh);
    arrowEntity->addComponent(arrowMaterial);
    arrowEntity->addComponent(this->compassTransform); // Kluczowe - ta transformacja będzie obracana
    return arrowEntity;
}

/***************************************************************************/
/**
 * @brief Dodaje etykiety kierunków (N, E, S, W) jako tekst 2D do sceny kompasu.
 * Wykorzystuje QText2DEntity do wyświetlenia liter w odpowiednich miejscach wokół pierścienia.
 * @param parentEntity Wskaźnik na byt nadrzędny, do którego zostaną dodane etykiety.
 */
void ImuDataHandler::addCompassLabels(Qt3DCore::QEntity* parentEntity) {
    // Funkcja pomocnicza do tworzenia pojedynczej etykiety tekstowej 2D
    auto addLabel = [&](const QString &text, const QVector3D &pos) {
        // Tworzenie bytu dla etykiety tekstowej 2D
        auto *label = new Qt3DExtras::QText2DEntity(parentEntity);
        label->setFont(QFont("Arial", 2, QFont::Bold)); // Ustawienie czcionki
        label->setText(text); // Tekst etykiety (N, E, S, W)
        label->setHeight(5); // Wysokość etykiety w jednostkach sceny
        label->setWidth(5);  // Szerokość etykiety w jednostkach sceny
        label->setColor(Qt::white); // Kolor tekstu
        // Tworzenie transformacji do pozycjonowania etykiety
        auto *t = new Qt3DCore::QTransform();
        t->setTranslation(pos); // Ustawienie pozycji etykiety
        label->addComponent(t); // Dodanie transformacji do bytu etykiety
    };

    // Dodanie etykiet dla głównych kierunków geograficznych w odpowiednich pozycjach
    // Pozycje są dobrane eksperymentalnie, aby pasowały do promienia pierścienia
    addLabel("N", QVector3D(-1, 4, 0));   // Północ (trochę na lewo, nad środkiem)
    addLabel("E", QVector3D(7, -4, 0));   // Wschód (na prawo, poniżej środka)
    addLabel("S", QVector3D(-1, -12, 5)); // Południe (trochę na lewo, pod pierścieniem, lekko wysunięte w Z)
    addLabel("W", QVector3D(-9, -4, 0));  // Zachód (na lewo, poniżej środka)
}