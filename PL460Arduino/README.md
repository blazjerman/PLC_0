# PL460Arduino

Arduino host driver for the Microchip PL460 power-line communication modem and
PL460-EK revision 5. The current primary target is an ESP32 Dev Module; the
transport uses only the standard Arduino `SPI`, GPIO, timing, and optional ADC
APIs so it can also be used on capable SAMD, STM32, Renesas, and RP2040-class
Arduino boards.

## What works

- Uploads PL460 firmware after every reset (the PL460 has no application flash).
- Checks the firmware mailbox key and reads product/model/version/band data.
- Provides raw read/write access to every firmware mailbox.
- Provides typed G3-PLC, PRIME, and Meters & More PHY P2P transmit/receive APIs.
- Loads Microchip's PL460-EK rev5 auxiliary-branch CENELEC-A calibration before
  G3 transmission (line-driver selection, DACC, RMS, gain and predistortion).
- Includes CENELEC-A/B, FCC, and multiband G3 PHY images.
- Includes CENELEC-A/B, FCC, and multiband G3 MAC-RT images.
- Includes PRIME, PRIME two-channel, and Meters & More PHY images.
- Includes two-board G3 PHY P2P and non-transmitting single-board tests.
- Prevents transmission when configured thermal or amplifier-supply inputs are
  unsafe.

## Important protocol boundary

PL460 supports several separate firmware personalities. G3 PHY, G3 MAC-RT,
PRIME and Meters & More are not modes that can run simultaneously: reset the
PL460 and upload the corresponding image when switching.

On ESP32, this library can directly host the PL460 bootloader, G3 PHY, G3
MAC-RT mailbox, PRIME mailbox, and Meters & More mailbox. The complete Harmony
G3 stack above MAC-RT (6LoWPAN, IPv6, UDP, coordinator/device management and
security) is supplied by Microchip as precompiled ARM Cortex-M libraries. It
cannot be linked into Xtensa or RISC-V ESP32 firmware. For complete G3 on an
ESP32 project, use either:

1. an ARM Cortex-M host supported by Microchip Harmony, or
2. Microchip's serialized G3 modem application on a supported ARM MCU and let
   the ESP32 command that modem.

Typed physical-layer frame APIs are included for PRIME and Meters & More too.
These are raw PHY tests, not complete PRIME/Meters & More network stacks. Do not
call raw G3 PHY frame formats “full G3 networking”; the included P2P example
deliberately tests the G3 physical layer.

## PL460-EK rev5 to ESP32 wiring

The checked-in examples now use your ESP32 pin assignment:

| PL460-EK J2 | Signal | ESP32 example |
|---:|---|---:|
| 2, 19 | GND | GND |
| 20 | 3V3 logic supply | 3V3 |
| 4 | XPL_TXEN | GPIO 6 |
| 5 | XPL_NRST | GPIO 3 |
| 6 | XPL_ENABLE (optional R71) | GPIO 4 / LDO_EN |
| 9 | XPL_EXTIN (active low) | GPIO 2 |
| 10 | XPL_NTHW0 (active low thermal) | GPIO 7 |
| 13 | XPL_STBY (optional R68) | GPIO 5 |
| 15 | XPL_CS | GPIO 10 |
| 16 | MOSI | GPIO 11 |
| 17 | MISO | GPIO 13 |
| 18 | SCK | GPIO 12 |

J4 separately requires **15 V ±5%, 12 W** for the coupling/amplifier supply.
Do not power J4 from the ESP32. Keep all logic at 3.3 V.

J2 pin 3 is the analog supply monitor. To use it, pass its ESP32 ADC pin and a
calibrated raw threshold as the last two `Pins` arguments. The divider and ADC
scaling must be checked on the actual rev5 board. A threshold of zero disables
this check. R57/R68/R71 are not fitted by default, and R80 changes the TX-enable
supply-monitor routing; do not assume those optional paths are connected.

Power-line voltage is hazardous. Develop first through an isolated PLC coupling
fixture and follow the PL460-EK user guide and local electrical-safety rules.

For this non-default ESP32 SPI mapping, pass all three bus pins explicitly:

```cpp
pl460::Pins control(10, 3, 2, 6, 7);       // CS, RESET, IRQ, TX_EN, NTHW0
pl460::SpiPins bus(12, 13, 11);             // SCK, MISO, MOSI
pl460::ArduinoTransport transport(SPI, control, bus);
```

Set PL460 `ENABLE/LDO_EN` high and `STBY` low before `modem.begin()`. A stock
rev5 evaluation kit already fixes ENABLE high and STBY low; its J2 versions are
not connected unless R71 and R68 respectively are fitted.

## First test

1. Copy the `PL460Arduino` directory into the Arduino libraries directory, or
   add this repository as an Arduino CLI library path.
2. Wire the table above and power both the 3.3 V logic domain and J4 amplifier
   supply.
3. Open `BoardDiagnostics` and adjust its pins.
4. Select **ESP32 Dev Module**, upload, and open Serial Monitor at 115200 baud.
5. Run `SingleBoardSelfTest` if only one modem is available. It intentionally
   does not transmit.
6. If transmission is not working, run `G3PhyFullDebug` first. It uses a forced,
   relative-time, robust-BPSK CENELEC-A frame and does not require a receiver.
   The sketch loads the rev5 auxiliary-branch tables, forces VLO impedance for
   maximum test signal, and enables the PHY CRC checker. A reported receive
   length can include one padding byte; `CRC=OK` is the integrity result.
7. For two boards, flash `G3PhyP2P`, `PrimeP2P`, or `MetersAndMoreP2P` with
   `ROLE_SENDER=1` on one and `ROLE_SENDER=0` on the other. G3 uses Slovenia's
   CENELEC-A image.

For the shortest text demonstration, use `G3HelloWorld`: upload it with
`ROLE_SENDER=1` to one board, change the setting to `0`, and upload it to the
other board. The sender transmits `Hello world!` every two seconds.

Each firmware header is opt-in. Include only the image being used; otherwise a
sketch can waste hundreds of kilobytes of flash.

## Firmware selection

| Header | Personality | Band |
|---|---|---|
| `PLC_PHY_G3_CENA.h` | G3 PHY | CENELEC-A (Slovenia default) |
| `PLC_PHY_G3_CENB.h` | G3 PHY | CENELEC-B |
| `PLC_PHY_G3_FCC.h` | G3 PHY | FCC |
| `PLC_PHY_G3_MULTIBAND.h` | G3 PHY | runtime selectable |
| `G3_MAC_RT_CENA.h` | G3 MAC-RT | CENELEC-A |
| `G3_MAC_RT_CENB.h` | G3 MAC-RT | CENELEC-B |
| `G3_MAC_RT_FCC.h` | G3 MAC-RT | FCC |
| `G3_MAC_RT_MULTIBAND.h` | G3 MAC-RT | runtime selectable |
| `PLC_PHY_PRIME.h` | PRIME PHY | standard channel |
| `PLC_PHY_PRIME_2CHN.h` | PRIME PHY | two channel |
| `PLC_PHY_MM.h` | Meters & More PHY | CENELEC |

The binary headers can be regenerated from a current Harmony checkout with
`tools/generate_firmware_headers.py`. Firmware compatibility matters for rev5;
do not substitute old PL360/early PL460 binaries without checking the board
revision and release notes.

## Upstream references and license

- [Microchip Smart Energy drivers](https://github.com/Microchip-MPLAB-Harmony/smartenergy)
- [Microchip G3 stack](https://github.com/Microchip-MPLAB-Harmony/smartenergy_g3)
- [Microchip G3 applications](https://github.com/Microchip-MPLAB-Harmony/smartenergy_g3_apps)

Read `NOTICE.md` and `LICENSE-MICROCHIP.md` before redistribution. Microchip's
license requires the adapted software and firmware to be used with or in
combination with a Microchip device; this project uses the PL460.
