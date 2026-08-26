#!/usr/bin/env python3
"""Convert Microchip PL460 firmware binaries into opt-in Arduino headers."""

from pathlib import Path
import sys


def convert(source: Path, target: Path) -> None:
    symbol = source.stem.lower()
    data = source.read_bytes()
    metadata = {
        "PLC_PHY_G3_CENA": ("Protocol::G3Phy", "Band::CenelecA", "PL460::kPhyRuntimeKey"),
        "PLC_PHY_G3_CENB": ("Protocol::G3Phy", "Band::CenelecB", "PL460::kPhyRuntimeKey"),
        "PLC_PHY_G3_FCC": ("Protocol::G3Phy", "Band::FCC", "PL460::kPhyRuntimeKey"),
        "PLC_PHY_G3_MULTIBAND": ("Protocol::G3Phy", "Band::Multiband", "PL460::kPhyRuntimeKey"),
        "G3_MAC_RT_CENA": ("Protocol::G3MacRt", "Band::CenelecA", "PL460::kMacRtRuntimeKey"),
        "G3_MAC_RT_CENB": ("Protocol::G3MacRt", "Band::CenelecB", "PL460::kMacRtRuntimeKey"),
        "G3_MAC_RT_FCC": ("Protocol::G3MacRt", "Band::FCC", "PL460::kMacRtRuntimeKey"),
        "G3_MAC_RT_MULTIBAND": ("Protocol::G3MacRt", "Band::Multiband", "PL460::kMacRtRuntimeKey"),
        "PLC_PHY_PRIME": ("Protocol::Prime", "Band::CenelecA", "PL460::kPhyRuntimeKey"),
        "PLC_PHY_PRIME_2CHN": ("Protocol::PrimeTwoChannel", "Band::CenelecA", "PL460::kPhyRuntimeKey"),
        "PLC_PHY_MM": ("Protocol::MetersAndMore", "Band::CenelecA", "PL460::kPhyRuntimeKey"),
    }
    protocol, band, key = metadata[source.stem]
    lines = [
        "#pragma once",
        "",
        '#include "../PL460Firmware.h"',
        "",
        "namespace pl460 {",
        "",
        f"static const uint8_t {symbol}[] PL460_FIRMWARE_STORAGE = {{",
    ]
    for start in range(0, len(data), 12):
        chunk = data[start : start + 12]
        lines.append("  " + ", ".join(f"0x{byte:02x}" for byte in chunk) + ",")
    lines += [
        "};",
        f"static const uint32_t {symbol}_length = {len(data)}UL;",
        f"static const FirmwareImage {source.stem}_IMAGE = {{",
        f"  {symbol}, {symbol}_length, {protocol}, {band}, {key},",
        f'  "{source.stem}", readFirmwareByte',
        "};",
        "",
        "}  // namespace pl460",
        "",
    ]
    target.write_text("\n".join(lines), encoding="ascii")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: generate_firmware_headers.py INPUT.bin OUTPUT.h")
    convert(Path(sys.argv[1]), Path(sys.argv[2]))
