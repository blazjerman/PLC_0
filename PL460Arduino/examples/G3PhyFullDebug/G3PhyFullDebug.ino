#include <PL460.h>
#include <PL460G3Phy.h>
#include <PL460G3Coupling.h>
#include <firmware/PLC_PHY_G3_CENA.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <Esp.h>
#endif

// Use 1 for the single-board forced transmitter test. A receiver is not
// required to obtain a local TX confirmation or see a waveform at the TX test
// points. Change to 0 when flashing the second board as a receiver.
#define DEBUG_SENDER 1

#define PIN_CS      10
#define PIN_MOSI    11
#define PIN_MISO    13
#define PIN_SCK     12
#define PIN_IRQ      2
#define PIN_RESET    3
#define PIN_LDO_EN   4
#define PIN_STBY     5
#define PIN_TX_EN    6
#define PIN_NTHW0    7

pl460::Pins controlPins(PIN_CS, PIN_RESET, PIN_IRQ, PIN_TX_EN, PIN_NTHW0);
pl460::SpiPins busPins(PIN_SCK, PIN_MISO, PIN_MOSI);
pl460::ArduinoTransport transport(SPI, controlPins, busPins, 2000000UL);
pl460::PL460 modem(transport);
pl460::G3Phy phy(modem);

uint32_t sequenceNumber = 0;
uint32_t requestTime = 0;
uint32_t lastRequest = 0;
uint32_t lastHeartbeat = 0;
bool awaitingConfirmation = false;

const char *txResultName(uint8_t result) {
  switch (result) {
    case 0: return "PROCESS";
    case 1: return "SUCCESS";
    case 2: return "INVALID_LENGTH";
    case 3: return "BUSY_CHANNEL";
    case 4: return "BUSY_TX";
    case 5: return "BUSY_RX";
    case 6: return "INVALID_SCHEME";
    case 7: return "TIMEOUT";
    case 8: return "INVALID_TONEMAP";
    case 9: return "INVALID_MODULATION";
    case 10: return "INVALID_DELIMITER";
    case 11: return "CANCELLED";
    case 12: return "HIGH_TEMPERATURE_120C";
    case 13: return "HIGH_TEMPERATURE_WARNING_110C";
    case 255: return "NO_TX";
    default: return "UNKNOWN";
  }
}

void printPin(const char *name, int pin) {
  Serial.printf("  %-9s GPIO%-2d = %s\n", name, pin,
                digitalRead(pin) ? "HIGH" : "LOW");
}

void printPins() {
  Serial.println("Control-pin levels:");
  printPin("CS", PIN_CS);
  printPin("RESET", PIN_RESET);
  printPin("IRQ", PIN_IRQ);
  printPin("LDO_EN", PIN_LDO_EN);
  printPin("STBY", PIN_STBY);
  printPin("TX_EN", PIN_TX_EN);
  printPin("NTHW0", PIN_NTHW0);
  Serial.println("Expected normal: CS=HIGH RESET=HIGH LDO_EN=HIGH STBY=LOW NTHW0=HIGH");
}

void fatal(const char *stage) {
  Serial.printf("\nFATAL at %s: %s\n", stage, modem.lastErrorString());
  printPins();
  Serial.println("Check SPI with CS active-low, mode 0, and 3.3 V logic.");
  while (true) delay(1000);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n===============================================");
  Serial.println(" PL460-EK rev5 G3 CENELEC-A FULL DEBUG");
  Serial.println("===============================================");

#if defined(ARDUINO_ARCH_ESP32)
  Serial.printf("ESP chip: %s, revision %d, CPU %u MHz\n",
                ESP.getChipModel(), ESP.getChipRevision(), ESP.getCpuFreqMHz());
#if defined(CONFIG_IDF_TARGET_ESP32)
  Serial.println("WARNING: original ESP32 normally reserves GPIO6..GPIO11 for flash.");
  Serial.println("This pin mapping is usually INVALID on ESP32-WROOM/ESP32 DevKit.");
#endif
#endif

  Serial.println("SPI: SCK=12 MISO=13 MOSI=11 CS=10, 2 MHz, mode 0");
  Serial.println("PLC: IRQ=2 RESET=3 LDO_EN=4 STBY=5 TX_EN=6 NTHW0=7");

  pinMode(PIN_LDO_EN, OUTPUT);
  digitalWrite(PIN_LDO_EN, HIGH);
  pinMode(PIN_STBY, OUTPUT);
  digitalWrite(PIN_STBY, LOW);
  delay(20);

  Serial.println("[1/6] Starting transport...");
  if (!modem.begin()) fatal("transport begin");
  printPins();

  Serial.printf("[2/6] Uploading %s (%lu bytes)...\n",
                pl460::PLC_PHY_G3_CENA_IMAGE.name,
                (unsigned long)pl460::PLC_PHY_G3_CENA_IMAGE.size);
  uint32_t bootStarted = millis();
  if (!modem.boot(pl460::PLC_PHY_G3_CENA_IMAGE, 5000)) fatal("firmware boot");
  Serial.printf("PASS: boot in %lu ms, mailbox key=0x%04X (expected 0x1122)\n",
                (unsigned long)(millis() - bootStarted), modem.runtimeKey());

  Serial.println("[3/6] Repeating mailbox communication test 10 times...");
  for (uint8_t i = 0; i < 10; ++i) {
    bool ok = modem.communicationTest(pl460::PL460::kPhyRuntimeKey);
    Serial.printf("  mailbox %u: %s%s%s\n", i + 1, ok ? "PASS" : "FAIL",
                  ok ? "" : " - ", ok ? "" : modem.lastErrorString());
    delay(20);
  }

  Serial.println("[4/6] Reading firmware information...");
  pl460::BoardInfo info;
  if (modem.getBoardInfo(info)) {
    Serial.printf("  product=0x%04X model=0x%04X version=%s number=0x%08lX band=%u\n",
                  info.productId, info.model, info.version,
                  (unsigned long)info.versionNumber, info.band);
  } else {
    Serial.printf("  INFO READ FAILED (non-fatal): %s\n", modem.lastErrorString());
  }

  Serial.println("[5/6] Applying PL460-EK rev5 CENELEC-A coupling calibration...");
  if (!pl460::configureG3CenelecARev5(modem))
    fatal("rev5 CENELEC-A coupling configuration");
  Serial.println("PASS: auxiliary branch selected and calibration tables loaded");
  if (!phy.setImpedance(pl460::G3Impedance::VeryLow, false))
    fatal("force very-low TX impedance");
  if (!phy.enableCrc(true)) fatal("enable PHY CRC");
  Serial.println("PASS: VLO transmit mode forced and PHY CRC enabled");

  Serial.println("[6/6] Enabling the PL460 transmit amplifier...");
  modem.enableTransmitter(true);
  printPins();
  if (digitalRead(PIN_NTHW0) == LOW)
    Serial.println("BLOCKER: NTHW0 is LOW: thermal warning blocks TX.");
  if (digitalRead(PIN_TX_EN) != HIGH)
    Serial.println("BLOCKER: TX_EN did not go HIGH. Firmware/safety check rejected TX.");
  Serial.printf("modem.canTransmit() = %s\n", modem.canTransmit() ? "YES" : "NO");

  Serial.println();
  Serial.println("Hardware checklist:");
  Serial.println("  * PL460-EK J4 must have 15 V (+/-5%), capable of 12 W.");
  Serial.println("  * PL460 J2 pin20 must have 3.3 V and grounds must be common.");
  Serial.println("  * Probe a TX test point with proper isolated equipment.");
  Serial.println("  * A receiver is NOT needed for this forced local TX test.");
  Serial.printf("Starting in %s mode...\n\n", DEBUG_SENDER ? "SENDER" : "RECEIVER");
}

void loop() {
  if (!phy.poll()) {
    Serial.printf("POLL FAIL: %s IRQ=%d\n", modem.lastErrorString(), digitalRead(PIN_IRQ));
    delay(100);
    return;
  }

  pl460::G3TxConfirm confirm;
  if (phy.takeTxConfirm(confirm)) {
    awaitingConfirmation = false;
    Serial.printf("TX CONFIRM after %lu ms: result=%u (%s), RMS=%lu, end=%lu\n",
                  (unsigned long)(millis() - requestTime), confirm.result,
                  txResultName(confirm.result), (unsigned long)confirm.rms,
                  (unsigned long)confirm.endTime);
    if (confirm.result != 1)
      Serial.println("  Transmission was NOT completed successfully; use the result name above.");
  }

  if (phy.available()) {
    uint8_t data[513];
    pl460::G3RxInfo info;
    uint16_t length = phy.receive(data, sizeof(data) - 1, &info);
    data[length] = 0;
    const char *crc = info.crcOk == 1 ? "OK" :
                      info.crcOk == 0 ? "BAD" :
                      info.crcOk == 0xFE ? "TIMEOUT" :
                      info.crcOk == 0xFF ? "DISABLED" : "UNKNOWN";
    Serial.printf("RX: len=%u (may include padding) RSSI=%u dBuV LQI=%u CRC=%s SNRpay=%d data='%s'\n",
                  length, info.rssi, info.lqi, crc, info.snrPayload, data);
  }

#if DEBUG_SENDER
  if (!awaitingConfirmation && !phy.busy() && millis() - lastRequest >= 3000) {
    lastRequest = millis();
    char message[64];
    snprintf(message, sizeof(message), "G3 DEBUG SEQ=%lu", (unsigned long)sequenceNumber++);

    pl460::G3TxConfig config = pl460::G3TxConfig::cenelecARobust();
    config.mode = 0x03;        // bit0 forced + bit1 relative time
    config.startTime = 0;      // transmit as soon as firmware accepts request
    config.attenuation = 0;    // maximum configured transmit level
    config.toneMap[0] = 0x3F;  // all six CENELEC-A sub-bands

    Serial.printf("TX REQUEST: '%s', len=%u mode=0x%02X tone=0x%02X TX_EN=%d NTHW0=%d\n",
                  message, (unsigned)(strlen(message) + 1), config.mode,
                  config.toneMap[0], digitalRead(PIN_TX_EN), digitalRead(PIN_NTHW0));
    if (phy.send((const uint8_t *)message, strlen(message) + 1, config)) {
      requestTime = millis();
      awaitingConfirmation = true;
      Serial.println("  mailbox request accepted; waiting for TX confirmation...");
    } else {
      Serial.printf("  SEND REJECTED: %s, canTransmit=%d busy=%d\n",
                    modem.lastErrorString(), modem.canTransmit(), phy.busy());
    }
  }
  if (awaitingConfirmation && millis() - requestTime > 5000) {
    Serial.println("TX CONFIRM TIMEOUT: IRQ/mailbox event did not arrive within 5 seconds.");
    Serial.printf("  IRQ=%d TX_EN=%d NTHW0=%d lastError=%s\n",
                  digitalRead(PIN_IRQ), digitalRead(PIN_TX_EN),
                  digitalRead(PIN_NTHW0), modem.lastErrorString());
    awaitingConfirmation = false;
  }
#endif

  if (millis() - lastHeartbeat >= 1000) {
    lastHeartbeat = millis();
    Serial.printf("HEARTBEAT: IRQ=%d TX_EN=%d NTHW0=%d busy=%d awaiting=%d\n",
                  digitalRead(PIN_IRQ), digitalRead(PIN_TX_EN),
                  digitalRead(PIN_NTHW0), phy.busy(), awaitingConfirmation);
  }
  delay(2);
}
