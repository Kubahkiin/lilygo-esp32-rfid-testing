/**
 * \file door.h
 * Obsługa drzwi, działanie elektrozamka i sprawdzanie czy drzwi są otwarte i czy powinny być otwarte lub zamknięte.
 * Sterowanie oświetleniem drzwi.
 */
#ifndef _DOOR_H_
#define _DOOR_H_
/** @name Flagi
 *   Flagi które określają stan drzwi
 */
///@{
/** Wybór funkcji sterującej monitorowaniem */
bool useLockSecurity = true;
/** Drzwi zostały otwarte w poprawny sposób */
bool doorIsOpen = false;
/** Drzwi są zamknięte */
bool doorIsClosed = false;
/** Drzwi nie powinny być otwarte (a pewnie są) */
bool doorShouldntBeOpen = false;
/** Uruchomiono ostrzeżenie (i miganie ledów) */
bool warningIsOn = false;
/** Alarm włączony */
bool alarmIsOn = false;
/** Bool pomocniczy do migania ledów */
bool flashed = false;
///@}
/** @name Zmienne czasowe
 *   Zmienne do zapisywania zdarzeń,
 *   np. kiedy elektrozamek został zwolniony
 */
///@{
/** Wartość -10001 gwarantuje wykrycie otwartych drzwi na starcie programu */
int32_t lockOpened_ms = -10001;
uint32_t doorOpened_ms = 0;
uint32_t warningStarted_ms = 0;
uint32_t alarmStarted_ms = 0;
uint32_t lastFlash_ms = 0;
///@}
/** @name Stałe czasowe
 *   Stałe określające ile czasu ma minąć przed jakimś zdarzeniem,
 *   np przez jaki czas drzwi można otworzyć (TIME_TO_OPEN_MS)
 */
///@{
constexpr uint32_t TIME_TO_OPEN_MS = 10000;
constexpr uint32_t TIME_TO_CHOOSE_MS = 20000;
constexpr uint32_t TIME_TO_WARN_MS = 10000;
constexpr uint32_t FLASH_INTERVAL_MS = 500;
///@}
/** @name Konfiguracja paska LED
 *   
 */
///@{
constexpr uint8_t PIXEL_COUNT = 3;
constexpr neoPixelType STRIP_FORMAT = NEO_WRGB + NEO_KHZ800;
Adafruit_NeoPixel strip_1(PIXEL_COUNT, RGB_1, STRIP_FORMAT);
Adafruit_NeoPixel strip_2(PIXEL_COUNT, RGB_2, STRIP_FORMAT);
///@}

void doorStatus();
void lockOpen();
void doorOpen();
void lockSecurity();
void doorSecurity();
void flashLed();

/** 
 *   Sprawdzenie czy drzwi są otwarte czy nie.
 * 
 *   Stan jest następnie przesyłany na serwer MQTT,
 *   1 to drzwi otwarte a 0 to drzwi zamknięte
 */
void doorStatus() {
    Serial.println("[MQTT] Request to check the door status");
    if(digitalRead(LOCK_SWITCH) == HIGH) {
        Serial.println("[LOCK] Door is open");
        publishMessage(door_status.c_str(), "1", false, true);
    } else {
        Serial.println("[LOCK] Door is closed");
        publishMessage(door_status.c_str(), "0", false, true);
    }
}

/**
 *   Zwolnienie elektrozamka i zaświecenie zielonych świateł
 *   Moment otwarcia jest zapisywany w celu sprawdzenia ile
 *   minęło czasu w funkcji lockSecurity
 */
void lockOpen() {
    doorShouldntBeOpen = false;
    doorIsClosed = false;
    digitalWrite(LOCK, HIGH);
    lockOpened_ms = millis();
    for (uint8_t pixel = 0; pixel < PIXEL_COUNT; ++pixel) {
        strip_1.setPixelColor(pixel, strip_1.Color(0, 255, 0, 0));
        strip_2.setPixelColor(pixel, strip_2.Color(0, 255, 0, 0));
    }
    strip_1.show();
    strip_2.show();
}

/**
 *   Ustawienie flagi otwartych drzwi, zapisanie momentu otwarcia,
 *   zmiana flagi określającej wybór funkcji sprawdzającej stan drzwi
 *   i zamknięcie elektrozamka.
 */
void doorOpen() {
    doorOpened_ms = millis();
    useLockSecurity = false;
    digitalWrite(LOCK, LOW);
}

/**
 *  Sprawdzenie czy drzwi są otwarte i czy powinny być otwarte
 *  za pomocą porównywania obecnego czasu z momentem otwarcia elektrozamka.
 *  Funkcja resetuje także flagi i wyłącza światła gdy drzwi zostaną zamknięte.
 */
void lockSecurity() {
    if(digitalRead(LOCK_SWITCH) == LOW &&
            (millis() - lockOpened_ms > TIME_TO_OPEN_MS || doorIsOpen) &&
            !doorIsClosed ) {
        doorIsClosed = true;
        digitalWrite(LOCK, LOW);
        // digitalWrite(BUZZER, LOW);
        strip_1.clear();
        strip_2.clear();
        strip_1.show();
        strip_2.show();
        doorIsOpen = false;
        warningIsOn = false;
        alarmIsOn = false;
        doorShouldntBeOpen = false;
        Serial.print("\nDrzwi są zamknięte");
    }

    if (digitalRead(LOCK_SWITCH) == HIGH &&
            millis() - lockOpened_ms < TIME_TO_OPEN_MS &&
            !doorIsOpen) {
        doorIsOpen = true;
        doorIsClosed = false;
        doorOpen();
        for (uint8_t pixel = 0; pixel < PIXEL_COUNT; ++pixel)
        {
            strip_1.setPixelColor(pixel, strip_1.Color(0, 0, 0, 255));
            strip_2.setPixelColor(pixel, strip_2.Color(0, 0, 0, 255));
        }
        strip_1.show();
        strip_2.show();
        Serial.print("\nDrzwi zostały otwarte.");
    }

    if (digitalRead(LOCK_SWITCH) == HIGH &&
            millis() - lockOpened_ms > TIME_TO_OPEN_MS &&
            !doorShouldntBeOpen) {
        doorShouldntBeOpen = true;
        doorIsClosed = false;
        for (uint8_t pixel = 0; pixel < PIXEL_COUNT; ++pixel)
        {
            strip_1.setPixelColor(pixel, strip_1.Color(255, 0, 0, 0));
            strip_2.setPixelColor(pixel, strip_2.Color(255, 0, 0, 0));
        }
        strip_1.show();
        strip_2.show();
        Serial.print("\nDrzwi nie powinny być otwarte");
    }
}

/**
 *  Sprawdzenie czy drzwi są otwarte i czy powinny już zostać zamknięte
 *  za pomocą porównania obecnego czasu z momentem (poprawnego) otwarcia drzwi.
 *  W zależności od tego jak długo drzwi są otwarte, ustawia flagę ostrzeżenia
 *  lub flagę alarmu. W przypadku zamknięcia drzwi zmienia flagę sterującą wyborem
 *  funkcji sprawdzającej z powrotem na lockSecurity
 */
void doorSecurity() {
    if (digitalRead(LOCK_SWITCH) == HIGH &&
            millis() - doorOpened_ms > TIME_TO_CHOOSE_MS &&
            !warningIsOn) {
        warningIsOn = true;
        warningStarted_ms = millis();
        
        Serial.print("\nOstrzeżenie, drzwi powinny być zamknięte");
    }

    if(digitalRead(LOCK_SWITCH) == HIGH &&
            millis() - warningStarted_ms > TIME_TO_WARN_MS &&
            warningIsOn &&
            !alarmIsOn) {
        alarmIsOn = true;
        //digitalWrite(BUZZER, HIGH);
         for (uint8_t pixel = 0; pixel < PIXEL_COUNT; ++pixel) {
            strip_1.setPixelColor(pixel, strip_1.Color(255, 0, 0, 0));
            strip_2.setPixelColor(pixel, strip_2.Color(255, 0, 0, 0));
        }
        strip_1.show();
        strip_2.show();
        Serial.print("\nAlarm włączony");
    }

    if(digitalRead(LOCK_SWITCH) == LOW) {
        useLockSecurity = true;
        Serial.print("\nDrzwi zostały zamknięte");
    }
}

/**
 *  Zapewnia miganie świateł w odpowiednich odstępach czasowych,
 *  gdy lodówka jest w stanie ostrzeżenia
 */
void flashLed() {
    if(warningIsOn && millis() - lastFlash_ms > FLASH_INTERVAL_MS && !alarmIsOn) {
        if (!flashed) {
        for (uint8_t pixel = 0; pixel < PIXEL_COUNT; ++pixel) {
            strip_1.setPixelColor(pixel, strip_1.Color(255, 255, 0, 0));
            strip_2.setPixelColor(pixel, strip_2.Color(255, 255, 0, 0));
        }
        flashed = true;
        } else {
        strip_1.clear();
        strip_2.clear();
        flashed = false;
        }
        strip_1.show();
        strip_2.show();
        lastFlash_ms = millis();
    }
}

#endif