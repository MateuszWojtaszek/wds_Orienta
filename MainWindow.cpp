// #############################################################################
// #                            IMPLEMENTACJA                                  #
// #                    Autor: Mateusz Wojtaszek 275419                        #
// #############################################################################

#include "MainWindow.h"
/******************************/
/**
 * @brief Konstruktor klasy MainWindow.
 * @details Inicjalizuje główne okno aplikacji, tworzy i konfiguruje niezbędne komponenty:
 * widgety interfejsu użytkownika (QStackedWidget), handlery danych
 * (ImuDataHandler, GPSDataHandler), handler portu szeregowego (SerialPortHandler),
 * timer symulacji. Ładuje dane symulacyjne, tworzy menu, ustawia połączenia
 * sygnałów i slotów oraz wyświetla okno na pełnym ekranie.
 * @param parent Wskaźnik na widget rodzica, domyślnie nullptr.
 */
/******************************/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    simulationTimer(new QTimer(this)), // Inicjalizacja timera
    serialHandler(new SerialPortHandler(this)) // Inicjalizacja handlera portu szeregowego
{
    setWindowTitle(tr("Sensor Visualizer")); // Ustawienie tytułu okna (z obsługą tłumaczeń)

    // Próba załadowania danych symulacyjnych
    if (!loadSimulationData(SIMULATION_DATA_FILE_PATH)) {
         // Wyświetlenie ostrzeżenia, jeśli ładowanie się nie powiodło
         QMessageBox::warning(this, tr("Simulation Data"), tr("Could not load simulation data from: %1").arg(SIMULATION_DATA_FILE_PATH));
    }

    // Inicjalizacja handlerów danych
    imuHandler = new ImuDataHandler(this);
    gpsHandler = new GPSDataHandler(this);

    // Inicjalizacja i konfiguracja QStackedWidget
    stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(imuHandler); // Dodanie widoku IMU
    stackedWidget->addWidget(gpsHandler); // Dodanie widoku GPS

    setCentralWidget(stackedWidget); // Ustawienie stackedWidget jako centralnego widgetu okna
    stackedWidget->setCurrentWidget(imuHandler); // Ustawienie widoku IMU jako domyślnego

    createMenus(); // Utworzenie menu aplikacji

    showFullScreen(); // Wyświetlenie okna na pełnym ekranie

    // Połączenie sygnałów i slotów
    connect(this, &MainWindow::switchToIMU, this, &MainWindow::showIMUHandler); // Przełączanie na widok IMU
    connect(this, &MainWindow::switchToGPS, this, &MainWindow::showGPSHandler); // Przełączanie na widok GPS
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::updateSimulationData); // Obsługa timera symulacji
    connect(serialHandler, &SerialPortHandler::newDataReceived, this, &MainWindow::handleSerialData); // Obsługa danych z portu szeregowego
}

/******************************/
/**
 * @brief Destruktor klasy MainWindow.
 * @details Domyślny destruktor. Qt zajmuje się zwolnieniem pamięci obiektów potomnych
 * dzięki mechanizmowi rodzic-dziecko.
 */
/******************************/
MainWindow::~MainWindow() = default;

/******************************/
/**
 * @brief Tworzy i konfiguruje paski menu aplikacji.
 * @details Inicjalizuje główny pasek menu (`QMenuBar`) oraz dodaje do niego
 * menu "Sensor" (z akcjami IMU, GPS) i "Settings" (z podmenu "Language",
 * akcją "Simulation Mode" i "Select Port"). Łączy akcje menu
 * z odpowiednimi slotami.
 */
/******************************/
void MainWindow::createMenus() {
    QMenuBar *bar = menuBar(); // Pobranie paska menu

    // Menu "Sensor"
    QMenu *sensorMenu = bar->addMenu(tr("Sensor"));
    QAction *imuAction = sensorMenu->addAction(tr("IMU"));
    QAction *gpsAction = sensorMenu->addAction(tr("GPS"));

    // Połączenie akcji menu "Sensor" ze slotami przełączającymi widoki
    connect(imuAction, &QAction::triggered, this, &MainWindow::switchToIMU);
    connect(gpsAction, &QAction::triggered, this, &MainWindow::switchToGPS);

    // Menu "Settings"
    QMenu *settingsMenu = bar->addMenu(tr("Settings"));

    // Podmenu "Language"
    QMenu *languageMenu = settingsMenu->addMenu(tr("Language"));
    QAction *englishAction = languageMenu->addAction(tr("English"));
    QAction *polishAction = languageMenu->addAction(tr("Polish"));

    settingsMenu->addSeparator(); // Separator w menu

    // Akcja "Simulation Mode"
    QAction *simulationModeAction = settingsMenu->addAction(tr("Simulation Mode"));
    simulationModeAction->setCheckable(true); // Umożliwia zaznaczenie/odznaczenie
    simulationModeAction->setChecked(simulationMode); // Ustawienie początkowego stanu

    // Akcja "Select Port"
    QAction *selectPortAction = settingsMenu->addAction(tr("Select Port"));

    // Połączenie akcji menu "Settings" z odpowiednimi slotami
    connect(englishAction, &QAction::triggered, this, &MainWindow::setEnglishLanguage);
    connect(polishAction, &QAction::triggered, this, &MainWindow::setPolishLanguage);
    connect(simulationModeAction, &QAction::triggered, this, &MainWindow::toggleSimulationMode);
    connect(selectPortAction, &QAction::triggered, this, &MainWindow::selectPort);
}

/******************************/
/**
 * @brief Slot do ustawienia języka angielskiego (placeholder).
 * @details Wywoływany po wybraniu opcji "English" w menu języka. Aktualnie
 * wyświetla jedynie komunikat informacyjny o zmianie języka.
 * W przyszłości powinien implementować faktyczną zmianę języka interfejsu.
 */
/******************************/
void MainWindow::setEnglishLanguage() {
    // Wyświetlenie komunikatu - implementacja zmiany języka wymagałaby np. QTranslator
    QMessageBox::information(this, tr("Language Change"), tr("Language changed to English (placeholder)."));
}

/******************************/
/**
 * @brief Slot do ustawienia języka polskiego (placeholder).
 * @details Wywoływany po wybraniu opcji "Polish" w menu języka. Aktualnie
 * wyświetla jedynie komunikat informacyjny o zmianie języka.
 * W przyszłości powinien implementować faktyczną zmianę języka interfejsu.
 */
/******************************/
void MainWindow::setPolishLanguage() {
    // Wyświetlenie komunikatu - implementacja zmiany języka wymagałaby np. QTranslator
    QMessageBox::information(this, tr("Language Change"), tr("Language changed to Polish (placeholder)."));
}

/******************************/
/**
 * @brief Ładuje dane symulacyjne z podanego pliku.
 * @details Otwiera plik tekstowy pod wskazaną ścieżką, czyta go linia po linii,
 * dzieli każdą linię na wartości liczbowe (oczekując formatu CSV),
 * konwertuje je na `float` i zapisuje w wektorze `loadedData`.
 * Sprawdza poprawność formatu danych (oczekiwana liczba wartości).
 * @param filePath Ścieżka do pliku z danymi symulacyjnymi.
 * @return `true` jeśli ładowanie danych powiodło się, `false` w przeciwnym razie.
 */
/******************************/
bool MainWindow::loadSimulationData(const QString& filePath) {
    QFile file(filePath); // Utworzenie obiektu QFile
    // Próba otwarcia pliku do odczytu w trybie tekstowym
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file); // Strumień tekstowy do odczytu z pliku
        loadedData.clear(); // Wyczyszczenie ewentualnych starych danych
        // Odczyt pliku linia po linii
        while (!in.atEnd()) {
            QString line = in.readLine(); // Odczytanie linii
            // Podział linii na części względem przecinka, pomijając puste części
            QStringList values = line.split(',', Qt::SkipEmptyParts);
            // Sprawdzenie, czy liczba wartości zgadza się z oczekiwaną
            if (values.size() == EXPECTED_DATA_SIZE) {
                QVector<float> parsedLine; // Wektor na sparsowane wartości z linii
                parsedLine.reserve(EXPECTED_DATA_SIZE); // Rezerwacja miejsca dla optymalizacji
                bool conversionOk = true; // Flaga poprawności konwersji
                // Iteracja przez wartości tekstowe
                for (const QString &val : values) {
                    float fVal = val.toFloat(&conversionOk); // Konwersja na float
                    // Sprawdzenie, czy konwersja się powiodła
                    if (!conversionOk) {
                         qWarning() << "Failed to convert value to float in line:" << line; // Logowanie błędu
                         parsedLine.clear(); // Wyczyść częściowo sparsowaną linię
                         break; // Przerwij przetwarzanie tej linii
                    }
                    parsedLine.append(fVal); // Dodanie sparsowanej wartości do wektora
                }
                // Jeśli linia została poprawnie sparsowana (nie była pusta po błędzie konwersji)
                if (!parsedLine.isEmpty()) {
                   loadedData.append(parsedLine); // Dodanie sparsowanej linii do głównych danych
                }
            } else if (!line.trimmed().isEmpty()) { // Jeśli linia nie jest pusta, ale ma złą liczbę wartości
                 qWarning() << "Skipping line with incorrect number of values (" << values.size() << "):" << line; // Logowanie ostrzeżenia
            }
        }
        file.close(); // Zamknięcie pliku
        qInfo() << "Successfully loaded" << loadedData.size() << "lines of simulation data from" << filePath; // Logowanie sukcesu
        return true; // Zwrócenie informacji o sukcesie
    } else {
        // Logowanie błędu, jeśli nie udało się otworzyć pliku
        qWarning() << "Failed to open simulation data file:" << filePath << "Error:" << file.errorString();
        return false; // Zwrócenie informacji o porażce
    }
}

/******************************/
/**
 * @brief Przełącza tryb działania aplikacji między symulacją a odczytem z portu szeregowego.
 * @details Aktywowany przez akcję "Simulation Mode" w menu. Jeśli tryb symulacji jest
 * włączany, zamyka port szeregowy (jeśli był otwarty), uruchamia timer symulacji
 * (jeśli dane symulacyjne są załadowane) i aktualizuje stan znacznika GPS.
 * Jeśli tryb symulacji jest wyłączany, zatrzymuje timer. Aktualizuje również
 * stan zaznaczenia odpowiedniej akcji w menu.
 */
/******************************/
void MainWindow::toggleSimulationMode() {
    simulationMode = !simulationMode; // Przełączenie flagi trybu symulacji
    currentDataIndex = 0; // Zresetowanie indeksu danych symulacyjnych

    if (simulationMode) { // Włączanie trybu symulacji
        // Jeśli port szeregowy jest połączony, zamknij go
        if (serialConnected) {
            serialHandler->closePort();
            serialConnected = false;
            qInfo() << "Serial port closed due to enabling simulation mode.";
        }
        // Sprawdź, czy dane symulacyjne są załadowane
        if (!loadedData.isEmpty()) {
             // Aktualizacja pozycji startowej markera GPS (jeśli handler istnieje)
             if (gpsHandler) {
                 gpsHandler->updateMarker(baseLatitude, baseLongitude);
             }
            simulationTimer->start(SIMULATION_TIMER_INTERVAL_MS); // Uruchomienie timera symulacji
            qInfo() << "Simulation mode enabled. Timer started.";
        } else {
            // Ostrzeżenie, jeśli próbowano włączyć symulację bez danych
            QMessageBox::warning(this, tr("Simulation Mode"), tr("Simulation mode enabled, but no simulation data loaded."));
            qInfo() << "Simulation mode enabled, but no data loaded.";
            simulationMode = false; // Cofnięcie zmiany trybu
            // Aktualizacja stanu przycisku menu - odznaczenie
             QList<QAction*> actions = menuBar()->findChildren<QAction*>();
             for(QAction* action : actions) {
                 if(action->text() == tr("Simulation Mode")) {
                     action->setChecked(false); // Odznaczenie akcji w menu
                     break;
                 }
             }
        }
    } else { // Wyłączanie trybu symulacji
        simulationTimer->stop(); // Zatrzymanie timera
        qInfo() << "Simulation mode disabled. Timer stopped.";
         // Aktualizacja stanu przycisku menu - odznaczenie
         QList<QAction*> actions = menuBar()->findChildren<QAction*>();
         for(QAction* action : actions) {
             if(action->text() == tr("Simulation Mode")) {
                 action->setChecked(simulationMode); // Ustawienie stanu akcji (na false)
                 break;
             }
         }
    }
     // Ponowna (lub pierwsza, jeśli nie było danych) aktualizacja stanu przycisku menu
     QList<QAction*> actions = menuBar()->findChildren<QAction*>();
     for(QAction* action : actions) {
         if(action->text() == tr("Simulation Mode")) {
             action->setChecked(simulationMode); // Ustawienie właściwego stanu akcji
             break;
         }
     }
}

/******************************/
/**
 * @brief Aktualizuje dane w trybie symulacji.
 * @details Slot wywoływany cyklicznie przez `simulationTimer`. Sprawdza, czy symulacja
 * powinna być kontynuowana. Jeśli tak, pobiera kolejny zestaw danych
 * z `loadedData`, przetwarza go za pomocą `processImuData`, aktualizuje
 * symulowany znacznik GPS i inkrementuje indeks bieżących danych.
 * Obsługuje również zakończenie symulacji.
 */
/******************************/
void MainWindow::updateSimulationData() {
    // Warunki przerwania symulacji: połączenie szeregowe, wyłączony tryb symulacji, brak danych
    if (serialConnected || !simulationMode || loadedData.isEmpty()) {
        simulationTimer->stop(); // Zatrzymaj timer, jeśli warunki są spełnione
        return; // Zakończ funkcję
    }

    // Sprawdź, czy osiągnięto koniec danych symulacyjnych
    if (checkSimulationEndAndUpdateState()) {
        return; // Jeśli symulacja się zakończyła, zakończ funkcję
    }

    // Pobranie bieżącego zestawu danych IMU z załadowanych danych
    const QVector<float> &imuData = loadedData[currentDataIndex];
    processImuData(imuData); // Przetworzenie danych IMU

    updateSimulatedGPSMarker(); // Aktualizacja symulowanej pozycji GPS

    currentDataIndex++; // Przejście do następnego zestawu danych
}

/******************************/
/**
 * @brief Sprawdza, czy osiągnięto koniec danych symulacyjnych i aktualizuje stan.
 * @details Porównuje bieżący indeks danych (`currentDataIndex`) z rozmiarem
 * załadowanych danych (`loadedData.size()`). Jeśli indeks jest większy lub równy,
 * oznacza to koniec symulacji. W takim przypadku zatrzymuje timer, wyświetla
 * komunikat, wyłącza tryb symulacji (ustawia `simulationMode` na `false`)
 * i odznacza odpowiednią akcję w menu.
 * @return `true` jeśli symulacja dobiegła końca, `false` w przeciwnym razie.
 */
/******************************/
bool MainWindow::checkSimulationEndAndUpdateState() {
    // Sprawdzenie, czy indeks wyszedł poza zakres załadowanych danych
    if (currentDataIndex >= loadedData.size()) {
        simulationTimer->stop(); // Zatrzymaj timer symulacji
        // Wyświetl komunikat o zakończeniu symulacji
        QMessageBox::information(this, tr("Simulation Ended"), tr("End of simulation data reached."));
        qInfo() << "Simulation data playback finished.";
        simulationMode = false; // Wyłącz tryb symulacji
        // Zaktualizuj stan zaznaczenia akcji "Simulation Mode" w menu
        QList<QAction*> actions = menuBar()->findChildren<QAction*>();
        for(QAction* action : actions) {
            if(action->text() == tr("Simulation Mode")) {
                 action->setChecked(false); // Odznacz akcję
                 break;
            }
        }
        return true; // Zwróć true, informując, że symulacja się zakończyła
    }
    return false; // Zwróć false, symulacja trwa nadal
}

/******************************/
/**
 * @brief Aktualizuje pozycję markera GPS w trybie symulacji.
 * @details Jeśli aplikacja jest w trybie symulacji i `gpsHandler` istnieje,
 * oblicza nową pozycję GPS na podstawie `baseLatitude`, `baseLongitude`,
 * `currentDataIndex` oraz parametrów oscylacji (`gpsOscillationAmplitude`,
 * `gpsOscillationSpeedFactor`), używając funkcji sinus i cosinus do
 * stworzenia ruchu kołowego/eliptycznego. Następnie wywołuje
 * `gpsHandler->updateMarker` z nowymi współrzędnymi.
 */
/******************************/
void MainWindow::updateSimulatedGPSMarker() {
    // Sprawdź, czy tryb symulacji jest aktywny i czy handler GPS istnieje
    if (simulationMode && gpsHandler) {
        // Obliczenie kąta oscylacji na podstawie indeksu danych i współczynnika prędkości
        double angleRad = static_cast<double>(currentDataIndex) * gpsOscillationSpeedFactor;
        // Obliczenie przesunięć dla szerokości i długości geograficznej
        double latOffset = gpsOscillationAmplitude * std::sin(angleRad);
        double lonOffset = gpsOscillationAmplitude * std::cos(angleRad);

        // Obliczenie aktualnych współrzędnych GPS
        double currentLatitude = baseLatitude + latOffset;
        double currentLongitude = baseLongitude + lonOffset;

        // Aktualizacja pozycji markera w handlerze GPS
        gpsHandler->updateMarker(currentLatitude, currentLongitude);
    }
}

/******************************/
/**
 * @brief Obsługuje nowe dane otrzymane z portu szeregowego.
 * @details Slot połączony z sygnałem `newDataReceived` obiektu `SerialPortHandler`.
 * Sprawdza, czy połączenie szeregowe jest aktywne i czy aplikacja nie jest
 * w trybie symulacji. Jeśli warunki są spełnione, przekazuje otrzymane dane
 * do przetworzenia przez funkcję `processImuData`.
 * @param data Wektor `float` zawierający dane sensoryczne odebrane przez port szeregowy.
 */
/******************************/
void MainWindow::handleSerialData(QVector<float> data) {
    // Przetwarzaj dane tylko jeśli port jest połączony i nie jesteśmy w trybie symulacji
    if (!serialConnected || simulationMode) {
        return; // Ignoruj dane w przeciwnym razie
    }
    processImuData(data); // Przekaż dane do głównej funkcji przetwarzającej
}

/******************************/
/**
 * @brief Przetwarza surowy wektor danych sensorycznych.
 * @details Funkcja przyjmuje wektor danych (z pliku symulacyjnego lub portu szeregowego),
 * sprawdza jego rozmiar, a następnie ekstrahuje wartości dla akcelerometru,
 * żyroskopu, magnetometru oraz kątów Roll, Pitch, Yaw. Przekazuje te dane
 * do `imuHandler` w celu aktualizacji wizualizacji 3D i wskaźników.
 * Oblicza również kurs (heading) na podstawie danych magnetometru
 * i aktualizuje kompas w `imuHandler`.
 * @param data Wektor `float` zawierający kompletny zestaw danych z sensorów
 * (Gyro X,Y,Z, Acc X,Y,Z, Mag X,Y,Z, Roll, Pitch, Yaw).
 */
/******************************/
void MainWindow::processImuData(const QVector<float>& data) {
    // Sprawdzenie, czy rozmiar otrzymanych danych jest zgodny z oczekiwanym
    if (data.size() != EXPECTED_DATA_SIZE) {
        qWarning() << "Received data packet with incorrect size:" << data.size() << "Expected:" << EXPECTED_DATA_SIZE;
        return; // Ignoruj niepoprawny pakiet danych
    }

    // Ekstrakcja danych żyroskopu (konwersja na int - może wymagać weryfikacji, czy to zamierzone)
    QVector<int> gyro = { static_cast<int>(data[GYRO_X_IDX]), static_cast<int>(data[GYRO_Y_IDX]), static_cast<int>(data[GYRO_Z_IDX]) };
    // Ekstrakcja danych akcelerometru (konwersja na int)
    QVector<int> acc  = { static_cast<int>(data[ACC_X_IDX]), static_cast<int>(data[ACC_Y_IDX]), static_cast<int>(data[ACC_Z_IDX]) };
    // Ekstrakcja danych magnetometru (konwersja na int)
    QVector<int> mag  = { static_cast<int>(data[MAG_X_IDX]), static_cast<int>(data[MAG_Y_IDX]), static_cast<int>(data[MAG_Z_IDX]) };

    // Ekstrakcja kątów orientacji
    float roll = data[ROLL_IDX];
    float pitch = data[PITCH_IDX];
    float yaw = data[YAW_IDX];

    // Sprawdzenie, czy handler IMU istnieje
    if (imuHandler) {
        // Aktualizacja danych surowych w handlerze IMU
        imuHandler->updateData(acc, gyro, mag);
        // Ustawienie rotacji modelu 3D w handlerze IMU
        imuHandler->setRotation(yaw, pitch, roll);

        // Obliczenie i aktualizacja kursu kompasu
        // Sprawdzenie, czy dane magnetometru są wystarczające i niezerowe (aby uniknąć atan2(0,0))
        if (mag.size() >= 2 && (mag[0] != 0 || mag[1] != 0)) {
            // Obliczenie kąta za pomocą atan2 (daje wynik w radianach od -PI do PI)
            float heading_rad = std::atan2(static_cast<float>(mag[1]), static_cast<float>(mag[0]));
            // Konwersja radianów na stopnie
            float heading_deg = heading_rad * 180.0f / M_PI;
            // Normalizacja kąta do zakresu [0, 360) stopni
            if (heading_deg < 0) {
                heading_deg += 360.0f;
            }
             // Aktualizacja wskaźnika kompasu w handlerze IMU
            imuHandler->updateCompass(heading_deg);
        } else {
            // Jeśli dane magnetometru są niewystarczające lub zerowe, ustaw kurs na 0
            imuHandler->updateCompass(0.0f);
        }

    } else {
        // Logowanie ostrzeżenia, jeśli handler IMU nie został zainicjalizowany
        qWarning() << "processImuData called but imuHandler is null!";
    }
}

/******************************/
/**
 * @brief Wyświetla widżet obsługujący dane IMU.
 * @details Slot podłączony do sygnału `switchToIMU` lub akcji menu. Ustawia
 * `imuHandler` jako bieżący widżet w `stackedWidget`.
 */
/******************************/
void MainWindow::showIMUHandler() {
    // Sprawdzenie, czy stackedWidget i imuHandler istnieją
    if (stackedWidget && imuHandler) {
        stackedWidget->setCurrentWidget(imuHandler); // Ustawienie widoku IMU
        qDebug() << "Switched view to IMU Handler"; // Logowanie informacji
    }
}

/******************************/
/**
 * @brief Wyświetla widżet obsługujący dane GPS.
 * @details Slot podłączony do sygnału `switchToGPS` lub akcji menu. Ustawia
 * `gpsHandler` jako bieżący widżet w `stackedWidget`.
 */
/******************************/
void MainWindow::showGPSHandler() {
     // Sprawdzenie, czy stackedWidget i gpsHandler istnieją
    if (stackedWidget && gpsHandler) {
        stackedWidget->setCurrentWidget(gpsHandler); // Ustawienie widoku GPS
        qDebug() << "Switched view to GPS Handler"; // Logowanie informacji
    }
}

/******************************/
/**
 * @brief Otwiera okno dialogowe do wyboru portu szeregowego.
 * @details Wyświetla listę dostępnych portów szeregowych w systemie i pozwala
 * użytkownikowi wybrać jeden z nich. Po dokonaniu wyboru, próbuje
 * nawiązać połączenie za pomocą `handlePortConnectionAttempt`.
 */
/******************************/
void MainWindow::selectPort() {
    QStringList ports; // Lista nazw portów
    // Pobranie informacji o dostępnych portach szeregowych
    const auto availablePorts = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : availablePorts) {
        ports << info.portName(); // Dodanie nazwy portu do listy
    }

    // Sprawdzenie, czy znaleziono jakiekolwiek porty
    if (ports.isEmpty()) {
        // Wyświetlenie ostrzeżenia, jeśli brak dostępnych portów
        QMessageBox::warning(this, tr("No Ports"), tr("No serial ports available. Check connections and permissions."));
        return; // Zakończenie funkcji
    }

    bool ok; // Flaga wskazująca, czy użytkownik kliknął OK w dialogu
    // Wyświetlenie dialogu QInputDialog typu lista rozwijana
    QString port = QInputDialog::getItem(this, tr("Select Port"), tr("Available Ports:"), ports, 0, false, &ok);

    // Sprawdzenie, czy użytkownik wybrał port i kliknął OK
    if (ok && !port.isEmpty()) {
        handlePortConnectionAttempt(port); // Próba nawiązania połączenia z wybranym portem
    }
}

/******************************/
/**
 * @brief Obsługuje próbę nawiązania połączenia z wybranym portem szeregowym.
 * @details Zapisuje nazwę wybranego portu. Jeśli istnieje aktywne połączenie szeregowe,
 * zamyka je. Jeśli aktywny jest tryb symulacji, wyłącza go. Następnie próbuje
 * otworzyć wybrany port za pomocą `serialHandler`. Aktualizuje flagę
 * `serialConnected` i wyświetla odpowiedni komunikat (sukces lub błąd).
 * @param portName Nazwa portu szeregowego do połączenia (np. "COM3", "/dev/ttyACM0").
 */
/******************************/
void MainWindow::handlePortConnectionAttempt(const QString& portName) {
    selectedPort = portName; // Zapisanie nazwy wybranego portu

    // Jeśli już istnieje połączenie, zamknij je przed próbą otwarcia nowego
    if (serialConnected) {
        serialHandler->closePort();
        serialConnected = false;
    }

    // Jeśli tryb symulacji jest aktywny, wyłącz go
    if (simulationMode) {
        simulationTimer->stop(); // Zatrzymaj timer
        simulationMode = false; // Zmień flagę
        qInfo() << "Simulation mode disabled due to serial port selection.";
        // Odznacz akcję "Simulation Mode" w menu
        QList<QAction*> actions = menuBar()->findChildren<QAction*>();
        for(QAction* action : actions) {
            if(action->text() == tr("Simulation Mode")) {
                action->setChecked(false);
                break;
            }
        }
    }

    // Próba otwarcia wybranego portu za pomocą handlera
    if (serialHandler->openPort(selectedPort)) {
        serialConnected = true; // Ustawienie flagi połączenia na true
        // Wyświetlenie komunikatu o sukcesie
        QMessageBox::information(this, tr("Connected"), tr("Successfully connected to %1").arg(portName));
        qInfo() << "Connected to serial port:" << selectedPort; // Logowanie sukcesu
    } else {
        // Wyświetlenie komunikatu o błędzie połączenia
        QMessageBox::critical(this, tr("Connection Error"), tr("Failed to open port %1. Reason: %2").arg(portName).arg(serialHandler->getLastError()));
        qWarning() << "Failed to open serial port:" << selectedPort; // Logowanie błędu
        serialConnected = false; // Upewnienie się, że flaga połączenia jest false
    }
}