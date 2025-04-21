/**
 * @file ImuDataHandler.h
 * @brief Definicja klas SensorGraph i ImuDataHandler do wizualizacji danych IMU.
 * @author Mateusz Wojtaszek
 * @date 2025-03-31 (Zaktualizowano 2025-04-21)
 */
#ifndef IMUDATAHANDLER_H
#define IMUDATAHANDLER_H

#include <QWidget>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QVector>
#include <QLabel>
#include <QGroupBox> // Dodano
#include <QQuaternion> // Dodano
#include <QDebug> // Dodano
#include <QList> // Dodano

// --- Oryginalne Include dla Qt Charts ---
#include <QChartView>
#include <QLineSeries>
#include <QChart>
// --- Koniec Oryginalnych Include ---

// --- Nagłówki Qt 3D ---
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform> // Dodano
#include <Qt3DCore/QComponent> // Dodano

// --- Forward declarations ---
class QValueAxis; // Potrzebny dla SensorGraph::addData w .cpp
namespace Qt3DExtras {
    class QTorusMesh;
    class QConeMesh;
    class QText2DEntity;
}
namespace Qt3DRender {
    class QDirectionalLight;
}

/**
 * @class SensorGraph
 * @brief Widżet wykresu QtCharts wyświetlający dane XYZ.
 * @note Używa QChartView, QLineSeries, QChart.
 */
class SensorGraph : public QChartView { // Bez QtCharts::
    Q_OBJECT
public:
    explicit SensorGraph(const QString &title, int minY, int maxY, QWidget *parent = nullptr);
    void addData(const QVector<int> &data);
    void setSampleCount(int count);
    void setYRange(int minY, int maxY);

private:
    QList<QLineSeries *> seriesList; // Bez QtCharts::
    int sampleCount = 1000;
    qint64 currentSampleIndex = 0;
};

/**
 * @class ImuDataHandler
 * @brief Główny widżet do obsługi i wizualizacji danych IMU.
 */
class ImuDataHandler : public QWidget {
    Q_OBJECT

public:
    explicit ImuDataHandler(QWidget *parent = nullptr);
    void updateData(const QVector<int> &acc, const QVector<int> &gyro, const QVector<int> &mag);
    void setSampleCount(int samples);
    void setRange(int minVal, int maxVal);
    void setRotation(float yaw, float pitch, float roll);
    void updateCompass(float heading);

private slots:
    void showCurrentData();
    void showGraph();

private:
    QWidget *create3DView();
    QWidget *createCompassView();
    QGroupBox* createSensorBarGroup(const QString& title, QProgressBar*& xBar, QProgressBar*& yBar, QProgressBar*& zBar, int rangeMin, int rangeMax);
    /** @brief Inicjalizuje wspólny panel wizualizacji (3D + Kompas). */
    void setupVisualizationPanel();

    // --- Składowe UI ---
    QProgressBar *accXBar = nullptr, *accYBar = nullptr, *accZBar = nullptr;
    QProgressBar *gyroXBar = nullptr, *gyroYBar = nullptr, *gyroZBar = nullptr;
    QProgressBar *magXBar = nullptr, *magYBar = nullptr, *magZBar = nullptr;

    SensorGraph *accGraph = nullptr;
    SensorGraph *gyroGraph = nullptr;
    SensorGraph *magGraph = nullptr;

    QStackedWidget *stackedWidget = nullptr;
    int currentSampleCount = 1000;

    // Współdzielony panel dla wizualizacji
    QWidget* visualizationPanelWidget = nullptr;
    QWidget* view3DContainerWidget = nullptr;
    QWidget* compassContainerWidget = nullptr;

    // --- Składowe Qt 3D ---
    // Usunięto zbędny wskaźnik na encję strzałki
    Qt3DCore::QTransform *compassTransform = nullptr;
    Qt3DCore::QTransform *boardTransform = nullptr;
};

#endif // IMUDATAHANDLER_H