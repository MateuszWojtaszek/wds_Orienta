#ifndef SERIALPORTHANDLER_H
#define SERIALPORTHANDLER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVector>
#include <QString>

/**
 * @brief Klasa obsługująca komunikację przez port szeregowy.
 *
 * Odpowiada za otwieranie, zamykanie portu, odczytywanie danych,
 * parsowanie ich do formatu QVector<float> oraz obsługę błędów.
 */
class SerialPortHandler : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy SerialPortHandler.
     * @param parent Wskaźnik na obiekt nadrzędny (opcjonalny).
     */
    explicit SerialPortHandler(QObject *parent = nullptr);

    /**
     * @brief Destruktor klasy SerialPortHandler.
     * Zamyka port szeregowy, jeśli jest otwarty.
     */
    ~SerialPortHandler() override;

    /**
     * @brief Otwiera i konfiguruje port szeregowy.
     * Ustawia nazwę portu, prędkość transmisji (baud rate) oraz inne parametry komunikacji.
     * Jeśli port był już otwarty, najpierw go zamyka.
     * Czyści bufor wejściowy po pomyślnym otwarciu.
     * @param portName Nazwa portu szeregowego do otwarcia (np. "COM3" lub "/dev/ttyUSB0").
     * @param baudRate Prędkość transmisji (domyślnie 115200).
     * @return Zwraca `true` jeśli port został pomyślnie otwarty, `false` w przeciwnym razie.
     */
    bool openPort(const QString &portName, qint32 baudRate = 115200);

    /**
     * @brief Zamyka aktualnie otwarty port szeregowy.
     * Jeśli port nie jest otwarty, funkcja nic nie robi.
     * Czyści wewnętrzny bufor danych.
     */
    void closePort();

    /**
     * @brief Zwraca opis ostatniego błędu portu szeregowego.
     * @return QString zawierający opis ostatniego błędu lub informację, że obiekt QSerialPort nie został zainicjalizowany.
     */
    QString getLastError() const;

signals:
    /**
     * @brief Sygnał emitowany po otrzymaniu i sparsowaniu nowej linii danych.
     * Dane są oczekiwane w formacie CSV, rozdzielane przecinkami, zakończone znakiem nowej linii.
     * @param data Wektor zawierający sparsowane wartości zmiennoprzecinkowe (float).
     */
    void newDataReceived(QVector<float> data);

    /**
     * @brief Sygnał emitowany w przypadku wystąpienia błędu portu szeregowego.
     * @param error Kod błędu typu QSerialPort::SerialPortError.
     * @param errorString Opis błędu w formacie QString.
     */
    void errorOccurred(QSerialPort::SerialPortError error, const QString& errorString);

private slots:
    /**
     * @brief Slot wywoływany, gdy nowe dane są dostępne do odczytu z portu szeregowego.
     * Odczytuje dostępne dane, dodaje je do wewnętrznego bufora, a następnie próbuje
     * przetwarzać kompletne linie (zakończone znakiem '\n'), parsować je i emitować sygnał newDataReceived.
     */
    void readData();

    /**
     * @brief Slot wywoływany w przypadku wystąpienia błędu sprzętowego lub konfiguracyjnego portu szeregowego.
     * Ignoruje błędy typu NoError oraz TimeoutError. Dla innych błędów emituje sygnał errorOccurred.
     * @param error Kod błędu zgłoszony przez QSerialPort.
     */
    void handleError(QSerialPort::SerialPortError error);

private:
    /**
     * @brief Wskaźnik na obiekt QSerialPort używany do komunikacji.
     */
    QSerialPort *serial = nullptr;
    /**
     * @brief Bufor przechowujący dane odczytane z portu szeregowego przed ich przetworzeniem.
     */
    QByteArray buffer;
};
#endif //SERIALPORTHANDLER_H