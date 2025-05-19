/**
 * @file SensorGraphUsage_PL.cpp
 * @brief Przykład użycia klasy SensorGraph do wyświetlania symulowanych danych.
 * @details Demonstruje tworzenie obiektu SensorGraph, dodawanie danych
 * oraz interakcję z niektórymi jego metodami, np. zmianę liczby próbek.
 */

#include "SensorGraph.h" // Załóżmy, że SensorGraph.h jest w ścieżce include
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QRandomGenerator> // Do generowania losowych danych

// Przykładowy widget główny aplikacji
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle(tr("Przykład SensorGraph"));
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        // Tworzenie wykresu dla "Akcelerometru"
        // Klucz "AccelerometerTitle" powinien być zdefiniowany w plikach .ts
        m_accelerometerGraph = new SensorGraph(tr("Akcelerometr [jednostki]"), -1000, 1000, this);
        layout->addWidget(m_accelerometerGraph);

        // Przycisk do zmiany liczby próbek
        QPushButton *sampleButton = new QPushButton(tr("Zmień liczbę próbek (na 50)"), this);
        connect(sampleButton, &QPushButton::clicked, this, &MainWindow::changeSampleCount);
        layout->addWidget(sampleButton);

        setCentralWidget(centralWidget);
        resize(800, 600);

        // Timer do symulacji napływu danych
        m_dataTimer = new QTimer(this);
        connect(m_dataTimer, &QTimer::timeout, this, &MainWindow::simulateData);
        m_dataTimer->start(100); // Dodawaj dane co 100 ms
    }

public slots:
    void simulateData() {
        QVector<int> accData;
        // Symulacja trzech osi danych akcelerometru
        accData.append(QRandomGenerator::global()->bounded(-500, 500)); // Oś X
        accData.append(QRandomGenerator::global()->bounded(-400, 400)); // Oś Y
        accData.append(QRandomGenerator::global()->bounded(800, 1200)); // Oś Z (np. z offsetem grawitacji)
        m_accelerometerGraph->addData(accData);
    }

    void changeSampleCount() {
        static bool useSmallSamples = true;
        if (useSmallSamples) {
            m_accelerometerGraph->setSampleCount(50);
            qobject_cast<QPushButton*>(sender())->setText(tr("Zmień liczbę próbek (na 200)"));
        } else {
            m_accelerometerGraph->setSampleCount(200);
            qobject_cast<QPushButton*>(sender())->setText(tr("Zmień liczbę próbek (na 50)"));
        }
        useSmallSamples = !useSmallSamples;
    }

protected:
    // Przykładowa obsługa zdarzenia zmiany języka (dla retranslateUi)
    // W rzeczywistej aplikacji to byłoby wywoływane przez system zarządzania tłumaczeniami Qt.
    void changeEvent(QEvent *event) override {
        if (event->type() == QEvent::LanguageChange) {
            setWindowTitle(tr("Przykład SensorGraph"));
            if (m_accelerometerGraph) {
                m_accelerometerGraph->retranslateUi();
            }
            // Przetłumacz inne elementy UI, np. tekst przycisku, jeśli jest ustawiany dynamicznie
            // (w tym przykładzie tekst przycisku jest zmieniany w changeSampleCount)
        }
        QMainWindow::changeEvent(event);
    }


private:
    SensorGraph *m_accelerometerGraph;
    QTimer *m_dataTimer;
};

#include "main.moc" // Potrzebne, jeśli MainWindow ma sloty i jest w .cpp

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Tutaj można by załadować tłumaczenia, np. z QTranslator

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}