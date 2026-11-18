/* ***************************************************************
 * smokerStates.h - cloudSmoker state machine: state names and shared globals
 *   The states of the cloudSmoker state machine. Implementation is in smokerStates.cpp.
 *
 *   Origin: cloudSmoker1 (ESP8266) state machine, C W Greenstreet.
 *     State diagram (cloudSmoker1, to be updated for cloudSmoker2):
 *     https://github.com/cwgstreet/cloudSmoker/blob/470cf69b76abfa9f5d605230191536e27d5985e3/Images/CloudSmoker%20state%20diagram2.jpg
 *   Extended for cloudSmoker2 with AI assistance (Claude) with review by the author.
 *
 *   C W Greenstreet, cloudSmoker2, 17Nov2026
 *   Licence (this file): GPLv3
 * ************************************************************* */

#ifndef SMOKER_STATES_H
#define SMOKER_STATES_H

#include <Arduino.h>
#include <WiFi.h>

// Prepare case names for cloudSmoker State Machine switch-case structure
enum entryStates_t {
  splashScreen = 1,  // start at 1 rather than the default 0
  launchPad,
  changeSettings,
  setMeatDoneTemp,
  setPitTempTarget,
  setTempUnits,
  getTemp,
  txTemp,
  Sleep,
  bbqStatus,
  calProbeMenu,    // choose which probe to calibrate, or reset it
  calWaitIce,      // settling and capture at 0 degF
  calWaitBoil,     // reference temperature entry and capture at boiling
  calResult,       // show gain and offset, save or cancel
  railStatus,      // bench check: INA226 rail and stored rail voltage
  resumePrompt,    // interrupted cook found at boot: tap to resume, hold to restart
  meatTempMenu,    // list of present meat probes, then Back
  editMeatTarget,  // target editor for one meat probe, always entered in degF
  pitTempMenu,     // PIT Min, Max, Back
  editPitLimit,
  bankMenu,  // power bank settings: capacity and charge
  editBank,  // capacity or charge editor
  cookMenu,
  bleMenu,
  setupPortal,  // Connectivity: setup portal open, Hold to finish
  updateCheck   // Check Update: runs the check, shows the result, tap or Hold to return
};

bool cookApplyOnBoot();

// smokerState variable defined in smokerStates.cpp, hence the extern keyword
extern entryStates_t smokerState;

// globals, defined in main
extern long unsigned startCookTime_ms;
extern long unsigned previousMillis;
extern long unsigned transmitInterval;
extern long unsigned previousDisplayMillis;
extern long unsigned displayInterval;
extern bool hasRunFlag;  // defined in main

/******************************************************
// Helper function prototype:
*******************************************************/

class CWG_LCD;  // full definition in lcd.h

// processState() prototype
void processState(CWG_LCD& lcd);

extern bool bleSerialEnabled;
#endif  // end header guard