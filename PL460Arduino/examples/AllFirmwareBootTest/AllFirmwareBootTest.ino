#include <PL460.h>
#include <PL460G3Coupling.h>
#include <firmware/PLC_PHY_G3_CENA.h>
#include <firmware/G3_MAC_RT_CENA.h>
#include <firmware/PLC_PHY_PRIME.h>
#include <firmware/PLC_PHY_PRIME_2CHN.h>
#include <firmware/PLC_PHY_MM.h>

pl460::Pins pins(10, 3, 2, 6, 7);
pl460::SpiPins spiPins(12, 13, 11);
pl460::ArduinoTransport transport(SPI, pins, spiPins);
pl460::PL460 modem(transport);

const pl460::FirmwareImage *images[] = {
  &pl460::PLC_PHY_G3_CENA_IMAGE,
  &pl460::G3_MAC_RT_CENA_IMAGE,
  &pl460::PLC_PHY_PRIME_IMAGE,
  &pl460::PLC_PHY_PRIME_2CHN_IMAGE,
  &pl460::PLC_PHY_MM_IMAGE,
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(4, OUTPUT); digitalWrite(4, HIGH);  // LDO_EN
  pinMode(5, OUTPUT); digitalWrite(5, LOW);   // STBY: normal operation
  delay(10);
  modem.begin();
  for (const pl460::FirmwareImage *image : images) {
    Serial.printf("Boot %-24s ", image->name);
    if (modem.boot(*image)) {
      Serial.printf("PASS key=0x%04X\n", modem.runtimeKey());
      if (image->protocol == pl460::Protocol::G3Phy &&
          image->band == pl460::Band::CenelecA) {
        Serial.printf("  Rev5 CENELEC-A coupling: %s\n",
                      pl460::configureG3CenelecARev5(modem) ? "PASS" : "FAIL");
      }
    } else {
      Serial.printf("FAIL %s\n", modem.lastErrorString());
    }
    delay(250);
  }
  Serial.println("All personalities tested. No PLC transmission was requested.");
}

void loop() {}
