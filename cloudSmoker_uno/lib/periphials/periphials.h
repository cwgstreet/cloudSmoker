/* ***************************************************************
* periphials.h - library containing functions to test and use
*   cloudSmoker hardware serial monitor and LCD periphials
* 
*  C W Greenstreet, Ver1, 27Sep21
*    MIT Licence - Released into the public domain
*
** ************************************************************ */

#ifndef PERIPHIALS_H
#define PERIPHIALS_H

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
#include <Wire.h>  

//YwRobot 1602 LCD with i2c i/o exapander backpack (PCF8574 or MCP23008)
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

// KY40 Rotary Encoder - encoder CW / CCW counts
//#include <NewEncoder.h>

// KY40 push button
/* #include <Bounce2.h>
#include <Yabl.h> */

/******************************************************
// Class definitions for periphial functions and tests:
*******************************************************/

class CWG_SerialMonitor {
   public:
    void functionTest();
};

class CWG_LCD {
   public:
    CWG_LCD(const int lcdCols, const int lcdRows);  //constructor - will initialise lcdCols, lcdRows

    void functionTest();

   private:
    int _numCols;
    int _numRows;
};

#endif  // end header guard