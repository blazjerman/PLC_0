#include <PL460.h>
#include <firmware/PLC_PHY_G3_CENA.h>

pl460::Pins pins(5, 4, 27, 26, 25);
pl460::ArduinoTransport transport(SPI, pins);
pl460::PL460 modem(transport);

void report(const char *name, bool pass) {
  Serial.printf("%-28s %s\n", name, pass ? "PASS" : "FAIL");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("PL460 single-board, receive-only self-test");
  report("Arduino transport", modem.begin());
  report("Firmware upload/start", modem.boot(pl460::PLC_PHY_G3_CENA_IMAGE));
  report("Mailbox round trips", modem.communicationTest(pl460::PL460::kPhyRuntimeKey));

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

