#include <PL460.h>

// Include exactly one image to avoid wasting application flash.
#include <firmware/PLC_PHY_PRIME.h>
// Other choices:
// #include <firmware/PLC_PHY_MM.h>
// #include <firmware/G3_MAC_RT_CENA.h>

pl460::Pins pins(5, 4, 27, 26, 25);
pl460::ArduinoTransport transport(SPI, pins);
pl460::PL460 modem(transport);

void setup() {
  Serial.begin(115200);
  delay(1000);
  modem.begin();
  if (modem.boot(pl460::PLC_PHY_PRIME_IMAGE)) {
    Serial.println("PRIME firmware loaded; use raw mailboxRead/mailboxWrite for PRIME host commands.");
  } else {
    Serial.printf("Firmware load failed: %s\n", modem.lastErrorString());
  }
}

void loop() {}
