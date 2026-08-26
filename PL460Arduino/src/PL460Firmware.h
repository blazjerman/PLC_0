#pragma once

#include "PL460.h"

#if defined(ARDUINO_ARCH_AVR)
#include <avr/pgmspace.h>
#define PL460_FIRMWARE_STORAGE PROGMEM
#else
#define PL460_FIRMWARE_STORAGE
#endif

namespace pl460 {

inline uint8_t readFirmwareByte(const uint8_t *address) {
#if defined(ARDUINO_ARCH_AVR)
  return pgm_read_byte(address);
#else
  return *address;
#endif
}

}  // namespace pl460
