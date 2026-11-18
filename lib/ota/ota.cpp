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
 *   Written with AI assistance (Claude) with review by the author.
 * *************************************************************** */

#include "ota.h"

#include <ArduinoOTA.h>

// static flag: true from a successful otaBegin() until otaStop()
static bool running = false;

// --------------------------
//  otaBegin() function purpose: starts the ArduinoOTA service so firmware can be uploaded
//    over Wi-Fi. Does nothing if the service is already running.
//  arguments:
//    hostname: name the board advertises on the network (cloudsmoker2)
//    password: upload password. Must not be empty
//! refuses to start, and prints a message, if the password is null or empty
//? called at boot, only when no cook is stored (see main setup)
// --------------------------
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

// --------------------------
//  otaLoop() function purpose: lets ArduinoOTA handle any pending upload
//  arguments: none
//? must run on every pass of loop(). Does nothing when OTA is not running
// --------------------------
void otaLoop() {
  if (running) {
    ArduinoOTA.handle();
  }
}

// --------------------------
//  otaStop() function purpose: stops the ArduinoOTA service
//  arguments: none
//? does nothing if OTA is not running
// --------------------------
void otaStop() {
  if (!running) {
    return;
  }
  ArduinoOTA.end();
  running = false;
  Serial.println(F("OTA: stopped"));
}
