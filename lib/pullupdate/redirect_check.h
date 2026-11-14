#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>

// Sends one GET with redirects off. Returns true only for a 302 whose Location
// starts with https://release-assets.githubusercontent.com/. On success, `out` holds that URL.
bool fetchRedirectTarget(WiFiClientSecure &client, const String &url, String &out);
