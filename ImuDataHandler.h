//
// Created by Mateusz Wojtaszek on 31/03/2025.
//
#ifndef IMUDATAHANDLER_H
#define IMUDATAHANDLER_H

#include <QWidget>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QVector>
#include <QChartView>
#include <QLineSeries>
#include <QChart>
#include <QLabel>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DCore/QEntity>
#include <QWidget>

class SensorGraph : public QChartView {
    Q_OBJECT
public:
    explicit SensorGraph(const QString &title, int minY, int maxY, QWidget *parent = nullptr);
    void addData(const QVector<int> &data);
    void setSampleCount(int count);

private:
    QList<QLineSeries *> seriesList;
    int sampleCount;
};

class ImuDataHandler : public QWidget {
    Q_OBJECT

public:
    explicit ImuDataHandler(QWidget *parent = nullptr);
    void setRange(int min, int max);
    void updateData(const QVector<int> &acc, const QVector<int> &gyro, const QVector<int> &mag);
    void setSampleCount(int samples);

    private slots:
        void showCurrentData();
    void showGraph();

private:
    QWidget* create3DView();

    QProgressBar *accXBar, *accYBar, *accZBar;
    QProgressBar *gyroXBar, *gyroYBar, *gyroZBar;
    QProgressBar *magXBar, *magYBar, *magZBar;

    SensorGraph *accGraph;
    SensorGraph *gyroGraph;
    SensorGraph *magGraph;

    QWidget *visualizationCurrent;
    QWidget *visualizationGraph;

    QStackedWidget *stackedWidget;
    int sampleCount;
};

#endif // IMUDATAHANDLER_H