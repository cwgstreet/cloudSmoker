/* ***************************************************************
 * redirect_check.h - follows GitHub's download redirect one hop at a time, checking each host
 *
 *   C W Greenstreet, cloudSmoker2, 17Nov2026
 *   Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *     distribution must also share the full source code under this same license.)
 *
 *   Written with AI assistance (Claude). Under review by the author.
 * *************************************************************** */

#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>

// Sends one GET with redirects off. Returns true only for a 302 whose Location
// starts with https://release-assets.githubusercontent.com/. On success, `out` holds that URL.
bool fetchRedirectTarget(WiFiClientSecure &client, const String &url, String &out);
