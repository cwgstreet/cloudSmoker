# patch_tft_c3.py
# Temporary workaround for TFT_eSPI's ESP32-C3 support.
# The C3 processor header sets SPI_PORT to SPI2_HOST (enum value 1), but the
# ESP-IDF C3 register macros expect the peripheral number 2. Before each build,
# this script replaces that line with SPI_PORT 2, the value confirmed on hardware.
# Remove this script and its extra_scripts line once upstream is fixed.

import re
from pathlib import Path

Import("env")

HEADER_REL = Path("TFT_eSPI/Processors/TFT_eSPI_ESP32_C3.h")
PATCH_MARKER = "PATCH: IDF REG_SPI_BASE"

# Matches the single SPI_PORT define line, whether it is the original
# SPI2_HOST line or an earlier test edit (#define SPI_PORT 2, with or without a comment).
SPI_PORT_LINE = re.compile(
    r"^[ \t]*#define[ \t]+SPI_PORT[ \t]+(SPI2_HOST|2)\b.*$",
    re.MULTILINE,
)

BLOCK = (
    "// " + PATCH_MARKER + ": IDF expects peripheral number, not SPI2_HOST enum\n"
    "#define SPI_PORT 2"
)


def patch():
    libdeps = Path(env.subst("$PROJECT_LIBDEPS_DIR")) / env.subst("$PIOENV")
    header = libdeps / HEADER_REL

    if not header.exists():
        print(f"[patch_tft_c3] {header} not found yet (first build?); skipping")
        return

    text = header.read_text()

    if PATCH_MARKER in text:
        print("[patch_tft_c3] already patched, nothing to do")
        return

    new_text, count = SPI_PORT_LINE.subn(lambda m: BLOCK, text)
    if count != 1:
        print(f"[patch_tft_c3] expected 1 SPI_PORT line, found {count}; not patching")
        return

    header.write_text(new_text)
    print("[patch_tft_c3] patched SPI_PORT to 2 in TFT_eSPI_ESP32_C3.h")


patch()
