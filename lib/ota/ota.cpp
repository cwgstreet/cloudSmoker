/* ***************************************************************
 * ota.cpp - implementation of the ArduinoOTA wrapper declared in ota.h
 *
 *   C W Greenstreet, cloudSmoker2, 16Nov2026
 *   Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *     distribution must also share the full source code under this same license.)
 *
 *   Built on:
 *     ArduinoOTA, part of the Arduino-ESP32 core (Espressif Systems). This file
 *     calls its functions and does not copy its code.
 *
 *   Written with AI assistance (Claude). Under review by the author.
 * *************************************************************** */

#include "ota.h"

#include <ArduinoOTA.h>

static bool running = false;

void otaBegin(const char* hostname, const char* password) {
  if (running) {
    return;
  }
  if (password == nullptr || password[0] == '\0') {
    Serial.println(F("OTA: refused to start, no password set"));
    return;
  }

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.setPassword(password);

  ArduinoOTA.onStart([]() { Serial.println(F("OTA: update started")); });
  ArduinoOTA.onEnd([]() { Serial.println(F("\nOTA: update complete, rebooting")); });
  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {
    Serial.printf("OTA: %u%%\r", (done * 100) / total);
  });
  ArduinoOTA.onError([](ota_error_t err) { Serial.printf("OTA: error %u\n", (unsigned int)err); });

  ArduinoOTA.begin();
  running = true;
  Serial.printf("OTA: ready as %s\n", hostname);
}

void otaLoop() {
  if (running) {
    ArduinoOTA.handle();
  }
}

void otaStop() {
  if (!running) {
    return;
  }
  ArduinoOTA.end();
  running = false;
  Serial.println(F("OTA: stopped"));
}

bool otaRunning() { return running; }
