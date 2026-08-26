#include <PL460.h>
#include <PL460G3Phy.h>
#include <firmware/PLC_PHY_G3_CENA.h>

// Flash ROLE_SENDER=1 onto one ESP32 and ROLE_SENDER=0 onto the other.
#define ROLE_SENDER 1

pl460::Pins pins(5, 4, 27, 26, 25);
pl460::ArduinoTransport transport(SPI, pins);
pl460::PL460 modem(transport);
pl460::G3Phy phy(modem);
uint32_t sequenceNumber = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  if (!modem.begin() || !modem.boot(pl460::PLC_PHY_G3_CENA_IMAGE)) {
    Serial.printf("Boot failed: %s\n", modem.lastErrorString());
    while (true) delay(1000);
  }
  modem.enableTransmitter(true);
  Serial.printf("G3 CENELEC-A P2P %s ready\n", ROLE_SENDER ? "sender" : "receiver");
}

void loop() {
  phy.poll();

#if ROLE_SENDER
  static uint32_t lastSend = 0;
  if (!phy.busy() && millis() - lastSend >= 2000) {
    lastSend = millis();
    char message[64];
    snprintf(message, sizeof(message), "PL460 G3 packet %lu", (unsigned long)sequenceNumber++);
    if (phy.send(reinterpret_cast<uint8_t *>(message), strlen(message) + 1))
      Serial.printf("TX: %s\n", message);
    else
      Serial.printf("TX blocked/failed: %s\n", modem.lastErrorString());
  }
  pl460::G3TxConfirm confirm;
  if (phy.takeTxConfirm(confirm))
    Serial.printf("TX confirm result=%u rms=%lu\n", confirm.result, (unsigned long)confirm.rms);
#else
  if (phy.available()) {
    uint8_t data[513];
    pl460::G3RxInfo info;
    uint16_t length = phy.receive(data, sizeof(data) - 1, &info);
    data[length] = 0;
    Serial.printf("RX len=%u RSSI=%u LQI=%u CRC=%u: %s\n",
                  length, info.rssi, info.lqi, info.crcOk, data);
  }
#endif
}

