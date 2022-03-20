/* ***************************************************************
 * secretsTemplate.h - template library listing confidential constants
 *   usernames, passwords, API keys and similar
 *   >>>>> NO CONFIDENTIAL DATA IN THIS FILE _ JUST PLACEHOLDER NAMES <<<<<<<
 *  Per .gitignore, the actual secrets.h file will not be uploaded into public repository
 *
 *  C W Greenstreet, Ver1, 20Mar2022
 *    MIT Licence - Released into the public domain
 *
 ** ************************************************************* */
// Used pre-processor debug scaffold to inactivate this library; otherwise linker 
//  will generate multiple definition errors as actual security values are in secrets.h 
//  which is not being uploaded to github.  
//
//  uncomment the following line if you want compliler to process this library
//#define DEBUG 1


#if DEBUG

#ifndef SECRETS_TEMPLATE_H
#define SECRETS_TEMPLATE_H

#if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
#include <Arduino.h>
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block

/* ******************************************************
 *   WiFi Connection Parameters
 * *******************************************************/
const char* SSID = "YOUR_SSID";                    // The SSID (name) of the Wi-Fi network you want to connect to
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";  // The password of the Wi-Fi network

/* ******************************************************
 *   ThingSpeak Connection Parameters
 * *******************************************************/
const unsigned long SECRET_THNGSPK_CHANNEL_ID = CHANNEL_ID_NUMBER;   // Replace with Thingspeak Channel ID
const char* SECRET_THNGSPK_WRITE_API_KEY = "APIKEY";    // Replace with Thingspeak Write API Key

// includ Read API Key for potential future use:
//const char* SECRET_THNGSPK_READ_API_KEY = "APIKEY";  // Replace with Thingspeak Read API Key

#endif  // end header guard

#endif  // end debug scaffold