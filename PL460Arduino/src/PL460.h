#pragma once

#include <Arduino.h>
#include "PL460Transport.h"

namespace pl460 {

enum class Protocol : uint8_t {
  G3Phy,
  G3MacRt,
  Prime,
  PrimeTwoChannel,
  MetersAndMore,
  Unknown
};

enum class Band : uint8_t { CenelecA, CenelecB, FCC, ARIB, Multiband, Unknown };

enum class Error : uint8_t {
  None,
  InvalidArgument,
  Transport,
  FirmwareTooLarge,
  FirmwareStartTimeout,
  UnexpectedKey,
  MailboxLength,
  ThermalAlarm,
  SupplyUnsafe,
  Busy,
  Timeout,
  UnsupportedFirmware
};

typedef uint8_t (*FirmwareRead)(const uint8_t *address);

struct FirmwareImage {
  const uint8_t *data;
  uint32_t size;
  Protocol protocol;
  Band band;
  uint16_t runtimeKey;
  const char *name;
  FirmwareRead read;
};

struct MailboxInfo {
  uint16_t key;
  uint16_t flags;
};

struct BoardInfo {
  uint16_t productId;
  uint16_t model;
  uint32_t versionNumber;
  uint8_t band;
  char version[17];
};

class PL460 {
 public:
  static const uint16_t kPhyRuntimeKey = 0x1122;
  static const uint16_t kMacRtRuntimeKey = 0x5A5A;
  static const uint16_t kMaxMailboxPayload = 630;

  explicit PL460(Transport &transport);

  bool begin();
  void end();
  bool boot(const FirmwareImage &image, uint32_t timeoutMs = 3000);
  bool mailboxRead(uint16_t memoryId, void *data, uint16_t length,
                   MailboxInfo *info = nullptr);
  bool mailboxWrite(uint16_t memoryId, const void *data, uint16_t length,
                    MailboxInfo *info = nullptr);
  bool readRegister(uint32_t address, void *data, uint16_t length,
                    uint32_t timeoutMs = 250);
  bool writeRegister(uint32_t address, const void *data, uint16_t length);
  bool getBoardInfo(BoardInfo &info);
  bool communicationTest(uint16_t expectedKey = 0);

  bool canTransmit() const;
  void enableTransmitter(bool enabled);
  uint16_t runtimeKey() const { return runtimeKey_; }
  Protocol protocol() const { return protocol_; }
  Band band() const { return band_; }
  Error lastError() const { return error_; }
  const char *lastErrorString() const;

 private:
  bool bootCommand(uint16_t command, uint32_t address, const uint8_t *writeData,
                   uint8_t *readData, uint16_t length);
  bool mailboxTransfer(bool write, uint16_t memoryId, uint8_t *data,
                       uint16_t length, MailboxInfo *info);
  uint8_t imageByte(const FirmwareImage &image, uint32_t offset) const;
  bool waitInterrupt(bool active, uint32_t timeoutMs);
  void setError(Error error) { error_ = error; }

  Transport &transport_;
  Error error_;
  Protocol protocol_;
  Band band_;
  uint16_t runtimeKey_;
  uint8_t transferBuffer_[kMaxMailboxPayload + 6];
  uint8_t payloadBuffer_[kMaxMailboxPayload + 6];
};

}  // namespace pl460
