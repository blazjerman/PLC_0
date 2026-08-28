# Notices and provenance

PL460Arduino is an independent, unofficial Arduino port for use with the
Microchip PL460. Microchip Technology Inc. has not tested, approved, endorsed,
or agreed to support this project.

## Microchip-derived material

The PL460 bootloader sequence, mailbox protocol, virtual PIB/register access,
PHY wire structures and parts of the protocol behavior were adapted from
Microchip MPLAB Harmony Smart Energy drivers and applications.

The rev5 G3 CENELEC-A coupling configuration in `PL460G3Coupling.cpp` uses
Microchip's PL460-EK values for line-driver selection, DACC, RMS targets,
thresholds, gains and predistortion coefficients.

These portions are governed by the applicable terms in:

- `LICENSE-MICROCHIP.md`
- `LICENSE-MICROCHIP-APPS.md`

In particular, the Microchip-controlled material may be used and execute only
with or in combination with a device manufactured by or for Microchip. The
intended device in this project is the Microchip PL460.

## Firmware provenance

The firmware images encoded under `src/firmware` are unmodified binary releases
from the official Microchip MPLAB Harmony `smartenergy` repository:

https://github.com/Microchip-MPLAB-Harmony/smartenergy

Source revision verified during preparation of this notice:

`91b27a76a301a39b9d04fb513ac7ca8acd047ab2`

Each checked-in header was regenerated from the corresponding upstream `.bin`
using `tools/generate_firmware_headers.py` and compared byte-for-byte with the
checked-in result.

| Firmware | SHA-256 of upstream binary |
|---|---|
| PLC_PHY_G3_CENA | `cee3e8dc83e5e53ea814bbc3fa768624780fa04eae213a5809c7c6dd6e1398ae` |
| PLC_PHY_G3_CENB | `1f02e6ce4af7341a75e48b957e8eedd2d7a68b69efd33aea64f6763d4f2a70f7` |
| PLC_PHY_G3_FCC | `86697e4da1cff4eaf25016b0c305d28db90ce9c8f8b95e4e005f5fe0d17fd5b3` |
| PLC_PHY_G3_MULTIBAND | `f6a248a8f053375ef5136b94c878770a217d9c3fd598b87689f02f54431043e2` |
| G3_MAC_RT_CENA | `187586201709f3914886b3b4f12dab4c9441c2b20ec90de7cfd8bad902784dbc` |
| G3_MAC_RT_CENB | `204c7ef00297bea03c114ccbf10e0d7159c27bb5f5a93ccee136c13d7659d8b5` |
| G3_MAC_RT_FCC | `76a853f4ef6f7f71ac12fca14bd8afb4fa85b5876f5f5a7fdda35460ee7d0256` |
| G3_MAC_RT_MULTIBAND | `6df31b5862bda2dc73f520710f7349565417e821dffbd4a48a1a20b911ee37da` |
| PLC_PHY_PRIME | `99d3bc20bc8903be8dd96d87f6ae638621b9f5592ed6b65137fdae56ef48a949` |
| PLC_PHY_PRIME_2CHN | `3e4d212703e236592a507305e95d00cdc31ca3d77edcf986a1e7c90a462a76da` |
| PLC_PHY_MM | `124a38cd58c22e3c9da27434165f0aad8c5bba324f86ca57c7dd636d14df84dc` |

## Original project material

The Arduino transport, portable C++ interface, examples, native test, firmware
conversion tool and documentation were created for this project except for the
Microchip-derived portions described above. The original portions are offered
under `LICENSE-PORT.md`.

## Trademarks and endorsement

Microchip, MPLAB, PL460 and related marks are trademarks or registered
trademarks of Microchip Technology Inc. Arduino is a trademark of Arduino SA.
All other trademarks are the property of their owners. Names are used only to
identify compatibility and do not imply endorsement.

