/* ***************************************************************
 * smokerStates.h - cloudSmoker state machine implementation routines
 *
 * See (github link tba) for cloudSmoker State Machine diagram
 *
 *  C W Greenstreet, Ver1, 16Dec21
 *    MIT Licence - Released into the public domain
 *
 ** ************************************************************* */

#ifndef SMOKER_STATES_H
#define SMOKER_STATES_H

#if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
#include <Arduino.h>
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block

// Prepare case names for cloudSmoker State Machine switch-case stucture
enum entryStates_t { splashScreen = 1,  // set enum 1 to 10 rather than default 0 for first element
                     launchPad,
                     changeSettings,
                     setMeatDoneTemp,
                     setPitTempTarget,
                     setTempUnits,
                     getTemp,
                     txTemp,
                     modemSleep,
                     bbqStatus };  // 10

// smokerState type defined in implementation smokerState.cpp, hence extern keyword
extern entryStates_t smokerState;


extern long unsigned startCookTime_ms;

/******************************************************
// Helper function prototype:
*******************************************************/

// process_state() prototype
void processState(CWG_LCD &lcd);


#endif  // end header guard