/**
 * @file SerialPortHandler.h
 * @brief Definiuje klasę SerialPortHandler do obsługi komunikacji przez port szeregowy.
 * @author Mateusz Wojtaszek
 * - 2025-05-20 - Wersja 1.2: Dodano obsługę GPS w ramce danych.
 * @bug Brak znanych błędów.
 *
 * @details Plik zawiera deklarację klasy SerialPortHandler, która jest odpowiedzialna za zarządzanie
 * operacjami portu szeregowego, takimi jak otwieranie, zamykanie, odczytywanie, weryfikacja (CRC) i parsowanie
 * danych przy użyciu QSerialPort z biblioteki Qt. Obsługuje format danych CSV (IMU + GPS) z sumą kontrolną CRC-16
 * i sygnalizuje błędy komunikacji.
 * @note Ta klasa opiera się na frameworku Qt, w szczególności na QSerialPort.
 * Oczekiwany format ramki danych: CSV_PAYLOAD*CRC16_HEX\r\n
 * CSV_PAYLOAD: 12 wartości IMU, 2 wartości GPS (LAT, LON)
 */

#ifndef SERIALPORTHANDLER_H
#define SERIALPORTHANDLER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVector>
#include <QString>

/**
 * @class SerialPortHandler
 * @brief Zarządza komunikacją przez port szeregowy, w tym odbiorem, weryfikacją CRC i parsowaniem danych.
 * @author Mateusz Wojtaszek
 *
 * @details Zapewnia solidny mechanizm interakcji z portem szeregowym. Kluczowe funkcjonalności obejmują:
 * - Otwieranie i zamykanie portu szeregowego z określonymi parametrami.
 * - Buforowanie i odczytywanie przychodzących danych.
 * - Weryfikację integralności danych za pomocą sumy kontrolnej CRC-16.
 * - Parsowanie danych w formacie CSV (12 wartości IMU + 2 wartości GPS) do wektora liczb zmiennoprzecinkowych.
 * - Emitowanie sygnałów o nowych, zweryfikowanych danych i błędach komunikacji, wykorzystując mechanizm sygnałów i slotów Qt.
 */
class SerialPortHandler : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor obiektu SerialPortHandler.
     * @author Mateusz Wojtaszek
     * @param parent [in] Opcjonalny wskaźnik na obiekt nadrzędny QObject.
     */
    explicit SerialPortHandler(QObject *parent = nullptr);

    /**
     * @brief Destruktor obiektu SerialPortHandler.
     * @author Mateusz Wojtaszek
     */
    ~SerialPortHandler() override;

    /**
     * @brief Otwiera i konfiguruje określony port szeregowy.
     * @author Mateusz Wojtaszek
     * @param portName [in] Nazwa portu szeregowego (np. "COM3" w Windows, "/dev/ttyUSB0" w Linux).
     * @param baudRate [in] Żądana prędkość transmisji (domyślnie 115200).
     * @return `true` jeśli port został pomyślnie otwarty i skonfigurowany.
     * @return `false` jeśli otwarcie portu nie powiodło się.
     */
    bool openPort(const QString &portName, qint32 baudRate = 115200);

    /**
     * @brief Zamyka aktualnie otwarty port szeregowy.
     * @author Mateusz Wojtaszek
     */
    void closePort();

    /**
     * @brief Pobiera czytelny dla człowieka opis ostatniego błędu QSerialPort.
     * @author Mateusz Wojtaszek
     * @return QString zawierający opis błędu.
     */
    QString getLastError() const;

signals:
    /**
     * @brief Emitowany, gdy kompletna linia danych została odebrana, zweryfikowana przez CRC i pomyślnie sparsowana.
     * @author Mateusz Wojtaszek
     *
     * @details Oczekuje się, że przychodzące dane będą w formacie CSV_PAYLOAD*CRC16_HEX.
     * Ładunek CSV (CSV_PAYLOAD) powinien zawierać określone wartości telemetryczne.
     * Oczekiwany format ładunku danych CSV to:
     * GYRO_X, GYRO_Y, GYRO_Z, ACC_X, ACC_Y, ACC_Z, MAG_X, MAG_Y, MAG_Z, ROLL, PITCH, YAW, GPS_LAT, GPS_LON.
     * Sygnał ten dostarcza sparsowane dane jako wektor liczb zmiennoprzecinkowych.
     * @param parsedDataFromSensors [out] Stała referencja do QVector<float> zawierającego sparsowane wartości (12 IMU + 2 GPS).
     * @note Liczba wartości w ładunku CSV musi odpowiadać oczekiwanej liczbie (14 dla obecnego formatu).
     */
    void newDataReceived(const QVector<float> &parsedDataFromSensors);

    /**
     * @brief Emitowany, gdy wystąpi błąd komunikacji szeregowej.
     * @author Mateusz Wojtaszek
     * @param error [out] Kod błędu QSerialPort::SerialPortError.
     * @param errorString [out] Stała referencja do QString opisującego błąd.
     */
    void errorOccurred(QSerialPort::SerialPortError error, const QString &errorString);

private slots:
    /**
     * @brief Wewnętrzny slot odczytujący i przetwarzający dane z portu szeregowego.
     * @author Mateusz Wojtaszek
     */
    void readData();

    /**
     * @brief Wewnętrzny slot obsługujący błędy portu szeregowego.
     * @author Mateusz Wojtaszek
     * @param error [in] Kod błędu QSerialPort::SerialPortError.
     */
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *serial = nullptr; ///< Wskaźnik na obiekt QSerialPort. @brief Wskaźnik na obiekt QSerialPort.
    QByteArray buffer;             ///< Wewnętrzny bufor na dane. @brief Wewnętrzny bufor na dane.

    /**
     * @var EXPECTED_VALUE_COUNT_SERIAL
     * @brief Definiuje oczekiwaną liczbę wartości w ładunku CSV (12 IMU + 2 GPS = 14).
     */
    const int EXPECTED_VALUE_COUNT_SERIAL = 14;

    /**
     * @brief Oblicza sumę kontrolną CRC-16/CCITT-FALSE.
     * @author Mateusz Wojtaszek
     * @param data Dane wejściowe.
     * @return 16-bitowa suma kontrolna CRC.
     */
    static uint16_t calculateCrc16(const QByteArray &data);
};

#endif // SERIALPORTHANDLER_H