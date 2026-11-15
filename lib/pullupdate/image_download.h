#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>

// Downloads the image through a signed release-assets URL into the inactive OTA slot.
// Size and SHA-256 are checked against the signed manifest before the slot is marked
// bootable. On false the running image is untouched. The caller sets the pinned roots
// on `client` first, and reboots only after true, with no cook stored.
// commit=false is a dry run: verified, then discarded, no reboot.
bool downloadImageToUpdate(WiFiClientSecure &client, const String &signedUrl,
                           uint32_t expectedSize, const char *expectedHex, bool commit);
