/*  G3 MAC — Software MAC over G3 Phy
 *
 *  Lightweight MAC layer providing:
 *    - 16-bit addressing (src/dst)
 *    - CSMA/CA (listen before talk + random backoff)
 *    - ACK/NACK with retransmission (up to 3 retries)
 *    - Frame sequencing (duplicate detection)
 *    - Automatic ACK response
 *
 *  Uses the existing G3Phy (which handles CRC, modulation, hardware TX/RX)
 *  and adds protocols in software on the Arduino host.
 *
 *  Wiring (PL460-EK rev5):
 *    CS=10, MOSI=11, MISO=13, SCK=12, IRQ=2, RST=3
 *    LDO_EN=4, STBY=5, TX_EN=6, NTHW0=7
 *
 *  Upload with ROLE_SENDER=1 to coordinator, 0 to end device.
 */

#include <PL460.h>
#include <PL460G3Phy.h>
#include <PL460G3Coupling.h>
#include <PL460G3Mac.h>
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

const uint16_t kCoordinatorAddr = 0x0001;
const uint16_t kDeviceAddr = 0x10F2;
const uint16_t kPanId = 0x0ABC;

// 1. Transport
pl460::Pins controlPins(PIN_CS, PIN_RESET, PIN_IRQ, PIN_TX_EN, PIN_NTHW0);
pl460::SpiPins spiPins(PIN_SCK, PIN_MISO, PIN_MOSI);
pl460::ArduinoTransport transport(SPI, controlPins, spiPins, 2000000UL);

// 2. PL460 device
pl460::PL460 modem(transport);

// 3. G3 PHY (handles CRC, modulation, raw frame send/receive)
pl460::G3Phy phy(modem);

// 4. G3 MAC (handles addressing, CSMA/CA, ACK, retransmission)
pl460::G3Mac mac(phy);

static const char message[] = "Hello from G3 MAC!";
uint32_t lastSend = 0;
uint32_t sendCount = 0;
uint32_t ackOk = 0;
uint32_t ackFail = 0;

const char *txStatusName(pl460::MacTxStatus s) {
  switch (s) {
    case pl460::MacTxStatus::Success:              return "OK";
    case pl460::MacTxStatus::ChannelAccessFailure: return "BUSY";
    case pl460::MacTxStatus::NoAck:                return "NO_ACK";
    default:                                       return "?";
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

  Serial.println("PL460 G3 MAC (software MAC over PHY)");

  // Init PL460 + PHY firmware
  if (!modem.begin())
    { Serial.printf("modem: %s\\n", modem.lastErrorString()); while (true); }
  if (!modem.boot(pl460::PLC_PHY_G3_CENA_IMAGE, 5000))
    { Serial.printf("boot: %s\\n", modem.lastErrorString()); while (true); }
  if (!pl460::configureG3CenelecARev5(modem))
    { Serial.printf("coupling: %s\\n", modem.lastErrorString()); while (true); }

  // Configure MAC
  const uint16_t myAddr = ROLE_SENDER ? kCoordinatorAddr : kDeviceAddr;
  mac.setShortAddress(myAddr);
  mac.setPanId(kPanId);
  mac.begin();

  modem.enableTransmitter(ROLE_SENDER != 0);
  Serial.printf("Ready. Addr=0x%04X PAN=0x%04X\\n", myAddr, kPanId);
}

void loop() {
  // MAC state machine must run every iteration
  mac.poll();

  // Handle TX confirmations
  if (mac.transmissionComplete()) {
    pl460::MacTxConfirm cfm = mac.takeTxConfirm();
    if (cfm.status == pl460::MacTxStatus::Success) ++ackOk;
    else ++ackFail;
    Serial.printf("TX %s (retries=%u) OK=%u FAIL=%u\\n",
                  txStatusName(cfm.status), cfm.retries,
                  ackOk, ackFail);
  }

  // Handle received data
  if (mac.available()) {
    uint8_t buf[256];
    pl460::MacRxInfo info;
    uint16_t len = mac.receive(buf, sizeof(buf) - 1, &info);
    buf[len] = 0;
    Serial.printf("RX from 0x%04X: %s\\n", info.srcAddr, buf);

    // Echo back
    if (!mac.busy())
      mac.send(info.srcAddr, buf, len);
  }

#if ROLE_SENDER
  // Send every 3 seconds
  if (!mac.busy() && millis() - lastSend >= 3000) {
    lastSend = millis();
    ++sendCount;
    if (mac.send(kDeviceAddr,
                 reinterpret_cast<const uint8_t *>(message),
                 sizeof(message))) {
      Serial.printf("Send #%u -> 0x%04X\\n", sendCount, kDeviceAddr);
    }
  }
#endif
}
