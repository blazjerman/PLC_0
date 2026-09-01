/*  G3 MAC — Reliable P2P with software ACK and automatic TX_EN management
 *
 *  G3Mac manages TX_EN internally (toggle before/after each send).
 *  This means:
 *    - Sender: PA on during TX, off during RX (can hear ACK)
 *    - Receiver: PA off during RX (LNA connected), on briefly for ACK
 *
 *  Flash first board with ROLE_SENDER=1, second with ROLE_SENDER=0.
 */

#include <PL460.h>
#include <PL460G3Phy.h>
#include <PL460G3Mac.h>
#include <PL460G3Coupling.h>
#include <firmware/PLC_PHY_G3_CENA.h>

#define ROLE_SENDER 1

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

pl460::Pins pins(PIN_CS, PIN_RESET, PIN_IRQ, PIN_TX_EN, PIN_NTHW0);
pl460::SpiPins spi(PIN_SCK, PIN_MISO, PIN_MOSI);
pl460::ArduinoTransport transport(SPI, pins, spi, 2000000UL);
pl460::PL460 modem(transport);
pl460::G3Phy phy(modem);
pl460::G3Mac mac(phy);

// Addressing
const uint16_t myAddr = ROLE_SENDER ? 0x0001 : 0x10F2;
const uint16_t peerAddr = ROLE_SENDER ? 0x10F2 : 0x0001;

void stop(const char *msg) {
  Serial.printf("FAIL: %s\n", msg);
  while (true) delay(1000);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_LDO_EN, OUTPUT);  digitalWrite(PIN_LDO_EN, HIGH);
  pinMode(PIN_STBY, OUTPUT);    digitalWrite(PIN_STBY, LOW);
  pinMode(PIN_TX_EN, OUTPUT);   digitalWrite(PIN_TX_EN, LOW);
  delay(20);

  Serial.println("G3MAC P2P test");

  if (!modem.begin()) stop("modem");
  if (!modem.boot(pl460::PLC_PHY_G3_CENA_IMAGE, 5000)) stop("boot");
  if (!pl460::configureG3CenelecARev5(modem)) stop("coupling");
  if (!phy.setImpedance(pl460::G3Impedance::VeryLow, false)) stop("impedance");
  if (!phy.enableCrc(true)) stop("crc");

  // MAC config
  mac.setShortAddress(myAddr);
  mac.setPanId(0x0ABC);
  mac.setManageTxEn(true);  // G3Mac toggles TX_EN before/after every send
  mac.setAckTimeoutMs(1000);
  mac.setMaxRetries(3);
  mac.begin();

  Serial.printf("Ready addr=0x%04X role=%s\n", myAddr,
                ROLE_SENDER ? "SENDER" : "RECEIVER");
}

uint32_t lastSend = 0;
uint32_t sentCount = 0;

void loop() {
  mac.poll();

#if ROLE_SENDER
  // --- Sender ---
  // Read TX confirm FIRST (before send() clears it)
  pl460::MacTxConfirm cfm = mac.takeTxConfirm();
  if (cfm.status != pl460::MacTxStatus::Invalid) {
    if (cfm.status == pl460::MacTxStatus::Success) {
      Serial.printf("ACK OK retries=%u\n", cfm.retries);
    } else {
      Serial.printf("ACK FAIL status=%u retries=%u\n",
                    (uint8_t)cfm.status, cfm.retries);
    }
  }

  if (!mac.busy() && millis() - lastSend >= 3000) {
    lastSend = millis();
    sentCount++;
    const char msg[] = "Hello";
    if (mac.send(peerAddr, (uint8_t*)msg, sizeof(msg))) {
      Serial.printf("Send #%u\n", sentCount);
    } else {
      Serial.printf("Send FAIL (busy?)\n");
    }
  }

#else
  // --- Receiver ---
  if (mac.available()) {
    uint8_t data[128];
    pl460::MacRxInfo info;
    uint16_t len = mac.receive(data, sizeof(data) - 1, &info);
    data[len] = 0;
    Serial.printf("RX from 0x%04X len=%u: %s\n", info.srcAddr, len, data);
  }
#endif

  delay(5);
}
