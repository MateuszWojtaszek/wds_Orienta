/**
 * @file SerialPortHandler.h
 * @brief Definiuje klasę SerialPortHandler do obsługi komunikacji przez port szeregowy.
 * @details Plik zawiera deklarację klasy SerialPortHandler, która jest odpowiedzialna za zarządzanie
 * operacjami portu szeregowego, takimi jak otwieranie, zamykanie, odczytywanie i parsowanie
 * danych przy użyciu QSerialPort z biblioteki Qt. Obsługuje format danych CSV i sygnalizuje błędy komunikacji.
 * @author [Twoje Imię/Nazwa Organizacji]
 * @date [Data Utworzenia/Ostatniej Głównej Poprawki, np. 19 maja 2025]
 * @version 1.0
 *
 * @note Ta klasa opiera się na frameworku Qt, w szczególności na QSerialPort.
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
 * @brief Zarządza komunikacją przez port szeregowy, w tym odbiorem i parsowaniem danych.
 * @details Zapewnia solidny mechanizm interakcji z portem szeregowym. Kluczowe funkcjonalności obejmują:
 * - Otwieranie i zamykanie portu szeregowego z określonymi parametrami.
 * - Buforowanie i odczytywanie przychodzących danych.
 * - Parsowanie danych w formacie CSV (wartości oddzielone przecinkami) do wektora liczb zmiennoprzecinkowych.
 * - Emitowanie sygnałów o nowych danych i błędach komunikacji, wykorzystując mechanizm sygnałów i slotów Qt.
 *
 * @example SerialPortUsage_PL.cpp
 * To jest przykład użycia klasy SerialPortHandler:
 */
class SerialPortHandler : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor obiektu SerialPortHandler.
     * @details Inicjalizuje nową instancję QSerialPort i łączy jej sygnały (readyRead, errorOccurred)
     * z odpowiednimi slotami obsługującymi w tej klasie.
     * @param parent [in] Opcjonalny wskaźnik na obiekt nadrzędny QObject.
     */
    explicit SerialPortHandler(QObject *parent = nullptr);

    /**
     * @brief Destruktor obiektu SerialPortHandler.
     * @details Zapewnia, że port szeregowy zostanie zamknięty, jeśli był otwarty, zapobiegając wyciekom zasobów.
     */
    ~SerialPortHandler() override;

    /**
     * @brief Otwiera i konfiguruje określony port szeregowy.
     * @details Zamyka każdy wcześniej otwarty port przed próbą otwarcia nowego.
     * Konfiguruje parametry transmisji: prędkość transmisji (baud rate), bity danych (8), parzystość (Brak),
     * bity stopu (Jeden) i kontrolę przepływu (Brak).
     * Czyści wewnętrzny bufor danych po pomyślnym otwarciu.
     * @param portName [in] Nazwa portu szeregowego (np. "COM3" w Windows, "/dev/ttyUSB0" w Linux).
     * @param baudRate [in] Żądana prędkość transmisji (domyślnie 115200).
     * @return `true` jeśli port został pomyślnie otwarty i skonfigurowany.
     * @return `false` jeśli otwarcie portu nie powiodło się (np. port niedostępny, odmowa dostępu).
     * W przypadku niepowodzenia emitowany jest sygnał errorOccurred.
     * @see errorOccurred
     * @see closePort()
     * @note Jeśli port jest już otwarty, zostanie zamknięty i ponownie otwarty z nowymi ustawieniami.
     */
    bool openPort(const QString &portName, qint32 baudRate = 115200);

    /**
     * @brief Zamyka aktualnie otwarty port szeregowy.
     * @details Jeśli port nie jest otwarty, ta funkcja nic nie robi.
     * Czyści wewnętrzny bufor danych.
     */
    void closePort();

    /**
     * @brief Pobiera czytelny dla człowieka opis ostatniego błędu QSerialPort.
     * @details Jeśli obiekt QSerialPort nie został zainicjalizowany, zwracany jest komunikat
     * "Obiekt szeregowy nie został zainicjalizowany." ("Serial object not initialized.").
     * @return QString zawierający opis błędu.
     */
    QString getLastError() const;

signals:
    /**
     * @brief Emitowany, gdy kompletna linia danych została odebrana i pomyślnie sparsowana.
     * @details Oczekuje się, że przychodzące dane będą w formacie CSV.
     * Oczekiwany format danych to: ACC_X, ACC_Y, ACC_Z, GYRO_X, GYRO_Y, GYRO_Z, MAG_X, MAG_Y, MAG_Z, ROLL, PITCH, YAW.
     * Sygnał ten dostarcza sparsowane dane jako wektor liczb zmiennoprzecinkowych.
     * @param parsedDataFromSensors [out] Stała referencja do QVector<float> zawierającego sparsowane wartości czujników.
     * Użycie `const&` pozwala uniknąć niepotrzebnego kopiowania danych.
     * @note Liczba wartości musi odpowiadać oczekiwanej liczbie (12 dla określonego formatu).
     */
    void newDataReceived(const QVector<float> &parsedDataFromSensors);

    /**
     * @brief Emitowany, gdy wystąpi błąd komunikacji szeregowej.
     * @details Sygnał ten jest emitowany dla błędów QSerialPort innych niż `QSerialPort::NoError` oraz
     * `QSerialPort::TimeoutError` (ponieważ TimeoutError może być częstym, niekrytycznym zdarzeniem).
     * @param error [out] Kod błędu QSerialPort::SerialPortError wskazujący typ błędu.
     * @param errorString [out] Stała referencja do QString opisującego błąd.
     */
    void errorOccurred(QSerialPort::SerialPortError error, const QString &errorString);

private slots:
    /**
     * @brief Wewnętrzny slot wywoływany, gdy nowe dane są dostępne do odczytu z portu szeregowego.
     * @details Odczytuje wszystkie dostępne dane z QSerialPort, dołącza je do wewnętrznego bufora
     * i przetwarza bufor linia po linii (oczekuje się, że linie są zakończone znakiem '\\n').
     * Każda linia jest parsowana jako CSV, a jeśli jest poprawna, emitowany jest sygnał `newDataReceived`.
     * @see newDataReceived
     * @warning Ten slot bezpośrednio oddziałuje z wewnętrznym stanem QSerialPort.
     */
    void readData();

    /**
     * @brief Wewnętrzny slot wywoływany, gdy wystąpi błąd QSerialPort.
     * @details Filtruje błędy `QSerialPort::NoError` i `QSerialPort::TimeoutError`.
     * Dla innych błędów loguje błąd i emituje sygnał `errorOccurred`.
     * @param error [in] Kod błędu QSerialPort::SerialPortError zgłoszony przez QSerialPort.
     * @see errorOccurred
     */
    void handleError(QSerialPort::SerialPortError error);

private:
    /**
     * @var serial
     * @brief Wskaźnik na obiekt QSerialPort używany do komunikacji szeregowej.
     * @details Ten obiekt obsługuje niskopoziomowe interakcje z portem szeregowym.
     * Jest zarządzany przez system własności rodzic-dziecko Qt.
     */
    QSerialPort *serial = nullptr;

    /**
     * @var buffer
     * @brief Wewnętrzny bufor do przechowywania przychodzących danych z portu szeregowego.
     * @details Dane są tutaj gromadzone, dopóki nie zostanie napotkany znak nowej linii ('\\n'),
     * sygnalizujący kompletną linię gotową do parsowania.
     */
    QByteArray buffer;

    /**
     * @var EXPECTED_VALUE_COUNT_SERIAL
     * @brief Definiuje oczekiwaną liczbę wartości oddzielonych przecinkami w linii danych.
     * @details Używane przez readData() do walidacji integralności przychodzących danych CSV.
     * Aktualnie ustawione na 12 na podstawie formatu danych z czujników.
     */
    static const int EXPECTED_VALUE_COUNT_SERIAL = 12;
};

#endif // SERIALPORTHANDLER_H