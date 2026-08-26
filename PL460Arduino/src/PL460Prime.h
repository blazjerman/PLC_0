#pragma once

#include "PL460.h"

namespace pl460 {

enum class PrimeScheme : uint8_t {
  DBPSK = 0, DQPSK = 1, D8PSK = 2, DBPSK_C = 4, DQPSK_C = 5,
  D8PSK_C = 6, RobustDBPSK = 12, RobustDQPSK = 13
};
enum class PrimeFrameType : uint8_t { TypeA = 0, TypeB = 2, TypeBC = 3 };

struct PrimeTxConfig {
  uint32_t startTime;
  uint8_t mode;
  uint8_t attenuation;
  PrimeScheme scheme;
  PrimeFrameType frameType;
  uint8_t buffer;
  bool disableCarrierSense;
  uint8_t carrierSenseCount;
  uint8_t carrierSenseDelayMs;
  PrimeTxConfig();
};

struct PrimeRxInfo {
  uint32_t startTime;
  uint16_t length;
  PrimeScheme scheme;
  PrimeFrameType frameType;
  uint8_t headerType;
  uint8_t rssi;
  uint8_t cinrAverage;
  uint8_t cinrMinimum;
  uint8_t softBerAverage;
  uint8_t softBerMaximum;
};

class PrimePhy {
 public:
  explicit PrimePhy(PL460 &device);
  bool send(const uint8_t *data, uint16_t length,
            const PrimeTxConfig &config = PrimeTxConfig());
  bool poll();
  bool busy(uint8_t buffer = 0) const;
  bool available() const { return rxReady_; }
  uint16_t receive(uint8_t *data, uint16_t capacity, PrimeRxInfo *info = nullptr);
  bool takeTxResult(uint8_t &result, uint8_t &buffer);

 private:
  static uint16_t le16(const uint8_t *p);
  static uint32_t le32(const uint8_t *p);
  PL460 &device_;
  bool txBusy_[2];
  bool txResultReady_;
  uint8_t txResult_;
  uint8_t txResultBuffer_;
  bool rxDataReady_;
  bool rxParametersReady_;
  bool rxReady_;
  PrimeRxInfo rxInfo_;
  uint8_t rxData_[512];
};

}  // namespace pl460

