#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

#include "PL460.h"
#include "PL460G3Phy.h"
#include "PL460MetersAndMore.h"
#include "PL460Prime.h"

using namespace pl460;

class MockTransport : public Transport {
 public:
  bool begin() override { return true; }
  void end() override {}
  void resetPulse() override { reset = true; }
  bool transfer(uint8_t *data, size_t length) override {
    frames.emplace_back(data, data + length);
    if (runtime) {
      data[0] = 0x11;
      data[1] = 0x22;
      data[2] = 0;
      data[3] = 0;
      for (size_t i = 4; i < length; ++i) data[i] = 0;
    } else if (length == 6 && data[4] == 0x6A && data[5] == 0xA6) {
      runtime = true;
    }
    return true;
  }
  bool interruptActive() const override { return runtime; }
  bool thermalAlarm() const override { return false; }
  bool supplyGood() const override { return true; }
  void setTxEnabled(bool enabled) override { txEnabled = enabled; }

  bool reset = false;
  bool runtime = false;
  bool txEnabled = false;
  std::vector<std::vector<uint8_t>> frames;
};

int main() {
  MockTransport transport;
  PL460 modem(transport);
  assert(modem.begin());
  const uint8_t tinyFirmware[] = {1, 2, 3, 4};
  const FirmwareImage image = {tinyFirmware, sizeof(tinyFirmware), Protocol::G3Phy,
                               Band::CenelecA, PL460::kPhyRuntimeKey, "test", nullptr};
  assert(modem.boot(image));
  assert(transport.reset);
  assert(transport.frames.size() >= 7);
  assert(transport.frames[0][4] == 0x05 && transport.frames[0][5] == 0xDE);
  assert(transport.frames[0][6] == 0xBA && transport.frames[0][9] == 0x53);
  assert(transport.frames[3][4] == 0x01 && transport.frames[3][5] == 0x00);
  assert(transport.frames[3][6] == 1 && transport.frames[3][9] == 4);

  const uint8_t payload[] = {0xAA, 0xBB, 0xCC, 0xDD};
  assert(modem.mailboxWrite(0x1234, payload, sizeof(payload)));
  const auto &mailbox = transport.frames.back();
  assert(mailbox[0] == 0x12 && mailbox[1] == 0x34);
  assert(mailbox[2] == 0x80 && mailbox[3] == 0x02);
  assert(mailbox[4] == 0xBB && mailbox[5] == 0xAA);
  assert(mailbox[6] == 0xDD && mailbox[7] == 0xCC);

  modem.enableTransmitter(true);
  assert(transport.txEnabled);
  G3Phy g3(modem);
  const uint8_t message[] = {'O', 'K'};
  assert(g3.send(message, sizeof(message)));
  assert(transport.frames[transport.frames.size() - 2][0] == 0);
  assert(transport.frames[transport.frames.size() - 2][1] == 1);
  assert(transport.frames.back()[1] == 2);

  MockTransport primeTransport;
  PL460 primeModem(primeTransport);
  assert(primeModem.begin() && primeModem.boot(image));
  primeModem.enableTransmitter(true);
  PrimePhy prime(primeModem);
  assert(prime.send(message, sizeof(message)));
  assert(primeTransport.frames.back()[1] == 1);

  MockTransport mmTransport;
  PL460 mmModem(mmTransport);
  assert(mmModem.begin() && mmModem.boot(image));
  mmModem.enableTransmitter(true);
  MetersAndMorePhy mm(mmModem);
  assert(mm.send(message, sizeof(message)));
  assert(mmTransport.frames.back()[1] == 1);
  return 0;
}
