/* ***************************************************************
 * smokerStates.h - cloudSmoker state machine implementation routines
 *
 * See
 * https://github.com/cwgstreet/cloudSmoker/blob/470cf69b76abfa9f5d605230191536e27d5985e3/Images/CloudSmoker%20state%20diagram2.jpg
 * for cloudSmoker1 State Machine diagram (to be updated for cloudSmoker2)
 *
 * C W Greenstreet, cloudSmoker2, 1Jul26
 * Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 * distribution must also share the full source code under this same licence.)
 *
 *************************************************************** */

#ifndef SMOKER_STATES_H
#define SMOKER_STATES_H

#if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
#include <Arduino.h>
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block

// UPGRADED: Conditional include to handle the network stack swap cleanly
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

// Prepare case names for cloudSmoker State Machine switch-case stucture
enum entryStates_t {
  splashScreen = 1,  // set enum 1 to 10 rather than default 0 for first element
  launchPad,
  changeSettings,
  setMeatDoneTemp,
  setPitTempTarget,
  setTempUnits,
  getTemp,
  txTemp,
  Sleep,
  bbqStatus
};  // 10

// smokerState type defined in implementation smokerState.cpp, hence extern keyword
extern entryStates_t smokerState;

// globals, defined in main
extern long unsigned startCookTime_ms;
extern long unsigned previousMillis;
extern long unsigned transmitInterval;
extern long unsigned previousDisplayMillis;
extern long unsigned displayInterval;

/******************************************************
// Helper function prototype:
*******************************************************/

// process_state() prototype
void processState(CWG_LCD& lcd);

#endif  // end header guard