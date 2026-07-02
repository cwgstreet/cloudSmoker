/* ***************************************************************
 * secrets.h - Confidential constants for cloudSmoker2
 * * Per .gitignore, this file is excluded from public repositories.
 * ************************************************************* */

#ifndef SECRETS_H
#define SECRETS_H

#include <Arduino.h>

// ******************************************************
//?   WiFi Connection Parameters
//    Note: Handled dynamically at runtime by WiFiManager
// *******************************************************

// ******************************************************
//?  ThingSpeak Connection Parameters
// *******************************************************/
const unsigned long THNGSPK_CHANNEL_ID = 1679986; 
const char* THNGSPK_WRITE_API_KEY = "NCLS0W62VML4VO11"; 

#endif  // SECRETS_H
