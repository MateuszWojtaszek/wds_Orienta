/**
 * @file SerialPortHandler.h
 * @brief Definicja klasy SerialPortHandler do obsługi komunikacji przez port szeregowy.
 * @details Plik zawiera deklarację klasy SerialPortHandler, odpowiedzialnej za otwieranie,
 * zamykanie oraz odbieranie i parsowanie danych z portu szeregowego przy użyciu Qt.
 * Dane odbierane są w formacie CSV, a błędy komunikacji są obsługiwane poprzez sygnały.
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
 * @brief Klasa obsługująca komunikację przez port szeregowy.
 * @details Zapewnia mechanizm otwierania i zamykania portu szeregowego, odbierania i buforowania danych,
 * parsowania ich w formacie CSV na wektor wartości zmiennoprzecinkowych oraz obsługę błędów
 * z wykorzystaniem mechanizmu sygnałów i slotów Qt.
 */
class SerialPortHandler : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy SerialPortHandler.
     * @details Tworzy nowy obiekt QSerialPort i podłącza odpowiednie sygnały do slotów.
     * @param parent [in] Opcjonalny wskaźnik na obiekt nadrzędny Qt.
     */
    explicit SerialPortHandler(QObject *parent = nullptr);

    /**
     * @brief Destruktor klasy SerialPortHandler.
     * @details Automatycznie zamyka port, jeśli jest otwarty.
     */
    ~SerialPortHandler() override;

    /**
     * @brief Otwiera i konfiguruje port szeregowy.
     * @details Jeśli port był wcześniej otwarty, zostaje zamknięty. Ustawiane są parametry
     * transmisji (baud rate, bity danych, parzystość, stop bity, kontrola przepływu).
     * Po pomyślnym otwarciu portu czyszczony jest bufor.
     * @param portName [in] Nazwa portu, np. "COM3" lub "/dev/ttyUSB0".
     * @param baudRate [in] Prędkość transmisji (domyślnie 115200).
     * @return `true`, jeśli port został pomyślnie otwarty, w przeciwnym razie `false`.
     */
    bool openPort(const QString &portName, qint32 baudRate = 115200);

    /**
     * @brief Zamyka port szeregowy.
     * @details Jeśli port nie jest otwarty, funkcja nic nie robi. Czyści również bufor odbiorczy.
     */
    void closePort();

    /**
     * @brief Zwraca opis ostatniego błędu QSerialPort.
     * @details W przypadku braku inicjalizacji portu zwraca komunikat o błędzie.
     * @return Opis błędu w postaci QString.
     */
    QString getLastError() const;

signals:
    /**
     * @brief Sygnał emitowany po odebraniu i przetworzeniu nowej linii danych.
     * @details Parsuje dane wejściowe w formacie CSV. Wymagana jest dokładnie określona liczba wartości.
     * Format danych: ACC_X, ACC_Y, ACC_Z, GYRO_X, GYRO_Y, GYRO_Z, MAG_X, MAG_Y, MAG_Z, ROLL, PITCH, YAW.
     * @param parsedDataFromSensors [in] Wektor sparsowanych wartości (przekazywany jako `const&`).
     * Użycie `const referencji` w celu uniknięcia niepotrzebnej kopii.
     */
    void newDataReceived(const QVector<float> &parsedDataFromSensors);

    /**
     * @brief Sygnał emitowany w przypadku błędu komunikacji szeregowej.
     * @details Emitowany dla błędów innych niż `NoError` i `TimeoutError`.
     * @param error Kod błędu QSerialPort (parametr wyjściowy sygnału).
     * @param errorString Opis błędu (parametr wyjściowy sygnału, przekazywany jako `const&`).
     */
    void errorOccurred(QSerialPort::SerialPortError error, const QString &errorString);

private slots:
    /**
     * @brief Slot wywoływany po odebraniu danych z portu szeregowego.
     * @details Odczytuje dane z portu, dodaje je do bufora i przetwarza linie zakończone znakiem `\n`.
     * Linie są parsowane i konwertowane na wartości float, które następnie są przekazywane przez sygnał `newDataReceived`.
     */
    void readData();

    /**
     * @brief Slot reagujący na wystąpienie błędu portu szeregowego.
     * @details Dla błędów innych niż `NoError` i `TimeoutError` emituje sygnał `errorOccurred`.
     * @param error [in] Kod błędu zgłoszony przez QSerialPort.
     */
    void handleError(QSerialPort::SerialPortError error);

private:
    /**
     * @var serial
     * @brief Obiekt portu szeregowego.
     * @details Używany do odbioru danych z urządzenia zewnętrznego. Wskaźnik zarządzany przez Qt.
     */
    QSerialPort *serial = nullptr;

    /**
     * @var buffer
     * @brief Bufor na dane odebrane z portu szeregowego.
     * @details Zawiera dane przed ich konwersją. Linie są oddzielane znakiem `\n` i przetwarzane pojedynczo.
     */
    QByteArray buffer;
};

#endif // SERIALPORTHANDLER_H