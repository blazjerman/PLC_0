#include "PL460G3Coupling.h"

namespace pl460 {
namespace {

const uint32_t kRegisterBase = 0x80000000UL;

const uint32_t kRmsHigh[8] = {1991, 1381, 976, 695, 495, 351, 250, 179};
const uint32_t kRmsVLow[8] = {6356, 4706, 3317, 2308, 1602, 1112, 778, 546};
const uint32_t kThresholdHigh[16] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1685, 1173, 828, 589, 419, 298, 212, 151};
const uint32_t kThresholdVLow[16] = {
    0, 0, 0, 0, 0, 0, 0, 0, 8988, 6370, 4466, 3119, 2171, 1512, 1061, 752};
const uint32_t kDacc[17] = {
    0x00000000UL, 0x21200000UL, 0x073F0000UL, 0x3F3F0000UL,
    0x00000CCCUL, 0x00000000UL, 0xA20000FFUL, 0x14141414UL,
    0x20200000UL, 0x00004400UL, 0x0FD20004UL, 0x000003AAUL,
    0xF0000000UL, 0x001020F0UL, 0x000003AAUL, 0xF0000000UL,
    0x001020FFUL};
const uint16_t kGainHigh[3] = {142, 70, 336};
const uint16_t kGainVLow[3] = {474, 230, 597};
const uint16_t kPredistHigh[36] = {
    0x670A, 0x660F, 0x676A, 0x6A6B, 0x6F3F, 0x7440, 0x74ED, 0x7792,
    0x762D, 0x7530, 0x7938, 0x7C0A, 0x7C2A, 0x7B0E, 0x7AF2, 0x784B,
    0x7899, 0x76F9, 0x76D6, 0x769F, 0x775D, 0x70C0, 0x6EB9, 0x6F18,
    0x6F1E, 0x6FA2, 0x6862, 0x67C9, 0x68F9, 0x68A5, 0x6CA3, 0x7153,
    0x7533, 0x750B, 0x7B59, 0x7FFF};
const uint16_t kPredistVLow[36] = {
    0x7FFF, 0x7DB1, 0x7CE6, 0x7B36, 0x772F, 0x7472, 0x70AA, 0x6BC2,
    0x682D, 0x6618, 0x6384, 0x6210, 0x61D7, 0x6244, 0x6269, 0x63A8,
    0x6528, 0x65CC, 0x67F6, 0x693B, 0x6B13, 0x6C29, 0x6D43, 0x6E26,
    0x6D70, 0x6C94, 0x6BB5, 0x6AC9, 0x6A5F, 0x6B65, 0x6B8C, 0x6A62,
    0x6CEC, 0x6D5A, 0x6F9D, 0x6FD3};

bool writeBytes(PL460 &device, uint16_t pibId, const uint8_t *data,
                uint16_t length, uint16_t guardUs = 50) {
  const bool ok = device.writeRegister(kRegisterBase + (pibId & 0x0FFFU), data, length);
  if (ok && guardUs > 50) delayMicroseconds(guardUs - 50);
  return ok;
}

bool writeU8(PL460 &device, uint16_t id, uint8_t value) {
  return writeBytes(device, id, &value, 1);
}

bool writeU16Array(PL460 &device, uint16_t id, const uint16_t *values,
                   uint8_t count, uint16_t guardUs = 50) {
  uint8_t wire[72];
  for (uint8_t i = 0; i < count; ++i) {
    wire[i * 2] = static_cast<uint8_t>(values[i]);
    wire[i * 2 + 1] = static_cast<uint8_t>(values[i] >> 8);
  }
  return writeBytes(device, id, wire, static_cast<uint16_t>(count * 2), guardUs);
}

bool writeU32Array(PL460 &device, uint16_t id, const uint32_t *values,
                   uint8_t count) {
  uint8_t wire[68];
  for (uint8_t i = 0; i < count; ++i) {
    wire[i * 4] = static_cast<uint8_t>(values[i]);
    wire[i * 4 + 1] = static_cast<uint8_t>(values[i] >> 8);
    wire[i * 4 + 2] = static_cast<uint8_t>(values[i] >> 16);
    wire[i * 4 + 3] = static_cast<uint8_t>(values[i] >> 24);
  }
  return writeBytes(device, id, wire, static_cast<uint16_t>(count * 4));
}

}  // namespace

bool configureG3CenelecARev5(PL460 &device) {
  // PIB identifiers match the current Microchip Harmony G3 PHY firmware.
  bool ok = writeU8(device, 0x4045, 8);  // Rev5 auxiliary CENELEC-A branch
  ok = writeU8(device, 0x4032, 8) && ok;
  ok = writeU32Array(device, 0x4030, kDacc, 17) && ok;
  ok = writeU32Array(device, 0x4025, kRmsHigh, 8) && ok;
  ok = writeU32Array(device, 0x4026, kRmsVLow, 8) && ok;
  ok = writeU32Array(device, 0x4027, kThresholdHigh, 16) && ok;
  ok = writeU32Array(device, 0x4029, kThresholdVLow, 16) && ok;
  ok = writeU16Array(device, 0x402D, kGainHigh, 3) && ok;
  ok = writeU16Array(device, 0x402F, kGainVLow, 3) && ok;
  ok = writeU16Array(device, 0x402A, kPredistHigh, 36, 250) && ok;
  ok = writeU16Array(device, 0x402C, kPredistVLow, 36, 350) && ok;
  return ok;
}

}  // namespace pl460
