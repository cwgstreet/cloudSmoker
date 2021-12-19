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

// global variables declared in main program but needed in this library before they are declared elsewhere
extern float meatDoneTemp;
extern float pitTempTarget;
extern float currentMeatTemp;
extern float currentPitTemp;
extern bool degCFlag;

// define LCD geometry (YwRobot 1602 LCD)
extern const int LCD_COLS;
extern const int LCD_ROWS;

/******************************************************
// Class definitions for lcd functions and tests:
*******************************************************/

// child class of hd44780_I2Cexp which is a child of hd44780
class CWG_LCD : public hd44780_I2Cexp {
   public:
    CWG_LCD(const int LCD_COLS, const int LCD_ROWS);  //constructor - will initialise lcdCols, lcdRows

    void printMenuLine(char *c);

    void functionTest();
    void displayTest();
    void initialiseCustomCharSet();
    void showSplashScreen(bool degCFlag, float meatDoneTemp, float pitTempTarget);
    void showLaunchPad();
    void showSettingsMenu();

   private:
    int _numCols;
    int _numRows;
};

extern CWG_LCD lcd;  // ensure lcd object is visable everywhere

/******************************************************
// Helper function prototype:
*******************************************************/

//void gotoRowCol(int thisRow, int thatCol);  //move into class?

#endif  // end header guard