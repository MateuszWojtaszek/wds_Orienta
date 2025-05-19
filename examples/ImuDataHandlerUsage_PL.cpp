/**
 * @file ImuDataHandlerUsage_PL.cpp
 * @brief Przykład użycia klasy ImuDataHandler.
 * @details Demonstruje, jak utworzyć instancję ImuDataHandler, dodać ją do layoutu
 * okna głównego oraz jak symulować aktualizację danych sensorów i orientacji.
 */

#include "ImuDataHandler.h" // Załóżmy, że ImuDataHandler.h jest w ścieżce include
#include <QApplication>
#include <QMainWindow>
#include <QTimer>
#include <QRandomGenerator> // Do generowania losowych danych
#include <QVBoxLayout>      // Do prostego layoutu
#include <QPushButton>      // Do przykładowej interakcji

// Przykładowe okno główne do demonstracji ImuDataHandler
class DemoWindow : public QMainWindow {
    Q_OBJECT

public:
    DemoWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle(tr("Przykład ImuDataHandler"));
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        m_imuHandler = new ImuDataHandler(this);
        layout->addWidget(m_imuHandler);

        // Przycisk do symulacji zmiany orientacji
        QPushButton *rotateButton = new QPushButton(tr("Symuluj obrót"), this);
        connect(rotateButton, &QPushButton::clicked, this, &DemoWindow::simulateRotation);
        layout->addWidget(rotateButton);

        setCentralWidget(centralWidget);
        resize(1000, 700); // Rozsądny rozmiar, aby wszystko było widać

        // Timer do symulacji napływu danych z sensorów
        m_dataTimer = new QTimer(this);
        connect(m_dataTimer, &QTimer::timeout, this, &DemoWindow::simulateSensorData);
        m_dataTimer->start(100); // Aktualizuj dane co 100 ms

        // Początkowe ustawienie liczby próbek i zakresu (opcjonalne)
        m_imuHandler->setSampleCount(200);
        // m_imuHandler->setRange(0,0); // Wywoływane w konstruktorze ImuDataHandler

        // Początkowe ustawienie kompasu
        m_imuHandler->updateCompass(45.0f); // Początkowo na NE
    }

public slots:
    void simulateSensorData() {
        QVector<int> accData, gyroData, magData;
        // Symulacja danych dla akcelerometru (zakres np. +/- 2000)
        accData.append(QRandomGenerator::global()->bounded(-2000, 2000));
        accData.append(QRandomGenerator::global()->bounded(-2000, 2000));
        accData.append(QRandomGenerator::global()->bounded(-2000, 2000));

        // Symulacja danych dla żyroskopu (zakres np. +/- 200)
        gyroData.append(QRandomGenerator::global()->bounded(-200, 200));
        gyroData.append(QRandomGenerator::global()->bounded(-200, 200));
        gyroData.append(QRandomGenerator::global()->bounded(-200, 200));

        // Symulacja danych dla magnetometru (zakres np. +/- 1000)
        magData.append(QRandomGenerator::global()->bounded(-1000, 1000));
        magData.append(QRandomGenerator::global()->bounded(-1000, 1000));
        magData.append(QRandomGenerator::global()->bounded(-1000, 1000));

        m_imuHandler->updateData(accData, gyroData, magData);
    }

    void simulateRotation() {
        static float yaw = 0.0f;
        static float pitch = 0.0f;
        static float roll = 0.0f;

        // Prosta animacja obrotu
        yaw = static_cast<float>(QRandomGenerator::global()->bounded(360));   // 0 do 359 stopni
        pitch = static_cast<float>(QRandomGenerator::global()->bounded(-90, 91)); // -90 do +90 stopni
        roll = static_cast<float>(QRandomGenerator::global()->bounded(-180, 181));// -180 do +180 stopni

        m_imuHandler->setRotation(yaw, pitch, roll);

        // Symulacja zmiany kierunku kompasu wraz z obrotem yaw
        m_imuHandler->updateCompass(yaw);
    }

protected:
    // Przykładowa obsługa zdarzenia zmiany języka (dla retranslateUi)
    void changeEvent(QEvent *event) override {
        if (event->type() == QEvent::LanguageChange) {
            setWindowTitle(tr("Przykład ImuDataHandler"));
            if (m_imuHandler) {
                m_imuHandler->retranslateUi();
            }
            // Przetłumacz inne elementy UI, np. tekst przycisku
            // (w tym przykładzie przycisk jest tworzony z tr())
        }
        QMainWindow::changeEvent(event);
    }

private:
    ImuDataHandler *m_imuHandler;
    QTimer *m_dataTimer;
};

#include "main.moc" // Potrzebne, jeśli DemoWindow ma sloty i jest w .cpp

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Tutaj można załadować tłumaczenia aplikacji (np. dla tr())

    DemoWindow demoWindow;
    demoWindow.show();

    return app.exec();
}