/* ***************************************************************
 * redirect_check.cpp - implementation of the redirect check declared in redirect_check.h
 *
 *   C W Greenstreet, cloudSmoker2, 17Nov2026
 *   Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *     distribution must also share the full source code under this same license.)
 *
 *   Built on:
 *     Arduino-ESP32 core (Espressif Systems): HTTPClient and WiFiClientSecure.
 *     This file calls their functions and does not copy their code.
 *
 *   Written with AI assistance (Claude) with review by the author.
 * *************************************************************** */

#include "redirect_check.h"

#include <HTTPClient.h>

// The trailing slash makes the host match exact, so a lookalike host fails.
static const char* kReleaseHostPrefix = "https://release-assets.githubusercontent.com/";

bool fetchRedirectTarget(WiFiClientSecure& client, const String& url, String& out) {
  HTTPClient http;
  http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
  bool ok = false;

  if (http.begin(client, url)) {
    int code = http.GET();
    if (code == 302) {
      String location = http.getLocation();
      if (location.length() == 0) {
        log_e("redirect check: 302 without Location header");
      } else if (location.startsWith(kReleaseHostPrefix)) {
        out = location;
        ok = true;
      } else {
        log_e("redirect check: Location host not allowed");
      }
    } else {
      log_e("redirect check: expected 302, got %d", code);
    }
  } else {
    log_e("redirect check: begin failed");
  }

  http.end();
  return ok;
}
