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
  // Must call PHY poll() every iteration to update RX/TX state
  phy_.poll();

  // --- 1. Check for received data frames ---
  // (PHY-level SofResponse handles ACK delivery at firmware level,
  //  so we never send software ACK frames — that would clash with the
  //  PHY ACK and cause both boards to deadlock.)
  if (phy_.available()) {
    uint8_t raw[PL460::kMaxMailboxPayload];
    G3RxInfo phyInfo;
    uint16_t rawLen = phy_.receive(raw, sizeof(raw), &phyInfo);

    if (rawLen >= kHeaderSize && phyInfo.crcOk == 1) {
      const uint16_t frameDst = static_cast<uint16_t>(raw[0]) |
                                (static_cast<uint16_t>(raw[1]) << 8);
      const uint16_t frameSrc = static_cast<uint16_t>(raw[2]) |
                                (static_cast<uint16_t>(raw[3]) << 8);
      const uint8_t flags = raw[5];
      const uint16_t payloadLen = rawLen - kHeaderSize;

      // Data frame addressed to us?
      if ((frameDst == shortAddr_ || frameDst == 0xFFFF) &&
          !(flags & kFlagAck)) {  // skip software ACK frames
        const uint16_t copyLen = payloadLen < sizeof(rxBuffer_)
            ? payloadLen : sizeof(rxBuffer_);
        memcpy(rxBuffer_, raw + kHeaderSize, copyLen);
        rxLength_ = copyLen;
        rxInfo_.srcAddr = frameSrc;
        rxInfo_.lqi = phyInfo.lqi;
        rxInfo_.snr = phyInfo.snrPayload;
        rxReady_ = true;
        // No software ACK sent — PHY-level SofResponse handles it
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
        // Use PHY-level ACK (SofResponse) — the PL460 firmware
        // on the receiver auto-sends an ACK delimiter; no software
        // ACK frame needed.
        G3TxConfig cfg = G3TxConfig::cenelecARobust();
        cfg.delimiter = G3Delimiter::SofResponse;
        phy_.send(txPayload_, txLength_, cfg);
      }
      retryCount_ = 0;
      state_ = State::WaitAck;
      stateTimer_ = millis();
      break;

    case State::WaitAck:
      // Wait for PHY TX confirm (which includes SofResponse result).
      // Timeout is a safety net only.
      if (millis() - stateTimer_ >= ackTimeoutMs_) {
        // PHY never reported — force abort
        txCfm_.status = MacTxStatus::NoAck;
        txCfm_.retries = retryCount_;
        txCfmReady_ = true;
        state_ = State::Idle;
      }
      break;
  }

  // --- 3. Check PHY TX confirm for SofResponse ACK result ---
  if (phy_.transmissionComplete()) {
    G3TxConfirm cfm;
    phy_.takeTxConfirm(cfm);

    if (state_ == State::WaitAck) {
      if (cfm.result == 0) {
        // PHY-level SofResponse ACK received
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
