#include <PL460.h>
#include <PL460G3Phy.h>
#include <PL460G3Coupling.h>
#include <firmware/PLC_PHY_G3_CENA.h>

// Flash ROLE_SENDER=1 onto one ESP32 and ROLE_SENDER=0 onto the other.
#define ROLE_SENDER 1

pl460::Pins pins(10, 3, 2, 6, 7);
pl460::SpiPins spiPins(12, 13, 11);
pl460::ArduinoTransport transport(SPI, pins, spiPins);
pl460::PL460 modem(transport);
pl460::G3Phy phy(modem);
uint32_t sequenceNumber = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(4, OUTPUT); digitalWrite(4, HIGH);  // LDO_EN
  pinMode(5, OUTPUT); digitalWrite(5, LOW);   // STBY: normal operation
  delay(10);
  if (!modem.begin() || !modem.boot(pl460::PLC_PHY_G3_CENA_IMAGE)) {
    Serial.printf("Boot failed: %s\n", modem.lastErrorString());
    while (true) delay(1000);
  }
  if (!pl460::configureG3CenelecARev5(modem)) {
    Serial.printf("Coupling configuration failed: %s\n", modem.lastErrorString());
    while (true) delay(1000);
  }
  if (!phy.setImpedance(pl460::G3Impedance::VeryLow, false) ||
      !phy.enableCrc(true)) {
    Serial.printf("G3 PHY configuration failed: %s\n", modem.lastErrorString());
    while (true) delay(1000);
  }
  // The receiver does not need the external transmit amplifier enabled.
  modem.enableTransmitter(ROLE_SENDER != 0);
  Serial.printf("G3 CENELEC-A P2P %s ready\n", ROLE_SENDER ? "sender" : "receiver");
  Serial.printf("ROLE_SENDER=%d TX_EN=%d\n", ROLE_SENDER, digitalRead(6));
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
    const char *crc = info.crcOk == 1 ? "OK" :
                      info.crcOk == 0 ? "BAD" :
                      info.crcOk == 0xFE ? "TIMEOUT" :
                      info.crcOk == 0xFF ? "DISABLED" : "UNKNOWN";
    Serial.printf("RX len=%u (may include padding) RSSI=%u LQI=%u CRC=%s: %s\n",
                  length, info.rssi, info.lqi, crc, data);
  }
#endif
}
