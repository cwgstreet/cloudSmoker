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
