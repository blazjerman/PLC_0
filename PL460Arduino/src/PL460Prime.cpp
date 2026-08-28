#include "PL460Prime.h"
#include <string.h>

namespace pl460 {
namespace {
void putLe32(uint8_t *p, uint32_t v) {
  p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24;
}
}

PrimeTxConfig::PrimeTxConfig()
    : startTime(0), mode(1), attenuation(0), scheme(PrimeScheme::RobustDBPSK),
      frameType(PrimeFrameType::TypeA), buffer(0), disableCarrierSense(false),
      carrierSenseCount(3), carrierSenseDelayMs(2) {}

PrimePhy::PrimePhy(PL460 &device)
    : device_(device), txResultReady_(false), txResult_(0), txResultBuffer_(0),
      rxDataReady_(false), rxParametersReady_(false), rxReady_(false) {
  txBusy_[0] = txBusy_[1] = false;
  memset(&rxInfo_, 0, sizeof(rxInfo_));
}

uint16_t PrimePhy::le16(const uint8_t *p) { return p[0] | (p[1] << 8); }
uint32_t PrimePhy::le32(const uint8_t *p) {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

bool PrimePhy::send(const uint8_t *data, uint16_t length, const PrimeTxConfig &c) {
  if (!data || !length || length > 512 || c.buffer > 1 || txBusy_[c.buffer] ||
      !device_.canTransmit()) return false;
  uint8_t wire[524];
  putLe32(wire, c.startTime);
  wire[4] = length; wire[5] = length >> 8;
  wire[6] = c.attenuation;
  wire[7] = static_cast<uint8_t>(c.scheme);
  wire[8] = (c.disableCarrierSense ? 1U : 0U) |
            ((c.carrierSenseCount & 7U) << 1) |
            ((c.carrierSenseDelayMs & 15U) << 4);
  wire[9] = static_cast<uint8_t>(c.frameType);
  wire[10] = c.mode;
  wire[11] = c.buffer;
  memcpy(wire + 12, data, length);
  const uint16_t mailbox = c.buffer ? 4 : 1;
  if (!device_.mailboxWrite(mailbox, wire, length + 12)) return false;
  txBusy_[c.buffer] = true;
  delayMicroseconds(20);
  return true;
}

bool PrimePhy::poll() {
  uint8_t status[8];
  MailboxInfo mb;
  if (!device_.mailboxRead(0, status, sizeof(status), &mb)) return false;
  for (uint8_t buffer = 0; buffer < 2; ++buffer) {
    if (mb.flags & (1U << buffer)) {
      uint8_t cfm[20];
      if (!device_.mailboxRead(buffer ? 6 : 3, cfm, sizeof(cfm))) return false;
      txResult_ = cfm[9];
      txResultBuffer_ = cfm[10];
      if (txResultBuffer_ > 1) txResultBuffer_ = buffer;
      txBusy_[txResultBuffer_] = false;
      txResultReady_ = true;
    }
  }
  const uint16_t length = le16(status + 4);
  if ((mb.flags & 0x0004U) && length <= sizeof(rxData_)) {
    if (!device_.mailboxRead(8, rxData_, length)) return false;
    rxInfo_.length = length;
    rxDataReady_ = true;
  }
  if (mb.flags & 0x0020U) {
    uint8_t p[37];
    if (!device_.mailboxRead(7, p, sizeof(p))) return false;
    rxInfo_.startTime = le32(p + 8);
    rxInfo_.length = le16(p + 16);
    rxInfo_.scheme = static_cast<PrimeScheme>(p[18]);
    rxInfo_.frameType = static_cast<PrimeFrameType>(p[19]);
    rxInfo_.headerType = p[20];
    rxInfo_.rssi = p[21];
    rxInfo_.cinrAverage = p[22];
    rxInfo_.cinrMinimum = p[23];
    rxInfo_.softBerAverage = p[24];
    rxInfo_.softBerMaximum = p[25];
    rxParametersReady_ = true;
  }
  if (rxDataReady_ && rxParametersReady_) rxReady_ = true;
  return true;
}

bool PrimePhy::busy(uint8_t buffer) const { return buffer < 2 && txBusy_[buffer]; }

bool PrimePhy::takeTxResult(uint8_t &result, uint8_t &buffer) {
  if (!txResultReady_) return false;
  result = txResult_; buffer = txResultBuffer_; txResultReady_ = false;
  return true;
}

uint16_t PrimePhy::receive(uint8_t *data, uint16_t capacity, PrimeRxInfo *info) {
  if (!rxReady_ || !data || !capacity) return 0;
  const uint16_t length = capacity < rxInfo_.length ? capacity : rxInfo_.length;
  memcpy(data, rxData_, length);
  if (info) *info = rxInfo_;
  rxReady_ = rxDataReady_ = rxParametersReady_ = false;
  return length;
}

}  // namespace pl460
