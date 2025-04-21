/**
 * @file SerialPortHandler.h
 * @brief Definicja klasy SerialPortHandler do obsługi komunikacji przez port szeregowy.
 * @author Mateusz Wojtaszek
 * @date 2025-04-21 // Zaktualizowano datę zgodnie z kontekstem
 */

#ifndef SERIALPORTHANDLER_H
#define SERIALPORTHANDLER_H

#include <QObject>
#include <QSerialPort> // Wymagane dla QSerialPort
#include <QVector>     // Wymagane dla QVector
#include <QString>     // Wymagane dla QString

/**
 * @brief Klasa obsługująca komunikację przez port szeregowy.
 * @details Odpowiada za otwieranie, zamykanie portu, odczytywanie danych,
 * parsowanie ich (oczekując określonego formatu CSV) i emitowanie
 * sygnału z przetworzonymi danymi.
 */
class SerialPortHandler : public QObject {
    Q_OBJECT // Makro dla sygnałów i slotów

public:
    /**
     * @brief Konstruktor klasy SerialPortHandler.
     * @param parent Wskaźnik na obiekt nadrzędny (opcjonalny).
     */
    explicit SerialPortHandler(QObject *parent = nullptr);

    /**
     * @brief Destruktor klasy SerialPortHandler.
     * @details Zamyka port szeregowy, jeśli jest otwarty.
     */
    ~SerialPortHandler() override; // Użyj override, bo dziedziczy z QObject

    /**
     * @brief Otwiera określony port szeregowy z zadaną prędkością.
     * @param portName Nazwa portu do otwarcia (np. "COM3" lub "/dev/ttyUSB0").
     * @param baudRate Prędkość transmisji (domyślnie 115200).
     * @return true jeśli port został pomyślnie otwarty, false w przeciwnym razie.
     */
    bool openPort(const QString &portName, qint32 baudRate = 115200);

    /**
     * @brief Zamyka aktualnie otwarty port szeregowy.
     */
    void closePort();

    /**
     * @brief Zwraca ostatni błąd, jaki wystąpił w QSerialPort.
     * @details Przydatne do diagnozowania problemów z otwarciem portu.
     * @return Łańcuch znaków opisujący ostatni błąd.
     */
    QString getLastError() const;

signals:
    /**
     * @brief Sygnał emitowany, gdy nowa, poprawnie sparsowana linia danych zostanie odebrana.
     * @param data Wektor float zawierający sparsowane wartości z linii.
     */
    void newDataReceived(QVector<float> data);

    /**
     * @brief Sygnał emitowany, gdy wystąpi błąd portu szeregowego.
     * @param error Kod błędu QSerialPort::SerialPortError.
     * @param errorString Opis błędu.
     */
    void errorOccurred(QSerialPort::SerialPortError error, const QString& errorString);


private slots:
    /**
     * @brief Slot wywoływany, gdy dane są dostępne do odczytu z portu szeregowego.
     * @details Odczytuje dane, buforuje je, szuka kompletnych linii (zakończonych '\n'),
     * parsuje je i emituje sygnał newDataReceived.
     */
    void readData();

    /**
     * @brief Slot obsługujący błędy zgłaszane przez QSerialPort.
     * @param error Kod błędu.
     */
    void handleError(QSerialPort::SerialPortError error);

private:
    //!< Wskaźnik na obiekt QSerialPort zarządzający połączeniem.
    QSerialPort *serial;
    //!< Bufor przechowujący niekompletne dane odczytane z portu.
    QByteArray buffer;
};

#endif // SERIALPORTHANDLER_H