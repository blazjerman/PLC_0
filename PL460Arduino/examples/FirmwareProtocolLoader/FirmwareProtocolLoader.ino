#include <PL460.h>

// Include exactly one image to avoid wasting application flash.
#include <firmware/PLC_PHY_PRIME.h>
// Other choices:
// #include <firmware/PLC_PHY_MM.h>
// #include <firmware/G3_MAC_RT_CENA.h>

#define PIN_CS     10
#define PIN_MOSI   11
#define PIN_MISO   13
#define PIN_SCK    12
#define PIN_IRQ     2
#define PIN_RESET   3
#define PIN_LDO_EN  4
#define PIN_STBY    5
#define PIN_TX_EN   6
#define PIN_NTHW0   7

pl460::Pins pins(PIN_CS, PIN_RESET, PIN_IRQ, PIN_TX_EN, PIN_NTHW0);
pl460::SpiPins spiPins(PIN_SCK, PIN_MISO, PIN_MOSI);
pl460::ArduinoTransport transport(SPI, pins, spiPins);
pl460::PL460 modem(transport);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Safe normal-operation states. On a stock PL460-EK rev5 these two Xplained
  // signals are not connected unless R71 (ENABLE) and R68 (STBY) are fitted.
  pinMode(PIN_LDO_EN, OUTPUT);
  digitalWrite(PIN_LDO_EN, HIGH);  // PL460 ENABLE/LDO: active high
  pinMode(PIN_STBY, OUTPUT);
  digitalWrite(PIN_STBY, LOW);     // STBY: high would enter sleep mode
  delay(10);

  modem.begin();
  if (modem.boot(pl460::PLC_PHY_PRIME_IMAGE)) {
    Serial.println("PRIME firmware loaded; use raw mailboxRead/mailboxWrite for PRIME host commands.");
  } else {
    Serial.printf("Firmware load failed: %s\n", modem.lastErrorString());
  }
}

void loop() {}
