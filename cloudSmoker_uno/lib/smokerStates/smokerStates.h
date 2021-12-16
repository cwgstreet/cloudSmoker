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

/*****************
// Libraries required for periphial function tests:
*****************/

// i2C devices
//#include <Wire.h>

// Prepare case names for cloudSmoker State Machine switch-case stucture
enum entryStates { splashScreen = 1, //set enum 1 to 11 rather than default 0 for first element
                   launchPad,          
                   changeSettings,
                   setMeatDoneTemp,
                   setPitMinTemp,
                   setPitMaxTemp,
                   setTempUnits,
                   getTemp,
                   txTemp,
                   modemSleep, 
                   bbqStatus };   // 11

// smokerState defined in implementation smokerState.cpp, hence extern keyword
extern entryStates smokerState;

/******************************************************
// Class definitions for cloudSmoker state machine:
*******************************************************/

class CloudSmoker_State {
   public:
    //CWG_LCD(const int lcdCols, const int lcdRows);  //constructor - will initialise lcdCols, lcdRows

    
    void processState();
    

   private:
    void showSplashScreen();
    void showStatusScreen();
    //int _numCols;
    //int _numRows;
};

/******************************************************
// Helper function prototype:
*******************************************************/

//void gotoRowCol(int thisRow, int thatCol);  //move into class?

#endif  // end header guard