#include "PL460MetersAndMore.h"
#include <string.h>

namespace pl460 {
namespace {
void putLe32(uint8_t *p, uint32_t v) {
  p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24;
}
}

MetersAndMorePhy::MetersAndMorePhy(PL460 &device)
    : device_(device), txBusy_(false), txResultReady_(false), txResult_(0),
      rxDataReady_(false), rxParametersReady_(false), rxReady_(false) {
  memset(&rxInfo_, 0, sizeof(rxInfo_));
}

uint16_t MetersAndMorePhy::le16(const uint8_t *p) { return p[0] | (p[1] << 8); }
uint32_t MetersAndMorePhy::le32(const uint8_t *p) {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

bool MetersAndMorePhy::send(const uint8_t *data, uint16_t length,
                            const MetersAndMoreTxConfig &c) {
  if (!data || !length || length > 256 || txBusy_ || !device_.canTransmit()) return false;
  uint8_t wire[265];
  putLe32(wire, c.startTime);
  wire[4] = length; wire[5] = length >> 8;
  wire[6] = c.mode; wire[7] = c.attenuation; wire[8] = c.narrowbandFrame;
  memcpy(wire + 9, data, length);
  if (!device_.mailboxWrite(1, wire, length + 9)) return false;
  txBusy_ = true;
  delayMicroseconds(20);
  return true;
}

bool MetersAndMorePhy::poll() {
  uint8_t status[8];
  MailboxInfo mb;
  if (!device_.mailboxRead(0, status, sizeof(status), &mb)) return false;
  if (mb.flags & 0x0001U) {
    uint8_t cfm[12];
    if (!device_.mailboxRead(3, cfm, sizeof(cfm))) return false;
    txResult_ = cfm[8]; txBusy_ = false; txResultReady_ = true;
  }
  const uint16_t length = le16(status + 4);
  if ((mb.flags & 0x0002U) && length <= sizeof(rxData_)) {
    if (!device_.mailboxRead(5, rxData_, length)) return false;
    rxInfo_.length = length; rxDataReady_ = true;
  }
  if (mb.flags & 0x0008U) {
    uint8_t p[18];
    if (!device_.mailboxRead(4, p, sizeof(p))) return false;
    rxInfo_.endTime = le32(p);
    rxInfo_.frameDuration = le32(p + 4);
    rxInfo_.length = le16(p + 8);
    rxInfo_.snrHeader = static_cast<int16_t>(le16(p + 10));
    rxInfo_.snrPayload = static_cast<int16_t>(le16(p + 12));
    rxInfo_.narrowband = p[14]; rxInfo_.lqi = p[15];
    rxInfo_.rssi = p[16]; rxInfo_.crcOk = p[17];
    rxParametersReady_ = true;
  }
  if (rxDataReady_ && rxParametersReady_) rxReady_ = true;
  return true;
}

bool MetersAndMorePhy::takeTxResult(uint8_t &result) {
  if (!txResultReady_) return false;
  result = txResult_; txResultReady_ = false; return true;
}

uint16_t MetersAndMorePhy::receive(uint8_t *data, uint16_t capacity,
                                   MetersAndMoreRxInfo *info) {
  if (!rxReady_ || !data || !capacity) return 0;
  const uint16_t length = capacity < rxInfo_.length ? capacity : rxInfo_.length;
  memcpy(data, rxData_, length);
  if (info) *info = rxInfo_;
  rxReady_ = rxDataReady_ = rxParametersReady_ = false;
  return length;
}

}  // namespace pl460
