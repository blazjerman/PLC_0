/*  G3 MAC — minimal software ACK test
 *
 *  Board 1 (ROLE_SENDER=1): sends frame every 3s, waits for ACK
 *  Board 2 (ROLE_SENDER=0): receives frame, prints it, sends ACK back
 *
 *  Both boards have TX enabled.
 *  No state machine, no CSMA — just phy.send + phy.receive.
 */

#include <PL460.h>
#include <PL460G3Phy.h>
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

void stop(const char *msg) {
  Serial.printf("FAIL: %s\n", msg);
  while (true) delay(1000);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_LDO_EN, OUTPUT);
  digitalWrite(PIN_LDO_EN, HIGH);
  pinMode(PIN_STBY, OUTPUT);
  digitalWrite(PIN_STBY, LOW);
  delay(20);

  Serial.println("G3 ACK test");

  if (!modem.begin()) stop("modem");
  if (!modem.boot(pl460::PLC_PHY_G3_CENA_IMAGE, 5000)) stop("boot");
  if (!pl460::configureG3CenelecARev5(modem)) stop("coupling");
  if (!phy.setImpedance(pl460::G3Impedance::VeryLow, false)) stop("impedance");
  if (!phy.enableCrc(true)) stop("crc");

  // Both boards must TX for ACK to work
  modem.enableTransmitter(true);
  Serial.printf("Ready role=%s\n", ROLE_SENDER ? "SENDER" : "RECEIVER");
}

uint32_t lastSend = 0;
uint32_t sentCount = 0;
uint32_t ackCount = 0;

void loop() {
  // Call PHY poll every iteration
  if (!phy.poll()) {
    Serial.printf("poll fail\n");
    delay(10);
    return;
  }

  // Check TX confirm
  pl460::G3TxConfirm cfm;
  if (phy.takeTxConfirm(cfm)) {
    Serial.printf("TX done result=%u rms=%lu\n", cfm.result, (unsigned long)cfm.rms);
  }

  // Check received data
  if (phy.available()) {
    uint8_t data[128];
    pl460::G3RxInfo info;
    uint16_t len = phy.receive(data, sizeof(data) - 1, &info);
    data[len] = 0;
    Serial.printf("RX len=%u crc=%u rssi=%u: %s\n", len, info.crcOk, info.rssi, data);

    if (info.crcOk && len > 0) {
      // Send ACK: just the byte 0x06 (ACK) back to sender
      uint8_t ack = 0x06;
      phy.send(&ack, 1);
      Serial.println("-> ACK sent");
    }
  }

#if ROLE_SENDER
  // Send every 3 seconds
  if (!phy.busy() && millis() - lastSend >= 3000) {
    lastSend = millis();
    sentCount++;
    const char msg[] = "Hello";
    if (phy.send((const uint8_t*)msg, sizeof(msg))) {
      Serial.printf("SEND #%u\n", sentCount);
    } else {
      Serial.printf("SEND FAIL: %s\n", modem.lastErrorString());
    }
  }
#endif

  delay(2);
}
