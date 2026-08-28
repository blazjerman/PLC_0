# PL460Arduino

Experimental Arduino host library for the Microchip PL460 power-line
communication modem and the PL460-EK revision 5 evaluation kit.

The library can upload PL460 firmware over SPI, communicate through the PL460
mailboxes, configure the revision 5 CENELEC-A coupling circuit, and exchange
raw PLC PHY frames. It has been tested end-to-end with two PL460-EK rev5 boards
and an ESP32-S3 using G3-PLC CENELEC-A.

> **Unofficial project:** this port is not produced, reviewed, supported, or
> endorsed by Microchip Technology Inc. It is an experimental engineering
> project, not a certified modem or a complete G3/PRIME product.

## Verified status

| Capability | Status |
|---|---|
| ESP32-S3 SPI transport with custom pins | Hardware verified |
| PL460 bootloader and runtime mailbox | Hardware verified |
| G3 PHY CENELEC-A firmware boot | Hardware verified |
| PL460-EK rev5 CENELEC-A auxiliary coupling calibration | Hardware verified |
| G3 PHY forced P2P transmit and receive | Hardware verified with two boards |
| G3 PHY CRC, RSSI, LQI, SNR and TX RMS reporting | Hardware verified |
| G3 CENELEC-B, FCC and multiband images | Included; not hardware verified here |
| G3 MAC-RT images and raw mailbox access | Boot support; typed MAC API not implemented |
| PRIME PHY typed P2P API | Experimental; protocol-specific rev5 calibration pending |
| Meters & More PHY typed P2P API | Experimental; protocol-specific rev5 calibration pending |
| Complete G3 IPv6/6LoWPAN/UDP stack on ESP32 | Not included |
| SAMD, STM32, RP2040 and Renesas Arduino cores | Designed to be portable; not hardware verified |

The successful G3 test produced local TX RMS values around 8600 and received
`Hello world!` with PHY CRC validation. Actual values depend on the coupling,
line impedance, supply, attenuation and test setup.

## What the library provides

- PL460 bootloader reset, firmware upload and runtime-key validation.
- Standard Arduino `SPI`, GPIO, timing and optional ADC transport.
- Explicit SPI-pin selection for boards such as ESP32-S3.
- Raw mailbox and virtual-register read/write APIs.
- Firmware communication tests and board-information requests.
- Typed G3 PHY, PRIME PHY and Meters & More PHY frame APIs.
- Microchip's PL460-EK rev5 G3 CENELEC-A line-driver, DACC, RMS, gain,
  threshold and predistortion configuration.
- G3 impedance selection and PHY CRC control.
- Optional thermal-warning and amplifier-supply transmit interlocks.
- Opt-in firmware headers for all included personalities.
- Single-board diagnostics and two-board examples.

## How the PL460 works

The PL460 does not retain its communication firmware in application flash.
After reset, the Arduino host must upload one firmware image and then use the
mailbox layout belonging to that image.

G3 PHY, G3 MAC-RT, PRIME and Meters & More are separate firmware personalities;
they do not operate simultaneously. Reset and boot the corresponding image
when changing protocol.

This library directly supports raw PHY frames. Raw G3 PHY communication is not
the same as a complete G3 network. Microchip's complete G3 stack adds MAC,
6LoWPAN, IPv6, UDP, coordinator/device management and security. Its precompiled
host libraries target supported Arm Cortex-M devices and cannot be linked
directly into Xtensa or RISC-V ESP32 firmware. A full G3 ESP32 design therefore
needs either a supported Arm host or a serialized Microchip modem running on a
supported host MCU.

## Hardware requirements

- Microchip PL460 or PL460-EK revision 5.
- Arduino-compatible 3.3 V host with SPI and enough flash for the selected
  firmware image. The verified host is ESP32-S3.
- Regulated 3.3 V logic supply.
- Separate PL460-EK J4 supply: **15 V +/-5%, approximately 12 W capability**.
- A safe, isolated PLC test bus and appropriate measurement equipment.

Do not power J4 from the ESP32. Do not apply 5 V logic to the PL460 host pins.

## Safety warning

Power-line communication hardware may be connected to lethal mains voltage.
Develop first on a current-limited, isolated AC or DC PLC test bus. Follow the
PL460-EK user guide, local electrical regulations, clearance requirements and
proper isolation practices.

Never connect the earth clip of an ordinary grounded oscilloscope to a live
PLC conductor. Use a correctly rated isolated or differential probe. This
software does not provide electrical isolation, regulatory compliance,
functional safety or protection against incorrect wiring.

## Verified ESP32-S3 wiring

| PL460-EK J2 | Signal | ESP32-S3 GPIO |
|---:|---|---:|
| 2, 19 | GND | GND |
| 20 | 3V3 logic supply | 3V3 |
| 4 | XPL_TXEN | 6 |
| 5 | XPL_NRST | 3 |
| 6 | XPL_ENABLE / LDO_EN (optional R71) | 4 |
| 9 | XPL_EXTIN / IRQ, active low | 2 |
| 10 | XPL_NTHW0, active-low thermal warning | 7 |
| 13 | XPL_STBY (optional R68) | 5 |
| 15 | XPL_CS | 10 |
| 16 | MOSI | 11 |
| 17 | MISO | 13 |
| 18 | SCK | 12 |

J2 pin 3 is the optional analog supply monitor. To use it, pass the connected
ADC GPIO and a calibrated raw threshold as the final two `Pins` constructor
arguments. A threshold of zero disables the ADC check. Verify the actual rev5
resistor population: R57, R68 and R71 are not fitted by default, and R80 affects
the supply-monitor/TX-enable routing.

The original ESP32 commonly reserves GPIO6-GPIO11 for integrated flash. This
mapping is for the **ESP32-S3 board used during testing**. Do not copy it to an
ESP32-WROOM or another board without checking its flash pins and schematic.

## Transport construction

The five control pins and three SPI bus pins are separate:

```cpp
pl460::Pins controlPins(10, 3, 2, 6, 7);
//                        CS RESET IRQ TX_EN NTHW0

pl460::SpiPins spiPins(12, 13, 11);
//                       SCK MISO MOSI

pl460::ArduinoTransport transport(SPI, controlPins, spiPins, 2000000UL);
pl460::PL460 modem(transport);
```

`SpiPins(12, 13, 11)` does not replace the control pins. It only tells the
Arduino SPI implementation which SCK, MISO and MOSI pins to use.

Before `modem.begin()`, set `LDO_EN` high and `STBY` low if those signals are
connected on the board:

```cpp
pinMode(4, OUTPUT);
digitalWrite(4, HIGH);
pinMode(5, OUTPUT);
digitalWrite(5, LOW);
```

## Installation

### Arduino IDE

1. Download or clone the repository.
2. Copy the `PL460Arduino` directory into the Arduino sketchbook `libraries`
   directory. The resulting path should end in `libraries/PL460Arduino`.
3. Restart Arduino IDE.
4. Select the ESP32-S3 board and open an example from
   **File > Examples > PL460Arduino**.

To distribute this directory as a ZIP, zip the `PL460Arduino` directory itself,
not the parent repository containing the original G3 SDK.

### Arduino CLI

```sh
arduino-cli compile \
  --fqbn esp32:esp32:esp32s3 \
  --libraries ./PL460Arduino \
  ./PL460Arduino/examples/G3HelloWorld
```

## G3 Hello World

Open `examples/G3HelloWorld/G3HelloWorld.ino`.

On the first board use:

```cpp
#define ROLE_SENDER 1
```

Upload the sketch, then change the setting for the second board:

```cpp
#define ROLE_SENDER 0
```

Connect both PL460-EK boards to the same safe, isolated PLC bus. The sender
transmits a null-terminated `Hello world!` every two seconds. Typical sender
output is:

```text
PL460-EK rev5 G3 Hello World
Role: SENDER
Ready. TX_EN=1 NTHW0=1
Sent: Hello world!
TX confirmation: result=1 RMS=8616
```

Typical receiver output is:

```text
PL460-EK rev5 G3 Hello World
Role: RECEIVER
Ready. TX_EN=0 NTHW0=1
Received: Hello world! | bytes=14 RSSI=117 dBuV LQI=139 CRC=OK
```

The receive byte count may contain one alignment byte. `CRC=OK` is the PHY
integrity result.

## Minimal G3 initialization

For PL460-EK rev5 CENELEC-A, booting firmware is not enough. Apply the coupling
tables before transmitting:

```cpp
#include <PL460.h>
#include <PL460G3Coupling.h>
#include <PL460G3Phy.h>
#include <firmware/PLC_PHY_G3_CENA.h>

pl460::G3Phy phy(modem);

if (!modem.begin()) {
  // Handle transport failure.
}
if (!modem.boot(pl460::PLC_PHY_G3_CENA_IMAGE)) {
  // Handle firmware failure.
}
if (!pl460::configureG3CenelecARev5(modem)) {
  // Handle coupling-configuration failure.
}
if (!phy.setImpedance(pl460::G3Impedance::VeryLow, false)) {
  // Handle impedance-configuration failure.
}
if (!phy.enableCrc(true)) {
  // Handle CRC-configuration failure.
}

modem.enableTransmitter(true);
```

Without the revision 5 coupling configuration, the firmware can return a
successful TX confirmation while producing only a very small analog signal.
The local TX confirmation means that the PHY completed its operation; it is
not an acknowledgment from another modem.

## Examples

| Example | Purpose | Receiver required? |
|---|---|---:|
| `G3HelloWorld` | Smallest G3 text sender/receiver | Two boards for end-to-end test |
| `G3PhyP2P` | Sequenced G3 PHY packets with link information | Yes |
| `G3PhyFullDebug` | Boot, mailbox, pins, calibration, TX/RX and heartbeat diagnostics | No for local TX test |
| `BoardDiagnostics` | Boot, mailbox, board info, safety and calibration check | No |
| `SingleBoardSelfTest` | Non-transmitting periodic mailbox test | No |
| `AllFirmwareBootTest` | Boot every included firmware personality without transmitting | No |
| `FirmwareProtocolLoader` | Minimal one-image firmware loader | No |
| `PrimeP2P` | Experimental raw PRIME PHY frames | Yes; not yet hardware-verified |
| `MetersAndMoreP2P` | Experimental raw Meters & More PHY frames | Yes; not yet hardware-verified |

In sender/receiver examples, compile one board with `ROLE_SENDER=1` and the
other with `ROLE_SENDER=0`. If both serial terminals print `TX:`, both boards
were compiled as senders.

## Firmware images

Firmware headers are opt-in. Include exactly the image used by the sketch to
avoid consuming flash for unused personalities.

| Header | Personality | Band |
|---|---|---|
| `PLC_PHY_G3_CENA.h` | G3 PHY | CENELEC-A |
| `PLC_PHY_G3_CENB.h` | G3 PHY | CENELEC-B |
| `PLC_PHY_G3_FCC.h` | G3 PHY | FCC |
| `PLC_PHY_G3_MULTIBAND.h` | G3 PHY | Multiband |
| `G3_MAC_RT_CENA.h` | G3 MAC-RT | CENELEC-A |
| `G3_MAC_RT_CENB.h` | G3 MAC-RT | CENELEC-B |
| `G3_MAC_RT_FCC.h` | G3 MAC-RT | FCC |
| `G3_MAC_RT_MULTIBAND.h` | G3 MAC-RT | Multiband |
| `PLC_PHY_PRIME.h` | PRIME PHY | Default PRIME channel |
| `PLC_PHY_PRIME_2CHN.h` | PRIME PHY | Two-channel firmware |
| `PLC_PHY_MM.h` | Meters & More PHY | CENELEC |

The generated headers correspond byte-for-byte to the `.bin` files in
Microchip's `smartenergy` repository at commit
`91b27a76a301a39b9d04fb513ac7ca8acd047ab2`. They can be regenerated with:

```sh
python3 tools/generate_firmware_headers.py INPUT.bin src/firmware/OUTPUT.h
```

Do not substitute firmware for another PL360/PL460 board revision without
checking the firmware release notes, mailbox ABI and coupling configuration.

## API overview

### `pl460::ArduinoTransport`

- Configures reset, IRQ, CS, TX enable and safety pins.
- Starts SPI with default or explicit bus pins.
- Uses SPI mode 0 and MSB-first transfers.
- Implements the portable `pl460::Transport` interface.

### `pl460::PL460`

- `begin()` / `end()` - start or stop the host transport.
- `boot(image)` - reset, upload and start one firmware image.
- `communicationTest()` - verify runtime mailbox communication.
- `mailboxRead()` / `mailboxWrite()` - raw firmware mailbox access.
- `readRegister()` / `writeRegister()` - virtual PIB/register access.
- `getBoardInfo()` - request product, model and firmware information.
- `enableTransmitter()` / `canTransmit()` - control and check TX safety.
- `lastError()` / `lastErrorString()` - inspect the latest driver error.

### `pl460::G3Phy`

- `send()` and `takeTxConfirm()`.
- `poll()`, `available()` and `receive()`.
- CENELEC-A robust-BPSK defaults.
- TX RMS/result and RX RSSI/LQI/SNR/CRC metadata.
- `setImpedance()` and `enableCrc()`.

### PRIME and Meters & More

`pl460::PrimePhy` and `pl460::MetersAndMorePhy` expose experimental raw PHY
frame APIs. Their mailbox code compiles, but the rev5 protocol-specific
calibration and end-to-end behavior have not yet been validated. Do not treat
these APIs as production-ready or as full network stacks.

## Troubleshooting

### Firmware boots and mailbox passes, but the signal is tiny

Confirm that `configureG3CenelecARev5()` succeeds after every G3 firmware boot.
On rev5, CENELEC-A uses the auxiliary coupling branch. `TXCAUX` may pulse while
`TXMAIN` remains off. Short packets can make the LED difficult to see.

### `TX confirmation: result=1`

The local PHY completed transmission. It does not prove another modem received
the frame. Use a receiver and require `CRC=OK` for an end-to-end result.

### `CRC=255`

Value `0xFF` means PHY CRC capability is disabled, not a bad CRC. The G3 P2P
examples enable CRC on both boards.

### Receiver length is one byte larger

The PHY/mailbox transport can report an even padded size. Use an application
length field for arbitrary binary protocols. The text examples send a null
terminator, so printing stops before padding.

### Both boards only print `TX:`

Both were compiled with `ROLE_SENDER=1`. Recompile the receiving board with
`ROLE_SENDER=0`.

### Board-information request fails but communication works

Board-information readback is separate from boot and mailbox validation. Use
`G3PhyFullDebug` and treat successful firmware boot, runtime key, repeated
mailbox tests and TX/RX confirmations as separate diagnostics.

### Original ESP32 fails with these pins

GPIO6-GPIO11 are commonly connected to flash on original ESP32 modules. Choose
safe SPI pins for that board. The documented mapping is verified on ESP32-S3.

## Project structure

```text
PL460Arduino/
  examples/                 Arduino sketches
  src/                      Transport and protocol APIs
  src/firmware/             Generated opt-in firmware headers
  tests/native/             Host-side mock test
  tools/                    Firmware-header generator
  LICENSE.md                License map
  LICENSE-PORT.md           BSD-3-Clause for original project portions
  LICENSE-MICROCHIP.md      Microchip smartenergy terms
  LICENSE-MICROCHIP-APPS.md Microchip application terms
  NOTICE.md                 Attribution and provenance
```

## Building and testing

ESP32-S3 example build:

```sh
arduino-cli compile \
  --fqbn esp32:esp32:esp32s3 \
  --libraries PL460Arduino \
  PL460Arduino/examples/G3HelloWorld
```

The repository's examples have been compile-checked with ESP32 Arduino core
3.3.11. Hardware behavior remains dependent on the actual board, supplies,
coupling network and firmware personality.

## License and redistribution

This is a **mixed-license, Microchip-device-only distribution**. Read
`LICENSE.md` and `NOTICE.md` before copying or publishing it.

- Original Arduino transport, examples, tests, tooling and documentation are
  offered under BSD-3-Clause in `LICENSE-PORT.md`, except where a file or
  portion is identified as derived from Microchip material.
- Firmware images, Microchip-derived boot/mailbox/protocol material and
  Microchip calibration values are governed by `LICENSE-MICROCHIP.md` and, as
  applicable, `LICENSE-MICROCHIP-APPS.md`.
- Microchip's terms permit source and binary redistribution with conditions,
  including retention of notices and disclaimers and use solely with or in
  combination with a device manufactured by or for Microchip.
- The Microchip-controlled portions must not be subjected to an incompatible
  open-source license. The library as a whole should therefore be described as
  **source-available**, not OSI-approved open source.
- Microchip's name must not be used to imply endorsement.
- Users and redistributors are responsible for third-party terms, export laws,
  electrical safety and regulatory compliance.

The license files must remain beside the source and must be included in source
archives, binary distributions and accompanying documentation as required by
their terms. This summary is informational and is not legal advice.

## Publishing this project

The `PL460Arduino` directory can be published as a public source repository if
the complete license and notice set is retained and the Microchip restrictions
are followed. The parent development repository also contains a large original
`G3_v2.1.0` SDK distribution with many third-party components. That SDK is not
needed to build this Arduino library and should **not** be included in a public
library repository unless every component and its redistribution terms are
separately audited.

For a clean public/Arduino Library Manager repository:

1. Make `PL460Arduino` the repository root.
2. Keep `library.properties`, all license files and `NOTICE.md` at that root.
3. Exclude the parent `G3_v2.1.0` directory and local history/build files.
4. Run Arduino Lint and compile all examples.
5. Create a semantic version tag such as `0.1.0`.
6. Do not market the project as Microchip-approved or as a certified G3 stack.

Arduino Library Manager requires `library.properties` at the repository root,
so the current parent-repository layout is not ready for registry submission.

## Upstream references

- [Microchip Smart Energy drivers](https://github.com/Microchip-MPLAB-Harmony/smartenergy)
- [Microchip G3 stack](https://github.com/Microchip-MPLAB-Harmony/smartenergy_g3)
- [Microchip G3 applications](https://github.com/Microchip-MPLAB-Harmony/smartenergy_g3_apps)
- [Microchip PRIME applications](https://github.com/Microchip-MPLAB-Harmony/smartenergy_prime_apps)
- [PL460-EK rev5 documentation](https://onlinedocs.microchip.com/oxy/GUID-32027C30-4315-4CFB-98CC-182810A6C695-en-US-4/index.html)
- [Arduino Library Manager requirements](https://github.com/arduino/library-registry/blob/main/FAQ.md#what-are-the-requirements-for-a-library-to-be-added-to-library-manager)

## Trademarks

Microchip, MPLAB, PL460 and related marks are trademarks or registered
trademarks of Microchip Technology Inc. Arduino is a trademark of Arduino SA.
All other marks belong to their respective owners. Use of a name identifies
compatibility only and does not imply endorsement.
