#pragma once
#include <Arduino.h>

// Starts the ArduinoOTA service. Call after Wi-Fi has connected.
void otaBegin(const char* hostname, const char* password);

// Must be called every pass of loop() while OTA is running.
void otaLoop();

// Stops the OTA service. Call when a cook starts.
void otaStop();

bool otaRunning();
