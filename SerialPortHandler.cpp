#include "SerialPortHandler.h"
#include <QDebug>

/**
 * @brief Oczekiwana liczba wartości oddzielonych przecinkami w jednej linii danych.
 */
const int EXPECTED_VALUE_COUNT_SERIAL = 12;

/******************************************************************************/
/**
 * @brief Konstruktor klasy SerialPortHandler.
 * @param parent Wskaźnik na obiekt nadrzędny (opcjonalny).
 */
SerialPortHandler::SerialPortHandler(QObject *parent)
    : QObject(parent),
      serial(new QSerialPort(this)) // Inicjalizacja obiektu QSerialPort
{
    // Połączenie sygnału readyRead z portu do slotu readData w tej klasie
    connect(serial, &QSerialPort::readyRead, this, &SerialPortHandler::readData);
    // Połączenie sygnału errorOccurred z portu do slotu handleError w tej klasie
    connect(serial, &QSerialPort::errorOccurred, this, &SerialPortHandler::handleError);
}

/******************************************************************************/
/**
 * @brief Destruktor klasy SerialPortHandler.
 * Zamyka port szeregowy, jeśli jest otwarty.
 */
SerialPortHandler::~SerialPortHandler() {
    closePort(); // Wywołanie metody zamykającej port
}

/******************************************************************************/
/**
 * @brief Otwiera i konfiguruje port szeregowy.
 * Ustawia nazwę portu, prędkość transmisji (baud rate) oraz inne parametry komunikacji.
 * Jeśli port był już otwarty, najpierw go zamyka.
 * Czyści bufor wejściowy po pomyślnym otwarciu.
 * @param portName Nazwa portu szeregowego do otwarcia (np. "COM3" lub "/dev/ttyUSB0").
 * @param baudRate Prędkość transmisji (domyślnie 115200).
 * @return Zwraca `true` jeśli port został pomyślnie otwarty, `false` w przeciwnym razie.
 */
bool SerialPortHandler::openPort(const QString &portName, qint32 baudRate) {
    // Sprawdzenie, czy port jest już otwarty
    if (serial->isOpen()) {
        qInfo() << "Closing previously open port:" << serial->portName();
        serial->close(); // Zamknięcie poprzednio otwartego portu
    }
    // Ustawienie parametrów portu
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
         buffer.clear(); // Wyczyszczenie bufora danych
         serial->clear(QSerialPort::Input); // Wyczyszczenie bufora wejściowego portu
         return true; // Zwrócenie sukcesu
    } else {
         // Zgłoszenie ostrzeżenia w przypadku niepowodzenia
         qWarning() << "Failed to open port" << portName << "Error:" << getLastError();
         return false; // Zwrócenie porażki
    }
}

/******************************************************************************/
/**
 * @brief Zamyka aktualnie otwarty port szeregowy.
 * Jeśli port nie jest otwarty, funkcja nic nie robi.
 * Czyści wewnętrzny bufor danych.
 */
void SerialPortHandler::closePort() {
    // Sprawdzenie, czy obiekt portu istnieje i czy port jest otwarty
    if (serial && serial->isOpen()) {
        qInfo() << "Closing port:" << serial->portName();
        serial->close(); // Zamknięcie portu
        buffer.clear(); // Wyczyszczenie bufora danych
    }
}

/******************************************************************************/
/**
 * @brief Zwraca opis ostatniego błędu portu szeregowego.
 * @return QString zawierający opis ostatniego błędu lub informację, że obiekt QSerialPort nie został zainicjalizowany.
 */
QString SerialPortHandler::getLastError() const {
    // Sprawdzenie, czy obiekt portu istnieje
    if (serial) {
        return serial->errorString(); // Zwrócenie opisu błędu z obiektu QSerialPort
    }
    // Zwrócenie informacji o braku inicjalizacji obiektu
    return tr("Serial object not initialized.");
}

/******************************************************************************/
/**
 * @brief Slot wywoływany, gdy nowe dane są dostępne do odczytu z portu szeregowego.
 * Odczytuje dostępne dane, dodaje je do wewnętrznego bufora, a następnie próbuje
 * przetwarzać kompletne linie (zakończone znakiem '\n'), parsować je i emitować sygnał newDataReceived.
 */
void SerialPortHandler::readData() {
    // Sprawdzenie, czy port jest poprawnie zainicjalizowany, otwarty i czy można z niego czytać
    if (!serial || !serial->isOpen() || !serial->isReadable()) {
        return; // Wyjście z funkcji, jeśli warunki nie są spełnione
    }

    try {
         // Sprawdzenie, czy są dostępne dane do odczytu
         if (serial->bytesAvailable() > 0) {
             buffer.append(serial->readAll()); // Dodanie odczytanych danych do bufora
         } else {
             return; // Wyjście, jeśli nie ma danych
         }
    } catch(...) {
         // Obsługa nieoczekiwanego wyjątku podczas odczytu
         qWarning() << "Exception while reading serial data.";
         buffer.clear(); // Wyczyszczenie bufora w razie błędu
         return;
    }

    // Przetwarzanie bufora w poszukiwaniu kompletnych linii (zakończonych '\n')
    while (buffer.contains('\n')) {
        int index = buffer.indexOf('\n'); // Znalezienie pozycji znaku nowej linii
        QByteArray line = buffer.left(index).trimmed(); // Wyodrębnienie linii i usunięcie białych znaków z początku/końca
        buffer.remove(0, index + 1); // Usunięcie przetworzonej linii z bufora

        // Pominięcie pustych linii
        if (line.isEmpty()) {
            continue;
        }

        // Podział linii na części względem przecinka
        QList<QByteArray> values = line.split(',');

        // Sprawdzenie, czy liczba wartości zgadza się z oczekiwaną
        if (values.size() == EXPECTED_VALUE_COUNT_SERIAL) {
            QVector<float> parsedValues; // Wektor na sparsowane wartości
            parsedValues.reserve(EXPECTED_VALUE_COUNT_SERIAL); // Rezerwacja pamięci
            bool conversionOk = true; // Flaga poprawności konwersji

            // Iteracja przez podzielone wartości i próba konwersji na float
            for (const QByteArray &val : values) {
                bool ok;
                float floatVal = val.toFloat(&ok); // Konwersja na float
                if (!ok) {
                    // Zgłoszenie ostrzeżenia w przypadku błędu konwersji
                    qWarning() << "Failed to convert value to float:" << val << "in line:" << line;
                    conversionOk = false; // Ustawienie flagi błędu
                    break; // Przerwanie pętli dla tej linii
                }
                parsedValues.append(floatVal); // Dodanie sparsowanej wartości do wektora
            }

            // Jeśli wszystkie konwersje się powiodły, emituj sygnał
            if (conversionOk) {
                emit newDataReceived(parsedValues);
            }

        } else {
            // Zgłoszenie ostrzeżenia o nieprawidłowej liczbie wartości w linii
            qWarning() << "Received line with incorrect value count (" << values.size()
                       << ", expected" << EXPECTED_VALUE_COUNT_SERIAL << "):" << line;
        }
    }
}

/******************************************************************************/
/**
 * @brief Slot wywoływany w przypadku wystąpienia błędu sprzętowego lub konfiguracyjnego portu szeregowego.
 * Ignoruje błędy typu NoError oraz TimeoutError. Dla innych błędów emituje sygnał errorOccurred.
 * @param error Kod błędu zgłoszony przez QSerialPort.
 */
void SerialPortHandler::handleError(QSerialPort::SerialPortError error) {
    // Ignorowanie braku błędu
    if (error == QSerialPort::NoError) {
        return;
    }
    // Ignorowanie błędu timeout (często nie jest to krytyczny błąd)
    if (error == QSerialPort::TimeoutError) {
         return;
    }

    // Pobranie opisu błędu
    QString errorString = getLastError();
    // Zgłoszenie ostrzeżenia z kodem i opisem błędu
    qWarning() << "Serial port error occurred:" << error << "-" << errorString;
    // Emitowanie sygnału o błędzie
    emit errorOccurred(error, errorString);
}