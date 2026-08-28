#include <PL460.h>
#include <PL460Prime.h>
#include <firmware/PLC_PHY_PRIME.h>

#define ROLE_SENDER 1

pl460::Pins pins(10, 3, 2, 6, 7);
pl460::SpiPins spiPins(12, 13, 11);
pl460::ArduinoTransport transport(SPI, pins, spiPins);
pl460::PL460 modem(transport);
pl460::PrimePhy prime(modem);
uint32_t sequenceNumber = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(4, OUTPUT); digitalWrite(4, HIGH);  // LDO_EN
  pinMode(5, OUTPUT); digitalWrite(5, LOW);   // STBY: normal operation
  delay(10);
  if (!modem.begin() || !modem.boot(pl460::PLC_PHY_PRIME_IMAGE)) {
    Serial.printf("Boot failed: %s\n", modem.lastErrorString());
    while (true) delay(1000);
  }
  modem.enableTransmitter(true);
  Serial.printf("PRIME PHY P2P %s ready\n", ROLE_SENDER ? "sender" : "receiver");
}

void loop() {
  prime.poll();
#if ROLE_SENDER
  static uint32_t lastSend = 0;
  if (!prime.busy() && millis() - lastSend > 2000) {
    lastSend = millis();
    char message[64];
    snprintf(message, sizeof(message), "PL460 PRIME packet %lu", (unsigned long)sequenceNumber++);
    if (prime.send((uint8_t *)message, strlen(message) + 1)) Serial.printf("TX: %s\n", message);
  }
  uint8_t result, buffer;
  if (prime.takeTxResult(result, buffer)) Serial.printf("TX result=%u buffer=%u\n", result, buffer);
#else
  if (prime.available()) {
    uint8_t data[513];
    pl460::PrimeRxInfo info;
    uint16_t length = prime.receive(data, sizeof(data) - 1, &info);
    data[length] = 0;
    Serial.printf("RX len=%u RSSI=%u CINR=%u: %s\n", length, info.rssi, info.cinrAverage, data);
  }
#endif
}
