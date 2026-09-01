#include "PL460G3Mac.h"
#include <string.h>

namespace pl460 {

// MAC frame header format (over-the-wire)
// [DST_LO][DST_HI][SRC_LO][SRC_HI][SEQ][FLAGS][PAYLOAD...]
// Overhead: 6 bytes
const uint8_t kHeaderSize = 6;

// Flags
const uint8_t kFlagAckReq = 0x01;
const uint8_t kFlagAck    = 0x02;

// CCA: CSMA random backoff window (ms)
const uint16_t kMinBackoffMs = 10;
const uint16_t kMaxBackoffMs = 100;
const uint16_t kCcaRetryLimit = 50;  // max CSMA attempts before fail

G3Mac::G3Mac(G3Phy &phy) : phy_(phy), device_(phy.device()) {}

bool G3Mac::begin() {
  state_ = State::Idle;
  txCfmReady_ = false;
  rxReady_ = false;
  rxLength_ = 0;
  retryCount_ = 0;
  memset(&txCfm_, 0, sizeof(txCfm_));
  memset(&rxInfo_, 0, sizeof(rxInfo_));
  return true;
}

uint16_t G3Mac::randomBackoff() {
  // Simple LCG; deterministic but good enough for CSMA.
  static uint16_t rng = 1;
  rng = rng * 1103515245U + 12345U;
  return kMinBackoffMs + (rng % (kMaxBackoffMs - kMinBackoffMs));
}

bool G3Mac::send(uint16_t dstAddr, const uint8_t *data, uint16_t length) {
  if (!data || !length || length > PL460::kMaxMailboxPayload - kHeaderSize)
    return false;
  if (state_ != State::Idle) return false;

  dstAddr_ = dstAddr;
  txLength_ = length + kHeaderSize;
  txSeq_ = seqNum_++;

  // Build MAC frame header
  uint8_t *p = txPayload_;
  *p++ = static_cast<uint8_t>(dstAddr);
  *p++ = static_cast<uint8_t>(dstAddr >> 8);
  *p++ = static_cast<uint8_t>(shortAddr_);
  *p++ = static_cast<uint8_t>(shortAddr_ >> 8);
  *p++ = static_cast<uint8_t>(txSeq_);
  *p++ = kFlagAckReq;  // always request ACK for reliability
  memcpy(p, data, length);

  retryCount_ = 0;
  state_ = State::CcaBackoff;
  stateTimer_ = millis();
  return true;
}

void G3Mac::poll() {
  // Must call PHY poll() every iteration to update RX/TX status
  phy_.poll();

  // --- 1. Check for incoming data from PHY ---
  if (phy_.available()) {
    uint8_t raw[PL460::kMaxMailboxPayload];
    G3RxInfo phyInfo;
    uint16_t rawLen = phy_.receive(raw, sizeof(raw), &phyInfo);

    if (rawLen >= kHeaderSize && phyInfo.crcOk == 1) {
      const uint16_t frameDst = static_cast<uint16_t>(raw[0]) |
                                (static_cast<uint16_t>(raw[1]) << 8);
      const uint16_t frameSrc = static_cast<uint16_t>(raw[2]) |
                                (static_cast<uint16_t>(raw[3]) << 8);
      const uint8_t frameSeq = raw[4];
      const uint8_t flags = raw[5];
      const uint16_t payloadLen = rawLen - kHeaderSize;

      if (flags & kFlagAck) {
        // --- ACK frame ---
        // Match against pending TX by destination address and sequence
        if (state_ == State::WaitAck &&
            frameDst == shortAddr_ &&   // ACK addressed to us
            frameSrc == dstAddr_ &&     // from our destination
            frameSeq == txSeq_ &&       // matching sequence
            retryCount_ <= maxRetries_) {
          // ACK match!
          txCfm_.status = MacTxStatus::Success;
          txCfm_.retries = retryCount_;
          txCfmReady_ = true;
          state_ = State::Idle;
        }
        // Don't forward ACK frames to the application
        return;
      }

      // --- Data frame ---
      // Check if addressed to us or broadcast
      if (frameDst == shortAddr_ || frameDst == 0xFFFF) {
        // Copy payload to RX buffer
        const uint16_t copyLen = payloadLen < sizeof(rxBuffer_)
            ? payloadLen : sizeof(rxBuffer_);
        memcpy(rxBuffer_, raw + kHeaderSize, copyLen);
        rxLength_ = copyLen;
        rxInfo_.srcAddr = frameSrc;
        rxInfo_.lqi = phyInfo.lqi;
        rxInfo_.snr = phyInfo.snrPayload;
        rxReady_ = true;

        // Send ACK if requested
        if (flags & kFlagAckReq) {
          uint8_t ack[6];
          ack[0] = static_cast<uint8_t>(frameSrc);   // dst = original sender
          ack[1] = static_cast<uint8_t>(frameSrc >> 8);
          ack[2] = static_cast<uint8_t>(shortAddr_); // src = us
          ack[3] = static_cast<uint8_t>(shortAddr_ >> 8);
          ack[4] = frameSeq;                         // same seq
          ack[5] = kFlagAck;                         // this is an ACK
          // Fire and forget — don't block on ACK delivery
          (void)phy_.send(ack, sizeof(ack));
        }
      }
    }
  }

  // --- 2. TX state machine ---
  switch (state_) {
    case State::Idle:
      break;

    case State::CcaBackoff:
      // Wait for random backoff period, then check channel
      if (millis() - stateTimer_ >= randomBackoff()) {
        state_ = State::CcaWait;
        stateTimer_ = millis();
      }
      break;

    case State::CcaWait:
      // Clear channel assessment
      if (device_.canTransmit()) {
        // Channel clear — send frame
        state_ = State::SendFrame;
        stateTimer_ = millis();
      } else {
        // Channel busy — re-enter backoff or fail
        if (++retryCount_ > kCcaRetryLimit) {
          txCfm_.status = MacTxStatus::ChannelAccessFailure;
          txCfm_.retries = retryCount_;
          txCfmReady_ = true;
          state_ = State::Idle;
        } else {
          state_ = State::CcaBackoff;
          stateTimer_ = millis();
        }
      }
      break;

    case State::SendFrame:
      {
        // Use PHY-level ACK (SofResponse) for reliability
        pl460::G3TxConfig cfg = pl460::G3TxConfig::cenelecARobust();
        cfg.delimiter = pl460::G3Delimiter::SofResponse;
        phy_.send(txPayload_, txLength_, cfg);
      }
      retryCount_ = 0;
      state_ = State::WaitAck;
      stateTimer_ = millis();
      break;

    case State::WaitAck:
      if (millis() - stateTimer_ >= ackTimeoutMs_) {
        // Timeout — no ACK received
        if (++retryCount_ <= maxRetries_) {
          // Retransmit
          state_ = State::CcaBackoff;
          stateTimer_ = millis();
        } else {
          txCfm_.status = MacTxStatus::NoAck;
          txCfm_.retries = retryCount_;
          txCfmReady_ = true;
          state_ = State::Idle;
        }
      }
      break;
  }

  // 3. Check PHY TX confirm for SofResponse result
  if (phy_.transmissionComplete()) {
    pl460::G3TxConfirm cfm;
    phy_.takeTxConfirm(cfm);
    
    if (state_ == State::WaitAck) {
      if (cfm.result == 0) {
        // PHY-level ACK received via SofResponse
        txCfm_.status = MacTxStatus::Success;
        txCfm_.retries = retryCount_;
        txCfmReady_ = true;
        state_ = State::Idle;
      } else {
        // No PHY-level ACK — retransmit
        if (++retryCount_ <= maxRetries_) {
          state_ = State::CcaBackoff;
          stateTimer_ = millis();
        } else {
          txCfm_.status = MacTxStatus::NoAck;
          txCfm_.retries = retryCount_;
          txCfmReady_ = true;
          state_ = State::Idle;
        }
      }
    }
  }
}

uint16_t G3Mac::receive(uint8_t *data, uint16_t capacity, MacRxInfo *info) {
  if (!rxReady_ || !data || !capacity) return 0;
  const uint16_t copyLen = capacity < rxLength_ ? capacity : rxLength_;
  memcpy(data, rxBuffer_, copyLen);
  if (info) *info = rxInfo_;
  rxReady_ = false;
  return copyLen;
}

MacTxConfirm G3Mac::takeTxConfirm() {
  MacTxConfirm cfm = txCfm_;
  txCfmReady_ = false;
  return cfm;
}

}  // namespace pl460
