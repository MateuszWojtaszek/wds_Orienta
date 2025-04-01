//
// Created by Mateusz Wojtaszek on 31/03/2025.
//

#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>
#include <QVector3D>

class ImuDataHandler;
class GPSDataHandler;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    signals:
        void switchToIMU();
    void switchToGPS();

    private slots:
        void setEnglishLanguage();
    void setPolishLanguage();
    void toggleSimulationMode();
    void selectPort();
    void showIMUHandler();
    void showGPSHandler();
    void updateSimulationData();

    void setManualRotation(float yaw, float pitch, float roll);

private:
    void createMenus();
    QVector3D manualRotationEuler = {0.0f, 0.0f, 0.0f}; // yaw, pitch, roll
    QStackedWidget *stackedWidget;
    ImuDataHandler *imuHandler;
    GPSDataHandler *gpsHandler;
    bool simulationMode = false;
    QTimer *simulationTimer;
};