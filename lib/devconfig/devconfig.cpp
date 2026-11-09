#include "devconfig.h"

#include <Preferences.h>
#include <esp_system.h>

static Preferences cfg;

void devConfigBegin() {
  cfg.begin("cfg", false);
  if (!cfg.isKey("ntfy")) {
    // First boot: random topic starting with cloudSmoker2-, so it's easy to find in the app
    char topic[24];
    snprintf(topic, sizeof(topic), "cloudSmoker2-%08lu", (unsigned long)(esp_random() % 100000000UL));
    cfg.putString("ntfy", topic);
  }
  Serial.print(F("ntfy topic: "));
  Serial.println(devNtfyTopic());
}

String devNtfyTopic() {
  return cfg.getString("ntfy", "");
}


void devSetNtfyTopic(const String& topic) {
  cfg.putString("ntfy", topic);
}

void devSetThingSpeakKey(const String& key) {
  cfg.putString("tskey", key);
}

uint32_t devBlePasskey() {
  if (!cfg.isKey("passkey")) {
    cfg.putUInt("passkey", 100000 + esp_random() % 900000);  // always six digits
  }
  return cfg.getUInt("passkey", 100000);
}


// ThingSpeak key entered on the setup portal. Blank = cloud logging off.
String devThingSpeakKey() {
  return cfg.getString("tskey", "");
}
