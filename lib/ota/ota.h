/* ***************************************************************
 * ota.h - over-the-air (OTA) firmware upload over Wi-Fi, using ArduinoOTA
 *   Lets a computer push new firmware to the device across the Wi-Fi network
 *   (espota upload, set in platformio.ini). Not the GitHub pull-update, which is
 *   a separate library in lib/pullupdate.
 *
 *   C W Greenstreet, cloudSmoker2, 16Nov2026
 *   Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *     distribution must also share the full source code under this same license.)
 *
 *   Built on:
 *     ArduinoOTA, part of the Arduino-ESP32 core (Espressif Systems). This library
 *     calls its functions and does not copy its code.
 *
 *   Written with AI assistance (Claude) with review by the author.
 * *************************************************************** */

#pragma once
#include <Arduino.h>

// Starts the ArduinoOTA service. Called once at boot; does not check that Wi-Fi is connected.
void otaBegin(const char* hostname, const char* password);

// Must be called every pass of loop() while OTA is running.
void otaLoop();

// Stops the OTA service. Called when a stored cook is restored at boot.
//! open decision: no call yet when a new cook starts
void otaStop();
