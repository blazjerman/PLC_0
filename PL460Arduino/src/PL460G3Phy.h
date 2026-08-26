#pragma once

#include "PL460.h"

namespace pl460 {

enum class G3Modulation : uint8_t { BPSK = 0, QPSK = 1, PSK8 = 2, RobustBPSK = 4 };
enum class G3Scheme : uint8_t { Differential = 0, Coherent = 1 };
enum class G3Delimiter : uint8_t { SofNoResponse = 0, SofResponse = 1, Ack = 2, Nack = 3 };

struct G3TxConfig {
  uint32_t startTime;
  uint8_t preemphasis[24];
  uint8_t toneMap[3];
  uint8_t mode;
  uint8_t attenuation;
  G3Modulation modulation;
  G3Scheme scheme;
  uint8_t phaseDetectorCounter;
  bool twoReedSolomonBlocks;
  G3Delimiter delimiter;

  static G3TxConfig cenelecARobust();
};

struct G3TxConfirm {
  uint32_t rms;
  uint32_t endTime;
  uint8_t result;
};

struct G3RxInfo {
  uint32_t endTime;
  uint32_t frameDuration;
  uint16_t rssi;
  uint16_t length;
  uint8_t correctedErrors;
  G3Modulation modulation;
  G3Scheme scheme;
  int16_t snrPayload;
  uint8_t lqi;
  G3Delimiter delimiter;
  uint8_t crcOk;
  uint8_t toneMap[3];
};

class G3Phy {
 public:
  explicit G3Phy(PL460 &device);

  bool send(const uint8_t *data, uint16_t length,
            const G3TxConfig &config = G3TxConfig::cenelecARobust());
  bool poll();
  bool transmissionComplete() const { return txConfirmReady_; }
  bool takeTxConfirm(G3TxConfirm &confirm);
  bool available() const { return rxReady_; }
  uint16_t receive(uint8_t *data, uint16_t capacity, G3RxInfo *info = nullptr);
  bool busy() const { return txBusy_; }

 private:
  static uint16_t getLe16(const uint8_t *p);
  static uint32_t getLe32(const uint8_t *p);
  void parseRxInfo(const uint8_t *wire);

  PL460 &device_;
  bool txBusy_;
  bool txConfirmReady_;
  bool rxReady_;
  bool rxDataReady_;
  bool rxParametersReady_;
  G3TxConfirm txConfirm_;
  G3RxInfo rxInfo_;
  uint8_t rxData_[PL460::kMaxMailboxPayload];
};

}  // namespace pl460
