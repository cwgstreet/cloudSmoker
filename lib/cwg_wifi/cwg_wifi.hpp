/* ***************************************************************
 * cwg_wifi.hpp - library to further abstract and extend tzapu WifiManager Lib
 *
 *  Based on
 *   1)   tazpu / WifiManager, MIT Licence
 *         reference:  https://github.com/tzapu/WiFiManager
 * *
 *  C W Greenstreet, Ver2, 25Oct2026
 *    GPLv3 Licence
 *
 ** ************************************************************* */

#ifndef CWG_WIFI_HPP
#define CWG_WIFI_HPP

#if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
#include <Arduino.h>
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block

/******************************************************
//?   Required libraries
*******************************************************/
#include <Arduino.h>      // Arduino framework; not explicitly included in PlatformIO
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include "devconfig.h"

//------------------------------------------------
//?  WifiManager_initialise() - initialisation function for WiFiManager
//?    to be placed in setup() block in main code
//-----------------------------------------------
// Automatically connect using saved credentials,
//  if connection fails, it starts an access point with the specified name "cloudSmoker_setUpAP",
//  if empty, will auto-generate SSID, if password is blank, it will be anonymous AP
//  (wm.autoConnect())
// then goes into a blocking loop awaiting configuration and will return success result
//-----------------------------------------------


///////////////////////////////////////////////////////////////////

#endif  // end header guard
// Blank-or-valid check for the ntfy topic: letters, digits, hyphen and underscore only
static bool ntfyTopicValid(const char* t) {
  if (strlen(t) == 0) {
    return true;  // blank = phone alerts off
  }
  for (const char* p = t; *p; p++) {
    if (!(isalnum((unsigned char)*p) || *p == '-' || *p == '_')) {
      return false;
    }
  }
  return true;
}


// Setup portal, with Wi-Fi, ThingSpeak key and ntfy topic on one page.
// openPortalNow = false (boot): join saved Wi-Fi; open the portal only if that fails.
// openPortalNow = true (Settings > Setup): open the portal now for three minutes.
void WifiManager_initialise(bool openPortalNow = false) {
  WiFi.mode(WIFI_STA);

  WiFiManager wm;
  wm.setHostname("cloudSmoker2");
  wm.setDebugOutput(true);

  // Fields are pre-filled with the stored values, so a timeout keeps what is already saved
  String keyNow = devThingSpeakKey();
  String topicNow = devNtfyTopic();
  WiFiManagerParameter tsParam("tskey", "ThingSpeak write key (blank = off)", keyNow.c_str(), 40);
  WiFiManagerParameter ntfyParam("ntfy", "ntfy topic (blank = off)", topicNow.c_str(), 40);
  wm.addParameter(&tsParam);
  wm.addParameter(&ntfyParam);
  // Save on submit, so the values are stored even if the portal never returns
  wm.setSaveParamsCallback([&]() {
    devSetThingSpeakKey(String(tsParam.getValue()));
    if (ntfyTopicValid(ntfyParam.getValue())) {
      devSetNtfyTopic(String(ntfyParam.getValue()));
    }
    Serial.print(F("portal submitted, ntfy topic: "));
    Serial.println(devNtfyTopic());
  });

  bool res;
  if (openPortalNow) {
    wm.setConfigPortalTimeout(180);
    Serial.println(F("Setup portal open for 3 minutes"));
    res = wm.startConfigPortal("cloudSmoker_setUp_AP");
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.mode(WIFI_STA);
      WiFi.begin();  // rejoin saved Wi-Fi after the portal closes
    }
  } else {
    wm.setConfigPortalTimeout(wm.getWiFiIsSaved() ? 15 : 180);
    Serial.println(F("Connecting to WiFi..."));
    res = wm.autoConnect("cloudSmoker_setUp_AP");
  }

  // Save the fields. A blank value is stored as blank, which turns that feature off.
  devSetThingSpeakKey(String(tsParam.getValue()));
  if (ntfyTopicValid(ntfyParam.getValue())) {
    devSetNtfyTopic(String(ntfyParam.getValue()));
  } else {
    Serial.println(F("ntfy topic has invalid characters, previous topic kept"));
  }
  Serial.print(F("ThingSpeak logging: "));
  Serial.println(devThingSpeakKey().length() ? F("on") : F("off"));
  Serial.print(F("ntfy alerts: "));
  Serial.println(devNtfyTopic().length() ? F("on") : F("off"));

  if (!res) {
    Serial.println(F("Not connected to WiFi. Continuing offline."));
  } else {
    Serial.println("Connected to WiFi! IP: " + WiFi.localIP().toString());
  }
}

// ---------- Non-blocking setup portal, opened from Settings > Connectivity ----------
// Unlike WifiManager_initialise(), these return straight away, so the device keeps running
// and Hold can finish the portal early. The WiFiManager objects exist only while it is open.
static WiFiManager* portalWm = nullptr;
static WiFiManagerParameter* portalTs = nullptr;
static WiFiManagerParameter* portalNtfy = nullptr;
static uint32_t portalStartMs = 0;
static const uint32_t PORTAL_WINDOW_MS = 180000UL;  // three minutes

bool portalIsOpen() {
  return portalWm != nullptr;
}

void portalOpen() {
  if (portalIsOpen()) {
    return;
  }
  WiFi.mode(WIFI_AP_STA);  // keep the saved Wi-Fi link while the setup network is up

  portalWm = new WiFiManager();
  portalWm->setHostname("cloudSmoker2");
  portalWm->setDebugOutput(true);

  String keyNow = devThingSpeakKey();
  String topicNow = devNtfyTopic();
  portalTs = new WiFiManagerParameter("tskey", "ThingSpeak write key (blank = off)", keyNow.c_str(), 40);
  portalNtfy = new WiFiManagerParameter("ntfy", "ntfy topic (blank = off)", topicNow.c_str(), 40);
  portalWm->addParameter(portalTs);
  portalWm->addParameter(portalNtfy);

  // Saves on submit, so the values are stored even while the portal stays open
  portalWm->setSaveParamsCallback([]() {
    devSetThingSpeakKey(String(portalTs->getValue()));
    if (ntfyTopicValid(portalNtfy->getValue())) {
      devSetNtfyTopic(String(portalNtfy->getValue()));
    }
    Serial.print(F("portal submitted, ntfy topic: "));
    Serial.println(devNtfyTopic());
  });

  portalWm->setConfigPortalBlocking(false);
  portalWm->startConfigPortal("cloudSmoker_setUp_AP");
  portalStartMs = millis();
  Serial.println(F("Setup portal open for 3 minutes"));
}

void portalClose();

// Call each pass while the portal is open. Returns false once it has closed.
bool portalPump() {
  if (!portalIsOpen()) {
    return false;
  }
  portalWm->process();
  if ((uint32_t)(millis() - portalStartMs) >= PORTAL_WINDOW_MS) {
    portalClose();
    return false;
  }
  return true;
}

void portalClose() {
  if (!portalIsOpen()) {
    return;
  }
  portalWm->stopConfigPortal();  // also stops the access point
  delete portalWm;
  portalWm = nullptr;
  delete portalTs;
  portalTs = nullptr;
  delete portalNtfy;
  portalNtfy = nullptr;

  WiFi.mode(WIFI_STA);
  WiFi.begin();  // rejoin the saved Wi-Fi
  Serial.print(F("Setup portal closed. ThingSpeak: "));
  Serial.print(devThingSpeakKey().length() ? F("on") : F("off"));
  Serial.print(F(", ntfy: "));
  Serial.println(devNtfyTopic().length() ? F("on") : F("off"));
}
