/* ***************************************************************
 * nvs_stats.cpp - debug-only printout of NVS usage (built only with DEBUG_NVS_STATS)
 *
 *   C W Greenstreet, cloudSmoker2, 17Nov2026
 *   Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *     distribution must also share the full source code under this same license.)
 *
 *   Built on:
 *     Arduino-ESP32 core (Espressif Systems): NVS statistics API (nvs.h).
 *     This file calls their functions and does not copy their code.
 *
 *   Written with AI assistance (Claude). Under review by the author.
 * *************************************************************** */

#ifdef DEBUG_NVS_STATS
#include <Arduino.h>
#include "nvs.h"

void printNvsStats() {
  nvs_stats_t stats;
  esp_err_t err = nvs_get_stats("nvs", &stats);
  if (err != ESP_OK) {
    Serial.printf("NVS stats error: %d\n", err);
    return;
  }
  Serial.printf("NVS entries: used %u, free %u, total %u\n",
                stats.used_entries, stats.free_entries, stats.total_entries);
}
#endif
