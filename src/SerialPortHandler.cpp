/**
 * @file SerialPortHandler.cpp
 * @brief Implementacja metod klasy SerialPortHandler.
 * @author Mateusz Wojtaszek
 * - 2025-05-20 - Wersja 1.2: Dodano obsługę GPS w ramce danych.
 *
 * @details Zawiera logikę otwierania, zamykania portu szeregowego, odczytu danych,
 * weryfikacji CRC oraz parsowania danych telemetrycznych (IMU + GPS).
 */

#include "SerialPortHandler.h"
#include <QDebug>

// Implementacje metod (pozostała część pliku .cpp bez zmian w komentarzach Doxygen,
// ponieważ komentarze Doxygen dla metod są zwykle w pliku .h)

SerialPortHandler::SerialPortHandler(QObject *parent)
    : QObject(parent),
      serial(new QSerialPort(this)) {
    connect(serial, &QSerialPort::readyRead, this, &SerialPortHandler::readData);
    connect(serial, &QSerialPort::errorOccurred, this, &SerialPortHandler::handleError);
}

SerialPortHandler::~SerialPortHandler() {
    closePort();
}

bool SerialPortHandler::openPort(const QString &portName, qint32 baudRate) {
    if (serial->isOpen()) {
        qInfo() << "Closing previously open port:" << serial->portName();
        serial->close();
    }

    serial->setPortName(portName);
    serial->setBaudRate(baudRate);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    qInfo() << "Attempting to open port:" << portName << "at baud rate:" << baudRate;

    if (serial->open(QIODevice::ReadOnly)) {
        qInfo() << "Port" << portName << "opened successfully.";
        buffer.clear();
        serial->clear(QSerialPort::Input);
        return true;
    } else {
        qWarning() << "Failed to open port" << portName << "Error:" << getLastError();
        emit errorOccurred(serial->error(), getLastError());
        return false;
    }
}

void SerialPortHandler::closePort() {
    if (serial && serial->isOpen()) {
        qInfo() << "Closing port:" << serial->portName();
        serial->close();
        buffer.clear();
    }
}

QString SerialPortHandler::getLastError() const {
    if (serial) {
        return serial->errorString();
    }
    return tr("Serial object not initialized.");
}

uint16_t SerialPortHandler::calculateCrc16(const QByteArray &data) {
    uint16_t crc = 0xFFFF;
    const char *bytes = data.constData();
    int len = data.length();

    for (int i = 0; i < len; ++i) {
        crc ^= (static_cast<uint16_t>(static_cast<unsigned char>(bytes[i])) << 8);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void SerialPortHandler::readData() {
    if (!serial || !serial->isOpen() || !serial->isReadable()) {
        return;
    }

    try {
        if (serial->bytesAvailable() > 0) {
            buffer.append(serial->readAll());
        } else {
            return;
        }
    } catch (const std::exception &e) {
        qWarning() << "Exception while reading serial data:" << e.what();
        buffer.clear();
        return;
    } catch (...) {
        qWarning() << "Unknown exception while reading serial data.";
        buffer.clear();
        return;
    }

    while (buffer.contains('\n')) {
        int endOfLineIndex = buffer.indexOf('\n');
        QByteArray fullLineWithCrcAndMaybeCR = buffer.left(endOfLineIndex);
        buffer.remove(0, endOfLineIndex + 1);
        QByteArray trimmedFullLine = fullLineWithCrcAndMaybeCR.trimmed();

        if (trimmedFullLine.isEmpty()) {
            continue;
        }

        int checksumSeparatorIndex = trimmedFullLine.lastIndexOf('*');
        if (checksumSeparatorIndex == -1) {
            qWarning() << "Received line without CRC separator ('*'):" << trimmedFullLine;
            continue;
        }

        QByteArray dataPayload = trimmedFullLine.left(checksumSeparatorIndex);
        QByteArray receivedCrcHex = trimmedFullLine.mid(checksumSeparatorIndex + 1);
        uint16_t calculatedCrc = calculateCrc16(dataPayload);
        bool conversionOk;
        uint16_t receivedCrc = receivedCrcHex.toUShort(&conversionOk, 16);

        if (!conversionOk) {
            qWarning() << "Failed to convert received CRC from hex:" << receivedCrcHex
                       << "for payload:" << dataPayload << "in full line:" << trimmedFullLine;
            continue;
        }

        if (calculatedCrc != receivedCrc) {
            qWarning() << "Checksum Mismatch! Payload:" << dataPayload
                       << "Received CRC:" << receivedCrcHex << "(val:" << receivedCrc << ")"
                       << "Calculated CRC:" << QString::number(calculatedCrc, 16).toUpper().rightJustified(4, '0') << "(val:" << calculatedCrc << ")"
                       << "Full line:" << trimmedFullLine;
            continue;
        }

        QList<QByteArray> values = dataPayload.split(',');
        if (values.size() == EXPECTED_VALUE_COUNT_SERIAL) { // Oczekuje 14 wartości
            QVector<float> parsedValues;
            parsedValues.reserve(EXPECTED_VALUE_COUNT_SERIAL);
            bool allConversionsOk = true;

            for (const QByteArray &val : values) {
                bool ok;
                float floatVal = val.toFloat(&ok);
                if (!ok) {
                    qWarning() << "Failed to convert value to float:" << val
                               << "in payload:" << dataPayload << "(Full line:" << trimmedFullLine << ")";
                    allConversionsOk = false;
                    break;
                }
                parsedValues.append(floatVal);
            }

            if (allConversionsOk) {
                emit newDataReceived(parsedValues); // Emituje wektor 14 floatów
            }
        } else {
            qWarning() << "Received line with incorrect value count after CRC check. Count:" << values.size()
                       << ", Expected:" << EXPECTED_VALUE_COUNT_SERIAL
                       << "Payload:" << dataPayload << "(Full line:" << trimmedFullLine << ")";
        }
    }
}

void SerialPortHandler::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::NoError || error == QSerialPort::TimeoutError) {
        return;
    }

    QString errorString = getLastError();
    qWarning() << "Serial port error occurred:" << error << "-" << errorString;
    emit errorOccurred(error, errorString);
}