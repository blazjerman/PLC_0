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

// CSMA parameters
const uint16_t kMinBackoffMs = 10;
const uint16_t kMaxBackoffMs = 100;
const uint16_t kCcaRetryLimit = 50;

// Tiny delay after TX_EN toggle for PA/LNA settling
const uint8_t kTxEnSettleMs = 2;

G3Mac::G3Mac(G3Phy &phy) : phy_(phy), device_(phy.device()) {}

bool G3Mac::begin() {
  state_ = State::Idle;
  txCfmReady_ = false;
  rxReady_ = false;
  rxLength_ = 0;
  retryCount_ = 0;
  phySendStarted_ = false;
  txCfm_.status = MacTxStatus::Invalid;
  memset(&rxInfo_, 0, sizeof(rxInfo_));
  return true;
}

uint16_t G3Mac::randomBackoff() {
  static uint16_t rng = 1;
  rng = rng * 1103515245U + 12345U;
  return kMinBackoffMs + (rng % (kMaxBackoffMs - kMinBackoffMs));
}

bool G3Mac::send(uint16_t dstAddr, uint8_t *data, uint16_t length) {
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
  *p++ = kFlagAckReq;  // request ACK
  memcpy(p, data, length);

  retryCount_ = 0;
  txCfmReady_ = false;
  txCfm_.status = MacTxStatus::Invalid;
  phySendStarted_ = false;
  state_ = State::CcaBackoff;
  stateTimer_ = millis();
  return true;
}

// Send a short ACK frame (internal — no retry, no state change, unacknowledged).
// Used by poll() when we receive a data frame with AckReq.
bool G3Mac::sendAck(uint16_t dstAddr, uint8_t seq) {
  uint8_t ackFrame[kHeaderSize];
  ackFrame[0] = static_cast<uint8_t>(dstAddr);
  ackFrame[1] = static_cast<uint8_t>(dstAddr >> 8);
  ackFrame[2] = static_cast<uint8_t>(shortAddr_);
  ackFrame[3] = static_cast<uint8_t>(shortAddr_ >> 8);
  ackFrame[4] = seq;  // echo back the seq we're ACKing
  ackFrame[5] = kFlagAck;  // ACK frame, no AckReq

  if (manageTxEn_) {
    device_.enableTransmitter(true);
    delay(kTxEnSettleMs);
  }
  G3TxConfig cfg = G3TxConfig::cenelecARobust();
  cfg.delimiter = G3Delimiter::SofNoResponse;
  bool ok = phy_.send(ackFrame, kHeaderSize, cfg);
  if (manageTxEn_) {
    delay(kTxEnSettleMs);
    device_.enableTransmitter(false);
  }
  return ok;
}

void G3Mac::poll() {
  // Must call PHY poll() every iteration to update RX/TX state
  phy_.poll();

  // --- 1. Check for received frames ---
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

      // Only process frames addressed to us (broadcast 0xFFFF accepted too)
      if (frameDst == shortAddr_ || frameDst == 0xFFFF) {

        if (flags & kFlagAck) {
          // --- ACK frame ---
          // If we're waiting for an ACK, match by source address and seq
          if (state_ == State::WaitAck && dstAddr_ == frameSrc && txSeq_ == frameSeq) {
            txCfm_.status = MacTxStatus::Success;
            txCfm_.retries = retryCount_;
            txCfmReady_ = true;
            state_ = State::Idle;
          }
          // Otherwise discard — it's an ACK we weren't expecting

        } else {
          // --- Data frame ---
          const uint16_t payloadLen = rawLen - kHeaderSize;
          const uint16_t copyLen = payloadLen < sizeof(rxBuffer_)
              ? payloadLen : sizeof(rxBuffer_);
          memcpy(rxBuffer_, raw + kHeaderSize, copyLen);
          rxLength_ = copyLen;
          rxReady_ = true;
          rxInfo_.srcAddr = frameSrc;
          rxInfo_.lqi = phyInfo.lqi;
          rxInfo_.snr = phyInfo.snrPayload;

          // Send software ACK if requested
          if (flags & kFlagAckReq) {
            sendAck(frameSrc, frameSeq);
          }
        }
      }
    }
  }

  // --- 2. TX state machine ---
  switch (state_) {
    case State::Idle:
      break;

    case State::CcaBackoff:
      if (millis() - stateTimer_ >= randomBackoff()) {
        state_ = State::CcaWait;
        stateTimer_ = millis();
      }
      break;

    case State::CcaWait:
      if (device_.canTransmit()) {
        state_ = State::SendFrame;
      } else {
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
        // Enable TX (if managed), then start the PHY send
        if (manageTxEn_) {
          device_.enableTransmitter(true);
          delay(kTxEnSettleMs);
        }
        G3TxConfig cfg = G3TxConfig::cenelecARobust();
        cfg.delimiter = G3Delimiter::SofNoResponse;
        phy_.send(txPayload_, txLength_, cfg);
        phySendStarted_ = true;
      }
      retryCount_ = 0;
      state_ = State::WaitAck;
      stateTimer_ = millis();
      break;

    case State::WaitAck:
      if (millis() - stateTimer_ >= ackTimeoutMs_) {
        // Timeout — no ACK received
        if (manageTxEn_ && phySendStarted_) {
          device_.enableTransmitter(false);
          phySendStarted_ = false;
        }
        txCfm_.status = MacTxStatus::NoAck;
        txCfm_.retries = retryCount_;
        txCfmReady_ = true;
        state_ = State::Idle;
      }
      break;
  }

  // --- 3. Check PHY TX confirm (wait for PHY send to finish before disabling TX) ---
  if (phy_.transmissionComplete()) {
    G3TxConfirm cfm;
    phy_.takeTxConfirm(cfm);

    // PHY send just completed — turn off PA so LNA can hear ACK
    if (manageTxEn_ && phySendStarted_) {
      device_.enableTransmitter(false);
      phySendStarted_ = false;
    }

    // Check for PHY-level failure (only relevant while waiting for ACK)
    if (state_ == State::WaitAck && cfm.result != 0) {
      if (++retryCount_ <= maxRetries_) {
        state_ = State::CcaBackoff;
        stateTimer_ = millis();
      } else {
        txCfm_.status = MacTxStatus::NoCarrier;
        txCfm_.retries = retryCount_;
        txCfmReady_ = true;
        state_ = State::Idle;
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
  MacTxConfirm cfm;
  cfm.status = MacTxStatus::Invalid;
  cfm.retries = 0;
  if (txCfmReady_) {
    cfm = txCfm_;
    txCfmReady_ = false;
    txCfm_.status = MacTxStatus::Invalid;
  }
  return cfm;
}

}  // namespace pl460
