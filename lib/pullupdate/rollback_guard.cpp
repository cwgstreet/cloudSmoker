/* ***************************************************************
 * rollback_guard.cpp - keeps a new firmware image pending until it proves healthy
 *
 * C W Greenstreet, cloudSmoker2, 14Nov26
 * Licence: GPLv3 (see manifest_parser.h)
 *************************************************************** */

#include "rollback_guard.h"

#include <Arduino.h>
#include "esp_ota_ops.h"

namespace {

const unsigned long kHealthWindowMs = 120000UL;  // two minutes

bool gBleStarted = false;
unsigned long gStartMs = 0;
bool gDecided = false;  // true once the guard has confirmed, rolled back, or stood down

}  // namespace

// Replaces the framework's weak default, which accepts a new image at startup.
extern "C" bool verifyRollbackLater() { return true; }

void rollbackGuardBegin(bool bleStarted) {
  gBleStarted = bleStarted;
  gStartMs = millis();
}

void rollbackGuardLoop() {
  if (gDecided) return;

  // Only an image still waiting for confirmation needs a decision.
  const esp_partition_t* running = esp_ota_get_running_partition();
  esp_ota_img_states_t state;
  if (esp_ota_get_state_partition(running, &state) != ESP_OK ||
      state != ESP_OTA_IMG_PENDING_VERIFY) {
    gDecided = true;
    return;
  }

  if (millis() - gStartMs < kHealthWindowMs) return;  // still inside the window

  gDecided = true;
  if (gBleStarted) {
    if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
      Serial.println(F("rollback guard: image confirmed"));
    } else {
      Serial.println(F("rollback guard: confirm failed"));
    }
  } else {
    Serial.println(F("rollback guard: BLE not started, rolling back"));
    esp_ota_mark_app_invalid_rollback_and_reboot();  // reboots; returns only on error
  }
}
