/* ***************************************************************
 * cwg_wifi.hpp - library to further abstract and extend tzapu WifiManager Lib
 *
 *  Based on
 *   1)   tazpu / WifiManager, MIT Licence
 *         reference:  https://github.com/tzapu/WiFiManager
 * *
 *  C W Greenstreet, Ver1, 3Apr2022
 *    MIT Licence - Released into the public domain
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


//------------------------------------------------
//?  WifiManager_initialise() - initialisation function for WiFiManager
//?    to be placed in setup() block in main code
//-----------------------------------------------
// Automatically connect using saved credentials,
//  if connection fails, it starts an access point with the specified name "cloudSmoker_setUpAP",
//  if empty, will auto-generate SSID, if password is blank, it will be anonymous AP (wm.autoConnect())
// then goes into a blocking loop awaiting configuration and will return success result
//-----------------------------------------------

void WifiManager_initialise() {
    //! add lcd msg about server config

    // 1. Set mode to Station (STA)
    WiFi.mode(WIFI_STA);

    // 2. WiFiManager local initialisation
    WiFiManager wm;

    // 3. Set a hostname for your device on the network
    wm.setHostname("cloudSmoker2");

    // 4. SET TIMEOUT: This is the critical fix.
    // If it can't connect to saved WiFi or user doesn't configure in 180 seconds,
    // the portal will close and the function will continue execution.
    wm.setConfigPortalTimeout(15); 
    wm.setDebugOutput(true);         // ignore WM_NODEBUG flag, force verbose here
 
    // 5. Attempt connection
    Serial.println("Connecting to WiFi...");
    bool res = wm.autoConnect("cloudSmoker_setUp_AP");

    if (!res) {
        // This will be reached if the timeout expires or connection fails
        Serial.println("Failed to connect or connection timed out. Continuing offline.");
        Serial.println("Continuing in offline mode.");
    } else {
    Serial.println("Connected to WiFi! IP: " + WiFi.localIP().toString());
    }
// 6. Reset settings if needed (uncomment to enable)
//--------------------------------------------------------
//!  Warning - this will erase all saved WiFi credentials and settings!
//!    Run once, comment back out and recompile to avoid repeated resets
//--------------------------------------------------------
//   wm.resetSettings();

} // end wifi_initialise()




///////////////////////////////////////////////////////////////////

#endif  // end header guard