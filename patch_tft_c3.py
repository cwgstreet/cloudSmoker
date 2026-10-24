# patch_tft_c3.py
# Temporary workaround for TFT_eSPI's ESP32-C3 support.
# The C3 processor header sets SPI_PORT to SPI2_HOST (enum value 1), but the
# ESP-IDF C3 register macros expect the peripheral number 2. This script
# changes that one line in the library copy before each build.
# Remove this script and its extra_scripts line once upstream is fixed.

import re
from pathlib import Path

Import("env")

HEADER_REL = Path("TFT_eSPI/Processors/TFT_eSPI_ESP32_C3.h")
PATCH_MARKER = "PATCH: IDF REG_SPI_BASE"

# Matches the unpatched line exactly, e.g.  #define SPI_PORT SPI2_HOST
UNPATCHED = re.compile(r"^(\s*#define\s+SPI_PORT\s+)SPI2_HOST\s*$", re.MULTILINE)
# Matches an already-patched line, whether from this script or your manual test
ALREADY_2 = re.compile(r"^\s*#define\s+SPI_PORT\s+2\b", re.MULTILINE)
REPLACEMENT = (r"\g<1>2  // " + PATCH_MARKER +
               ": expects peripheral number, not SPI2_HOST enum")


def patch():
    libdeps = Path(env.subst("$PROJECT_LIBDEPS_DIR")) / env.subst("$PIOENV")
    header = libdeps / HEADER_REL

    if not header.exists():
        print(f"[patch_tft_c3] {header} not found yet (first build?); skipping")
        return

    text = header.read_text()

    if PATCH_MARKER in text or ALREADY_2.search(text):
        print("[patch_tft_c3] SPI_PORT already set to 2, nothing to do")
        return

    new_text, count = UNPATCHED.subn(REPLACEMENT, text)
    if count != 1:
        print(f"[patch_tft_c3] expected 1 SPI_PORT line, found {count}; not patching")
        return

    header.write_text(new_text)
    print("[patch_tft_c3] patched SPI_PORT in TFT_eSPI_ESP32_C3.h")


patch()
