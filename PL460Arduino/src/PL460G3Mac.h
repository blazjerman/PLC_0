#pragma once

#include "PL460.h"
#include "PL460G3Phy.h"

namespace pl460 {

// TX confirmation result
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
// Handles: CSMA/CA, ACK/retransmission via software ACK frames,
//          16-bit addressing, automatic TX_EN management.
//
// TX_EN protocol (for PL460-EK rev5):
//   - call setManageTxEn(true) to have G3Mac toggle TX_EN automatically:
//     * enable before TX, disable after TX (so LNA is connected for RX)
//   - setManageTxEn(false) if you manage TX_EN externally
//   - The caller must still set enableTransmitter(true) at init on the sender
//     (G3Mac only toggles during TX operations, not the static setting)
class G3Mac {
 public:
  explicit G3Mac(G3Phy &phy);

  // Addressing and PAN config
  void setShortAddress(uint16_t addr) { shortAddr_ = addr; }
  void setPanId(uint16_t panId) { panId_ = panId; }

  // If true, G3Mac toggles TX_EN before/after ACK sends
  void setManageTxEn(bool manage) { manageTxEn_ = manage; }

  // Initialize MAC
  bool begin();

  // Send data to destination. Handles CSMA/CA + ACK + retry.
  // Returns false if busy or invalid args.
  bool send(uint16_t dstAddr, uint8_t *data, uint16_t length);

  // Must call every loop iteration.
  void poll();

  // Status queries
  bool busy() const { return state_ != State::Idle; }
  bool available() const { return rxReady_; }
  MacTxConfirm takeTxConfirm();

  // Read received payload. Returns length copied.
  uint16_t receive(uint8_t *data, uint16_t capacity, MacRxInfo *info = nullptr);

  // Configuration
  void setMaxRetries(uint8_t r) { maxRetries_ = r; }
  void setAckTimeoutMs(uint16_t t) { ackTimeoutMs_ = t; }

  uint16_t shortAddress() const { return shortAddr_; }

 private:
  enum class State : uint8_t {
    Idle,
    CcaBackoff,
    CcaWait,
    SendFrame,
    WaitAck,
  };

  static uint16_t randomBackoff();

  // Send an ACK frame (internal — bypasses send() for ACK only)
  bool sendAck(uint16_t dstAddr, uint8_t seq);

  G3Phy &phy_;
  PL460 &device_;

  // Addressing
  uint16_t shortAddr_ = 0xFFFF;
  uint16_t panId_ = 0xFFFF;
  uint8_t seqNum_ = 0;

  // Config
  uint8_t maxRetries_ = 3;
  uint16_t ackTimeoutMs_ = 500;
  bool manageTxEn_ = false;

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

  // PHY send tracking (wait for completion before disabling TX)
  bool phySendStarted_ = false;

  // ACK PHY send tracking (same pattern — don't cut PA during ACK TX)
  bool ackWaitPhy_ = false;

  // RX buffer
  uint8_t rxBuffer_[PL460::kMaxMailboxPayload];
  uint16_t rxLength_ = 0;
  bool rxReady_ = false;
  MacRxInfo rxInfo_;
};

}  // namespace pl460
