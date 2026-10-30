/* ***************************************************************
 * secretsTemplate.h - template library listing confidential constants
 *   usernames, passwords, API keys and similar
 *   >>>>> NO CONFIDENTIAL DATA IN THIS FILE _ JUST PLACEHOLDER NAMES <<<<<<<
 *  Per .gitignore, the actual secrets.h file will not be uploaded into public repository
 *
 *  C W Greenstreet, Ver2, 25Oct2026
 *    GPLv3 Licence
 *
 ** ************************************************************* */

//! >>>>> template commented out to prevent multiple variable definition <<<<<

// #ifndef SECRETS_TEMPLATE_H
// #define SECRETS_TEMPLATE_H

// #if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
// #include <Arduino.h>
// #else
// #include <WConstants.h>
// #include <WProgram.h>
// #include <pins_arduino.h>
// #endif  // end if-block

// ******************************************************
//?    WiFi Connection Parameters
//!     Not used!  cloudSmoker went with more secure Wifi Manager
//!       instead where this info is entered on start-up
// *******************************************************
const char* SSID = "YOUR_SSID";
// The SSID (name) of the Wi-Fi network you want to connect to
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
// The password of the Wi-Fi network

// ******************************************************
//?  ThingSpeak Connection Parameters
// *******************************************************/
const unsigned long THNGSPK_CHANNEL_ID = CHANNEL_ID_NUMBER;
// Replace with Thingspeak Channel ID
const char* THNGSPK_WRITE_API_KEY = "APIKEY";
// Replace with Thingspeak Write API Key

// TODO include Read API Key for potential future use:
// const char* THNGSPK_READ_API_KEY = "APIKEY";
// Replace with Thingspeak Read API Key

// ******************************************************
//? ntfy.sh Connection Parameters
// *******************************************************/
const char* NTFY_TOPIC = "TOPIC KEY"
/* Replace with your ntfy.sh topic key - generate a long, random
    topic name rather than using your project name or another easily guessed string.
   Anyone who discovers the topic name could potentially subscribe to your notifications
     or send messages to it. */

#endif  // end header guard
