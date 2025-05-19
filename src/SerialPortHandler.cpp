#include "SerialPortHandler.h"
#include <QDebug>

// Definicja stałej wewnętrznej - nie wymaga dokumentacji Doxygen API
const int EXPECTED_VALUE_COUNT_SERIAL = 12;

SerialPortHandler::SerialPortHandler(QObject *parent)
    : QObject(parent),
      serial(new QSerialPort(this))
{
    // Połączenie sygnałów ze slotami (szczegół implementacyjny)
    connect(serial, &QSerialPort::readyRead, this, &SerialPortHandler::readData);
    connect(serial, &QSerialPort::errorOccurred, this, &SerialPortHandler::handleError);
}

SerialPortHandler::~SerialPortHandler() {
    // Zamknięcie portu w destruktorze jest dobrą praktyką
    closePort();
    // serial zostanie automatycznie usunięty przez mechanizm parent-child Qt
}

bool SerialPortHandler::openPort(const QString &portName, qint32 baudRate) {
    // Zamknięcie portu, jeśli był otwarty
    if (serial->isOpen()) {
        qInfo() << "Closing previously open port:" << serial->portName();
        serial->close();
    }

    // Konfiguracja parametrów portu
    serial->setPortName(portName);
    serial->setBaudRate(baudRate);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    qInfo() << "Attempting to open port:" << portName << "at baud rate:" << baudRate;

    // Próba otwarcia portu w trybie tylko do odczytu
    if (serial->open(QIODevice::ReadOnly)) {
        qInfo() << "Port" << portName << "opened successfully.";
        buffer.clear(); // Czyszczenie bufora po otwarciu
        serial->clear(QSerialPort::Input); // Czyszczenie bufora wejściowego portu
        return true;
    } else {
        // Logowanie błędu w przypadku niepowodzenia
        qWarning() << "Failed to open port" << portName << "Error:" << getLastError();
        emit errorOccurred(serial->error(), getLastError()); // Można rozważyć emitowanie błędu tutaj
        return false;
    }
}

void SerialPortHandler::closePort() {
    if (serial && serial->isOpen()) {
        qInfo() << "Closing port:" << serial->portName();
        serial->close();
        buffer.clear(); // Czyszczenie bufora przy zamykaniu
    }
}

QString SerialPortHandler::getLastError() const {
    // Zabezpieczenie przed odwołaniem do nullptr
    if (serial) {
        return serial->errorString();
    }
    // Zwrócenie komunikatu, jeśli obiekt serial nie został zainicjalizowany
    return tr("Serial object not initialized.");
}

void SerialPortHandler::readData() {
    // Sprawdzenie, czy można bezpiecznie odczytać dane
    if (!serial || !serial->isOpen() || !serial->isReadable()) {
        //qWarning() << "Cannot read data: Port not open or readable."; // Opcjonalny log
        return;
    }

    try {
        // Odczyt dostępnych danych i dołączenie do bufora
        if (serial->bytesAvailable() > 0) {
            buffer.append(serial->readAll());
        } else {
            // Brak nowych danych do odczytania
            return;
        }
    } catch (const std::exception& e) { // Lepiej łapać konkretne wyjątki, jeśli Qt może je rzucić
        qWarning() << "Exception while reading serial data:" << e.what();
        buffer.clear(); // Wyczyść bufor w razie problemu
        return;
    } catch (...) {
        qWarning() << "Unknown exception while reading serial data.";
        buffer.clear();
        return;
    }

    // Przetwarzanie bufora linia po linii
    while (buffer.contains('\n')) {
        int index = buffer.indexOf('\n');
        // Pobranie linii danych (bez znaku nowej linii) i usunięcie jej z bufora
        QByteArray line = buffer.left(index).trimmed(); // Usunięcie białych znaków z początku/końca
        buffer.remove(0, index + 1); // Usunięcie linii i znaku '\n'

        // Pomiń puste linie
        if (line.isEmpty()) {
            continue;
        }

        // Podział linii na wartości oddzielone przecinkiem
        QList<QByteArray> values = line.split(',');

        // Sprawdzenie, czy liczba wartości jest zgodna z oczekiwaną
        if (values.size() == EXPECTED_VALUE_COUNT_SERIAL) {
            QVector<float> parsedValues;
            parsedValues.reserve(EXPECTED_VALUE_COUNT_SERIAL); // Rezerwacja pamięci dla wydajności
            bool conversionOk = true;

            // Konwersja każdej wartości na float
            for (const QByteArray &val : values) {
                bool ok;
                float floatVal = val.toFloat(&ok);
                if (!ok) {
                    qWarning() << "Failed to convert value to float:" << val << "in line:" << line;
                    conversionOk = false;
                    break; // Przerwij przetwarzanie tej linii, jeśli konwersja się nie powiedzie
                }
                parsedValues.append(floatVal);
            }

            // Jeśli wszystkie wartości zostały poprawnie skonwertowane, wyemituj sygnał
            if (conversionOk) {
                // Użycie const& w sygnale nie wymaga zmiany w emisji
                emit newDataReceived(parsedValues);
            }

        } else {
            // Logowanie ostrzeżenia o nieprawidłowej liczbie wartości
            qWarning() << "Received line with incorrect value count (" << values.size()
                       << ", expected" << EXPECTED_VALUE_COUNT_SERIAL << "):" << line;
        }
    }
    // Uwaga: Może pozostać niekompletna linia w buforze, która zostanie przetworzona przy następnym odczycie.
}

void SerialPortHandler::handleError(QSerialPort::SerialPortError error) {
    // Ignorowanie braku błędu i błędu timeout (często nie są to krytyczne problemy)
    if (error == QSerialPort::NoError || error == QSerialPort::TimeoutError) {
        return;
    }

    // Pobranie opisu błędu i jego logowanie
    QString errorString = getLastError(); // Użyj metody klasy, aby uzyskać opis
    qWarning() << "Serial port error occurred:" << error << "-" << errorString;

    // Wyemitowanie sygnału o błędzie dla innych części aplikacji
    // Użycie const& w sygnale nie wymaga zmiany w emisji
    emit errorOccurred(error, errorString);
}