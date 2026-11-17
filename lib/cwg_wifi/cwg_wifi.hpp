/* ***************************************************************
 * cwg_wifi.hpp - Wi-Fi connection and setup portal, extending the tzapu WiFiManager library
 *   Connects to saved Wi-Fi at boot, or opens the setup portal. A non-blocking portal can be
 *   opened from Settings. Implementation is in cwg_wifi.cpp.
 *
 *   C W Greenstreet, cloudSmoker2, 25Oct2026
 *   Licence (this file): GPLv3
 *
 *   Built on:
 *     tzapu / WiFiManager, MIT Licence
 *       reference:  https://github.com/tzapu/WiFiManager
 * ************************************************************* */

#ifndef CWG_WIFI_HPP
#define CWG_WIFI_HPP

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

void WifiManager_initialise(bool openPortalNow = false);

// Non-blocking setup portal, opened from Settings > Connectivity
bool portalIsOpen();
void portalOpen();
bool portalPump();  // call each pass while the portal is open; false once it has closed
void portalClose();

#endif  // end header guard
