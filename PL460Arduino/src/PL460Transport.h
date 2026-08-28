#pragma once

#include <Arduino.h>
#include <SPI.h>

namespace pl460 {

struct Pins {
  int8_t cs;
  int8_t reset;
  int8_t extInt;
  int8_t txEnable;
  int8_t thermal;
  int8_t supplyMonitor;
  uint16_t supplyGoodThreshold;

  Pins(int8_t csPin, int8_t resetPin, int8_t extIntPin,
       int8_t txEnablePin = -1, int8_t thermalPin = -1,
       int8_t supplyMonitorPin = -1, uint16_t supplyThreshold = 0)
      : cs(csPin), reset(resetPin), extInt(extIntPin),
        txEnable(txEnablePin), thermal(thermalPin),
        supplyMonitor(supplyMonitorPin), supplyGoodThreshold(supplyThreshold) {}
};

struct SpiPins {
  int8_t sck;
  int8_t miso;
  int8_t mosi;

  SpiPins(int8_t sckPin = -1, int8_t misoPin = -1, int8_t mosiPin = -1)
      : sck(sckPin), miso(misoPin), mosi(mosiPin) {}
  bool custom() const { return sck >= 0 && miso >= 0 && mosi >= 0; }
};

class Transport {
 public:
  virtual ~Transport() {}
  virtual bool begin() = 0;
  virtual void end() = 0;
  virtual void resetPulse() = 0;
  virtual bool transfer(uint8_t *data, size_t length) = 0;
  virtual bool interruptActive() const = 0;
  virtual bool thermalAlarm() const = 0;
  virtual bool supplyGood() const = 0;
  virtual void setTxEnabled(bool enabled) = 0;
};

class ArduinoTransport : public Transport {
 public:
  ArduinoTransport(SPIClass &spi, const Pins &pins,
                   uint32_t frequency = 4000000UL);
  ArduinoTransport(SPIClass &spi, const Pins &pins, const SpiPins &spiPins,
                   uint32_t frequency = 4000000UL);

  bool begin() override;
  void end() override;
  void resetPulse() override;
  bool transfer(uint8_t *data, size_t length) override;
  bool interruptActive() const override;
  bool thermalAlarm() const override;
  bool supplyGood() const override;
  void setTxEnabled(bool enabled) override;

  void setFrequency(uint32_t frequency) { frequency_ = frequency; }
  const Pins &pins() const { return pins_; }

 private:
  SPIClass &spi_;
  Pins pins_;
  SpiPins spiPins_;
  uint32_t frequency_;
  bool started_;
};

}  // namespace pl460
