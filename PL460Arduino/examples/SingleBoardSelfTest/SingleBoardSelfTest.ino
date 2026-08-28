#include <PL460.h>
#include <PL460G3Coupling.h>
#include <firmware/PLC_PHY_G3_CENA.h>

pl460::Pins pins(10, 3, 2, 6, 7);
pl460::SpiPins spiPins(12, 13, 11);
pl460::ArduinoTransport transport(SPI, pins, spiPins);
pl460::PL460 modem(transport);

void report(const char *name, bool pass) {
  Serial.printf("%-28s %s\n", name, pass ? "PASS" : "FAIL");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(4, OUTPUT); digitalWrite(4, HIGH);  // LDO_EN
  pinMode(5, OUTPUT); digitalWrite(5, LOW);   // STBY: normal operation
  delay(10);
  Serial.println("PL460 single-board, receive-only self-test");
  report("Arduino transport", modem.begin());
  report("Firmware upload/start", modem.boot(pl460::PLC_PHY_G3_CENA_IMAGE));
  report("Mailbox round trips", modem.communicationTest(pl460::PL460::kPhyRuntimeKey));
  report("Rev5 coupling tables", pl460::configureG3CenelecARev5(modem));

  pl460::BoardInfo info;
  report("Firmware information", modem.getBoardInfo(info));
  report("TX safety inputs", modem.canTransmit());
  Serial.println("Self-test intentionally sends no signal onto the power line.");
}

void loop() {
  static uint32_t last = 0;
  if (millis() - last >= 1000) {
    last = millis();
    report("Periodic mailbox", modem.communicationTest());
  }
}
