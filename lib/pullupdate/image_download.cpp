/* ***************************************************************
 * image_download.cpp - streams the image into Update while hashing; commits only on a match
 *
 *   C W Greenstreet, cloudSmoker2, 17Nov2026
 *   Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *     distribution must also share the full source code under this same license.)
 *
 *   Built on:
 *     Arduino-ESP32 core (Espressif Systems): Update, HTTPClient and WiFiClientSecure.
 *     This file calls their functions and does not copy their code.
 *
 *   Written with AI assistance (Claude). Under review by the author.
 * *************************************************************** */

#include "image_download.h"
#include <HTTPClient.h>
#include <Update.h>
#include <stdio.h>
#include <string.h>
#include "mbedtls/sha256.h"

static const size_t kChunkSize = 1024;
static const uint32_t kStallTimeoutMs = 15000;

bool downloadImageToUpdate(WiFiClientSecure &client, const String &signedUrl,
                           uint32_t expectedSize, const char *expectedHex, bool commit) {
  if (strlen(expectedHex) != 64) {
    log_e("download: expected hash is not 64 characters");
    return false;
  }

  HTTPClient http;
  http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
  if (!http.begin(client, signedUrl)) {
    log_e("download: begin failed");
    return false;
  }

  int code = http.GET();
  if (code != 200) {
    log_e("download: expected 200, got %d", code);
    http.end();
    return false;
  }
  if (http.getSize() != (int)expectedSize) {
    log_e("download: size does not match the manifest");
    http.end();
    return false;
  }
  if (!Update.begin(expectedSize)) {
    log_e("download: Update.begin failed");
    http.end();
    return false;
  }

  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts_ret(&ctx, 0);  // 0 selects SHA-256 rather than SHA-224

  WiFiClient *stream = http.getStreamPtr();
  uint8_t buf[kChunkSize];
  uint32_t total = 0;
  uint32_t lastData = millis();
  bool ok = true;

  while (total < expectedSize) {
    int avail = stream->available();
    if (avail <= 0) {
      if (!http.connected()) {
        log_e("download: connection closed early");
        ok = false;
        break;
      }
      if (millis() - lastData > kStallTimeoutMs) {
        log_e("download: stalled");
        ok = false;
        break;
      }
      delay(1);
      continue;
    }

    size_t want = (size_t)avail < kChunkSize ? (size_t)avail : kChunkSize;
    if (want > expectedSize - total) {
      want = expectedSize - total;
    }
    size_t got = stream->readBytes(buf, want);
    if (got == 0) {
      log_e("download: read failed");
      ok = false;
      break;
    }
    if (Update.write(buf, got) != got) {
      log_e("download: flash write failed");
      ok = false;
      break;
    }
    if (mbedtls_sha256_update_ret(&ctx, buf, got) != 0) {
      log_e("download: hash update failed");
      ok = false;
      break;
    }
    total += got;
    lastData = millis();
  }

  unsigned char digest[32];
  mbedtls_sha256_finish_ret(&ctx, digest);
  mbedtls_sha256_free(&ctx);
  http.end();

  if (!ok || total != expectedSize) {
    Update.abort();
    log_e("download: incomplete, update aborted");
    return false;
  }

  char hex[65];
  for (int i = 0; i < 32; i++) {
    sprintf(hex + 2 * i, "%02x", digest[i]);
  }
  hex[64] = '\0';
  if (strcmp(hex, expectedHex) != 0) {
    Update.abort();
    log_e("download: SHA-256 mismatch, update aborted");
    return false;
  }

  if (!commit) {
    Update.abort();
    log_i("download: verified (dry run), nothing staged");
    return true;
  }

  if (!Update.end(true)) {
    log_e("download: Update.end failed: %s", Update.errorString());
    return false;
  }
  log_i("download: image verified and staged, reboot to apply");
  return true;
}
