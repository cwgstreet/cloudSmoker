#include "redirect_check.h"
#include <HTTPClient.h>

// The trailing slash makes the host match exact, so a lookalike host fails.
static const char *kReleaseHostPrefix = "https://release-assets.githubusercontent.com/";

bool fetchRedirectTarget(WiFiClientSecure &client, const String &url, String &out) {
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
