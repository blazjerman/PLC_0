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
  if (!configureG3CenelecARev5(device_)) return false;
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
  // Set up defaults after MAC RT firmware boot
  txBusy_ = false;
  txCfmReady_ = false;
  rxReady_ = false;
  return true;
}

// Build and write a MAC RT TX request.
// Format: [len_lo][len_hi][payload...]
bool G3Mac::send(uint16_t dstAddr, const uint8_t *data, uint16_t length) {
  if (!data || !length || length > 400 || txBusy_) return false;
  if (!device_.canTransmit()) return false;

  // Build the TX request packet:
  // The MAC RT firmware expects the complete MAC frame (header + payload).
  // We build a minimal data frame with ACK request:
  //
  // MAC frame format:
  //   Frame Control (2 bytes)
  //   Sequence Number (1 byte)   - handled by firmware
  //   Dest PAN ID   (2 bytes)
  //   Dest Address  (2 bytes, short)
  //   Source Address(2 bytes, short) - inserted by firmware
  //   Payload
  //
  // MAC RT TX_REQ expects: [payload_len_lo][payload_len_hi][complete MAC frame]
  // The firmware fills in source address and sequence number.

  uint8_t wire[512];
  uint8_t *p = wire;

  // Length prefix (2 bytes, little-endian payload length)
  uint16_t frameLen = 4 + length;  // FC(2) + PAN(2) + dst(2) + src(2) - src filled by fw = 4
  // Actually, let's include FC + dstPAN + destShort = 2+2+2 = 6 bytes header
  // Then src address is filled in by firmware... actually no.
  // Let me check what the firmware expects.
  //
  // Standard G3 MAC frame header (IEEE 802.15.4):
  // FC = Frame Control (2 bytes)
  // Seq = Sequence Number (1 byte)
  // Dest PAN = 2 bytes 
  // Dest = 2 bytes (short)
  // Src = 2 bytes (short) -- firmware fills this in
  //
  // Total header before payload: 9 bytes
  // But MacRtTxRequest takes the whole MAC frame (header+payload)

  // Build frame control: data frame, ACK request, short dest, short src, PAN ID compression
  uint16_t frameControl = 0;
  frameControl |= (0x01 << 0);     // Frame type: Data
  frameControl |= (1 << 5);        // ACK request
  frameControl |= (1 << 6);        // PAN ID compression (dest PAN present, src PAN = dest PAN)
  frameControl |= (0x02 << 10);    // Dest addressing mode: short
  frameControl |= (0x02 << 14);    // Source addressing mode: short

  p[0] = static_cast<uint8_t>(frameControl);
  p[1] = static_cast<uint8_t>(frameControl >> 8);
  p += 2;

  // Sequence number - firmware fills this
  p[0] = 0;
  p += 1;

  // Dest PAN ID
  p[0] = 0xFF;
  p[1] = 0xFF;  // Default broadcast PAN
  p += 2;

  // Dest address (short)
  p[0] = static_cast<uint8_t>(dstAddr);
  p[1] = static_cast<uint8_t>(dstAddr >> 8);
  p += 2;

  // Source address - firmware fills this in
  // (skip 2 bytes in the length calculation)
  p += 2;

  // Payload
  memcpy(p, data, length);
  p += length;

  uint16_t totalLen = static_cast<uint16_t>(p - wire);

  // Write to TX_REQ mailbox
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

  // TX confirmation
  if (info.flags & kMacFlagTxCfm) {
    uint8_t wire[12];
    if (device_.mailboxRead(kMacMailboxTxCfm, wire, sizeof(wire))) {
      lastTxCfm_.rms = getLe32(wire);
      lastTxCfm_.endTime = getLe32(wire + 4);
      lastTxCfm_.status = static_cast<MacTxStatus>(wire[8]);
    } else {
      lastTxCfm_.status = MacTxStatus::Invalid;
    }
    txBusy_ = false;
    txCfmReady_ = true;
  }

  // Data indication (received frame)
  if (info.flags & kMacFlagDataInd) {
    uint8_t wire[PL460::kMaxMailboxPayload];
    MailboxInfo indInfo;
    if (device_.mailboxRead(kMacMailboxDataInd, wire, sizeof(wire), &indInfo)) {
      // First 2 bytes are length (little-endian)
      uint16_t frameLen = static_cast<uint16_t>(wire[0] | (wire[1] << 8));
      if (frameLen > sizeof(rxBuffer_)) frameLen = sizeof(rxBuffer_);

      // Skip length prefix, copy frame data
      uint16_t dataLen = frameLen > 2 ? frameLen - 2 : 0;
      if (dataLen > 0) {
        memcpy(rxBuffer_, wire + 2, dataLen);
        rxLength_ = dataLen;

        // Try to parse source address from MAC header
        // FC(2) + Seq(1) = 3 bytes before addressing fields
        // With PAN ID compression (1): after 3 bytes is destPAN(2)+dest(2)+src(2)
        if (dataLen >= 9) {
          lastRxInfo_.srcAddr = static_cast<uint16_t>(rxBuffer_[7] | (rxBuffer_[8] << 8));
        }
      }

      rxReady_ = true;
    }
  }

  // RX parameters (optional - for SNR, LQI, RSSI)
  if (info.flags & kMacFlagRxParInd) {
    uint8_t wire[120];
    if (device_.mailboxRead(kMacMailboxRxParInd, wire, sizeof(wire))) {
      // Parse RX parameters
      // Format depends on the specific MAC RT firmware version
      lastRxInfo_.lqi = wire[1];
      lastRxInfo_.modulation = wire[5];
      // SNR at different offsets depending on firmware
      lastRxInfo_.snrPayload = static_cast<int16_t>(
          static_cast<uint16_t>(wire[7]) | (static_cast<uint16_t>(wire[8]) << 8));
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
  return device_.writeRegister(regAddr(kMacRtShortAddress), value, 2);
}

bool G3Mac::setPanId(uint16_t panId) {
  uint8_t value[2];
  value[0] = static_cast<uint8_t>(panId);
  value[1] = static_cast<uint8_t>(panId >> 8);
  return device_.writeRegister(regAddr(kMacRtPanId), value, 2);
}

bool G3Mac::setCoordinator(bool enable) {
  uint8_t value = enable ? 1 : 0;
  return device_.writeRegister(regAddr(kMacRtCoordinator), &value, 1);
}

uint16_t G3Mac::getLe16(const uint8_t *p) {
  return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

uint32_t G3Mac::getLe32(const uint8_t *p) {
  return static_cast<uint32_t>(p[0]) |
         (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) |
         (static_cast<uint32_t>(p[3]) << 24);
}

}  // namespace pl460
