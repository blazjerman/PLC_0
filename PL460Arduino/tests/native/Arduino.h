#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

using std::size_t;
static uint32_t fakeMillis;
inline uint32_t millis() { return fakeMillis++; }
inline void delay(uint32_t ms) { fakeMillis += ms; }
inline void delayMicroseconds(uint32_t) {}
inline void yield() {}

