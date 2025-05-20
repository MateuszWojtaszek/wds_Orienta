/**
 * @file Compass2DRenderer_Usage.cpp
 * @brief Przykład demonstrujący użycie klasy Compass2DRenderer.
 * @example Compass2DRenderer_Usage.cpp
 */

#include "Compass2DRenderer.h" // Załóżmy, że Compass2DRenderer.h znajduje się w ścieżce include
#include <QApplication>
#include <QMainWindow>
#include <QSlider>
#include <QVBoxLayout>
#include <QLabel>
#include <QWidget> // Dla widgetu centralnego

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Przykład Kompasu 2D");

    // Widget centralny i layout
    QWidget *centralWidget = new QWidget(&mainWindow);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Etykieta wyświetlająca aktualny kurs
    QLabel *headingLabel = new QLabel("Kurs: 0°", centralWidget);
    headingLabel->setAlignment(Qt::AlignCenter);
    QFont labelFont = headingLabel->font();
    labelFont.setPointSize(12);
    headingLabel->setFont(labelFont);

    // Utworzenie instancji Compass2DRenderer
    Compass2DRenderer *compassWidget = new Compass2DRenderer(centralWidget);
    // Ustawienie preferowanego rozmiaru dla kompasu w przykładzie
    compassWidget->setFixedSize(200, 200);


    // Suwak do zmiany kursu kompasu
    QSlider *headingSlider = new QSlider(Qt::Horizontal, centralWidget);
    headingSlider->setRange(0, 359); // Zakres od 0 do 359 stopni
    headingSlider->setValue(0);

    // Połączenie sygnału zmiany wartości suwaka ze slotem aktualizującym kompas i etykietę
    QObject::connect(headingSlider, &QSlider::valueChanged, [=](int value) {
        compassWidget->setHeading(static_cast<float>(value));
        headingLabel->setText(QString("Kurs: %1°").arg(value));
    });

    // Dodanie widgetów do layoutu
    mainLayout->addWidget(headingLabel);
    mainLayout->addWidget(compassWidget, 0, Qt::AlignCenter); // Kompas wyśrodkowany, bez rozciągania
    mainLayout->addWidget(headingSlider);

    centralWidget->setLayout(mainLayout);
    mainWindow.setCentralWidget(centralWidget);

    mainWindow.resize(300, 400); // Ustawienie rozmiaru okna głównego
    mainWindow.show();

    return app.exec();
}