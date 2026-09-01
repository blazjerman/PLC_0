#include "PL460G3Mac.h"
#include "PL460G3Coupling.h"
#include <string.h>

namespace pl460 {
namespace {

const uint32_t kRegisterBase = 0x80000000UL;

// Register offset helpers
uint32_t regAddr(uint16_t pibId) {
  return kRegisterBase + (pibId & 0x0FFFU);
}

// MAC RT PIB register IDs (from Microchip G3 MAC RT SDK)
const uint16_t kPibShortAddress     = 0x0021;  // 16-bit short address
const uint16_t kPibPanId            = 0x0022;  // 16-bit PAN identifier
const uint16_t kPibCoordinator      = 0x001E;  // Coordinator (set/clear)
const uint16_t kPibTxCoil           = 0x0114;  // TX coil driver config
const uint16_t kPibAgcConfig        = 0x0100;  // AGC auto-detection
const uint16_t kPibRxFcFilter       = 0x0200;  // RX frame control filter
const uint16_t kPibPromiscuous      = 0x0201;  // Promiscuous mode enable
const uint16_t kPibAutoAck          = 0x0202;  // Auto ACK enable

}  // namespace

G3Mac::G3Mac(PL460 &device)
    : device_(device),
      txBusy_(false),
      txCfmReady_(false),
      rxReady_(false),
      rxLength_(0) {
  memset(&lastTxCfm_, 0, sizeof(lastTxCfm_));
  memset(&lastRxInfo_, 0, sizeof(lastRxInfo_));
}

bool G3Mac::begin(const FirmwareImage &image,
                  uint16_t shortAddr,
                  uint16_t panId,
                  bool isCoordinator,
                  uint32_t bootTimeoutMs) {
  if (!device_.begin()) return false;
  if (!device_.boot(image, bootTimeoutMs)) return false;

  // Configure analog front-end (same calibration as PHY mode)
  if (!configureG3CenelecARev5(device_)) return false;

  // Enable promiscuous mode so we receive all frames (for pairing)
  // In production you would disable this.
  uint8_t promisc = 1;
  device_.writeRegister(regAddr(kPibPromiscuous), &promisc, 1);

  // Auto ACK - hardware generates ACKs automatically
  uint8_t autoAck = 1;
  device_.writeRegister(regAddr(kPibAutoAck), &autoAck, 1);

  // Set addressing
  if (!setShortAddress(shortAddr)) return false;
  if (!setPanId(panId)) return false;
  if (!setCoordinator(isCoordinator)) return false;

  device_.enableTransmitter(true);
  txBusy_ = false;
  txCfmReady_ = false;
  rxReady_ = false;
  return true;
}

bool G3Mac::begin() {
  txBusy_ = false;
  txCfmReady_ = false;
  rxReady_ = false;
  return true;
}

// MAC RT TX format:
//   [len_lo][len_hi][payload...]
// The MAC RT firmware on the PL460 handles:
//   - Frame control + sequence number
//   - Source/dest addressing (from PIB)
//   - ACK request
//   - CSMA/CA
//   - Retransmission
bool G3Mac::send(uint16_t dstAddr, const uint8_t *data, uint16_t length) {
  if (!data || !length || length > 400 || txBusy_) return false;
  if (!device_.canTransmit()) return false;

  // The MAC RT firmware on the PL460 needs a complete MAC 802.15.4 frame.
  // We build: FC(2) + Seq(1) + DestPAN(2) + Dest(2) + Src(2) + Payload
  //
  // Frame types are per G3-PLC (ITU-T G.9903) / IEEE 802.15.4:
  //   FC[0:2]   = 001 (Data frame)
  //   FC[5]     = 1   (ACK request)
  //   FC[6]     = 1   (PAN ID compression)
  //   FC[10:11] = 10  (Dest addressing: short)
  //   FC[14:15] = 10  (Src addressing: short)

  uint8_t wire[512];
  uint8_t *p = wire;

  // Frame Control
  const uint16_t fc = 0x61C1;  // Data + ACK+ PANcomp + short dest + short src
  *p++ = static_cast<uint8_t>(fc);
  *p++ = static_cast<uint8_t>(fc >> 8);

  // Sequence number (firmware may ignore/override)
  *p++ = 0;

  // Dest PAN ID (broadcast PAN unless configured otherwise)
  *p++ = 0xFF;
  *p++ = 0xFF;

  // Dest address (short)
  *p++ = static_cast<uint8_t>(dstAddr);
  *p++ = static_cast<uint8_t>(dstAddr >> 8);

  // Source address - will be inserted by firmware or use PIB setting
  *p++ = 0x00;
  *p++ = 0x00;

  // Payload
  memcpy(p, data, length);
  p += length;

  // Write complete frame to TX_REQ mailbox
  const uint16_t totalLen = static_cast<uint16_t>(p - wire);
  if (!device_.mailboxWrite(kMacMailboxTxReq, wire, totalLen)) return false;

  txBusy_ = true;
  txCfmReady_ = false;
  return true;
}

bool G3Mac::poll() {
  uint8_t status[8];
  MailboxInfo info;

  if (!device_.mailboxRead(kMacMailboxStatusInfo, status, sizeof(status), &info)) {
    return false;
  }

  const uint16_t flags = info.flags;

  // TX confirmation
  if (flags & kMacFlagTxCfm) {
    uint8_t wire[16];
    if (device_.mailboxRead(kMacMailboxTxCfm, wire, sizeof(wire))) {
      // Format varies by firmware version; common:
      // [RMS_LO][RMS_HI][TIME_0..3][STATUS][...]
      lastTxCfm_.rms = static_cast<uint32_t>(wire[0]) |
                       (static_cast<uint32_t>(wire[1]) << 8);
      lastTxCfm_.endTime = static_cast<uint32_t>(wire[2]) |
                           (static_cast<uint32_t>(wire[3]) << 8) |
                           (static_cast<uint32_t>(wire[4]) << 16) |
                           (static_cast<uint32_t>(wire[5]) << 24);
      lastTxCfm_.status = (wire[8] < 3)
          ? static_cast<MacTxStatus>(wire[8])
          : MacTxStatus::Invalid;
    } else {
      lastTxCfm_.status = MacTxStatus::Invalid;
    }
    txBusy_ = false;
    txCfmReady_ = true;
  }

  // Data indication (received frame)
  if (flags & kMacFlagDataInd) {
    uint8_t wire[PL460::kMaxMailboxPayload];
    MailboxInfo indInfo;
    if (device_.mailboxRead(kMacMailboxDataInd, wire, sizeof(wire), &indInfo)) {
      // DATA_IND format: [frame_len_lo][frame_len_hi][MAC frame...]
      const uint16_t frameLen = static_cast<uint16_t>(wire[0]) |
                                (static_cast<uint16_t>(wire[1]) << 8);

      const uint16_t copyLen = (frameLen > sizeof(rxBuffer_))
          ? sizeof(rxBuffer_) : frameLen;
      if (copyLen > 2) {
        memcpy(rxBuffer_, wire + 2, copyLen - 2);
        rxLength_ = copyLen - 2;

        // Parse source address from MAC header (bytes 7-8)
        if (rxLength_ >= 7) {
          lastRxInfo_.srcAddr = static_cast<uint16_t>(rxBuffer_[6]) |
                                (static_cast<uint16_t>(rxBuffer_[7]) << 8);
        }
      }
      rxReady_ = true;
    }
  }

  // RX parameters (LQI, SNR, RSSI)
  if (flags & kMacFlagRxParInd) {
    uint8_t wire[120];
    if (device_.mailboxRead(kMacMailboxRxParInd, wire, sizeof(wire))) {
      lastRxInfo_.lqi = wire[1];
      if (sizeof(wire) > 7) {
        lastRxInfo_.snrPayload = static_cast<int16_t>(
            static_cast<uint16_t>(wire[6]) |
            (static_cast<uint16_t>(wire[7]) << 8));
      }
    }
  }

  return true;
}

uint16_t G3Mac::receive(uint8_t *data, uint16_t capacity, MacRxInfo *info) {
  if (!rxReady_ || !data || !capacity) return 0;

  const uint16_t copyLen = capacity < rxLength_ ? capacity : rxLength_;
  memcpy(data, rxBuffer_, copyLen);
  if (info) *info = lastRxInfo_;

  rxReady_ = false;
  return copyLen;
}

MacTxConfirm G3Mac::takeTxConfirm() {
  MacTxConfirm confirm = lastTxCfm_;
  txCfmReady_ = false;
  return confirm;
}

bool G3Mac::setShortAddress(uint16_t address) {
  uint8_t value[2];
  value[0] = static_cast<uint8_t>(address);
  value[1] = static_cast<uint8_t>(address >> 8);
  return device_.writeRegister(regAddr(kPibShortAddress), value, 2);
}

bool G3Mac::setPanId(uint16_t panId) {
  uint8_t value[2];
  value[0] = static_cast<uint8_t>(panId);
  value[1] = static_cast<uint8_t>(panId >> 8);
  return device_.writeRegister(regAddr(kPibPanId), value, 2);
}

bool G3Mac::setCoordinator(bool enable) {
  uint8_t value = enable ? 1 : 0;
  return device_.writeRegister(regAddr(kPibCoordinator), &value, 1);
}

}  // namespace pl460
