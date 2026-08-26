#pragma once

#include "PL460.h"

namespace pl460 {

struct MetersAndMoreTxConfig {
  uint32_t startTime;
  uint8_t mode;
  uint8_t attenuation;
  uint8_t narrowbandFrame;
  MetersAndMoreTxConfig() : startTime(0), mode(1), attenuation(0), narrowbandFrame(0) {}
};

struct MetersAndMoreRxInfo {
  uint32_t endTime;
  uint32_t frameDuration;
  uint16_t length;
  int16_t snrHeader;
  int16_t snrPayload;
  uint8_t narrowband;
  uint8_t lqi;
  uint8_t rssi;
  uint8_t crcOk;
};

class MetersAndMorePhy {
 public:
  explicit MetersAndMorePhy(PL460 &device);
  bool send(const uint8_t *data, uint16_t length,
            const MetersAndMoreTxConfig &config = MetersAndMoreTxConfig());
  bool poll();
  bool busy() const { return txBusy_; }
  bool available() const { return rxReady_; }
  uint16_t receive(uint8_t *data, uint16_t capacity,
                   MetersAndMoreRxInfo *info = nullptr);
  bool takeTxResult(uint8_t &result);

 private:
  static uint16_t le16(const uint8_t *p);
  static uint32_t le32(const uint8_t *p);
  PL460 &device_;
  bool txBusy_;
  bool txResultReady_;
  uint8_t txResult_;
  bool rxDataReady_;
  bool rxParametersReady_;
  bool rxReady_;
  MetersAndMoreRxInfo rxInfo_;
  uint8_t rxData_[256];
};

}  // namespace pl460

