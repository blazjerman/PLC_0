/*  G3 MAC RT — Full protocol example (simple init)
 *
 *  Minimal setup using G3Mac::begin(image, addr, panId, isCoordinator).
 *
 *  Boots PL460 with G3 MAC RT firmware → configures coupling →
 *  sets address and PAN → enables transmitter — in a single call.
 *
 *  Wiring (PL460-EK rev5):
 *    CS=10, MOSI=11, MISO=13, SCK=12, IRQ=2, RST=3
 *    LDO_EN=4, STBY=5, TX_EN=6, NTHW0=7
 *
 *  Upload with ROLE_SENDER=1 to coordinator, 0 to end device.
 */

#include <PL460G3Mac.h>
#include <firmware/G3_MAC_RT_CENA.h>

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

const uint16_t kCoordinatorAddr = 0x0001;
const uint16_t kDeviceAddr = 0x0002;
const uint16_t kPanId = 0x0ABC;

pl460::Pins controlPins(PIN_CS, PIN_RESET, PIN_IRQ, PIN_TX_EN, PIN_NTHW0);
pl460::SpiPins spiPins(PIN_SCK, PIN_MISO, PIN_MOSI);
pl460::ArduinoTransport transport(SPI, controlPins, spiPins, 2000000UL);
pl460::PL460 modem(transport);
pl460::G3Mac mac(modem);

static const char message[] = "Hello from G3 MAC RT!";
uint32_t lastSend = 0;

const char *txStatusName(pl460::MacTxStatus s) {
  switch (s) {
    case pl460::MacTxStatus::Success:            return "SUCCESS";
    case pl460::MacTxStatus::ChannelAccessFailure: return "CHANNEL";
    case pl460::MacTxStatus::NoAck:              return "NO_ACK";
    default:                                     return "?";
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_LDO_EN, OUTPUT);
  digitalWrite(PIN_LDO_EN, HIGH);
  pinMode(PIN_STBY, OUTPUT);
  digitalWrite(PIN_STBY, LOW);
  delay(20);

  Serial.println("PL460 G3 MAC RT");

  // Single init call — boots firmware, configures coupling, sets address/PAN
  const uint16_t myAddr = ROLE_SENDER ? kCoordinatorAddr : kDeviceAddr;
  if (!mac.begin(pl460::G3_MAC_RT_CENA_IMAGE, myAddr, kPanId, ROLE_SENDER)) {
    Serial.printf("Init failed: %s\n", modem.lastErrorString());
    while (true) delay(1000);
  }

  Serial.printf("Ready. Addr=0x%04X PAN=0x%04X\n", myAddr, kPanId);
}

void loop() {
  if (!mac.poll()) {
    Serial.printf("Poll: %s\n", modem.lastErrorString());
    delay(100);
    return;
  }

  if (mac.transmissionComplete()) {
    auto cfm = mac.takeTxConfirm();
    Serial.printf("TX %s RMS=%lu\n",
                  txStatusName(cfm.status), (unsigned long)cfm.rms);
  }

#if ROLE_SENDER
  if (!mac.busy() && millis() - lastSend >= 2000) {
    lastSend = millis();
    mac.send(kDeviceAddr,
             reinterpret_cast<const uint8_t *>(message), sizeof(message));
  }
#else
  if (mac.available()) {
    uint8_t buf[256];
    pl460::MacRxInfo info;
    uint16_t len = mac.receive(buf, sizeof(buf) - 1, &info);
    buf[len] = 0;
    Serial.printf("RX from 0x%04X: %s\n", info.srcAddr, buf);
    mac.send(info.srcAddr, buf, len);
  }
#endif
}
