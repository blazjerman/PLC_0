#pragma once

#include "PL460.h"
#include "PL460G3Phy.h"

namespace pl460 {

// TX confirmation
enum class MacTxStatus : uint8_t {
  Success = 0,
  ChannelAccessFailure = 1,
  NoAck = 2,
  NoCarrier = 3,
  Invalid = 0xFF
};

struct MacTxConfirm {
  MacTxStatus status;
  uint8_t retries;
};

struct MacRxInfo {
  uint16_t srcAddr;
  uint8_t lqi;
  int16_t snr;
};

// Software G3 MAC layer over G3Phy.
// Handles: CSMA/CA, addressing, ACK/NACK, retransmission, frame sequencing.
class G3Mac {
 public:
  explicit G3Mac(G3Phy &phy);

  // Configure addressing. Must call before begin().
  void setShortAddress(uint16_t addr) { shortAddr_ = addr; }
  void setPanId(uint16_t panId) { panId_ = panId; }

  // Initialize MAC layer.
  bool begin();

  // Send data to destination. Handles CSMA/CA + ACK + retry.
  // Returns false if busy or invalid args.
  bool send(uint16_t dstAddr, const uint8_t *data, uint16_t length);

  // Must call every loop iteration.
  void poll();

  // --- Status queries ---
  bool busy() const { return state_ != State::Idle; }
  bool available() const { return rxReady_; }
  bool transmissionComplete() const { return txCfmReady_; }
  MacTxConfirm takeTxConfirm();

  // Read received payload. Returns length copied.
  uint16_t receive(uint8_t *data, uint16_t capacity, MacRxInfo *info = nullptr);

  // --- Configuration ---
  void setMaxRetries(uint8_t r) { maxRetries_ = r; }
  void setAckTimeoutMs(uint16_t t) { ackTimeoutMs_ = t; }

 private:
  enum class State : uint8_t {
    Idle,
    CcaWait,
    CcaBackoff,
    SendFrame,
    WaitAck,
  };

  static uint16_t randomBackoff();

  G3Phy &phy_;
  PL460 &device_;

  // Addressing
  uint16_t shortAddr_ = 0xFFFF;
  uint16_t panId_ = 0xFFFF;
  uint8_t seqNum_ = 0;

  // Config
  uint8_t maxRetries_ = 3;
  uint16_t ackTimeoutMs_ = 500;

  // TX state machine
  State state_ = State::Idle;
  uint16_t dstAddr_ = 0;
  uint8_t txPayload_[PL460::kMaxMailboxPayload];
  uint16_t txLength_ = 0;
  uint16_t txSeq_ = 0;
  uint8_t retryCount_ = 0;
  uint32_t stateTimer_ = 0;

  // TX confirm
  bool txCfmReady_ = false;
  MacTxConfirm txCfm_;

  // RX buffer
  uint8_t rxBuffer_[PL460::kMaxMailboxPayload];
  uint16_t rxLength_ = 0;
  bool rxReady_ = false;
  MacRxInfo rxInfo_;
};

}  // namespace pl460
