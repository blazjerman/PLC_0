#include <PL460.h>
#include <firmware/PLC_PHY_G3_CENA.h>

// ESP32 VSPI defaults: SCK=18, MISO=19, MOSI=23.
// Change these control pins to match your wiring.
pl460::Pins pins(5, 4, 27, 26, 25);
pl460::ArduinoTransport transport(SPI, pins);
pl460::PL460 modem(transport);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("PL460-EK rev5 diagnostics");

  if (!modem.begin() || !modem.boot(pl460::PLC_PHY_G3_CENA_IMAGE)) {
    Serial.printf("FAIL: %s\n", modem.lastErrorString());
    return;
  }

  Serial.printf("Mailbox key: 0x%04X\n", modem.runtimeKey());
  Serial.printf("SPI communication: %s\n", modem.communicationTest() ? "PASS" : "FAIL");

  pl460::BoardInfo info;
  if (modem.getBoardInfo(info)) {
    Serial.printf("Product: 0x%04X  Model: 0x%04X\n", info.productId, info.model);
    Serial.printf("Firmware: %s (0x%08lX), band=%u\n", info.version,
                  static_cast<unsigned long>(info.versionNumber), info.band);
  } else {
    Serial.printf("Board info failed: %s\n", modem.lastErrorString());
  }
  Serial.printf("Thermal/supply TX safety: %s\n", modem.canTransmit() ? "PASS" : "BLOCKED");
}

void loop() {}

