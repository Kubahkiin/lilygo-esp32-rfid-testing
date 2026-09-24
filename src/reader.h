#ifndef _READER_H_
#define _READER_H_



HardwareSerial RfidSerial(1);

// CRC16 ///////////////////////////////////////////////
#define PRESET_VALUE 0xFFFF
#define POLYNOMIAL 0x8408
unsigned int uiCrc16(unsigned char const *pucY, unsigned char ucX) {
  unsigned char ucI, ucJ;
  unsigned short int uiCrcValue = PRESET_VALUE;
  for (ucI = 0; ucI < ucX; ucI++) {
    uiCrcValue = uiCrcValue ^ *(pucY + ucI);
    for (ucJ = 0; ucJ < 8; ucJ++) {
      if (uiCrcValue & 0x0001) {
        uiCrcValue = (uiCrcValue >> 1)^POLYNOMIAL; // not really a polynomial, just bitwise XOR (^)
      }
      else {
        uiCrcValue = (uiCrcValue >> 1);
      }
    }
  }
  return uiCrcValue;
}
////////////////////////////////////////////////////////

// Constants ///////////////////////////////////////////
// Broadcast address - 0xFF, default address - 0x00
constexpr uint8_t ADDRESS = 0xFF;
constexpr uint32_t COMM_TIMEOUT_MS = 1000;
constexpr uint8_t INTER_BYTE_TIMEOUT_MS = 15;
constexpr uint32_t RFID_BAUD_RATE = 115200; // 115200;
constexpr size_t RFID_RX_BUFFER_SIZE = 4096;
constexpr uint8_t ANTENNA_PORT_COUNT = 16;
constexpr uint32_t FAST_INVENTORY_TIME_MS = 5000;
// Liczbę prób można bezpiecznie zmieniać także na wartości większe niż 255.
constexpr uint32_t FAST_INVENTORY_TRIAL_COUNT = 5;
constexpr uint32_t FAST_INVENTORY_TRIAL_PAUSE_MS = 5000;
constexpr uint32_t FAST_INVENTORY_DRAIN_QUIET_MS = 50;
constexpr uint32_t FAST_INVENTORY_DRAIN_MAX_MS = 500;
constexpr uint8_t MAX_RX_FRAMES_PER_LOOP = 32;
constexpr uint8_t FAST_INVENTORY_TARGET = 0x00; // Target A
constexpr size_t MAX_FAST_INVENTORY_TAGS = 160;
// Agregat przechowuje unię EPC ze wszystkich prób, więc ma większy limit.
constexpr size_t MAX_FAST_INVENTORY_TEST_TAGS = 256;
constexpr size_t MAX_FAST_EPC_LENGTH = 0x3F;
constexpr uint8_t RFID_POWER_DBM = 33;
constexpr uint8_t MAX_CONFIG_SET_ATTEMPTS = 1;
constexpr uint8_t ANTENNA_PROBE_Q_VALUE = 0x00;
constexpr uint8_t ANTENNA_PROBE_SESSION = 0x00;
constexpr uint8_t ANTENNA_PROBE_TARGET = 0x00;
constexpr uint8_t ANTENNA_PROBE_SCAN_TIME = 0x02; // 2 * 100 ms
constexpr uint8_t ANTENNA_SELECTOR_BASE = 0x80;
constexpr uint32_t ANTENNA_PROBE_MAX_DURATION_MS = 2000;
constexpr uint32_t ANTENNA_PROBE_TIMEOUT_RECOVERY_MS = 400;

constexpr uint8_t Q_VALUE = 0x07; // 2^Q tags in the range of the antena <0 - 15>
constexpr uint8_t SESSION = 0x00; // S0, S1, S2, S3, FF is auto
constexpr uint8_t TAG_FOCUS = 0x00;

static_assert(
  ANTENNA_PORT_COUNT > 0 && ANTENNA_PORT_COUNT <= 16,
  "Antenna masks support from 1 to 16 ports");
static_assert(
  FAST_INVENTORY_TRIAL_COUNT > 0,
  "Fast inventory requires at least one trial");

constexpr uint16_t ALL_ANTENNA_PORTS_MASK =
  static_cast<uint16_t>(
    (uint32_t{1} << ANTENNA_PORT_COUNT) - 1U);

// Lower ETSI RFID band: 865.7, 866.3, 866.9 and 867.5 MHz.
constexpr uint8_t RFID_REGION_EU3 = 0x09;
constexpr uint8_t RFID_REGION_MIN_CHANNEL = 0x00;
constexpr uint8_t RFID_REGION_MAX_CHANNEL = 0x03;

// Command bytes ///////////////////////////////////////
constexpr uint8_t CMD_GET_READER_INFO = 0x21;
constexpr uint8_t CMD_GET_READER_TEMPERATURE = 0x92;
constexpr uint8_t CMD_GET_WORK_MODE = 0x77;
constexpr uint8_t CMD_READ_REGION = 0x9E;
constexpr uint8_t CMD_SET_REGION = 0x22;
constexpr uint8_t CMD_READ_ANTENNA_POWER = 0x94;
constexpr uint8_t CMD_SET_RF_POWER = 0x2F;
constexpr uint8_t CMD_INVENTORY_G2 = 0x01;
// Fast inventory configuration commands
constexpr uint8_t CMD_SET_ACTIVE_ANTENNAS = 0x3F;
constexpr uint8_t CMD_SET_CFG = 0xEA;
// Configuration parameters IDs
constexpr uint8_t CFG_TAG_FOCUS = 0x08; // check later
constexpr uint8_t CFG_Q_AND_SESSION = 0x09;
constexpr uint8_t CFG_TID = 0x0A;
constexpr uint8_t CFG_MASK = 0x0B;


constexpr uint8_t CMD_START_FAST_INVENTORY = 0x50;
constexpr uint8_t CMD_STOP_FAST_INVENTORY = 0x51;
constexpr uint8_t CMD_SET_ANTENNA_CHECK = 0x66;
constexpr uint8_t CMD_FAST_INVENTORY_TAG = 0xEE;

constexpr uint8_t STATUS_INVENTORY_COMPLETED = 0x01;
constexpr uint8_t STATUS_INVENTORY_TIMEOUT = 0x02;
constexpr uint8_t STATUS_INVENTORY_MORE_FRAMES = 0x03;
constexpr uint8_t STATUS_INVENTORY_MEMORY_FULL = 0x04;
constexpr uint8_t STATUS_ANTENNA_CONNECTION_ERROR = 0xF8;
constexpr uint8_t STATUS_TAG_COMMUNICATION_ERROR = 0xFA;
constexpr uint8_t STATUS_NO_OPERABLE_TAGS = 0xFB;
constexpr uint8_t STATUS_TAG_ERROR = 0xFC;
////////////////////////////////////////////////////////

enum class PendingRequest {
  None,
  ReaderInfo,
  Temperature,
  WorkMode,
  ReadRegion,
  SetRegion,
  ReadAntennaPower,
  SetRfPower,
  EnableAntennaCheck,
  ProbeAntenna,
  ConfigureAntennas,
  DisableTagFocus,
  SetQAndSession,
  SetEpcMode,
  ClearInventoryMask,
  StartFastInventory,
  StopFastInventory
};

enum class FastInventoryState {
  Idle,
  Starting,
  Running,
  Stopping,
  Draining,
  // Osobny stan gwarantuje pełną przerwę między zakończonymi próbami.
  WaitingBetweenTrials,
  Error
};

enum class ReaderConfigurationStep {
  Idle,
  ReaderInfo,
  WorkMode,
  ReadRegion,
  SetRegion,
  VerifyRegion,
  ReadPower,
  SetPower,
  VerifyPower,
  EnableAntennaCheck,
  DetectAntennas,
  ConfigureAntennas,
  DisableTagFocus,
  SetQAndSession,
  SetEpcMode,
  ClearMask,
  Completed,
  Error
};

enum class ConfigurationCheck {
  Error,
  Matches,
  NeedsUpdate
};

enum class AntennaProbeResult {
  Ignore,
  MoreFrames,
  Connected,
  Disconnected,
  Unknown
};

struct RfidReceiver {
  uint8_t buffer[256] = {};
  size_t byteCount = 0;
  size_t expectedLength = 0;
  uint32_t lastByteMs = 0;
};

struct RfidResponseView {
  uint8_t status = 0;
  const uint8_t* data = nullptr;
  size_t dataLength = 0;
};

struct FastInventoryTag {
  uint8_t epc[MAX_FAST_EPC_LENGTH] = {};
  uint8_t epcLength = 0;
  uint16_t antennaMask = 0;
  uint32_t readCount = 0;
};

// Wynik całej serii; reliableAntennaMask jest przecięciem masek ze wszystkich prób.
struct FastInventoryTestTag {
  uint8_t epc[MAX_FAST_EPC_LENGTH] = {};
  uint8_t epcLength = 0;
  uint16_t reliableAntennaMask = 0;
  uint16_t anyAntennaMask = 0;
  uint32_t detectedTrialCount = 0;
  uint32_t totalReadCount = 0;
};

RfidReceiver rx;

PendingRequest pendingRequest = PendingRequest::None;
FastInventoryState fastInventoryState = FastInventoryState::Idle;
ReaderConfigurationStep readerConfigurationStep =
  ReaderConfigurationStep::Idle;
bool readerConfigurationDispatching = false;

uint32_t requestStartedMs = 0;
uint32_t fastInventoryStartedMs = 0;
uint32_t fastInventoryLastFrameMs = 0;
uint32_t fastInventoryDrainStartedMs = 0;
uint32_t fastInventoryPauseStartedMs = 0;
uint32_t antennaProbeStartedMs = 0;
uint32_t nextAntennaProbeEarliestMs = 0;
uint8_t regionSetAttempts = 0;
uint8_t powerSetAttempts = 0;

uint8_t testedAntennaPort = 0;
bool antennaDetectionActive = false;
uint16_t activeAntennaMask = 0;
uint16_t disconnectedAntennaMask = 0;
uint16_t unknownAntennaMask = 0;

FastInventoryTag fastInventoryTags[MAX_FAST_INVENTORY_TAGS];
size_t fastInventoryTagCount = 0;
uint32_t fastInventoryReadCount = 0;
uint32_t fastInventoryMalformedFrameCount = 0;
uint32_t fastInventoryDroppedTagCount = 0;

FastInventoryTestTag
  fastInventoryTestTags[MAX_FAST_INVENTORY_TEST_TAGS];
size_t fastInventoryTestTagCount = 0;
uint32_t fastInventoryRequestedTrialCount = 0;
uint32_t fastInventoryCompletedTrialCount = 0;
uint32_t fastInventoryTestReadCount = 0;
uint32_t fastInventoryTestMalformedFrameCount = 0;
uint32_t fastInventoryTestDroppedTagCount = 0;

// Antenna mask helper function/////////////////////////
uint16_t antennaBit(uint8_t port) {
  if (port < 1 || port > ANTENNA_PORT_COUNT) {
    return 0;
  }

  return static_cast<uint16_t>(
    1UL << (port - 1));
}
////////////////////////////////////////////////////////

// Function declarations ///////////////////////////////
// Frame functions /////////////////////////////////////
bool validFrame(const uint8_t* frame, size_t frameLength);
size_t buildCommandFrame(uint8_t command, const uint8_t* data, size_t dataLength, uint8_t* frame, size_t frameCapacity);
void resetResponseReceiver();
bool sendFrame(uint8_t command, const uint8_t* data, size_t dataLength);
bool receiveFrame(size_t &responseLength);


// Reader command functions
bool startTrackedRequest(PendingRequest request, uint8_t command, const uint8_t* data, size_t dataLength);
bool finishTrackedRequest(PendingRequest expectedRequest);
bool prepareResponse(const char* operationName, size_t responseLength, RfidResponseView& response);
bool prepareSuccessfulResponse(const char* operationName, size_t responseLength, size_t minimumDataLength, RfidResponseView& response);
bool startReaderConfiguration();
bool startReaderConfigurationStep(ReaderConfigurationStep step);
void continueReaderConfiguration(ReaderConfigurationStep expected, ReaderConfigurationStep next);
void handleRegionConfigurationResult(ConfigurationCheck result);
void handlePowerConfigurationResult(ConfigurationCheck result);
void failReaderConfiguration(const char* reason);
bool readerConfigurationIsActive();

bool requestReaderInfo();
bool handleReaderInfo(size_t responseLength);
bool requestReaderTemperature();
void handleReaderTemperature(size_t responseLength);
bool requestWorkMode();
bool handleWorkMode(size_t responseLength);
bool requestReadRegion();
ConfigurationCheck handleReadRegion(size_t responseLength);
bool requestSetRegion();
bool handleSetRegion(size_t responseLength);
bool requestReadAntennaPower();
ConfigurationCheck handleReadAntennaPower(size_t responseLength);
bool requestSetRfPower();
bool handleSetRfPower(size_t responseLength);
bool requestEnableAntennaCheck();
bool handleEnableAntennaCheck(size_t responseLength);
bool startAntennaDetection();
void serviceAntennaDetection();
bool requestAntennaProbe(uint8_t port);
AntennaProbeResult handleAntennaProbeResponse(size_t responseLength);
void recordAntennaProbeResult(AntennaProbeResult result);
void handleAntennaProbeTimeout();
void finishAntennaDetection();
void printAntennaPorts(const char* label, uint16_t mask);
bool requestConfigureAntennas();
bool handleConfigureAntennas(size_t responseLength);
bool requestDisableTagFocus();
bool handleDisableTagFocus(size_t responseLength);
bool requestSetQAndSession();
bool handleSetQAndSession(size_t responseLength);
bool requestSetEpcMode();
bool handleSetEpcMode(size_t responseLength);
bool requestClearInventoryMask();
bool handleClearInventoryMask(size_t responseLength);
bool startFastInventoryTest(uint32_t trialCount);
bool requestStartFastInventory();
bool handleStartFastInventory(size_t responseLength);
bool handleFastInventoryTag(size_t responseLength);
bool requestStopFastInventory();
bool handleStopFastInventory(size_t responseLength);
void serviceFastInventory();
void resetFastInventoryStatistics();
void resetFastInventoryTestStatistics(uint32_t trialCount);
bool mergeFastInventoryTrial();
void finishFastInventoryTrial();
void printFastInventorySummary();
void sortFastInventoryTags();
void sortFastInventoryTestTags();
void printFastInventoryTestSummary();
void printFastInventoryCsv();
void publishFastInventoryResult();
void printEpc(const uint8_t* epc, size_t epcLength);

////////////////////////////////////////////////////////

// LOOP /////////////////////////////////////////////////////////////////
void handleReaderRequest() {
  for (uint8_t processedFrameCount = 0;
       processedFrameCount < MAX_RX_FRAMES_PER_LOOP;
       ++processedFrameCount) {

    size_t responseLength = 0;
    if (!receiveFrame(responseLength)) {
      break;
    }

    const uint8_t responseCommand = rx.buffer[2];

    switch (responseCommand) {
      case CMD_GET_READER_INFO:
        if (!finishTrackedRequest(PendingRequest::ReaderInfo)) {
          break;
        }

        if (handleReaderInfo(responseLength)) {
          continueReaderConfiguration(ReaderConfigurationStep::ReaderInfo,
                                      ReaderConfigurationStep::WorkMode);
        } else if (readerConfigurationStep == ReaderConfigurationStep::ReaderInfo) {
          failReaderConfiguration("nie można odczytać informacji o czytniku");
        }
        break;

      case CMD_GET_READER_TEMPERATURE:
        if (!finishTrackedRequest(PendingRequest::Temperature)) {
          break;
        }

        handleReaderTemperature(responseLength);

        // // Temperature is optional on some models, so continue either way.
        // requestWorkMode();
        break;

      case CMD_GET_WORK_MODE: {
        if (!finishTrackedRequest(PendingRequest::WorkMode)) {
          break;
        }

        if (handleWorkMode(responseLength)) {
          continueReaderConfiguration(ReaderConfigurationStep::WorkMode,
                                      ReaderConfigurationStep::ReadRegion);
        } else if (readerConfigurationStep == ReaderConfigurationStep::WorkMode) {
          failReaderConfiguration("czytnik nie pracuje w answering mode");
        }
        break;
      }

      case CMD_READ_REGION: {
        if (!finishTrackedRequest(PendingRequest::ReadRegion)) {
          break;
        }

        handleRegionConfigurationResult(handleReadRegion(responseLength));
        break;
      }

      case CMD_SET_REGION:
        if (!finishTrackedRequest(PendingRequest::SetRegion)) {
          break;
        }

        if (handleSetRegion(responseLength)) {
          continueReaderConfiguration(ReaderConfigurationStep::SetRegion,
                                      ReaderConfigurationStep::VerifyRegion);
        } else if (readerConfigurationStep == ReaderConfigurationStep::SetRegion) {
          failReaderConfiguration("nie można ustawić regionu");
        }
        break;

      case CMD_READ_ANTENNA_POWER: {
        if (!finishTrackedRequest(PendingRequest::ReadAntennaPower)) {
          break;
        }

        handlePowerConfigurationResult(handleReadAntennaPower(responseLength));
        break;
      }

      case CMD_SET_RF_POWER:
        if (!finishTrackedRequest(PendingRequest::SetRfPower)) {
          break;
        }

        if (handleSetRfPower(responseLength)) {
          continueReaderConfiguration(ReaderConfigurationStep::SetPower,
                                      ReaderConfigurationStep::VerifyPower);
        } else if (readerConfigurationStep == ReaderConfigurationStep::SetPower) {
          failReaderConfiguration("nie można ustawić mocy anten");
        }
        break;

      case CMD_SET_ANTENNA_CHECK:
        if (!finishTrackedRequest(PendingRequest::EnableAntennaCheck)) {
          break;
        }

        if (handleEnableAntennaCheck(responseLength)) {
          continueReaderConfiguration(ReaderConfigurationStep::EnableAntennaCheck,
                                      ReaderConfigurationStep::DetectAntennas);
        } else if (readerConfigurationStep == ReaderConfigurationStep::EnableAntennaCheck) {
          failReaderConfiguration("nie można włączyć kontroli anten");
        }
        break;

      case CMD_INVENTORY_G2: {
        if (pendingRequest != PendingRequest::ProbeAntenna) {
          Serial.println(
            "[ERROR][RFID] Odpowiedź Inventory_G2 nie pasuje do oczekiwanej komendy");
          break;
        }

        const AntennaProbeResult result =
          handleAntennaProbeResponse(responseLength);

        if (result == AntennaProbeResult::Ignore) {
          break;
        }

        if (result == AntennaProbeResult::MoreFrames) {
          // Refresh the inactivity timeout; the absolute limit stays unchanged.
          requestStartedMs = millis();
          break;
        }

        if (!finishTrackedRequest(PendingRequest::ProbeAntenna)) {
          break;
        }

        recordAntennaProbeResult(result);
      }
      break;
      case CMD_SET_ACTIVE_ANTENNAS: {
        if (!finishTrackedRequest(PendingRequest::ConfigureAntennas)) {
          break;
        }

        // Błędny ACK konfiguracji ma zatrzymać automat zamiast zostawić go w Idle.
        if (handleConfigureAntennas(responseLength)) {
          continueReaderConfiguration(ReaderConfigurationStep::ConfigureAntennas,
                                      ReaderConfigurationStep::DisableTagFocus);
        } else if (readerConfigurationStep == ReaderConfigurationStep::ConfigureAntennas) {
          failReaderConfiguration("nie można skonfigurować aktywnych anten");
        }
        break;
      }

      case CMD_SET_CFG: {
        const PendingRequest completed = pendingRequest;
        bool success = false;
        ReaderConfigurationStep expected = ReaderConfigurationStep::Idle;
        ReaderConfigurationStep next = ReaderConfigurationStep::Error;

        if (completed == PendingRequest::DisableTagFocus) {
          expected = ReaderConfigurationStep::DisableTagFocus;
          next = ReaderConfigurationStep::SetQAndSession;
          success = finishTrackedRequest(completed) && handleDisableTagFocus(responseLength);
        } else if (completed == PendingRequest::SetQAndSession) {
          expected = ReaderConfigurationStep::SetQAndSession;
          next = ReaderConfigurationStep::SetEpcMode;
          success = finishTrackedRequest(completed) && handleSetQAndSession(responseLength);
        } else if (completed == PendingRequest::SetEpcMode) {
          expected = ReaderConfigurationStep::SetEpcMode;
          next = ReaderConfigurationStep::ClearMask;
          success = finishTrackedRequest(completed) && handleSetEpcMode(responseLength);
        } else if (completed == PendingRequest::ClearInventoryMask) {
          expected = ReaderConfigurationStep::ClearMask;
          next = ReaderConfigurationStep::Completed;
          success = finishTrackedRequest(completed) && handleClearInventoryMask(responseLength);
        } else {
          Serial.println("[ERROR][RFID] Nieoczekiwana odpowiedź konfiguracji 0xEA");
        }

        if (success) {
          continueReaderConfiguration(expected, next);
        } else if (readerConfigurationStep == expected) {
          failReaderConfiguration("błąd parametru fast inventory");
        }
        break;
      }

      case CMD_START_FAST_INVENTORY:
        if (!finishTrackedRequest(PendingRequest::StartFastInventory)) {
          break;
        }

        if (!handleStartFastInventory(responseLength)) {
          fastInventoryState = FastInventoryState::Error;
        }
        break;

      case CMD_FAST_INVENTORY_TAG:
        // Ramki 0xEE są asynchroniczne i nie kończą pendingRequest.
        handleFastInventoryTag(responseLength);
        break;

      case CMD_STOP_FAST_INVENTORY:
        if (!finishTrackedRequest(PendingRequest::StopFastInventory)) {
          break;
        }

        if (!handleStopFastInventory(responseLength)) {
          fastInventoryState = FastInventoryState::Error;
        }
        break;

      default:
        Serial.printf("\n[ERROR][RFID] Nieobsługiwana odpowiedź: 0x%02X\n", static_cast<unsigned>(responseCommand));
        break;
    }
  }

  const uint32_t now = millis();
  const bool responseTimedOut =
    pendingRequest != PendingRequest::None &&
    static_cast<uint32_t>(now - requestStartedMs) > COMM_TIMEOUT_MS;

  const bool antennaProbeExceededAbsoluteLimit =
    pendingRequest == PendingRequest::ProbeAntenna &&
    static_cast<uint32_t>(now - antennaProbeStartedMs) >
      ANTENNA_PROBE_MAX_DURATION_MS;

  if (responseTimedOut || antennaProbeExceededAbsoluteLimit) {
    const PendingRequest timedOutRequest = pendingRequest;

    if (timedOutRequest == PendingRequest::ProbeAntenna) {
      Serial.printf(
        "[ERROR][RFID] Timeout testu ANT%u\n",
        static_cast<unsigned>(testedAntennaPort));
    }
    else {
      Serial.println("[ERROR][RFID] Timeout odpowiedzi");
    }

    resetResponseReceiver();
    pendingRequest = PendingRequest::None;

    if (timedOutRequest == PendingRequest::ProbeAntenna) {
      handleAntennaProbeTimeout();
    }
    else if (timedOutRequest == PendingRequest::StartFastInventory ||
             timedOutRequest == PendingRequest::StopFastInventory) {
      fastInventoryState = FastInventoryState::Error;
      Serial.println(
        "[ERROR][RFID] Nie można potwierdzić stanu fast inventory");
    }
    else if (readerConfigurationIsActive()) {
      failReaderConfiguration("timeout odpowiedzi");
    }
  }

  serviceFastInventory();
  serviceAntennaDetection();
}
/////////////////////////////////////////////////////////////////////////


// Function definitions /////////////////////////////////////////////////

// check length and verify calculated CRC with received CRC
bool validFrame(const uint8_t* frame, size_t frameLength)
{
  constexpr size_t MIN_RESPONSE_LENGTH = 6;
  constexpr size_t MAX_FRAME_LENGTH = 256;

  if (frame == nullptr ||
      frameLength < MIN_RESPONSE_LENGTH ||
      frameLength > MAX_FRAME_LENGTH) {
    return false;
  }

  const size_t declaredLength = static_cast<size_t>(frame[0]) + 1;
  if (declaredLength != frameLength) {
    return false;
  }

  const size_t crcIndex = frameLength - 2;
  const uint16_t receivedCrc =
    static_cast<uint16_t>(frame[crcIndex]) | // bitwise OR
    (static_cast<uint16_t>(frame[crcIndex + 1]) << 8); // bitwise left-shift
  const uint16_t calculatedCrc =
    uiCrc16(frame, static_cast<uint8_t>(crcIndex));

  return calculatedCrc == receivedCrc;
}

// constructs command frame for the reader, calculates length, inserts data array and calculates crc
// Length | Address | Command | Data[] | LSB-CRC16 | MSB-CRC16
size_t buildCommandFrame(
  uint8_t        command,
  const uint8_t* data,
  size_t         dataLength,
  uint8_t*       frame,
  size_t         frameCapacity) {

  constexpr size_t MAX_DATA_LENGTH = 251;

  if (frame == nullptr) {
    return 0;
  }

  if (dataLength > 0 && data == nullptr) {
    return 0;
  }

  if (dataLength > MAX_DATA_LENGTH) {
    return 0;
  }

  const size_t frameLength = dataLength + 5;

  if (frameCapacity < frameLength) {
    return 0;
  }

  frame[0] = static_cast<uint8_t>(dataLength + 4); // Length
  frame[1] = ADDRESS; // Address
  frame[2] = command; // Command
  // frame[3] ... frame[3 + dataLength]
  if (dataLength > 0) {
    memcpy(frame + 3, data, dataLength);
  }

  const size_t crcByte = dataLength +3;
  const uint16_t crc = uiCrc16(frame, crcByte);
  frame[crcByte] = static_cast<uint8_t>(crc & 0x00FF); //bitwise AND
  frame[crcByte + 1] = static_cast<uint8_t>(crc >> 8); // bitwise shift to the right

  return frameLength;
}

//sets global variables for rx communications to zero, so that receiver function starts over  
void resetResponseReceiver() {
    rx.byteCount = 0;
    rx.expectedLength = 0;
    rx.lastByteMs = 0;
}

// Length | Address | reCommand | Status | Data[] | LSB-CRC16 | MSB-CRC16


// sends frame without waiting for a response
bool sendFrame(
  uint8_t command,
  const uint8_t* data,
  size_t dataLength) {

  if (dataLength > 0 && data == nullptr) {
    Serial.println("\n[ERROR][RFID] Brak danych wymaganych przez komendę");
    return false;
  }

  uint8_t frameBuf[256];
  const size_t frameLength = buildCommandFrame(
    command,
    data,
    dataLength,
    frameBuf,
    sizeof(frameBuf));

  if (frameLength == 0) {
    Serial.print("\n[ERROR][RFID] Nie udało się zbudować ramki komendy");
    return false;
  }

  const size_t written = RfidSerial.write(frameBuf, frameLength);
  if (written != frameLength) {
    Serial.print("\n[ERROR][RFID] Nie wysłano całej ramki komendy");
    return false;
  }

  RfidSerial.flush();
  return true;
}

bool receiveFrame(size_t& responseLength) {

  responseLength = 0;

  while (RfidSerial.available() > 0) {
    const uint32_t now = millis();

    // Odrzuć niekompletną ramkę przed potraktowaniem kolejnego bajtu jako Len.
    if (rx.byteCount > 0 &&
        static_cast<uint32_t>(now - rx.lastByteMs) >
          INTER_BYTE_TIMEOUT_MS) {
      Serial.println(
        "\n[RFID] Przekroczono 15 ms pomiędzy bajtami");
      resetResponseReceiver();
    }

    const int value = RfidSerial.read();
    if (value < 0) {
      break;
    }

    const uint8_t readByte = static_cast<uint8_t>(value);

    if (rx.byteCount == 0) {
      if (readByte < 5) {
        continue;
      }

      rx.expectedLength = static_cast<size_t>(readByte) + 1;
      if (rx.expectedLength > sizeof(rx.buffer)) {
        Serial.printf(
          "\n[ERROR][RFID] Ramka wymaga %u bajtów, a bufor ma %u\n",
          static_cast<unsigned>(rx.expectedLength),
          static_cast<unsigned>(sizeof(rx.buffer)));
        resetResponseReceiver();
        return false;
      }
    }

    rx.buffer[rx.byteCount++] = readByte;
    rx.lastByteMs = millis();

    if (rx.byteCount == rx.expectedLength) {
      const size_t completedLength = rx.expectedLength;

      if (!validFrame(rx.buffer, completedLength)) {
        Serial.println("\n[RFID] Błąd CRC16 odpowiedzi");
        resetResponseReceiver();
        return false;
      }

      responseLength = completedLength;
      resetResponseReceiver();
      return true;
    }
  }

  if (rx.byteCount > 0 &&
      static_cast<uint32_t>(millis() - rx.lastByteMs) > INTER_BYTE_TIMEOUT_MS) {
    Serial.println("\n[RFID] Przekroczono 15 ms pomiędzy bajtami");
    resetResponseReceiver();
  }

  return false;
}

// sends requests with cmd codes to the rfid reader
bool startTrackedRequest(PendingRequest request, uint8_t command, const uint8_t* data, size_t dataLength) {

  const bool configurationOwnsRequest =
    readerConfigurationDispatching ||
    (readerConfigurationStep == ReaderConfigurationStep::DetectAntennas &&
     request == PendingRequest::ProbeAntenna);
  if (readerConfigurationIsActive() && !configurationOwnsRequest) {
    Serial.println("[ERROR][RFID] Trwa konfiguracja startowa czytnika");
    return false;
  }

  if (pendingRequest != PendingRequest::None) {
    Serial.print("\n[ERROR][RFID] Inna komenda nadal oczekuje");
    return false;
  }

  if (!sendFrame(command, data, dataLength)) {
    Serial.printf("\n[ERROR][RFID] Nie wysłano komendy 0x%02X", command);
    return false;
  }

  pendingRequest = request;
  requestStartedMs = millis();

  return true;
}

bool finishTrackedRequest(PendingRequest expectedRequest) {
  if (pendingRequest != expectedRequest) {
    Serial.println(
      "[ERROR][RFID] Odpowiedź nie pasuje do oczekiwanej komendy");
    return false;
  }

  pendingRequest = PendingRequest::None;
  return true;
}

// Logs a response and creates a non-owning view into the RX buffer.
bool prepareResponse(
  const char* operationName,
  size_t responseLength,
  RfidResponseView& response) {

  response = {};

  Serial.printf("\n[RFID] %s:\n", operationName);

  if (responseLength > sizeof(rx.buffer)) {
    Serial.printf(
      "[ERROR][RFID] %s: długość ramki przekracza rozmiar bufora\n",
      operationName);
    return false;
  }

  Serial.print("[RFID] Ramka RX: ");

  for (size_t i = 0; i < responseLength; i++) {
    Serial.printf("%02X ", static_cast<unsigned>(rx.buffer[i]));
  }
  Serial.println();

  constexpr size_t MIN_RESPONSE_LENGTH = 6;
  if (responseLength < MIN_RESPONSE_LENGTH) {
    Serial.printf(
      "[ERROR][RFID] %s: ramka odpowiedzi jest za krótka\n",
      operationName);
    return false;
  }

  response.status = rx.buffer[3];
  response.data = rx.buffer + 4;
  response.dataLength = responseLength - MIN_RESPONSE_LENGTH;
  return true;
}

// Validates regular responses whose success status is 0x00.
bool prepareSuccessfulResponse(
  const char* operationName,
  size_t responseLength,
  size_t minimumDataLength,
  RfidResponseView& response) {

  if (!prepareResponse(operationName, responseLength, response)) {
    return false;
  }

  if (response.status != 0x00) {
    Serial.printf(
      "[ERROR][RFID] %s: czytnik zwrócił status 0x%02X\n",
      operationName,
      static_cast<unsigned>(response.status));
    response = {};
    return false;
  }

  if (response.dataLength < minimumDataLength) {
    Serial.printf(
      "[ERROR][RFID] %s: oczekiwano co najmniej %u bajtów danych, odebrano %u\n",
      operationName,
      static_cast<unsigned>(minimumDataLength),
      static_cast<unsigned>(response.dataLength));
    response = {};
    return false;
  }

  return true;
}

bool requestReaderInfo() {
  return startTrackedRequest(PendingRequest::ReaderInfo, CMD_GET_READER_INFO, nullptr, 0);
}

bool handleReaderInfo(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Odbieranie informacji czytnika",
        responseLength,
        3,
        response)) {
    return false;
  }

  Serial.printf(
    "Wersja firmware: %u.%02u, model: 0x%02X\n",
    static_cast<unsigned>(response.data[0]),
    static_cast<unsigned>(response.data[1]),
    static_cast<unsigned>(response.data[2]));

  return true;
}

bool requestReaderTemperature() {
  return startTrackedRequest(PendingRequest::Temperature, CMD_GET_READER_TEMPERATURE, nullptr, 0);
}

void handleReaderTemperature(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Odczyt temperatury czytnika",
        responseLength,
        2,
        response)) {
    return;
  }

  Serial.print("\n[RFID] Temperatura czytnika: ");
  // plus or minus
  if (response.data[0] == 0) {
    Serial.print("-");
  }
  Serial.printf("%d℃", static_cast<unsigned>(response.data[1]));
}

bool requestWorkMode() {
  return startTrackedRequest(PendingRequest::WorkMode, CMD_GET_WORK_MODE, nullptr, 0);
}

bool handleWorkMode(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Tryb pracy czytnika",
        responseLength,
        1,
        response)) {
    return false;
  }

  const uint8_t workMode = response.data[0];

  Serial.print("\n[RFID] Tryb czytnika: ");
  Serial.printf(
    "0x%02X",
    static_cast<unsigned>(workMode));

  if (workMode != 0x00) {
    Serial.println(
      "\n[ERROR][RFID] Czytnik nie jest w answering mode");
    return false;
  }

  return true;
}

bool requestReadRegion() {
  return startTrackedRequest(
    PendingRequest::ReadRegion,
    CMD_READ_REGION,
    nullptr,
    0);
}

ConfigurationCheck handleReadRegion(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Odczyt regionu",
        responseLength,
        3,
        response)) {
    return ConfigurationCheck::Error;
  }

  const uint8_t frequencyBand = response.data[0];
  const uint8_t maximumChannel = response.data[1];
  const uint8_t minimumChannel = response.data[2];

  Serial.printf(
    "[RFID] FreBand=0x%02X, MaxFre=0x%02X, MinFre=0x%02X\n",
    static_cast<unsigned>(frequencyBand),
    static_cast<unsigned>(maximumChannel),
    static_cast<unsigned>(minimumChannel));

  const bool regionMatches =
    frequencyBand == RFID_REGION_EU3 &&
    maximumChannel == RFID_REGION_MAX_CHANNEL &&
    minimumChannel == RFID_REGION_MIN_CHANNEL;

  if (regionMatches) {
    Serial.println(
      "[RFID] Region jest poprawny: EU3 865.7-867.5 MHz");
    return ConfigurationCheck::Matches;
  }

  Serial.println("[RFID] Region wymaga ustawienia");
  return ConfigurationCheck::NeedsUpdate;
}

bool requestSetRegion() {
  const uint8_t data[] = {
    0x01, // temporary setting, do not save on power-off
    RFID_REGION_EU3,
    RFID_REGION_MAX_CHANNEL,
    RFID_REGION_MIN_CHANNEL
  };

  return startTrackedRequest(
    PendingRequest::SetRegion,
    CMD_SET_REGION,
    data,
    sizeof(data));
}

bool handleSetRegion(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Ustawienie regionu EU3",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.println("[RFID] Region ustawiony tymczasowo");
  return true;
}

bool requestReadAntennaPower() {
  return startTrackedRequest(
    PendingRequest::ReadAntennaPower,
    CMD_READ_ANTENNA_POWER,
    nullptr,
    0);
}

ConfigurationCheck handleReadAntennaPower(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Odczyt mocy portów antenowych",
        responseLength,
        ANTENNA_PORT_COUNT,
        response)) {
    return ConfigurationCheck::Error;
  }

  bool allPowersMatch = true;

  for (size_t i = 0; i < ANTENNA_PORT_COUNT; i++) {
    const uint8_t powerDbm =
      static_cast<uint8_t>(response.data[i] & 0x7F);

    Serial.printf(
      "[RFID] ANT%u: %u dBm\n",
      static_cast<unsigned>(i + 1),
      static_cast<unsigned>(powerDbm));

    if (powerDbm != RFID_POWER_DBM) {
      allPowersMatch = false;
    }
  }

  if (allPowersMatch) {
    Serial.printf(
      "[RFID] Wszystkie porty mają ustawione %u dBm\n",
      static_cast<unsigned>(RFID_POWER_DBM));
    return ConfigurationCheck::Matches;
  }

  Serial.println("[RFID] Moc portów wymaga ustawienia");
  return ConfigurationCheck::NeedsUpdate;
}

bool requestSetRfPower() {

  uint8_t data[ANTENNA_PORT_COUNT];
  const uint8_t temporaryPower =
    static_cast<uint8_t>(0x80 | RFID_POWER_DBM);

  for (size_t i = 0; i < ANTENNA_PORT_COUNT; i++) {
    data[i] = temporaryPower;
  }

  return startTrackedRequest(
    PendingRequest::SetRfPower,
    CMD_SET_RF_POWER,
    data,
    sizeof(data));
}

bool handleSetRfPower(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Ustawienie mocy portów antenowych",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.println("[RFID] Moc wszystkich portów ustawiona tymczasowo");
  return true;
}

bool requestEnableAntennaCheck() {
  const uint8_t data[] = {
    0x01
  };

  return startTrackedRequest(PendingRequest::EnableAntennaCheck, CMD_SET_ANTENNA_CHECK, data, sizeof(data));
}

bool handleEnableAntennaCheck(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Włączenie Antenna Check",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.println("[RFID] Kontrola anten została włączona");
  return true;
}

bool startAntennaDetection() {
  if (antennaDetectionActive) {
    Serial.println("[ERROR][RFID] Sprawdzanie anten już trwa");
    return false;
  }

  if (pendingRequest != PendingRequest::None) {
    Serial.println(
      "[ERROR][RFID] Nie można rozpocząć sprawdzania anten, inna komenda oczekuje");
    return false;
  }

  testedAntennaPort = 1;
  activeAntennaMask = 0;
  disconnectedAntennaMask = 0;
  unknownAntennaMask = ALL_ANTENNA_PORTS_MASK;
  antennaDetectionActive = true;
  nextAntennaProbeEarliestMs = millis();

  Serial.printf(
    "\n[RFID] Rozpoczynam sprawdzanie %u portów antenowych\n",
    static_cast<unsigned>(ANTENNA_PORT_COUNT));
  return true;
}

void serviceAntennaDetection() {
  if (!antennaDetectionActive ||
      pendingRequest != PendingRequest::None) {
    return;
  }

  if (testedAntennaPort < 1 ||
      testedAntennaPort > ANTENNA_PORT_COUNT) {
    finishAntennaDetection();
    return;
  }

  const uint32_t now = millis();
  if (static_cast<int32_t>(
        now - nextAntennaProbeEarliestMs) < 0) {
    return;
  }

  if (!requestAntennaProbe(testedAntennaPort)) {
    Serial.printf(
      "[ERROR][RFID] Nie udało się wysłać testu ANT%u\n",
      static_cast<unsigned>(testedAntennaPort));
    recordAntennaProbeResult(AntennaProbeResult::Unknown);

    if (antennaDetectionActive) {
      nextAntennaProbeEarliestMs =
        millis() + ANTENNA_PROBE_TIMEOUT_RECOVERY_MS;
    }
  }
}

bool requestAntennaProbe(uint8_t port) {
  if (port < 1 || port > ANTENNA_PORT_COUNT) {
    Serial.printf(
      "[ERROR][RFID] Nieprawidłowy port anteny: %u\n",
      static_cast<unsigned>(port));
    return false;
  }

  const uint8_t data[] = {
    ANTENNA_PROBE_Q_VALUE,
    ANTENNA_PROBE_SESSION,
    ANTENNA_PROBE_TARGET,
    static_cast<uint8_t>(
      ANTENNA_SELECTOR_BASE | (port - 1)),
    ANTENNA_PROBE_SCAN_TIME
  };

  Serial.printf(
    "\n[RFID] Test ANT%u (%u ms)\n",
    static_cast<unsigned>(port),
    static_cast<unsigned>(ANTENNA_PROBE_SCAN_TIME) * 100U);

  if (!startTrackedRequest(
        PendingRequest::ProbeAntenna,
        CMD_INVENTORY_G2,
        data,
        sizeof(data))) {
    return false;
  }

  antennaProbeStartedMs = requestStartedMs;
  return true;
}

AntennaProbeResult handleAntennaProbeResponse(
  size_t responseLength) {

  RfidResponseView response;
  if (!prepareResponse(
        "Odpowiedź testu połączenia anteny",
        responseLength,
        response)) {
    return AntennaProbeResult::Unknown;
  }

  if (!antennaDetectionActive ||
      testedAntennaPort < 1 ||
      testedAntennaPort > ANTENNA_PORT_COUNT) {
    Serial.println(
      "[ERROR][RFID] Odebrano wynik testu bez aktywnego portu");
    return AntennaProbeResult::Ignore;
  }

  const bool inventoryStatusWithAntennaNumber =
    response.status >= STATUS_INVENTORY_COMPLETED &&
    response.status <= STATUS_INVENTORY_MEMORY_FULL;

  if (inventoryStatusWithAntennaNumber) {
    constexpr size_t MIN_INVENTORY_DATA_LENGTH = 2; // Ant + Num

    if (response.dataLength < MIN_INVENTORY_DATA_LENGTH) {
      Serial.printf(
        "[ERROR][RFID] ANT%u: odpowiedź inventory ma za mało danych\n",
        static_cast<unsigned>(testedAntennaPort));

      // Status 0x03 still announces another frame, so keep draining it.
      if (response.status == STATUS_INVENTORY_MORE_FRAMES) {
        return AntennaProbeResult::MoreFrames;
      }

      return AntennaProbeResult::Unknown;
    }

    const uint8_t expectedAntenna =
      static_cast<uint8_t>(testedAntennaPort - 1);

    if (response.data[0] != expectedAntenna) {
      Serial.printf(
        "[RFID] Pomijam odpowiedź ANT%u podczas oczekiwania na ANT%u\n",
        static_cast<unsigned>(response.data[0] + 1),
        static_cast<unsigned>(testedAntennaPort));
      return AntennaProbeResult::Ignore;
    }
  }

  switch (response.status) {
    case STATUS_INVENTORY_MORE_FRAMES:
      Serial.printf(
        "[RFID] ANT%u: oczekiwanie na kolejną ramkę inventory\n",
        static_cast<unsigned>(testedAntennaPort));
      return AntennaProbeResult::MoreFrames;

    case STATUS_INVENTORY_COMPLETED:
    case STATUS_INVENTORY_TIMEOUT:
    case STATUS_INVENTORY_MEMORY_FULL:
    case STATUS_TAG_COMMUNICATION_ERROR:
    case STATUS_NO_OPERABLE_TAGS:
    case STATUS_TAG_ERROR:
      return AntennaProbeResult::Connected;

    case STATUS_ANTENNA_CONNECTION_ERROR:
      return AntennaProbeResult::Disconnected;

    default:
      Serial.printf(
        "[ERROR][RFID] ANT%u: nieoczekiwany status 0x%02X\n",
        static_cast<unsigned>(testedAntennaPort),
        static_cast<unsigned>(response.status));
      return AntennaProbeResult::Unknown;
  }
}

void recordAntennaProbeResult(AntennaProbeResult result) {
  if (!antennaDetectionActive ||
      testedAntennaPort < 1 ||
      testedAntennaPort > ANTENNA_PORT_COUNT) {
    return;
  }

  const uint16_t bit = antennaBit(testedAntennaPort);

  activeAntennaMask &= static_cast<uint16_t>(~bit);
  disconnectedAntennaMask &= static_cast<uint16_t>(~bit);
  unknownAntennaMask &= static_cast<uint16_t>(~bit);

  switch (result) {
    case AntennaProbeResult::Connected:
      activeAntennaMask |= bit;
      Serial.printf(
        "[RFID] ANT%u: podłączona\n",
        static_cast<unsigned>(testedAntennaPort));
      break;

    case AntennaProbeResult::Disconnected:
      disconnectedAntennaMask |= bit;
      Serial.printf(
        "[RFID] ANT%u: nie przeszła kontroli połączenia (0xF8)\n",
        static_cast<unsigned>(testedAntennaPort));
      break;

    case AntennaProbeResult::Unknown:
      unknownAntennaMask |= bit;
      Serial.printf(
        "[RFID] ANT%u: wynik nieznany\n",
        static_cast<unsigned>(testedAntennaPort));
      break;

    case AntennaProbeResult::Ignore:
    case AntennaProbeResult::MoreFrames:
      return;
  }

  ++testedAntennaPort;

  if (testedAntennaPort > ANTENNA_PORT_COUNT) {
    finishAntennaDetection();
  }
  else {
    nextAntennaProbeEarliestMs = millis();
  }
}

void handleAntennaProbeTimeout() {
  recordAntennaProbeResult(AntennaProbeResult::Unknown);

  if (antennaDetectionActive) {
    // Leave time to drain a delayed frame before testing another port.
    nextAntennaProbeEarliestMs =
      millis() + ANTENNA_PROBE_TIMEOUT_RECOVERY_MS;
  }
}

void finishAntennaDetection() {
  antennaDetectionActive = false;
  testedAntennaPort = 0;

  Serial.println("\n[RFID] Zakończono sprawdzanie anten");
  printAntennaPorts("Podłączone", activeAntennaMask);
  printAntennaPorts("Odłączone", disconnectedAntennaMask);
  printAntennaPorts("Nieznane", unknownAntennaMask);

  if (activeAntennaMask == 0) {
    Serial.println(
      "[ERROR][RFID] Nie wykryto żadnej aktywnej anteny");
    if (readerConfigurationStep == ReaderConfigurationStep::DetectAntennas) {
      failReaderConfiguration("nie wykryto aktywnej anteny");
    }
    return;
  }

  continueReaderConfiguration(ReaderConfigurationStep::DetectAntennas,
                              ReaderConfigurationStep::ConfigureAntennas);
}

void printAntennaPorts(const char* label, uint16_t mask) {
  Serial.printf(
    "[RFID] %s [maska 0x%04X]: ",
    label,
    static_cast<unsigned>(mask));

  bool first = true;

  for (uint8_t port = 1; port <= ANTENNA_PORT_COUNT; ++port) {
    if ((mask & antennaBit(port)) == 0) {
      continue;
    }

    if (!first) {
      Serial.print(", ");
    }

    Serial.printf("ANT%u", static_cast<unsigned>(port));
    first = false;
  }

  if (first) {
    Serial.print("brak");
  }

  Serial.println();
}

bool requestConfigureAntennas() {
  if (activeAntennaMask == 0) {
    return false;
  }

  const uint8_t data[] = {
    0x01, // don't save
    static_cast<uint8_t>(activeAntennaMask >> 8), // antennas 9 - 16
    static_cast<uint8_t>(activeAntennaMask) // antennas 1 - 8
  };

  return startTrackedRequest(
    PendingRequest::ConfigureAntennas,
    CMD_SET_ACTIVE_ANTENNAS,
    data,
    sizeof(data));
}


bool handleConfigureAntennas(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Ustawianie maski aktywnych anten",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.print("\n[RFID] Skonfigurowano multipleksację anten");
  return true;
}

bool requestDisableTagFocus() {
  const uint8_t data[] = {
    0x01, // temporary setting, do not save on power-off
    CFG_TAG_FOCUS,
    TAG_FOCUS  // disable TagFocus
  };

  if (!startTrackedRequest(
        PendingRequest::DisableTagFocus,
        CMD_SET_CFG,
        data,
        sizeof(data))) {
    return false;
  }

  return true;
}

bool handleDisableTagFocus(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Wyłączanie TagFocus",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.println("[RFID] TagFocus wyłączony");
  return true;
}

bool requestSetQAndSession() {
  const uint8_t data[] = {
    0x01, // temporary setting, do not save on power-off
    CFG_Q_AND_SESSION,
    Q_VALUE,
    SESSION
  };

  if (!startTrackedRequest(
        PendingRequest::SetQAndSession,
        CMD_SET_CFG,
        data,
        sizeof(data))) {
    return false;
  }

  return true;
}

bool handleSetQAndSession(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Ustawianie parametru Q i Session",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.printf("\n[RFID] Ustawiono Q = %d, Session = S%d", Q_VALUE, SESSION);
  return true;
}

bool requestSetEpcMode() {
  const uint8_t data[] = {
    0x01, // temporary setting, do not save on power-off
    CFG_TID,
    0x06, // default TID word address, ignored for EPC mode
    0x00  // LenTID=0 selects EPC inventory
  };

  if (!startTrackedRequest(
        PendingRequest::SetEpcMode,
        CMD_SET_CFG,
        data,
        sizeof(data))) {
    return false;
  }

  return true;
}

bool handleSetEpcMode(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Ustawianie trybu danych fast inventory",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.println("[RFID] Skonfigurowano zwracanie EPC (LenTID=0)");
  return true;
}

bool requestClearInventoryMask() {
  const uint8_t data[] = {
    0x01, // temporary setting, do not save on power-off
    CFG_MASK,
    0x01,       // EPC memory
    0x00, 0x20, // default mask bit address
    0x00        // MaskLen=0 disables filtering
  };

  if (!startTrackedRequest(
        PendingRequest::ClearInventoryMask,
        CMD_SET_CFG,
        data,
        sizeof(data))) {
    return false;
  }

  return true;
}

bool handleClearInventoryMask(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Wyłączanie maski fast inventory",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.println("[RFID] Maska tagów wyłączona");
  return true;
}

void resetFastInventoryStatistics() {
  // Zerujemy tylko bieżącą próbę; wyniki całej serii muszą pozostać.
  memset(fastInventoryTags, 0, sizeof(fastInventoryTags));
  fastInventoryTagCount = 0;
  fastInventoryReadCount = 0;
  fastInventoryMalformedFrameCount = 0;
  fastInventoryDroppedTagCount = 0;
  fastInventoryStartedMs = 0;
  fastInventoryLastFrameMs = 0;
  fastInventoryDrainStartedMs = 0;
}

void resetFastInventoryTestStatistics(uint32_t trialCount) {
  // Agregat jest zerowany tylko raz, przed pierwszą próbą serii.
  memset(fastInventoryTestTags, 0, sizeof(fastInventoryTestTags));
  fastInventoryTestTagCount = 0;
  fastInventoryRequestedTrialCount = trialCount;
  fastInventoryCompletedTrialCount = 0;
  fastInventoryTestReadCount = 0;
  fastInventoryTestMalformedFrameCount = 0;
  fastInventoryTestDroppedTagCount = 0;
  fastInventoryPauseStartedMs = 0;
}

bool startFastInventoryTest(uint32_t trialCount) {
  if (trialCount == 0 ||
      fastInventoryState != FastInventoryState::Idle ||
      pendingRequest != PendingRequest::None ||
      activeAntennaMask == 0) {
    Serial.println(
      "[ERROR][RFID] Seria fast inventory nie może zostać uruchomiona");
    return false;
  }

  resetFastInventoryTestStatistics(trialCount);

  Serial.printf(
    "\n[RFID][FAST] Start serii: %lu prób, "
    "%lu ms skanowania i %lu ms przerwy\n",
    static_cast<unsigned long>(trialCount),
    static_cast<unsigned long>(FAST_INVENTORY_TIME_MS),
    static_cast<unsigned long>(FAST_INVENTORY_TRIAL_PAUSE_MS));

  return requestStartFastInventory();
}

bool requestStartFastInventory() {
  const bool stateAllowsStart =
    fastInventoryState == FastInventoryState::Idle ||
    fastInventoryState == FastInventoryState::WaitingBetweenTrials;

  if (!stateAllowsStart ||
      pendingRequest != PendingRequest::None ||
      activeAntennaMask == 0) {
    Serial.println(
      "[ERROR][RFID] Fast inventory nie może zostać uruchomione");
    return false;
  }

  resetFastInventoryStatistics();

  const uint8_t data[] = {
    FAST_INVENTORY_TARGET
  };

  if (!startTrackedRequest(
        PendingRequest::StartFastInventory,
        CMD_START_FAST_INVENTORY,
        data,
        sizeof(data))) {
    return false;
  }

  fastInventoryState = FastInventoryState::Starting;
  return true;
}

bool handleStartFastInventory(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Uruchamianie fast inventory",
        responseLength,
        0,
        response)) {
    return false;
  }

  fastInventoryStartedMs = millis();
  fastInventoryLastFrameMs = fastInventoryStartedMs;
  fastInventoryState = FastInventoryState::Running;

  Serial.printf(
    "[RFID][FAST] Rozpoczęto próbę %lu/%lu na %lu ms\n",
    static_cast<unsigned long>(
      fastInventoryCompletedTrialCount + 1U),
    static_cast<unsigned long>(
      fastInventoryRequestedTrialCount),
    static_cast<unsigned long>(FAST_INVENTORY_TIME_MS));
  return true;
}

bool handleFastInventoryTag(size_t responseLength) {
  const bool inventoryAcceptsTags =
    fastInventoryState == FastInventoryState::Starting ||
    fastInventoryState == FastInventoryState::Running ||
    fastInventoryState == FastInventoryState::Stopping ||
    fastInventoryState == FastInventoryState::Draining;

  if (!inventoryAcceptsTags) {
    return false;
  }

  fastInventoryLastFrameMs = millis();

  constexpr size_t MIN_FAST_TAG_FRAME_LENGTH = 9;
  if (responseLength < MIN_FAST_TAG_FRAME_LENGTH ||
      rx.buffer[3] != 0x00) {
    ++fastInventoryMalformedFrameCount;
    return false;
  }

  const size_t dataLength = responseLength - 6;
  const uint8_t* data = rx.buffer + 4;
  const uint8_t antennaRaw = data[0];
  const uint8_t lengthFlags = data[1];
  const size_t identifierLength = lengthFlags & 0x3F;
  const bool hasPhaseAndFrequency =
    (lengthFlags & 0x40) != 0;
  const bool hasFastId = (lengthFlags & 0x80) != 0;

  const size_t expectedDataLength =
    3 + identifierLength +
    (hasPhaseAndFrequency ? 7 : 0);

  if (antennaRaw >= ANTENNA_PORT_COUNT ||
      identifierLength == 0 ||
      identifierLength > MAX_FAST_EPC_LENGTH ||
      dataLength != expectedDataLength) {
    ++fastInventoryMalformedFrameCount;
    return false;
  }

  size_t epcLength = identifierLength;

  // W trybie FastID ostatnie 12 bajtów identyfikatora stanowi TID.
  if (hasFastId) {
    constexpr size_t FAST_ID_TID_LENGTH = 12;

    if (identifierLength <= FAST_ID_TID_LENGTH) {
      ++fastInventoryMalformedFrameCount;
      return false;
    }

    epcLength -= FAST_ID_TID_LENGTH;
  }

  const uint8_t* epc = data + 2;
  const uint16_t antennaMask =
    static_cast<uint16_t>(uint16_t{1} << antennaRaw);

  ++fastInventoryReadCount;

  for (size_t i = 0; i < fastInventoryTagCount; ++i) {
    FastInventoryTag& tag = fastInventoryTags[i];

    if (tag.epcLength == epcLength &&
        memcmp(tag.epc, epc, epcLength) == 0) {
      tag.antennaMask |= antennaMask;
      ++tag.readCount;
      return true;
    }
  }

  if (fastInventoryTagCount >= MAX_FAST_INVENTORY_TAGS) {
    ++fastInventoryDroppedTagCount;
    return false;
  }

  FastInventoryTag& tag =
    fastInventoryTags[fastInventoryTagCount++];
  tag = FastInventoryTag{};
  memcpy(tag.epc, epc, epcLength);
  tag.epcLength = static_cast<uint8_t>(epcLength);
  tag.antennaMask = antennaMask;
  tag.readCount = 1;
  return true;
}

int findFastInventoryTag(
  const uint8_t* epc,
  size_t epcLength) {
  for (size_t i = 0; i < fastInventoryTagCount; ++i) {
    const FastInventoryTag& tag = fastInventoryTags[i];

    if (tag.epcLength == epcLength &&
        memcmp(tag.epc, epc, epcLength) == 0) {
      return static_cast<int>(i);
    }
  }

  return -1;
}

int findFastInventoryTestTag(
  const uint8_t* epc,
  size_t epcLength) {
  for (size_t i = 0; i < fastInventoryTestTagCount; ++i) {
    const FastInventoryTestTag& tag = fastInventoryTestTags[i];

    if (tag.epcLength == epcLength &&
        memcmp(tag.epc, epc, epcLength) == 0) {
      return static_cast<int>(i);
    }
  }

  return -1;
}

bool mergeFastInventoryTrial() {
  // Brak EPC w tej próbie zeruje jego maskę niezawodnych anten.
  for (size_t i = 0; i < fastInventoryTestTagCount; ++i) {
    FastInventoryTestTag& aggregateTag = fastInventoryTestTags[i];
    const int currentTagIndex = findFastInventoryTag(
      aggregateTag.epc,
      aggregateTag.epcLength);

    if (currentTagIndex < 0) {
      aggregateTag.reliableAntennaMask = 0;
      continue;
    }

    const FastInventoryTag& currentTag =
      fastInventoryTags[static_cast<size_t>(currentTagIndex)];
    aggregateTag.reliableAntennaMask &= currentTag.antennaMask;
    aggregateTag.anyAntennaMask |= currentTag.antennaMask;
    ++aggregateTag.detectedTrialCount;
    aggregateTag.totalReadCount += currentTag.readCount;
  }

  // EPC pojawiające się pierwszy raz później nie było obecne we wcześniejszych próbach.
  for (size_t i = 0; i < fastInventoryTagCount; ++i) {
    const FastInventoryTag& currentTag = fastInventoryTags[i];

    if (findFastInventoryTestTag(
          currentTag.epc,
          currentTag.epcLength) >= 0) {
      continue;
    }

    if (fastInventoryTestTagCount >=
        MAX_FAST_INVENTORY_TEST_TAGS) {
      ++fastInventoryTestDroppedTagCount;
      return false;
    }

    FastInventoryTestTag& aggregateTag =
      fastInventoryTestTags[fastInventoryTestTagCount++];
    aggregateTag = FastInventoryTestTag{};
    memcpy(
      aggregateTag.epc,
      currentTag.epc,
      currentTag.epcLength);
    aggregateTag.epcLength = currentTag.epcLength;
    aggregateTag.reliableAntennaMask =
      fastInventoryCompletedTrialCount == 0
        ? currentTag.antennaMask
        : 0;
    aggregateTag.anyAntennaMask = currentTag.antennaMask;
    aggregateTag.detectedTrialCount = 1;
    aggregateTag.totalReadCount = currentTag.readCount;
  }

  fastInventoryTestReadCount += fastInventoryReadCount;
  fastInventoryTestMalformedFrameCount +=
    fastInventoryMalformedFrameCount;
  fastInventoryTestDroppedTagCount += fastInventoryDroppedTagCount;
  return true;
}

void finishFastInventoryTrial() {
  // Przepełniona tablica bieżącej próby dawałaby mylący wynik niezawodności.
  if (fastInventoryDroppedTagCount > 0) {
    fastInventoryTestDroppedTagCount += fastInventoryDroppedTagCount;
    fastInventoryState = FastInventoryState::Error;
    Serial.println(
      "[ERROR][RFID] Za dużo EPC w próbie; seria została przerwana");
    return;
  }

  if (!mergeFastInventoryTrial()) {
    fastInventoryState = FastInventoryState::Error;
    Serial.println(
      "[ERROR][RFID] Za dużo różnych EPC w całej serii; seria została przerwana");
    return;
  }

  // Sortujemy także listę pojedynczej próby przed jej wydrukowaniem.
  sortFastInventoryTags();
  ++fastInventoryCompletedTrialCount;
  printFastInventorySummary();

  if (fastInventoryCompletedTrialCount >=
      fastInventoryRequestedTrialCount) {
    fastInventoryState = FastInventoryState::Idle;
    sortFastInventoryTestTags();
    printFastInventoryTestSummary();
    printFastInventoryCsv();
    publishFastInventoryResult();
    return;
  }

  fastInventoryState = FastInventoryState::WaitingBetweenTrials;
  Serial.printf(
    "[RFID][FAST] Przerwa przed kolejną próbą: %lu ms\n",
    static_cast<unsigned long>(FAST_INVENTORY_TRIAL_PAUSE_MS));

  // Odliczanie zaczyna się po wydrukowaniu podsumowania bieżącej próby.
  fastInventoryPauseStartedMs = millis();
}

bool requestStopFastInventory() {
  if (!startTrackedRequest(
        PendingRequest::StopFastInventory,
        CMD_STOP_FAST_INVENTORY,
        nullptr,
        0)) {
    return false;
  }

  fastInventoryState = FastInventoryState::Stopping;
  Serial.println("[RFID][FAST] Wysłano komendę zatrzymania");
  return true;
}

bool handleStopFastInventory(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Zatrzymywanie fast inventory",
        responseLength,
        0,
        response)) {
    return false;
  }

  const uint32_t now = millis();
  fastInventoryDrainStartedMs = now;
  fastInventoryLastFrameMs = now;
  fastInventoryState = FastInventoryState::Draining;

  Serial.println(
    "[RFID][FAST] Czytnik zatrzymany, opróżniam bufor RX");
  return true;
}

void serviceFastInventory() {
  const uint32_t now = millis();

  // Następna próba startuje dopiero po pełnych 5 sekundach przerwy.
  if (fastInventoryState ==
      FastInventoryState::WaitingBetweenTrials) {
    if (static_cast<uint32_t>(
          now - fastInventoryPauseStartedMs) >=
        FAST_INVENTORY_TRIAL_PAUSE_MS) {
      if (!requestStartFastInventory()) {
        fastInventoryState = FastInventoryState::Error;
        Serial.println(
          "[ERROR][RFID] Nie rozpoczęto kolejnej próby fast inventory");
      }
    }

    return;
  }

  if (fastInventoryState == FastInventoryState::Running &&
      static_cast<uint32_t>(now - fastInventoryStartedMs) >=
        FAST_INVENTORY_TIME_MS) {
    if (!requestStopFastInventory()) {
      fastInventoryState = FastInventoryState::Error;
      Serial.println(
        "[ERROR][RFID] Nie wysłano komendy stop fast inventory");
    }

    return;
  }

  if (fastInventoryState != FastInventoryState::Draining) {
    return;
  }

  const bool receiverIsEmpty =
    RfidSerial.available() == 0 &&
    rx.byteCount == 0;
  const bool drainRanLongEnough =
    static_cast<uint32_t>(now - fastInventoryDrainStartedMs) >=
      FAST_INVENTORY_DRAIN_QUIET_MS;
  const bool noRecentTagFrames =
    static_cast<uint32_t>(now - fastInventoryLastFrameMs) >=
      FAST_INVENTORY_DRAIN_QUIET_MS;
  const bool drainLimitReached =
    static_cast<uint32_t>(now - fastInventoryDrainStartedMs) >=
      FAST_INVENTORY_DRAIN_MAX_MS;

  if (receiverIsEmpty &&
      drainRanLongEnough &&
      noRecentTagFrames) {
    // Dopiero po opróżnieniu RX próba jest kompletna i może trafić do agregatu.
    finishFastInventoryTrial();
    return;
  }

  if (drainLimitReached) {
    // Nie zaliczamy niepełnej próby, bo mogłaby zawyżyć liczbę zer w CSV.
    fastInventoryState = FastInventoryState::Error;
    Serial.println(
      "[ERROR][RFID] Nie opróżniono RX po fast inventory; seria przerwana");
  }
}

void printEpc(const uint8_t* epc, size_t epcLength) {
  for (size_t byteIndex = 0;
       byteIndex < epcLength;
       ++byteIndex) {
    Serial.printf(
      "%02X",
      static_cast<unsigned>(epc[byteIndex]));
  }
}

void publishFastInventoryResult() {
  constexpr char HEX_DIGITS[] = "0123456789ABCDEF";

  // {"tags":["AABB",...]}: dwa znaki na każdy bajt EPC oraz cudzysłowy
  // i przecinki dla każdego elementu.
  size_t payloadLength = sizeof("{\"tags\":[]}") - 1;
  for (size_t i = 0; i < fastInventoryTestTagCount; ++i) {
    payloadLength +=
      (2U * fastInventoryTestTags[i].epcLength) + 2U;
    if (i > 0) {
      ++payloadLength;
    }
  }

  String payload;
  if (!payload.reserve(payloadLength)) {
    Serial.println(
      "[ERROR][MQTT] Brak pamięci na JSON wyniku inwentaryzacji");
    return;
  }

  payload = "{\"tags\":[";
  for (size_t i = 0; i < fastInventoryTestTagCount; ++i) {
    const FastInventoryTestTag& tag = fastInventoryTestTags[i];

    if (i > 0) {
      payload += ',';
    }
    payload += '"';

    for (size_t byteIndex = 0;
         byteIndex < tag.epcLength;
         ++byteIndex) {
      const uint8_t value = tag.epc[byteIndex];
      payload += HEX_DIGITS[value >> 4];
      payload += HEX_DIGITS[value & 0x0F];
    }

    payload += '"';
  }
  payload += "]}";

  // Domyślny bufor PubSubClient (256 B) może być za mały dla listy EPC.
  const size_t mqttBufferSize =
    payload.length() + strlen(reader_read_tags.c_str()) + 16U;
  if (!client.setBufferSize(mqttBufferSize)) {
    Serial.println(
      "[ERROR][MQTT] Nie można powiększyć bufora wyniku inwentaryzacji");
    return;
  }

  publishMessage(reader_read_tags.c_str(), payload, false, true);
}

void printAntennaMask(uint16_t mask) {
  bool firstAntenna = true;

  for (uint8_t port = 1;
       port <= ANTENNA_PORT_COUNT;
       ++port) {
    if ((mask & antennaBit(port)) == 0) {
      continue;
    }

    if (!firstAntenna) {
      Serial.print(", ");
    }

    Serial.printf("ANT%u", static_cast<unsigned>(port));
    firstAntenna = false;
  }

  if (firstAntenna) {
    Serial.print("brak");
  }
}

void printFastInventorySummary() {
  Serial.printf(
    "\n[RFID][FAST] ===== PRÓBA %lu/%lu =====\n",
    static_cast<unsigned long>(fastInventoryCompletedTrialCount),
    static_cast<unsigned long>(fastInventoryRequestedTrialCount));
  Serial.printf(
    "[RFID][FAST] Odczyty: %lu, unikalne EPC: %u, "
    "błędne ramki: %lu, pominięte nowe EPC: %lu\n",
    static_cast<unsigned long>(fastInventoryReadCount),
    static_cast<unsigned>(fastInventoryTagCount),
    static_cast<unsigned long>(fastInventoryMalformedFrameCount),
    static_cast<unsigned long>(fastInventoryDroppedTagCount));

  for (size_t i = 0; i < fastInventoryTagCount; ++i) {
    const FastInventoryTag& tag = fastInventoryTags[i];

    Serial.print("[RFID][FAST] EPC=");
    printEpc(tag.epc, tag.epcLength);
    Serial.print(" | anteny: ");
    printAntennaMask(tag.antennaMask);
    Serial.printf(
      " | odczyty: %lu\n",
      static_cast<unsigned long>(tag.readCount));
  }

  Serial.println("[RFID][FAST] =========================");
}

int compareEpcNumerically(
  const uint8_t* left,
  size_t leftLength,
  const uint8_t* right,
  size_t rightLength) {
  // Pomijamy zera wiodące, aby porównywać EPC jak liczby big-endian.
  size_t leftFirstSignificantByte = 0;
  while (leftFirstSignificantByte < leftLength &&
         left[leftFirstSignificantByte] == 0) {
    ++leftFirstSignificantByte;
  }

  size_t rightFirstSignificantByte = 0;
  while (rightFirstSignificantByte < rightLength &&
         right[rightFirstSignificantByte] == 0) {
    ++rightFirstSignificantByte;
  }

  const size_t leftSignificantLength =
    leftLength - leftFirstSignificantByte;
  const size_t rightSignificantLength =
    rightLength - rightFirstSignificantByte;

  if (leftSignificantLength < rightSignificantLength) {
    return -1;
  }

  if (leftSignificantLength > rightSignificantLength) {
    return 1;
  }

  const int byteComparison = memcmp(
    left + leftFirstSignificantByte,
    right + rightFirstSignificantByte,
    leftSignificantLength);

  if (byteComparison != 0) {
    return byteComparison;
  }

  // Równe wartości z różną liczbą zer wiodących układamy deterministycznie.
  if (leftLength < rightLength) {
    return -1;
  }

  if (leftLength > rightLength) {
    return 1;
  }

  return 0;
}

int compareFastInventoryTags(
  const FastInventoryTag& left,
  const FastInventoryTag& right) {
  return compareEpcNumerically(
    left.epc,
    left.epcLength,
    right.epc,
    right.epcLength);
}

int compareFastInventoryTestTags(
  const FastInventoryTestTag& left,
  const FastInventoryTestTag& right) {
  return compareEpcNumerically(
    left.epc,
    left.epcLength,
    right.epc,
    right.epcLength);
}

void sortFastInventoryTags() {
  for (size_t i = 1; i < fastInventoryTagCount; ++i) {
    const FastInventoryTag current = fastInventoryTags[i];
    size_t destination = i;

    while (destination > 0 &&
           compareFastInventoryTags(
             current,
             fastInventoryTags[destination - 1]) < 0) {
      fastInventoryTags[destination] =
        fastInventoryTags[destination - 1];
      --destination;
    }

    fastInventoryTags[destination] = current;
  }
}

void sortFastInventoryTestTags() {
  // Ta sama kolejność numeryczna trafia do podsumowania końcowego i CSV.
  for (size_t i = 1; i < fastInventoryTestTagCount; ++i) {
    const FastInventoryTestTag current = fastInventoryTestTags[i];
    size_t destination = i;

    while (destination > 0 &&
           compareFastInventoryTestTags(
             current,
             fastInventoryTestTags[destination - 1]) < 0) {
      fastInventoryTestTags[destination] =
        fastInventoryTestTags[destination - 1];
      --destination;
    }

    fastInventoryTestTags[destination] = current;
  }
}

uint16_t reliableAntennaMaskForTag(
  const FastInventoryTestTag& tag) {
  // Dodatkowa kontrola wyklucza tag, którego zabrakło w całej jednej próbie.
  if (tag.detectedTrialCount !=
      fastInventoryRequestedTrialCount) {
    return 0;
  }

  return static_cast<uint16_t>(
    tag.reliableAntennaMask & activeAntennaMask);
}

uint8_t countAntennasInMask(uint16_t mask) {
  uint8_t antennaCount = 0;

  for (uint8_t port = 1;
       port <= ANTENNA_PORT_COUNT;
       ++port) {
    if ((mask & antennaBit(port)) != 0) {
      ++antennaCount;
    }
  }

  return antennaCount;
}

void printFastInventoryTestSummary() {
  Serial.println(
    "\n[RFID][FAST] ===== PODSUMOWANIE CAŁEJ SERII =====");
  Serial.printf(
    "[RFID][FAST] Próby: %lu/%lu, odczyty: %lu, "
    "unikalne EPC: %u, błędne ramki: %lu, pominięte EPC: %lu\n",
    static_cast<unsigned long>(fastInventoryCompletedTrialCount),
    static_cast<unsigned long>(fastInventoryRequestedTrialCount),
    static_cast<unsigned long>(fastInventoryTestReadCount),
    static_cast<unsigned>(fastInventoryTestTagCount),
    static_cast<unsigned long>(fastInventoryTestMalformedFrameCount),
    static_cast<unsigned long>(fastInventoryTestDroppedTagCount));

  for (size_t i = 0; i < fastInventoryTestTagCount; ++i) {
    const FastInventoryTestTag& tag = fastInventoryTestTags[i];
    const uint16_t reliableMask = reliableAntennaMaskForTag(tag);

    Serial.print("[RFID][FAST] EPC=");
    printEpc(tag.epc, tag.epcLength);
    Serial.printf(
      " | próby z tagiem: %lu/%lu | niezawodne anteny (%u): ",
      static_cast<unsigned long>(tag.detectedTrialCount),
      static_cast<unsigned long>(fastInventoryRequestedTrialCount),
      static_cast<unsigned>(countAntennasInMask(reliableMask)));
    printAntennaMask(reliableMask);
    Serial.print(" | wykryły przynajmniej raz: ");
    printAntennaMask(
      static_cast<uint16_t>(tag.anyAntennaMask & activeAntennaMask));
    Serial.printf(
      " | odczyty łącznie: %lu\n",
      static_cast<unsigned long>(tag.totalReadCount));
  }

  Serial.println(
    "[RFID][FAST] ======================================");
}

void printFastInventoryCsv() {
  // Format pozostaje zgodny z tools/plot_tag_coverage.py ze starego projektu.
  Serial.println("[RFID] TAG_CSV_BEGIN");
  Serial.print("epc,antenna_count");

  for (uint8_t port = 1;
       port <= ANTENNA_PORT_COUNT;
       ++port) {
    if ((activeAntennaMask & antennaBit(port)) != 0) {
      Serial.printf(",A%u", static_cast<unsigned>(port));
    }
  }

  Serial.println();

  for (size_t i = 0; i < fastInventoryTestTagCount; ++i) {
    const FastInventoryTestTag& tag = fastInventoryTestTags[i];
    const uint16_t reliableMask = reliableAntennaMaskForTag(tag);

    printEpc(tag.epc, tag.epcLength);
    Serial.printf(
      ",%u",
      static_cast<unsigned>(countAntennasInMask(reliableMask)));

    for (uint8_t port = 1;
         port <= ANTENNA_PORT_COUNT;
         ++port) {
      const uint16_t bit = antennaBit(port);

      if ((activeAntennaMask & bit) == 0) {
        continue;
      }

      Serial.printf(
        ",%u",
        static_cast<unsigned>((reliableMask & bit) != 0));
    }

    Serial.println();
  }

  Serial.println("[RFID] TAG_CSV_END");
}


#include "reader_configuration.h"

#endif
