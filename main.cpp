#include <QApplication>
#include <QSurfaceFormat>
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEnginePage>
#include <QtWebEngineQuick/QtWebEngineQuick> // dla QML
#include "MainWindow.h"

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);
    app.setApplicationName("Orienta");
    app.setOrganizationName("PWR");

    MainWindow w;
    w.show();

    return QApplication::exec();
}