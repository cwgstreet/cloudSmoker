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

//YwRobot 1602 LCD with i2c i/o exapander backpack (PCF8574 or MCP23008)
//#include <hd44780.h>
//#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

/******************************************************
// Class definitions for periphial functions and tests:
*******************************************************/

/* class CWG_SerialMonitor {
   public:
    void functionTest();
}; */


#endif  // end header guard