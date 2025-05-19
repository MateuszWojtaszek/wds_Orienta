/**
 * @file SerialPortUsage.cpp
 * @brief Przykład demonstrujący użycie klasy SerialPortHandler.
 * @details Ten przykład pokazuje, jak utworzyć instancję SerialPortHandler, połączyć się z jej sygnałami,
 * otworzyć port szeregowy i obsługiwać przychodzące dane oraz błędy.
 */

#include "SerialPortHandler.h"
#include <QCoreApplication>
#include <QDebug>

// Slot do obsługi nowych danych
void onNewData(const QVector<float> &data) {
    qDebug() << "Odebrano nowe dane:" << data;
    // Przetwórz dane (np. zaktualizuj interfejs użytkownika, zapisz do pliku itp.)
    // Przykład: Dostęp do poszczególnych wartości
    // if (data.size() == 12) {
    //     float accX = data[0];
    //     float accY = data[1];
    //     // ... i tak dalej
    // }
}

// Slot do obsługi błędów
void onError(QSerialPort::SerialPortError error, const QString &errorString) {
    qCritical() << "Błąd portu szeregowego:" << error << "-" << errorString;
    // Obsłuż błąd (np. spróbuj połączyć się ponownie, powiadom użytkownika)
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv); // Wymagane dla mechanizmu sygnałów/slotów Qt

    SerialPortHandler portHandler;

    // Połącz sygnały ze slotami
    QObject::connect(&portHandler, &SerialPortHandler::newDataReceived, &onNewData);
    QObject::connect(&portHandler, &SerialPortHandler::errorOccurred, &onError);

    // Skonfiguruj i otwórz port
    // Zastąp "COM3" lub "/dev/ttyS0" rzeczywistą nazwą portu
    QString portName = "COM3"; // Lub "/dev/ttyUSB0", itp.
    qint32 baudRate = 115200;

    if (portHandler.openPort(portName, baudRate)) {
        qInfo() << "Pomyślnie otwarto port" << portName;
        // Port jest teraz otwarty, a readData() będzie wywoływane asynchronicznie,
        // gdy nadejdą dane.
    } else {
        qWarning() << "Nie udało się otworzyć portu" << portName << ". Błąd:" << portHandler.getLastError();
        // Aplikacja może zakończyć działanie lub spróbować innych portów, jeśli otwarcie się nie powiedzie
        return -1;
    }

    qInfo() << "Aplikacja uruchomiona. Oczekiwanie na dane z portu szeregowego...";

    // Uruchom pętlę zdarzeń Qt, aby umożliwić przetwarzanie sygnałów/slotów
    // i utrzymać działanie aplikacji.
    int execResult = a.exec();

    // Sprzątanie: zamknij port po zakończeniu (chociaż destruktor również to obsługuje)
    portHandler.closePort();
    qInfo() << "Aplikacja zakończona.";

    return execResult;
}