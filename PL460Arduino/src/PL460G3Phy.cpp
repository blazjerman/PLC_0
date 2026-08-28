#include "PL460G3Phy.h"
#include <string.h>

namespace pl460 {
namespace {
const uint16_t kStatus = 0;
const uint16_t kTxParameters = 1;
const uint16_t kTxData = 2;
const uint16_t kTxConfirm = 3;
const uint16_t kRxParameters = 4;
const uint16_t kRxData = 5;
const uint16_t kTxConfirmFlag = 0x0001;
const uint16_t kRxDataFlag = 0x0002;
const uint16_t kRxParametersFlag = 0x0010;
const uint16_t kRxParametersLength = 117;
const uint32_t kRegisterBase = 0x80000000UL;
const uint16_t kCrcTxRxCapability = 0x401C;
const uint16_t kAutoDetectImpedance = 0x401E;
const uint16_t kImpedance = 0x401F;

void putLe32(uint8_t *p, uint32_t value) {
  p[0] = static_cast<uint8_t>(value);
  p[1] = static_cast<uint8_t>(value >> 8);
  p[2] = static_cast<uint8_t>(value >> 16);
  p[3] = static_cast<uint8_t>(value >> 24);
}
}  // namespace

G3TxConfig G3TxConfig::cenelecARobust() {
  G3TxConfig c;
  memset(&c, 0, sizeof(c));
  c.toneMap[0] = 0x3F;
  // Forced + relative avoids scheduling an absolute transmission at timestamp
  // zero, which is already in the past once firmware boot has completed.
  c.mode = 0x03;
  c.modulation = G3Modulation::RobustBPSK;
  c.scheme = G3Scheme::Differential;
  c.delimiter = G3Delimiter::SofNoResponse;
  return c;
}

G3Phy::G3Phy(PL460 &device)
    : device_(device), txBusy_(false), txConfirmReady_(false), rxReady_(false),
      rxDataReady_(false), rxParametersReady_(false) {
  memset(&txConfirm_, 0, sizeof(txConfirm_));
  memset(&rxInfo_, 0, sizeof(rxInfo_));
}

bool G3Phy::send(const uint8_t *data, uint16_t length, const G3TxConfig &config) {
  if (!data || !length || length > 512U || txBusy_) return false;
  if (!device_.canTransmit()) return false;

  uint8_t wire[40];
  uint8_t *p = wire;
  putLe32(p, config.startTime); p += 4;
  *p++ = static_cast<uint8_t>(length);
  *p++ = static_cast<uint8_t>(length >> 8);
  memcpy(p, config.preemphasis, 24); p += 24;
  memcpy(p, config.toneMap, 3); p += 3;
  *p++ = config.mode;
  *p++ = config.attenuation;
  *p++ = static_cast<uint8_t>(config.modulation);
  *p++ = static_cast<uint8_t>(config.scheme);
  *p++ = config.phaseDetectorCounter;
  *p++ = config.twoReedSolomonBlocks ? 1 : 0;
  *p++ = static_cast<uint8_t>(config.delimiter);

  if (!device_.mailboxWrite(kTxParameters, wire, sizeof(wire))) return false;
  delayMicroseconds(200);
  if (!device_.mailboxWrite(kTxData, data, length)) return false;
  txBusy_ = true;
  txConfirmReady_ = false;
  return true;
}

bool G3Phy::enableCrc(bool enabled) {
  const uint8_t value = enabled ? 1 : 0;
  return device_.writeRegister(kRegisterBase + (kCrcTxRxCapability & 0x0FFFU),
                               &value, 1);
}

bool G3Phy::setImpedance(G3Impedance impedance, bool autoDetect) {
  const uint8_t automatic = autoDetect ? 1 : 0;
  if (!device_.writeRegister(kRegisterBase + (kAutoDetectImpedance & 0x0FFFU),
                             &automatic, 1)) {
    return false;
  }
  const uint8_t value = static_cast<uint8_t>(impedance);
  return device_.writeRegister(kRegisterBase + (kImpedance & 0x0FFFU),
                               &value, 1);
}

uint16_t G3Phy::getLe16(const uint8_t *p) {
  return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

uint32_t G3Phy::getLe32(const uint8_t *p) {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

void G3Phy::parseRxInfo(const uint8_t *p) {
  rxInfo_.endTime = getLe32(p); p += 4;
  rxInfo_.frameDuration = getLe32(p); p += 4;
  rxInfo_.rssi = getLe16(p); p += 2;
  rxInfo_.length = getLe16(p); p += 2;
  p++;  // zero-cross difference
  rxInfo_.correctedErrors = *p++;
  rxInfo_.modulation = static_cast<G3Modulation>(*p++);
  rxInfo_.scheme = static_cast<G3Scheme>(*p++);
  p += 4 + 2 + 2 + 1 + 1 + 2;
  rxInfo_.snrPayload = static_cast<int16_t>(getLe16(p)); p += 2;
  p += 2 + 2 + 5;
  rxInfo_.lqi = *p++;
  rxInfo_.delimiter = static_cast<G3Delimiter>(*p++);
  rxInfo_.crcOk = *p++;
  memcpy(rxInfo_.toneMap, p, 3);
}

bool G3Phy::poll() {
  uint8_t status[8];
  MailboxInfo info;
  if (!device_.mailboxRead(kStatus, status, sizeof(status), &info)) return false;

  if (info.flags & kTxConfirmFlag) {
    uint8_t wire[12];
    if (!device_.mailboxRead(kTxConfirm, wire, sizeof(wire))) return false;
    txConfirm_.rms = getLe32(wire);
    txConfirm_.endTime = getLe32(wire + 4);
    txConfirm_.result = wire[8];
    txBusy_ = false;
    txConfirmReady_ = true;
  }

  const uint16_t rxLength = getLe16(status + 4);
  if ((info.flags & kRxDataFlag) && rxLength <= sizeof(rxData_)) {
    if (!device_.mailboxRead(kRxData, rxData_, rxLength)) return false;
    rxInfo_.length = rxLength;
    rxDataReady_ = true;
  }
  if (info.flags & kRxParametersFlag) {
    uint8_t wire[kRxParametersLength];
    if (!device_.mailboxRead(kRxParameters, wire, sizeof(wire))) return false;
    parseRxInfo(wire);
    rxParametersReady_ = true;
  }
  if (rxDataReady_ && rxParametersReady_) {
    rxReady_ = true;
  }
  return true;
}

bool G3Phy::takeTxConfirm(G3TxConfirm &confirm) {
  if (!txConfirmReady_) return false;
  confirm = txConfirm_;
  txConfirmReady_ = false;
  return true;
}

uint16_t G3Phy::receive(uint8_t *data, uint16_t capacity, G3RxInfo *info) {
  if (!rxReady_ || !data || !capacity) return 0;
  const uint16_t length = capacity < rxInfo_.length ? capacity : rxInfo_.length;
  memcpy(data, rxData_, length);
  if (info) *info = rxInfo_;
  rxReady_ = false;
  rxDataReady_ = false;
  rxParametersReady_ = false;
  return length;
}

}  // namespace pl460
