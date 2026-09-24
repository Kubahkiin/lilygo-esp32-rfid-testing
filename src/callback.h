/**
 * \file callback.h
 * Obsługa i interpretacja wiadomości MQTT.
 * Tu umieszczane są wszelkie działania, które mają zostać wykonane po przyjściu wiadomości na wybrane tematy.
 * Oprócz tematu można też sprawdzić treść wiadomości i jeśli to konieczne, dostosować reakcję programu na konkretną treść
 */
#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include <string>
#include <iostream>

void callback(char *topic, byte *message, unsigned int length);

/**
 *  Obsługa przychodzących wiadomości na subskrybowane topici.
 *  Główna funkcja do obsługi lodówki przez serwer MQTT.
 *  W zależności od tematu, wykonywane są konkretne funkcje
 *  \param topic Topic wiadomości
 *  \param message Treść wiadomości
 *  \param length Długość wiadomości
 */
void callback(char *topic, byte *message, unsigned int length)
{
  uint8_t testTrials;
  Serial.println("[MQTT] Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  if (String(topic) == door_status_request)
  {
    doorStatus();
  }
  else if (String(topic) == door_permit && messageTemp == "1")
  {
    lockOpen();
  }
  else if (String(topic) == reader_read_request)
  {
    testTrials = static_cast<uint8_t>(messageTemp.toInt());
    startFastInventoryTest(testTrials);
  }
  else if (String(topic) == diagnostics_reader_temperature_request)
  {
    requestReaderTemperature();
  }
  else if (String(topic) == diagnostics_reader_info_request)
  {
    requestReaderInfo();
  }
  else if (String(topic) == diagnostics_reader_antenna_detection_request)
  {
    startAntennaDetection();
  }
  else if (String(topic) == diagnostics_environment_temperature_request)
  {
    publishEnvironmentTemperature();
  }
  else if (String(topic) == diagnostics_environment_humidity_request)
  {
    publishEnvironmentHumidity();
  }
}

#endif
