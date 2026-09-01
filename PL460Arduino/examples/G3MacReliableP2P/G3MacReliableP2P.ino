/*  G3 MAC RT — Full protocol example
 *
 *  Boots the PL460 with G3 MAC RT firmware that provides:
 *    - CSMA/CA channel access
 *    - ACK/NACK with retransmission
 *    - Short (16-bit) addressing
 *    - Automatic frame sequencing
 *    - PAN-based network isolation
 *
 *  Wiring (PL460-EK rev5):
 *    CS=10, MOSI=11, MISO=13, SCK=12, IRQ=2, RST=3
 *    LDO_EN=4, STBY=5, TX_EN=6, NTHW0=7
 *
 *  Upload with ROLE_SENDER=1 to the coordinator (one board)
 *  and ROLE_SENDER=0 to end devices (any number).
 */

#include <PL460.h>
#include <PL460G3Coupling.h>
#include <PL460G3Mac.h>
#include <firmware/G3_MAC_RT_CENA.h>

// Set to 1 on the coordinator, 0 on end devices
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

pl460::Pins controlPins(PIN_CS, PIN_RESET, PIN_IRQ, PIN_TX_EN, PIN_NTHW0);
pl460::SpiPins spiPins(PIN_SCK, PIN_MISO, PIN_MOSI);
pl460::ArduinoTransport transport(SPI, controlPins, spiPins, 2000000UL);
pl460::PL460 modem(transport);
pl460::G3Mac mac(modem);

const uint16_t kCoordinatorAddr = 0x0001;
const uint16_t kDeviceAddr = 0x0002;
const uint16_t kPanId = 0x0ABC;

const char message[] = "Hello from G3 MAC RT!";
uint32_t lastSend = 0;
uint32_t sendCount = 0;
uint32_t ackCount = 0;
uint32_t nackCount = 0;

void stopWithError(const char *stage) {
  Serial.printf("ERROR during %s: %s\n", stage, modem.lastErrorString());
  while (true) delay(1000);
}

const char *txStatusName(pl460::MacTxStatus status) {
  switch (status) {
    case pl460::MacTxStatus::Success:            return "SUCCESS";
    case pl460::MacTxStatus::ChannelAccessFailure: return "CHANNEL_ACCESS_FAIL";
    case pl460::MacTxStatus::NoAck:              return "NO_ACK";
    default:                                     return "UNKNOWN";
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

  Serial.println("PL460-EK rev5 G3 MAC RT");
  Serial.printf("Role: %s\n", ROLE_SENDER ? "COORDINATOR" : "END_DEVICE");

  // 1. Start SPI transport
  if (!modem.begin()) stopWithError("transport start");

  // 2. Boot G3 MAC RT firmware (MUST use G3_MAC_RT variant!)
  if (!modem.boot(pl460::G3_MAC_RT_CENA_IMAGE, 5000))
    stopWithError("MAC RT firmware boot");

  // 3. Coupling configuration (same as PHY mode)
  if (!pl460::configureG3CenelecARev5(modem))
    stopWithError("coupling configuration");

  // 4. Configure MAC RT addressing
  const uint16_t myAddr = ROLE_SENDER ? kCoordinatorAddr : kDeviceAddr;
  if (!mac.setShortAddress(myAddr))
    stopWithError("short address");
  if (!mac.setPanId(kPanId))
    stopWithError("PAN ID");

  // 5. Set coordinator bit on the sending node
  if (!mac.setCoordinator(ROLE_SENDER != 0))
    stopWithError("coordinator config");

  // 6. Initialize MAC layer
  if (!mac.begin())
    stopWithError("MAC begin");

  modem.enableTransmitter(true);
  Serial.printf("Ready. Short address=0x%04X PAN=0x%04X Coord=%d\n",
                myAddr, kPanId, ROLE_SENDER);
}

void loop() {
  // Must call poll() every iteration — it processes TX confirmations
  // and received data from the MAC RT firmware.
  if (!mac.poll()) {
    Serial.printf("Poll failed: %s\n", modem.lastErrorString());
    delay(100);
    return;
  }

  // Handle TX confirmation
  if (mac.transmissionComplete()) {
    pl460::MacTxConfirm confirm = mac.takeTxConfirm();
    if (confirm.status == pl460::MacTxStatus::Success) {
      ++ackCount;
    } else {
      ++nackCount;
    }
    Serial.printf("TX confirm: %s RMS=%lu\n",
                  txStatusName(confirm.status), (unsigned long)confirm.rms);
  }

#if ROLE_SENDER
  // Coordiantor: send a message periodically
  if (!mac.busy() && millis() - lastSend >= 2000) {
    lastSend = millis();
    ++sendCount;

    // Send to device address with MAC layer ACK request
    // The MAC RT firmware handles:
    //   - CSMA/CA (listen before talk + random backoff)
    //   - MAC frame construction (FC, seq, PAN, addresses)
    //   - ACK request + wait for response
    //   - Retransmission on timeout (up to firmware limit)
    if (mac.send(kDeviceAddr,
                 reinterpret_cast<const uint8_t *>(message),
                 sizeof(message))) {
      Serial.printf("Sent #%u: %s -> 0x%04X\n",
                    sendCount, message, kDeviceAddr);
    } else {
      Serial.printf("Send failed: %s\n", modem.lastErrorString());
    }
  }
#else
  // End device: listen and reply
  if (mac.available()) {
    uint8_t data[256];
    pl460::MacRxInfo info;
    const uint16_t len = mac.receive(data, sizeof(data) - 1, &info);
    data[len] = 0;

    Serial.printf("Received from 0x%04X: %s (%u bytes) "
                  "LQI=%u SNR=%d\n",
                  info.srcAddr,
                  reinterpret_cast<char *>(data), len,
                  info.lqi, info.snrPayload);

    // Echo back
    if (!mac.busy()) {
      mac.send(info.srcAddr, data, len);
      Serial.println("Echo sent");
    }
  }
#endif
}
