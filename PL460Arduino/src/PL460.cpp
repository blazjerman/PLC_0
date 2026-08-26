#include "PL460.h"
#include <string.h>

namespace pl460 {
namespace {
const uint16_t kBootWriteWord = 0x0000;
const uint16_t kBootWriteBuffer = 0x0001;
const uint16_t kBootDisableClock = 0xA66A;
const uint16_t kBootEnableWrite = 0xDE05;
const uint32_t kMiscRegister = 0x400E1800UL;
const uint32_t kMiscRun = (1UL << 16) | (1UL << 24);
const uint32_t kMiscCpuWait = kMiscRun | 1UL;
const uint16_t kKeyMask = 0xFFFE;
const uint16_t kBootKey = 0x5634 & kKeyMask;
const uint16_t kRegisterMailbox = 6;
const uint16_t kStatusMailbox = 0;
const uint16_t kRegisterEvent = 0x0008;

uint8_t directRead(const uint8_t *address) { return *address; }

void putLe32(uint8_t *p, uint32_t value) {
  p[0] = static_cast<uint8_t>(value);
  p[1] = static_cast<uint8_t>(value >> 8);
  p[2] = static_cast<uint8_t>(value >> 16);
  p[3] = static_cast<uint8_t>(value >> 24);
}
}  // namespace

PL460::PL460(Transport &transport)
    : transport_(transport), error_(Error::None), protocol_(Protocol::Unknown),
      band_(Band::Unknown), runtimeKey_(0) {}

bool PL460::begin() {
  error_ = Error::None;
  if (!transport_.begin()) {
    error_ = Error::Transport;
    return false;
  }
  transport_.setTxEnabled(false);
  return true;
}

void PL460::end() {
  transport_.setTxEnabled(false);
  transport_.end();
  runtimeKey_ = 0;
}

uint8_t PL460::imageByte(const FirmwareImage &image, uint32_t offset) const {
  FirmwareRead reader = image.read ? image.read : directRead;
  return reader(image.data + offset);
}

bool PL460::bootCommand(uint16_t command, uint32_t address,
                        const uint8_t *writeData, uint8_t *readData,
                        uint16_t length) {
  if (length > kMaxMailboxPayload) {
    error_ = Error::MailboxLength;
    return false;
  }
  putLe32(transferBuffer_, address);
  transferBuffer_[4] = static_cast<uint8_t>(command);
  transferBuffer_[5] = static_cast<uint8_t>(command >> 8);
  if (writeData && length) memcpy(transferBuffer_ + 6, writeData, length);
  else if (length) memset(transferBuffer_ + 6, 0, length);
  if (!transport_.transfer(transferBuffer_, length + 6U)) {
    error_ = Error::Transport;
    return false;
  }
  if (readData && length) memcpy(readData, transferBuffer_ + 6, length);
  return true;
}

bool PL460::waitInterrupt(bool active, uint32_t timeoutMs) {
  const uint32_t started = millis();
  while (transport_.interruptActive() != active) {
    if (millis() - started >= timeoutMs) return false;
    delayMicroseconds(50);
    yield();
  }
  return true;
}

bool PL460::boot(const FirmwareImage &image, uint32_t timeoutMs) {
  if (!image.data || !image.size) {
    error_ = Error::InvalidArgument;
    return false;
  }
  if (image.size > 96UL * 1024UL) {
    error_ = Error::FirmwareTooLarge;
    return false;
  }

  transport_.setTxEnabled(false);
  transport_.resetPulse();

  const uint8_t key1[4] = {0xBA, 0xAC, 0x45, 0x53};
  const uint8_t key2[4] = {0x45, 0x53, 0xBA, 0xAC};
  uint8_t value[4];
  if (!bootCommand(kBootEnableWrite, 0, key1, nullptr, sizeof(key1)) ||
      !bootCommand(kBootEnableWrite, 0, key2, nullptr, sizeof(key2))) return false;
  putLe32(value, kMiscCpuWait);
  if (!bootCommand(kBootWriteWord, kMiscRegister, value, nullptr, sizeof(value))) return false;

  uint32_t offset = 0;
  while (offset < image.size) {
    const uint32_t remaining = image.size - offset;
    uint16_t realLength = static_cast<uint16_t>(remaining > 512U ? 512U : remaining);
    uint16_t paddedLength = static_cast<uint16_t>((realLength + 3U) & ~3U);
    for (uint16_t i = 0; i < realLength; ++i)
      payloadBuffer_[i] = imageByte(image, offset + i);
    if (paddedLength > realLength)
      memset(payloadBuffer_ + realLength, 0, paddedLength - realLength);
    if (!bootCommand(kBootWriteBuffer, offset, payloadBuffer_, nullptr, paddedLength)) return false;
    offset += realLength;
    yield();
  }

  putLe32(value, kMiscRun);
  if (!bootCommand(kBootWriteWord, kMiscRegister, value, nullptr, sizeof(value)) ||
      !bootCommand(kBootDisableClock, 0, nullptr, nullptr, 0)) return false;

  // Firmware startup signals inactive -> active on EXTINT. Accept an already
  // completed pulse too and rely on the mailbox validation below.
  (void)waitInterrupt(false, timeoutMs < 100U ? timeoutMs : 100U);
  (void)waitInterrupt(true, timeoutMs < 500U ? timeoutMs : 500U);

  const uint32_t started = millis();
  uint8_t status[8];
  MailboxInfo mb;
  do {
    if (mailboxRead(kStatusMailbox, status, sizeof(status), &mb)) {
      const uint16_t expected = image.runtimeKey & kKeyMask;
      if (!expected || mb.key == expected) {
        runtimeKey_ = mb.key;
        protocol_ = image.protocol;
        band_ = image.band;
        error_ = Error::None;
        return true;
      }
    }
    delay(10);
    yield();
  } while (millis() - started < timeoutMs);

  runtimeKey_ = 0;
  error_ = Error::FirmwareStartTimeout;
  return false;
}

bool PL460::mailboxTransfer(bool write, uint16_t memoryId, uint8_t *data,
                            uint16_t length, MailboxInfo *info) {
  if (!data || !length || length > kMaxMailboxPayload) {
    error_ = Error::MailboxLength;
    return false;
  }
  uint16_t words = static_cast<uint16_t>((length + 1U) >> 1);
  if (write) words |= 0x8000;
  transferBuffer_[0] = static_cast<uint8_t>(memoryId >> 8);
  transferBuffer_[1] = static_cast<uint8_t>(memoryId);
  transferBuffer_[2] = static_cast<uint8_t>(words >> 8);
  transferBuffer_[3] = static_cast<uint8_t>(words);
  const uint16_t padded = static_cast<uint16_t>((length + 1U) & ~1U);
  for (uint16_t i = 0; i < padded; i += 2) {
    const uint8_t first = i < length ? data[i] : 0;
    const uint8_t second = i + 1 < length ? data[i + 1] : 0;
    if (write) {
      transferBuffer_[4 + i] = second;
      transferBuffer_[5 + i] = first;
    } else {
      transferBuffer_[4 + i] = 0;
      transferBuffer_[5 + i] = 0;
    }
  }
  if (!transport_.transfer(transferBuffer_, 4U + padded)) {
    error_ = Error::Transport;
    return false;
  }
  MailboxInfo local;
  local.key = static_cast<uint16_t>((transferBuffer_[0] << 8) | transferBuffer_[1]) & kKeyMask;
  local.flags = static_cast<uint16_t>((transferBuffer_[2] << 8) | transferBuffer_[3]);
  if (info) *info = local;
  if (!write) {
    for (uint16_t i = 0; i < length; ++i) data[i] = transferBuffer_[4 + (i ^ 1U)];
  }
  if (runtimeKey_ && local.key != runtimeKey_) {
    error_ = local.key == kBootKey ? Error::FirmwareStartTimeout : Error::UnexpectedKey;
    return false;
  }
  error_ = Error::None;
  return true;
}

bool PL460::mailboxRead(uint16_t memoryId, void *data, uint16_t length,
                        MailboxInfo *info) {
  return mailboxTransfer(false, memoryId, static_cast<uint8_t *>(data), length, info);
}

bool PL460::mailboxWrite(uint16_t memoryId, const void *data, uint16_t length,
                         MailboxInfo *info) {
  return mailboxTransfer(true, memoryId, const_cast<uint8_t *>(static_cast<const uint8_t *>(data)), length, info);
}

bool PL460::readRegister(uint32_t address, void *data, uint16_t length,
                         uint32_t timeoutMs) {
  if (!data || !length || length > 0x1FF) {
    error_ = Error::InvalidArgument;
    return false;
  }
  uint8_t request[8] = {
      static_cast<uint8_t>(address >> 24), static_cast<uint8_t>(address >> 16),
      static_cast<uint8_t>(address >> 8), static_cast<uint8_t>(address),
      static_cast<uint8_t>(length >> 8), static_cast<uint8_t>(length), 0, 0};
  if (!mailboxWrite(kRegisterMailbox, request, sizeof(request))) return false;
  const uint32_t started = millis();
  uint8_t status[8];
  MailboxInfo mb;
  while (millis() - started < timeoutMs) {
    if (mailboxRead(kStatusMailbox, status, sizeof(status), &mb) &&
        (mb.flags & kRegisterEvent)) {
      const uint16_t responseLength = static_cast<uint16_t>(status[6] | (status[7] << 8));
      if (responseLength < length) {
        error_ = Error::MailboxLength;
        return false;
      }
      return mailboxRead(kRegisterMailbox, data, length);
    }
    delayMicroseconds(100);
    yield();
  }
  error_ = Error::Timeout;
  return false;
}

bool PL460::writeRegister(uint32_t address, const void *data, uint16_t length) {
  if (!data || !length || length > kMaxMailboxPayload - 6 || length > 0x1FF) {
    error_ = Error::InvalidArgument;
    return false;
  }
  payloadBuffer_[0] = static_cast<uint8_t>(address >> 24);
  payloadBuffer_[1] = static_cast<uint8_t>(address >> 16);
  payloadBuffer_[2] = static_cast<uint8_t>(address >> 8);
  payloadBuffer_[3] = static_cast<uint8_t>(address);
  const uint16_t command = static_cast<uint16_t>(0x0400U | length);
  payloadBuffer_[4] = static_cast<uint8_t>(command >> 8);
  payloadBuffer_[5] = static_cast<uint8_t>(command);
  memcpy(payloadBuffer_ + 6, data, length);
  const bool ok = mailboxWrite(kRegisterMailbox, payloadBuffer_, length + 6);
  if (ok) delayMicroseconds(50);
  return ok;
}

bool PL460::getBoardInfo(BoardInfo &info) {
  memset(&info, 0, sizeof(info));
  const uint32_t base = 0x80000000UL;
  bool ok = readRegister(base + 0, &info.productId, sizeof(info.productId));
  ok = readRegister(base + 1, &info.model, sizeof(info.model)) && ok;
  ok = readRegister(base + 2, info.version, 16) && ok;
  info.version[16] = '\0';
  ok = readRegister(base + 3, &info.versionNumber, sizeof(info.versionNumber)) && ok;
  if (protocol_ == Protocol::G3Phy)
    (void)readRegister(base + 0x50, &info.band, sizeof(info.band));
  else
    info.band = static_cast<uint8_t>(band_);
  error_ = ok ? Error::None : error_;
  return ok;
}

bool PL460::communicationTest(uint16_t expectedKey) {
  uint8_t status[8];
  MailboxInfo info;
  const bool ok = mailboxRead(kStatusMailbox, status, sizeof(status), &info);
  if (!ok) return false;
  if (expectedKey && info.key != (expectedKey & kKeyMask)) {
    error_ = Error::UnexpectedKey;
    return false;
  }
  return true;
}

bool PL460::canTransmit() const {
  return runtimeKey_ != 0 && !transport_.thermalAlarm() && transport_.supplyGood();
}

void PL460::enableTransmitter(bool enabled) {
  if (!enabled) {
    transport_.setTxEnabled(false);
    return;
  }
  if (transport_.thermalAlarm()) {
    transport_.setTxEnabled(false);
    error_ = Error::ThermalAlarm;
    return;
  }
  if (!transport_.supplyGood()) {
    transport_.setTxEnabled(false);
    error_ = Error::SupplyUnsafe;
    return;
  }
  transport_.setTxEnabled(runtimeKey_ != 0);
  error_ = runtimeKey_ ? Error::None : Error::FirmwareStartTimeout;
}

const char *PL460::lastErrorString() const {
  switch (error_) {
    case Error::None: return "none";
    case Error::InvalidArgument: return "invalid argument";
    case Error::Transport: return "SPI transport failed";
    case Error::FirmwareTooLarge: return "firmware exceeds PL460 memory";
    case Error::FirmwareStartTimeout: return "firmware did not start";
    case Error::UnexpectedKey: return "unexpected mailbox key";
    case Error::MailboxLength: return "invalid mailbox length";
    case Error::ThermalAlarm: return "thermal alarm";
    case Error::SupplyUnsafe: return "amplifier supply unsafe";
    case Error::Busy: return "busy";
    case Error::Timeout: return "timeout";
    case Error::UnsupportedFirmware: return "unsupported firmware";
  }
  return "unknown";
}

}  // namespace pl460
