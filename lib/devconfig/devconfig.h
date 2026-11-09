#ifndef DEVCONFIG_H
#define DEVCONFIG_H

#include <Arduino.h>

// Per-device settings entered by the user, kept in NVS (namespace "cfg").
// Blank ntfy topic or ThingSpeak key means that feature is off.

void devConfigBegin();                      // open the store; create the random ntfy topic on first boot
String devNtfyTopic();                      // empty = phone alerts off
String devThingSpeakKey();                  // empty = cloud logging off
void devSetNtfyTopic(const String& topic);
void devSetThingSpeakKey(const String& key);
uint32_t devBlePasskey();                   // six-digit random code, created on first use

#endif  // DEVCONFIG_H
