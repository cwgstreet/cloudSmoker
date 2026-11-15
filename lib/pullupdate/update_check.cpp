#include "update_check.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "cookstore.h"
#include "firmware_version.h"
#include "image_download.h"
#include "manifest_parser.h"
#include "redirect_check.h"
#include "release_keys.h"
#include "signature_check.h"

// Release tag to check. Official builds use the stable tag. A test build can override it by
// force-including a header that defines UPDATE_TAG.
#ifndef UPDATE_TAG
#define UPDATE_TAG "cloudSmoker2-stable"
#endif
static const String kManifestUrl =
    String("https://github.com/cwgstreet/cloudSmoker/releases/download/") + UPDATE_TAG + "/manifest.txt";
static const String kSigUrl =
    String("https://github.com/cwgstreet/cloudSmoker/releases/download/") + UPDATE_TAG + "/manifest.sig";

// The clients keep this pointer until the handshake, so it must outlive every request.
static String gCaPem;

static const char *caPem() {
  if (gCaPem.length() == 0) {
    gCaPem = String(ROOT_ISRG_X1_PEM) + String(ROOT_USERTRUST_ECC_PEM);
  }
  return gCaPem.c_str();
}

// Fetches a small release file through the checked redirect. Returns its length, or -1.
static int fetchSmallFile(const String &url, uint8_t *buf, size_t maxLen) {
  String target;
  {
    WiFiClientSecure first;
    first.setCACert(caPem());
    if (!fetchRedirectTarget(first, url, target)) {
      log_e("update check: redirect check failed");
      return -1;
    }
  }

  WiFiClientSecure second;
  second.setCACert(caPem());
  HTTPClient http;
  if (!http.begin(second, target)) {
    log_e("update check: begin failed");
    return -1;
  }
  int code = http.GET();
  if (code != 200) {
    log_e("update check: expected 200, got %d", code);
    http.end();
    return -1;
  }
  int len = http.getSize();
  if (len <= 0 || (size_t)len > maxLen) {
    log_e("update check: file size out of range");
    http.end();
    return -1;
  }

  WiFiClient *stream = http.getStreamPtr();
  size_t got = 0;
  uint32_t lastData = millis();
  while (got < (size_t)len) {
    int avail = stream->available();
    if (avail > 0) {
      size_t want = (size_t)avail < (size_t)len - got ? (size_t)avail : (size_t)len - got;
      got += stream->readBytes(buf + got, want);
      lastData = millis();
    } else if (!http.connected() || millis() - lastData > 15000) {
      break;
    } else {
      delay(1);
    }
  }
  http.end();
  return got == (size_t)len ? len : -1;
}

UpdateStatus runUpdateCheck() {
  Serial.printf("update check: tag %s\n", UPDATE_TAG);
  if (cookStoreActive()) {
    Serial.printf("update check: skipped, a cook is stored\n");
    return UPDATE_COOK_ACTIVE;
  }
  if (!clockValid()) {
    Serial.printf("update check: skipped, clock not set\n");
    return UPDATE_NO_CLOCK;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("update check: skipped, no Wi-Fi\n");
    return UPDATE_NO_WIFI;
  }

  static uint8_t manifest[1025];
  static uint8_t sig[128];
  int mLen = fetchSmallFile(kManifestUrl, manifest, sizeof(manifest) - 1);
  if (mLen < 0) return UPDATE_FAILED;
  int sLen = fetchSmallFile(kSigUrl, sig, sizeof(sig));
  if (sLen < 0) return UPDATE_FAILED;

  if (!verifyManifestSignature(manifest, mLen, sig, sLen, RELEASE_PUBKEY_PEM)) {
    log_e("update check: signature failed, nothing changed");
    return UPDATE_FAILED;
  }
  ManifestFields f;
  if (!parseManifest((const char *)manifest, mLen, RELEASE_KEY_ID, f)) {
    log_e("update check: manifest parse failed");
    return UPDATE_FAILED;
  }
  if (f.version <= (uint32_t)FIRMWARE_VERSION) {
    Serial.printf("update check: up to date (board %d, release %u)\n", FIRMWARE_VERSION, (unsigned)f.version);
    return UPDATE_UP_TO_DATE;
  }

  String target;
  {
    WiFiClientSecure first;
    first.setCACert(caPem());
    if (!fetchRedirectTarget(first, String(f.imageUrl), target)) {
      log_e("update check: image redirect check failed");
      return UPDATE_FAILED;
    }
  }
  WiFiClientSecure dl;
  dl.setCACert(caPem());
  if (!downloadImageToUpdate(dl, target, f.imageSize, f.imageSha256, true)) {
    log_e("update check: download failed, nothing staged");
    return UPDATE_FAILED;
  }
  Serial.printf("update check: release %u staged\n", (unsigned)f.version);
  return UPDATE_STAGED;
}

void updateCheckAtBoot() {
  if (runUpdateCheck() == UPDATE_STAGED) {
    Serial.printf("update check: restarting into staged release\n");
    delay(200);
    ESP.restart();
  }
}
