//
// Created by Mateusz Wojtaszek on 31/03/2025.
//

#include <QMainWindow>
#include <QStackedWidget>

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

private:
    void createMenus();

    QStackedWidget *stackedWidget;
    ImuDataHandler *imuHandler;
    GPSDataHandler *gpsHandler;
};