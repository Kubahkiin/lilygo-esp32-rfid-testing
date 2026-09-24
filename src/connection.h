/**
 * \file connection.h
 * Konfiguracja połączenia internetowego, połączenia z serwerem ntp oraz połączenie z brokerem MQTT
 * i funkcje powiązane np. timestamp
 */
#ifndef _CONNECTION_H_
#define _CONNECTION_H_

/** @name Stałe Ethernet  */
///@{
constexpr const char* ETHERNET_HOSTNAME = "esp32-p4";
constexpr uint32_t ETHERNET_DHCP_TIMEOUT_MS = 15000;
///@}

/** @name Stałe dla konfiguracji połączenia z serwerem NTP  */
///@{
constexpr const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;
///@}

/** @name Zmienne pomocnicze timestamp  */
///@{
uint32_t epochTime = 0;
uint32_t lastTimestamp_ms = 0;
constexpr uint16_t TIMESTAMP_INTERVAL_MS = 1000;
///@}

/** @name Bezpieczne połączenie w celu łączenia z MQTT  */
///@{
WiFiClientSecure espClient;
PubSubClient client(espClient);
///@}

/** @name Bufor dla wiadomości MQTT  */
///@{
uint32_t lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
///@}

const char* clientID = "ESP32-P4-ETH";

// Flag
bool isEthernetOnline = false;

void onEvent(arduino_event_id_t event);
bool startEthernet();
uint32_t getUnixTime();
void reconnect();
void publishMessage(const char* topic, String payload, boolean retained, boolean print);
void timestamp();

/**
 *  Obsługa zdarzeń arduino powiązanych z usługą Ethernet
 */
void onEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("[ETH] ETH Started");
      // The hostname must be set after the interface is started, but needs
      // to be set before DHCP, so set it from the event handler thread.
      ETH.setHostname(ETHERNET_HOSTNAME);
      break;
    case ARDUINO_EVENT_ETH_CONNECTED: Serial.println("[ETH] Connected"); break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.println("[ETH] Got IP");
      Serial.println(ETH);
      isEthernetOnline = true;
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      Serial.println("[ETH] Lost IP");
      isEthernetOnline = false;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("[ETH] Disconnected");
      isEthernetOnline = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("[ETH] Stopped");
      isEthernetOnline = false;
      break;
    default: break;
  }
}

/**
 *  Uruchomienie Ethernetu i oczekiwania na serwer DHCP w określonym czasie.
 *  Zwraca wartość true, jeśli udało się połączyć z internetem i false jeżeli nie.
 */
bool startEthernet() {
  Network.onEvent(onEvent);

  Serial.println("[ETH] Starting Ethernet");
  if (!ETH.begin(
    ETH_PHY_W5500,
    ETH_ADDR,
    ETH_CS_PIN,
    ETH_INT_PIN,
    ETH_RST_PIN,
    SPI3_HOST,
    ETH_SCLK_PIN,
    ETH_MISO_PIN,
    ETH_MOSI_PIN
  )) {
    Serial.println("[ETH] Ethernet failed to start");
    return false;
  }

  const uint32_t connectionStartMs = millis();
  // biblioteka ETH działa chyba na zasadzie przerwań dlatego ten while ma sens
  while (!isEthernetOnline && connectionStartMs < ETHERNET_DHCP_TIMEOUT_MS) {
    delay(100);
  }

  if (!isEthernetOnline) {
    Serial.println("[ETH] Couldn't connect with DHCP");
    return false;
  }

  return true;
}

/**
 *  Pobiera obecny czas w sekundach i zwraca go jako uint32_t (unsigned long)
 */
uint32_t getUnixTime() {
  time_t now;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);

  return now;
}

/**
 *  Łączy z serwerem MQTT jeżeli jeszcze nie nawiązano połęczenia.
 *  Większość funkconalności lodówki polega na odbieraniu i wysyłaniu wiadomości MQTT,
 *  więc program nie będzie działać dopóki połączenie nie zostanie nawiązane
 */
void reconnect() {
  // loop until connected
  while(!client.connected()) {
    Serial.print("\n[MQTT] Connecting to broker");
    if (client.connect(clientID, mqtt_username, mqtt_password)) {
      Serial.print("\n[MQTT] Connected");
    }
    else {
      Serial.print("\n[MQTT] Failed, rc=");
      Serial.print(client.state());
      Serial.print("\n[MQTT] Retrying in 2 seconds");
      delay(2000);
    }
  }
  client.subscribe(door_status_request.c_str());
  client.subscribe(door_permit.c_str());
  client.subscribe(reader_read_request.c_str());
  client.subscribe(diagnostics_reader_temperature_request.c_str());
  client.subscribe(diagnostics_reader_info_request.c_str());
  client.subscribe(diagnostics_reader_antenna_detection_request.c_str());
  client.subscribe(diagnostics_environment_temperature_request.c_str());
  client.subscribe(diagnostics_environment_humidity_request.c_str());
}

/**
 *  Publikuje wiadomość na podany topic na serwerze MQTT
 *  \param topic Topic MQTT w postaci stałej tablicy znaków
 *  \param payload Wiadomość która ma zostać przesłana w postaci łańcucha znaków
 *  \param retained Czy wiadomość ma być retained czy nie
 *  \param print Czy informować o przesłaniu tej wiadomości na Serialu czy nie
 */
void publishMessage(const char* topic, String payload, boolean retained, boolean print) {
  if(client.publish(topic, payload.c_str(), retained) && print) {
    Serial.print("\n[MQTT] Message published [" + String(topic) + "]: " + payload);
  }
}

/**
 *  Publikuje UNIX timestamp na serwerze MQTT co określony czas (co sekundę)
 */
void timestamp() {
  if(millis() - lastTimestamp_ms > TIMESTAMP_INTERVAL_MS)
  {
    publishMessage(diagnostics_timestamp.c_str(), String(getUnixTime()), true, false);
    lastTimestamp_ms = millis();
  }
}

#endif
