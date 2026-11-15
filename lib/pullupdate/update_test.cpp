#ifdef DEBUG_COOK_CMDS
#include "update_test.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "firmware_version.h"
#include "cookstore.h"
#include "image_download.h"
#include "manifest_parser.h"
#include "redirect_check.h"
#include "release_keys.h"
#include "signature_check.h"

// The clients keep this pointer until the handshake, so it must outlive every request.
static String gCaPem;

static const char *caPem() {
  if (gCaPem.length() == 0) {
    gCaPem = String(ROOT_ISRG_X1_PEM) + String(ROOT_USERTRUST_ECC_PEM);
  }
  return gCaPem.c_str();
}

// Fetches a small release file through the checked redirect.
// Returns its length, or -1 on any failure.
static int fetchSmallFile(const String &url, uint8_t *buf, size_t maxLen) {
  String target;
  {
    WiFiClientSecure first;
    first.setCACert(caPem());
    if (!fetchRedirectTarget(first, url, target)) {
      log_e("update test: redirect check failed");
      return -1;
    }
  }

  WiFiClientSecure second;
  second.setCACert(caPem());
  HTTPClient http;
  if (!http.begin(second, target)) {
    log_e("update test: begin failed");
    return -1;
  }
  int code = http.GET();
  if (code != 200) {
    log_e("update test: expected 200, got %d", code);
    http.end();
    return -1;
  }
  int len = http.getSize();
  if (len <= 0 || (size_t)len > maxLen) {
    log_e("update test: file size out of range");
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

bool runUpdateTest(const String &manifestUrl, const String &sigUrl, bool commit) {
  if (cookStoreActive()) {
    log_e("update test: refused, a cook is stored");
    return false;
  }
  static uint8_t manifest[1025];
  static uint8_t sig[128];

  int mLen = fetchSmallFile(manifestUrl, manifest, sizeof(manifest) - 1);
  if (mLen < 0) return false;
  int sLen = fetchSmallFile(sigUrl, sig, sizeof(sig));
  if (sLen < 0) return false;
  log_i("update test: manifest %d bytes, signature %d bytes", mLen, sLen);

  if (!verifyManifestSignature(manifest, mLen, sig, sLen, RELEASE_PUBKEY_PEM)) {
    log_e("update test: SIGNATURE FAILED");
    return false;
  }
  log_i("update test: signature ok");

  ManifestFields f;
  if (!parseManifest((const char *)manifest, mLen, RELEASE_KEY_ID, f)) {
    log_e("update test: manifest parse failed");
    return false;
  }
  log_i("update test: release version %u (%s), board version %d",
        (unsigned)f.version, f.versionName, (int)FIRMWARE_VERSION);
  if (commit && f.version <= (uint32_t)FIRMWARE_VERSION) {
    log_e("update test: refused, release %u is not newer than board", (unsigned)f.version);
    return false;
  }

  String target;
  {
    WiFiClientSecure first;
    first.setCACert(caPem());
    if (!fetchRedirectTarget(first, String(f.imageUrl), target)) {
      log_e("update test: image redirect check failed");
      return false;
    }
  }

  WiFiClientSecure dl;
  dl.setCACert(caPem());
  bool ok = downloadImageToUpdate(dl, target, f.imageSize, f.imageSha256, commit);
  log_i("update test: %s", ok ? "ok" : "FAILED");
  return ok;
}
bool runBenchUpdate(const char *tag, bool commit) {
  String base = String("https://github.com/cwgstreet/cloudSmoker/releases/download/") + tag;
  return runUpdateTest(base + "/manifest.txt", base + "/manifest.sig", commit);
}
#endif
