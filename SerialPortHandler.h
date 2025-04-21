/**
* @file SerialPortHandler.h
 * @brief Definicja klasy SerialPortHandler (wersja przywrócona + obsługa błędów).
 * @author Mateusz Wojtaszek
 * @date 2025-04-21
 */

#ifndef SERIALPORTHANDLER_H
#define SERIALPORTHANDLER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo> // Mimo że nie używane, zostawiam jak w Twoim kodzie
#include <QVector>     // Dodano dla QVector
#include <QString>     // Dodano dla QString

/**
 * @brief Klasa obsługująca komunikację przez port szeregowy.
 */
class SerialPortHandler : public QObject {
    Q_OBJECT

public:
    explicit SerialPortHandler(QObject *parent = nullptr);
    ~SerialPortHandler() override; // Dodano override

    bool openPort(const QString &portName, qint32 baudRate = 115200);
    void closePort();
    /** @brief Zwraca ostatni błąd zgłoszony przez QSerialPort. */
    QString getLastError() const; // Dodano

    signals:
        /** @brief Sygnał emitowany po odebraniu i sparsowaniu nowej linii danych. */
        void newDataReceived(QVector<float> data);
    /** @brief Sygnał emitowany, gdy wystąpi błąd portu szeregowego. */
    void errorOccurred(QSerialPort::SerialPortError error, const QString& errorString); // Dodano

private slots:
    /** @brief Odczytuje i przetwarza dane z portu (oryginalna, szybka pętla while). */
    void readData();
    /** @brief Slot obsługujący błędy zgłaszane przez QSerialPort. */
    void handleError(QSerialPort::SerialPortError error); // Dodano

private:
    QSerialPort *serial = nullptr; // Inicjalizacja nullptr
    QByteArray buffer;
};
#endif //SERIALPORTHANDLER_H