/**
 * @file SerialPortHandler.cpp
 * @brief Implementacja klasy SerialPortHandler.
 * @author Mateusz Wojtaszek
 * @date 2025-04-21 // Zaktualizowano datę zgodnie z kontekstem
 */

#include "SerialPortHandler.h"
#include <QDebug> // Do logowania ostrzeżeń i informacji

//! Oczekiwana liczba wartości float w pojedynczej linii danych.
const int EXPECTED_VALUE_COUNT_SERIAL = 12;

/**
 * @brief Konstruktor klasy SerialPortHandler.
 * @details Tworzy obiekt QSerialPort i łączy jego sygnały `readyRead` oraz `errorOccurred`
 * z odpowiednimi slotami tej klasy.
 * @param parent Wskaźnik na obiekt nadrzędny.
 */
SerialPortHandler::SerialPortHandler(QObject *parent)
    : QObject(parent),
      serial(new QSerialPort(this)) // Utworzenie obiektu QSerialPort jako dziecka
{
    // Połączenie sygnału gotowości do odczytu ze slotem readData
    connect(serial, &QSerialPort::readyRead, this, &SerialPortHandler::readData);
    // Połączenie sygnału błędu ze slotem handleError
    connect(serial, &QSerialPort::errorOccurred, this, &SerialPortHandler::handleError);
}

/**
 * @brief Destruktor klasy SerialPortHandler.
 * @details Wywołuje closePort() aby upewnić się, że port jest zamknięty.
 * Obiekt `serial` zostanie automatycznie usunięty, bo jest dzieckiem `SerialPortHandler`.
 */
SerialPortHandler::~SerialPortHandler() {
    closePort(); // Upewnij się, że port jest zamknięty przy niszczeniu obiektu
}

/**
 * @brief Otwiera określony port szeregowy.
 * @details Najpierw zamyka port, jeśli był już otwarty. Następnie ustawia nazwę portu,
 * prędkość transmisji i próbuje otworzyć port w trybie tylko do odczytu.
 * @param portName Nazwa portu.
 * @param baudRate Prędkość transmisji.
 * @return true jeśli otwarcie się powiodło, false w przeciwnym razie.
 */
bool SerialPortHandler::openPort(const QString &portName, qint32 baudRate) {
    if (serial->isOpen()) {
        qInfo() << "Closing previously open port:" << serial->portName();
        serial->close();
    }
    serial->setPortName(portName);
    serial->setBaudRate(baudRate);
    serial->setDataBits(QSerialPort::Data8);   // Zazwyczaj standard
    serial->setParity(QSerialPort::NoParity);  // Zazwyczaj standard
    serial->setStopBits(QSerialPort::OneStop); // Zazwyczaj standard
    serial->setFlowControl(QSerialPort::NoFlowControl); // Zazwyczaj standard

    qInfo() << "Attempting to open port:" << portName << "at baud rate:" << baudRate;
    if (serial->open(QIODevice::ReadOnly)) { // Otwarcie w trybie tylko do odczytu
        qInfo() << "Port" << portName << "opened successfully.";
        return true;
    } else {
        qWarning() << "Failed to open port" << portName << "Error:" << serial->errorString();
        return false;
    }
}

/**
 * @brief Zamyka port szeregowy, jeśli jest otwarty.
 */
void SerialPortHandler::closePort() {
    if (serial->isOpen()) {
        qInfo() << "Closing port:" << serial->portName();
        serial->close();
    }
}

/**
 * @brief Zwraca ostatni błąd zgłoszony przez QSerialPort.
 * @return Opis ostatniego błędu jako QString.
 */
QString SerialPortHandler::getLastError() const {
    return serial->errorString();
}

/**
 * @brief Odczytuje dostępne dane z portu szeregowego i przetwarza je.
 * @details Dodaje odczytane dane do wewnętrznego bufora. Następnie przetwarza bufor,
 * szukając kompletnych linii zakończonych znakiem nowej linii ('\n').
 * Każdą znalezioną linię próbuje sparsować jako `EXPECTED_VALUE_COUNT_SERIAL` wartości float
 * oddzielonych przecinkami. Jeśli parsowanie się powiedzie, emituje sygnał `newDataReceived`.
 * Linie o niepoprawnej liczbie wartości lub zawierające niepoprawne dane są ignorowane (z ostrzeżeniem).
 */
void SerialPortHandler::readData() {
    // Dodaj wszystkie dostępne dane do bufora
    buffer.append(serial->readAll());

    // Przetwarzaj bufor, dopóki zawiera znak nowej linii
    while (buffer.contains('\n')) {
        int index = buffer.indexOf('\n');
        // Wyodrębnij linię (bez znaku nowej linii) i usuń białe znaki z początku/końca
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
        if (values.size() == EXPECTED_VALUE_COUNT_SERIAL) {
            QVector<float> parsedValues;
            parsedValues.reserve(EXPECTED_VALUE_COUNT_SERIAL); // Optymalizacja
            bool conversionOk = true;

            // Spróbuj sparsować każdą wartość na float
            for (const QByteArray &val : values) {
                bool ok;
                float floatVal = val.toFloat(&ok);
                if (!ok) {
                    qWarning() << "Failed to convert value to float:" << val << "in line:" << line;
                    conversionOk = false;
                    break; // Przerwij parsowanie tej linii, jeśli jedna wartość jest błędna
                }
                parsedValues.append(floatVal);
            }

            // Jeśli wszystkie wartości w linii zostały poprawnie sparsowane, wyemituj sygnał
            if (conversionOk) {
                emit newDataReceived(parsedValues);
            }
        } else {
            // Zgłoś ostrzeżenie, jeśli linia ma nieoczekiwaną liczbę wartości
            qWarning() << "Received line with incorrect value count (" << values.size()
                       << ", expected" << EXPECTED_VALUE_COUNT_SERIAL << "):" << line;
        }
    }
    // Dane, które pozostały w buforze, nie tworzą pełnej linii i poczekają na kolejne dane.
}

/**
 * @brief Obsługuje błędy zgłaszane przez obiekt QSerialPort.
 * @details Emituje sygnał `errorOccurred` z kodem błędu i opisem.
 * Loguje również ostrzeżenie.
 * @param error Kod błędu typu QSerialPort::SerialPortError.
 */
void SerialPortHandler::handleError(QSerialPort::SerialPortError error) {
    // Ignoruj błąd "NoError" oraz "TimeoutError", który może być normalny
    if (error == QSerialPort::NoError || error == QSerialPort::TimeoutError) {
        return;
    }

    QString errorString = serial->errorString();
    qWarning() << "Serial port error occurred:" << error << "-" << errorString;

    // Wyemituj sygnał błędu, aby inne części aplikacji mogły zareagować
    emit errorOccurred(error, errorString);

    // Można dodać tutaj logikę np. próby zamknięcia portu przy krytycznych błędach
    // if (error == QSerialPort::ResourceError) { // Np. urządzenie odłączone
    //    closePort();
    // }
}