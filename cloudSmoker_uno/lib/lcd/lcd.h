/* ***************************************************************
* lcd.h - library containing functions to use lcd, including 
*   custom characters
* 
*  C W Greenstreet, Ver1, 4Dec21
*    MIT Licence - Released into the public domain
*
** ************************************************************* */

#ifndef LCD_H
#define LCD_H

#if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
#include <Arduino.h>
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block

/******************************************************
// Libraries required for periphial function tests:
*******************************************************/

// i2C devices
#include <Wire.h>

//YwRobot 1602 LCD with i2c i/o exapander backpack (PCF8574 or MCP23008)
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD


/******************************************************
// Class definitions for lcd functions and tests:
*******************************************************/

class CWG_LCD {
   public:
    CWG_LCD(const int lcdCols, const int lcdRows);  //constructor - will initialise lcdCols, lcdRows

    void functionTest();
    void initialiseCustomCharSet();
    void showSplashScreen(bool degCFlag, float meatDoneTemp, float pitTemp);
    void displayTest();


   private:
    int _numCols;
    int _numRows;
};

/******************************************************
// Helper function prototype:
*******************************************************/

void gotoRowCol(int thisRow, int thatCol);  //move into class?


#endif  // end header guard