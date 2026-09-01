#pragma once

#include "PL460.h"

namespace pl460 {

// MAC RT firmware mailbox IDs
const uint16_t kMacMailboxStatusInfo     = 0;
const uint16_t kMacMailboxSetCoord       = 1;
const uint16_t kMacMailboxTxReq          = 2;
const uint16_t kMacMailboxTxCfm          = 3;
const uint16_t kMacMailboxDataInd        = 4;
const uint16_t kMacMailboxCommStatus     = 6;
const uint16_t kMacMailboxRxParInd       = 7;

// Status event flags
const uint16_t kMacFlagTxCfm       = 0x0001;
const uint16_t kMacFlagDataInd     = 0x0002;
const uint16_t kMacFlagCommStatus  = 0x0008;
const uint16_t kMacFlagRxParInd    = 0x0010;

// TX confirmation status codes
enum class MacTxStatus : uint8_t {
  Success = 0,
  ChannelAccessFailure = 1,
  NoAck = 2,
  Invalid = 0xFF
};

struct MacTxConfirm {
  MacTxStatus status;
  uint32_t endTime;
  uint32_t rms;
};

struct MacRxInfo {
  uint16_t srcAddr;
  uint8_t lqi;
  int16_t snrPayload;
  uint16_t rssi;
  uint8_t modulation;
  uint8_t correctedErrors;
};

// Register-based PIB IDs for MAC RT
const uint16_t kMacRtShortAddress      = 0x403C;
const uint16_t kMacRtPanId             = 0x403D;
const uint16_t kMacRtCoordinator       = 0x403E;

class G3Mac {
 public:
  explicit G3Mac(PL460 &device);

  // Boot MAC RT firmware and configure
  bool begin();

  // Send data to destination short address
  // Returns false if hardware busy or args invalid.
  // The MAC RT firmware handles ACK request, CSMA/CA, and retransmission.
  bool send(uint16_t dstAddr, const uint8_t *data, uint16_t length);

  // Poll mailbox for events.  Call every loop iteration.
  bool poll();

  // New data frame received?
  bool available() const { return rxReady_; }

  // Copy received payload.  Returns length (0 on nothing available).
  uint16_t receive(uint8_t *data, uint16_t capacity,
                   MacRxInfo *info = nullptr);

  // TX lifecycle
  bool busy() const { return txBusy_; }
  bool transmissionComplete() const { return txCfmReady_; }
  MacTxConfirm takeTxConfirm();

  // Addressing configuration
  bool setShortAddress(uint16_t address);
  bool setPanId(uint16_t panId);
  bool setCoordinator(bool enable);

  // Direct register access helpers (for PIB tweaks)
  PL460 &device() { return device_; }

 private:
  static uint16_t getLe16(const uint8_t *p);
  static uint32_t getLe32(const uint8_t *p);

  PL460 &device_;

  bool txBusy_;
  bool txCfmReady_;
  bool rxReady_;
  MacTxConfirm lastTxCfm_;

  uint8_t rxBuffer_[PL460::kMaxMailboxPayload];
  uint16_t rxLength_;
  MacRxInfo lastRxInfo_;
};

}  // namespace pl460
