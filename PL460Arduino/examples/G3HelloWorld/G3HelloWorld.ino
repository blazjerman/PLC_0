#include <PL460.h>
#include <PL460G3Coupling.h>
#include <PL460G3Phy.h>
#include <firmware/PLC_PHY_G3_CENA.h>

// Upload with 1 to the transmitting board and 0 to the receiving board.
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
pl460::G3Phy phy(modem);

const char message[] = "Hello world!";
uint32_t lastSend = 0;

void stopWithError(const char *stage) {
  Serial.printf("ERROR during %s: %s\n", stage, modem.lastErrorString());
  while (true) delay(1000);
}

const char *crcName(uint8_t crc) {
  switch (crc) {
    case 1: return "OK";
    case 0: return "BAD";
    case 0xFE: return "TIMEOUT";
    case 0xFF: return "DISABLED";
    default: return "UNKNOWN";
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

  Serial.println("PL460-EK rev5 G3 Hello World");
  Serial.printf("Role: %s\n", ROLE_SENDER ? "SENDER" : "RECEIVER");

  if (!modem.begin()) stopWithError("transport start");
  if (!modem.boot(pl460::PLC_PHY_G3_CENA_IMAGE, 5000))
    stopWithError("G3 CENELEC-A firmware boot");
  if (!pl460::configureG3CenelecARev5(modem))
    stopWithError("rev5 coupling configuration");
  if (!phy.setImpedance(pl460::G3Impedance::VeryLow, false))
    stopWithError("VLO impedance configuration");
  if (!phy.enableCrc(true)) stopWithError("CRC configuration");

  modem.enableTransmitter(ROLE_SENDER != 0);
  Serial.printf("Ready. TX_EN=%d NTHW0=%d\n",
                digitalRead(PIN_TX_EN), digitalRead(PIN_NTHW0));
}

void loop() {
  if (!phy.poll()) {
    Serial.printf("Poll failed: %s\n", modem.lastErrorString());
    delay(100);
    return;
  }

#if ROLE_SENDER
  pl460::G3TxConfirm confirm;
  if (phy.takeTxConfirm(confirm)) {
    Serial.printf("TX confirmation: result=%u RMS=%lu\n",
                  confirm.result, (unsigned long)confirm.rms);
  }

  if (!phy.busy() && millis() - lastSend >= 2000) {
    lastSend = millis();
    if (phy.send(reinterpret_cast<const uint8_t *>(message), sizeof(message))) {
      Serial.printf("Sent: %s\n", message);
    } else {
      Serial.printf("Send failed: %s\n", modem.lastErrorString());
    }
  }
#else
  if (phy.available()) {
    uint8_t data[513];
    pl460::G3RxInfo info;
    const uint16_t length = phy.receive(data, sizeof(data) - 1, &info);
    data[length] = 0;
    Serial.printf("Received: %s | bytes=%u RSSI=%u dBuV LQI=%u CRC=%s\n",
                  reinterpret_cast<char *>(data), length, info.rssi, info.lqi,
                  crcName(info.crcOk));
  }
#endif

  delay(2);
}
