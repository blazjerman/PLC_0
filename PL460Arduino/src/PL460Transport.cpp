#include "PL460Transport.h"

namespace pl460 {

ArduinoTransport::ArduinoTransport(SPIClass &spi, const Pins &pins,
                                   uint32_t frequency)
    : spi_(spi), pins_(pins), frequency_(frequency), started_(false) {}

bool ArduinoTransport::begin() {
  pinMode(pins_.cs, OUTPUT);
  digitalWrite(pins_.cs, HIGH);
  pinMode(pins_.reset, OUTPUT);
  digitalWrite(pins_.reset, LOW);
  pinMode(pins_.extInt, INPUT_PULLUP);

  if (pins_.txEnable >= 0) {
    pinMode(pins_.txEnable, OUTPUT);
    digitalWrite(pins_.txEnable, LOW);
  }
  if (pins_.thermal >= 0) pinMode(pins_.thermal, INPUT_PULLUP);
  if (pins_.supplyMonitor >= 0) pinMode(pins_.supplyMonitor, INPUT);

  spi_.begin();
  started_ = true;
  return true;
}

void ArduinoTransport::end() {
  setTxEnabled(false);
  spi_.end();
  started_ = false;
}

void ArduinoTransport::resetPulse() {
  digitalWrite(pins_.reset, LOW);
  delayMicroseconds(50);
  digitalWrite(pins_.reset, HIGH);
  delayMicroseconds(1500);
}

bool ArduinoTransport::transfer(uint8_t *data, size_t length) {
  if (!started_ || !data || !length) return false;
  spi_.beginTransaction(SPISettings(frequency_, MSBFIRST, SPI_MODE0));
  digitalWrite(pins_.cs, LOW);
  for (size_t i = 0; i < length; ++i) data[i] = spi_.transfer(data[i]);
  digitalWrite(pins_.cs, HIGH);
  spi_.endTransaction();
  return true;
}

bool ArduinoTransport::interruptActive() const {
  return digitalRead(pins_.extInt) == LOW;
}

bool ArduinoTransport::thermalAlarm() const {
  return pins_.thermal >= 0 && digitalRead(pins_.thermal) == LOW;
}

bool ArduinoTransport::supplyGood() const {
  return pins_.supplyMonitor < 0 || pins_.supplyGoodThreshold == 0 ||
         analogRead(pins_.supplyMonitor) >= pins_.supplyGoodThreshold;
}

void ArduinoTransport::setTxEnabled(bool enabled) {
  if (pins_.txEnable >= 0) digitalWrite(pins_.txEnable, enabled ? HIGH : LOW);
}

}  // namespace pl460
