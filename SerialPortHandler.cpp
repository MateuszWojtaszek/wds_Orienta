/**
 * @file SerialPortHandler.cpp
 * @brief Implementacja klasy SerialPortHandler (wersja przywrócona + obsługa błędów).
 * @author Mateusz Wojtaszek
 * @date 2025-04-21
 */

#include "SerialPortHandler.h"
#include <QDebug>

//! Oczekiwana liczba wartości float w pojedynczej linii danych.
const int EXPECTED_VALUE_COUNT_SERIAL = 12; // Używamy stałej

/**
 * @brief Konstruktor.
 */
SerialPortHandler::SerialPortHandler(QObject *parent)
    : QObject(parent),
      serial(new QSerialPort(this))
{
    // Połączenie sygnału gotowości do odczytu ze slotem readData
    connect(serial, &QSerialPort::readyRead, this, &SerialPortHandler::readData);
    // Połączenie sygnału błędu ze slotem handleError (Dodano)
    connect(serial, &QSerialPort::errorOccurred, this, &SerialPortHandler::handleError);
}

/**
 * @brief Destruktor.
 */
SerialPortHandler::~SerialPortHandler() {
    closePort();
}

/**
 * @brief Otwiera port szeregowy.
 */
bool SerialPortHandler::openPort(const QString &portName, qint32 baudRate) {
    if (serial->isOpen()) {
        qInfo() << "Closing previously open port:" << serial->portName();
        serial->close();
    }
    serial->setPortName(portName);
    serial->setBaudRate(baudRate);
    // Można dodać ustawienia DataBits, Parity etc. dla pewności
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    qInfo() << "Attempting to open port:" << portName << "at baud rate:" << baudRate;
    if (serial->open(QIODevice::ReadOnly)) {
         qInfo() << "Port" << portName << "opened successfully.";
         buffer.clear(); // Wyczyść bufor po otwarciu
         serial->clear(QSerialPort::Input); // Wyczyść bufor systemowy
         return true;
    } else {
         qWarning() << "Failed to open port" << portName << "Error:" << getLastError(); // Użyj getLastError()
         return false;
    }
}

/**
 * @brief Zamyka port szeregowy.
 */
void SerialPortHandler::closePort() {
    if (serial && serial->isOpen()) { // Sprawdź czy serial nie jest nullptr
        qInfo() << "Closing port:" << serial->portName();
        serial->close();
        buffer.clear(); // Wyczyść bufor przy zamykaniu
    }
}

/**
 * @brief Zwraca ostatni błąd zgłoszony przez QSerialPort.
 */
QString SerialPortHandler::getLastError() const {
    if (serial) {
        return serial->errorString();
    }
    return tr("Serial object not initialized.");
}


/**
 * @brief Odczytuje dostępne dane z portu szeregowego i przetwarza je.
 * @details Wersja przywrócona (szybka pętla while) z dodanym sprawdzaniem
 * konwersji toFloat dla bezpieczeństwa.
 */
void SerialPortHandler::readData() {
    if (!serial || !serial->isOpen() || !serial->isReadable()) {
        return; // Dodatkowe zabezpieczenie
    }

    // Dodaj wszystkie dostępne dane do bufora
    try {
         if (serial->bytesAvailable() > 0) {
             buffer.append(serial->readAll());
         } else {
             return; // Nic do odczytania
         }
    } catch(...) { // Prosty catch-all dla bezpieczeństwa
         qWarning() << "Exception while reading serial data.";
         buffer.clear();
         return;
    }


    // Przetwarzaj bufor, dopóki zawiera znak nowej linii (oryginalna pętla)
    while (buffer.contains('\n')) {
        int index = buffer.indexOf('\n');
        // Wyodrębnij linię (bez znaku nowej linii) i usuń białe znaki
        QByteArray line = buffer.left(index).trimmed();
        // Usuń przetworzoną linię (wraz ze znakiem '\n') z bufora
        buffer.remove(0, index + 1);

        // Pomiń puste linie
        if (line.isEmpty()) {
            continue;
        }

        // Podziel linię na wartości rozdzielone przecinkami
        QList<QByteArray> values = line.split(',');

        // Sprawdź, czy liczba wartości jest zgodna z oczekiwaniami
        // Użyto stałej EXPECTED_VALUE_COUNT_SERIAL
        if (values.size() == EXPECTED_VALUE_COUNT_SERIAL) {
            QVector<float> parsedValues;
            parsedValues.reserve(EXPECTED_VALUE_COUNT_SERIAL);
            bool conversionOk = true; // Flaga do śledzenia błędów konwersji

            // Spróbuj sparsować każdą wartość na float
            for (const QByteArray &val : values) {
                bool ok; // Zmienna do sprawdzania wyniku toFloat
                float floatVal = val.toFloat(&ok);
                if (!ok) { // <<< Dodano sprawdzanie wyniku konwersji
                    qWarning() << "Failed to convert value to float:" << val << "in line:" << line;
                    conversionOk = false;
                    break; // Przerwij parsowanie tej linii
                }
                parsedValues.append(floatVal);
            }

            // Jeśli konwersja dla całej linii była OK, wyemituj sygnał
            if (conversionOk) {
                emit newDataReceived(parsedValues);
            }
            // Jeśli conversionOk == false, linia jest ignorowana (po cichu, bo warning był już wyżej)

        } else {
            // Zgłoś ostrzeżenie, jeśli linia ma nieoczekiwaną liczbę wartości
            qWarning() << "Received line with incorrect value count (" << values.size()
                       << ", expected" << EXPECTED_VALUE_COUNT_SERIAL << "):" << line;
        }
    }
    // Dane, które pozostały w buforze, poczekają na kolejne dane i sygnał readyRead.
}

/**
 * @brief Obsługuje błędy zgłaszane przez obiekt QSerialPort. (Dodano)
 */
void SerialPortHandler::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::NoError) {
        return;
    }
    if (error == QSerialPort::TimeoutError) {
         // qInfo() << "Serial port timeout occurred.";
         return;
    }

    QString errorString = getLastError(); // Użyj metody getLastError
    qWarning() << "Serial port error occurred:" << error << "-" << errorString;
    emit errorOccurred(error, errorString); // Emituj sygnał

    // Można dodać logikę zamykania portu przy krytycznych błędach
    // if (error == QSerialPort::ResourceError) { closePort(); }
}