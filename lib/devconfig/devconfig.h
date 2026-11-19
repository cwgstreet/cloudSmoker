/* ***************************************************************
 * devconfig.h - device settings in NVS: ntfy topic, ThingSpeak key, BLE passkey, OTA password
 *
 *   C W Greenstreet, cloudSmoker2, 17Nov2026
 *   Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *     distribution must also share the full source code under this same license.)
 *
 *   Written with AI assistance (Claude) with review by the author.
 * *************************************************************** */

#ifndef DEVCONFIG_H
#define DEVCONFIG_H

#include <Arduino.h>

// Per-device settings entered by the user, kept in NVS (namespace "cfg").
// Blank ntfy topic or ThingSpeak key means that feature is off.

void devConfigBegin();      // open the store; create the random ntfy topic on first boot
String devNtfyTopic();      // empty = phone alerts off
String devThingSpeakKey();  // empty = cloud logging off
void devSetNtfyTopic(const String& topic);
void devSetThingSpeakKey(const String& key);
String devThingSpeakChannel();  // ThingSpeak channel ID, empty = no dashboard link
void devSetThingSpeakChannel(const String& channel);  // digits only, blank clears
uint32_t devBlePasskey();  // six-digit random code, created on first use

String devOtaPassword();     // created on first use, never empty
String devOtaPasswordGet();  // returns the stored OTA password, or empty if none; never creates one

#ifdef DEBUG_OTA
void devOtaPasswordClear();
#endif
#endif  // DEVCONFIG_H
