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
    // explicitly set mode, ESP defaults to STA+AP
    WiFi.mode(WIFI_STA);  // good practice to ensure code sets wifi mode how you want it

    // WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // uncomment line below to reset settings - wipes stored testing credentials, stored by the esp library
    // wm.resetSettings();

    //? Debugging
    // Debug is enabled by default on Serial in non-stable releases.
    // To disable, uncomment following line (must add bcommandefore autoConnect/startConfigPortal
    //!  My testing showed no space savings by turning off debugging - use WM_NODEBUG in platformio.ini build flag instead
    //wm.setDebugOutput(false);

    //? set Sta custom hostname
    wm.setHostname("cloudSmoker");

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    res = wm.autoConnect("cloudSmoker_setUp_AP"); // anonymous ap w/ custom name
    //res = wm.autoConnect("AutoConnectAP", "password");  // password protected ap

    if (!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } else {
        // if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
        Serial.println();
        Serial.print("ESP Board MAC Address:  ");
        Serial.println(WiFi.macAddress());
    }

}  // end wifi_initialise()

///////////////////////////////////////////////////////////////////

#endif  // end header guard