#include <PL460.h>
#include <PL460MetersAndMore.h>
#include <firmware/PLC_PHY_MM.h>

#define ROLE_SENDER 1

pl460::Pins pins(5, 4, 27, 26, 25);
pl460::ArduinoTransport transport(SPI, pins);
pl460::PL460 modem(transport);
pl460::MetersAndMorePhy mm(modem);
uint32_t sequenceNumber = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  if (!modem.begin() || !modem.boot(pl460::PLC_PHY_MM_IMAGE)) {
    Serial.printf("Boot failed: %s\n", modem.lastErrorString());
    while (true) delay(1000);
  }
  modem.enableTransmitter(true);
  Serial.printf("Meters & More PHY P2P %s ready\n", ROLE_SENDER ? "sender" : "receiver");
}

void loop() {
  mm.poll();
#if ROLE_SENDER
  static uint32_t lastSend = 0;
  if (!mm.busy() && millis() - lastSend > 2000) {
    lastSend = millis();
    char message[64];
    snprintf(message, sizeof(message), "PL460 M&M packet %lu", (unsigned long)sequenceNumber++);
    if (mm.send((uint8_t *)message, strlen(message) + 1)) Serial.printf("TX: %s\n", message);
  }
  uint8_t result;
  if (mm.takeTxResult(result)) Serial.printf("TX result=%u\n", result);
#else
  if (mm.available()) {
    uint8_t data[257];
    pl460::MetersAndMoreRxInfo info;
    uint16_t length = mm.receive(data, sizeof(data) - 1, &info);
    data[length] = 0;
    Serial.printf("RX len=%u RSSI=%u LQI=%u CRC=%u: %s\n",
                  length, info.rssi, info.lqi, info.crcOk, data);
  }
#endif
}

