/* ***************************************************************
*  menu.h - cloudSmoker lcd menu library
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
//#include <Wire.h>

//YwRobot 1602 LCD with i2c i/o exapander backpack (PCF8574 or MCP23008)
//#include <hd44780.h>
//#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD


// Prepare menu entry case names for switch-case menu stucture
enum entryStates {
    Menu1_bbqStatus = 1,  //set enum 1 to 4 rather than default 0 for first element
    Menu1_setMeatDoneTemp 
    Menu2_setPitMinTemp,
    Menu3_setPitMaxTemp,
    Menu4_setTempUnits
};

// menuState defined in implementation menu.cpp, hence extern keyword
extern entryStates menuState;


/******************************************************
// Class definitions for lcd functions and tests:
*******************************************************/

class CloudSmoker_Menu {
   public:
    //CWG_LCD(const int lcdCols, const int lcdRows);  //constructor - will initialise lcdCols, lcdRows

   void showSplashScreen();
   void processMenu();
    
   private:
    int _numCols;
    int _numRows;
};
 
/******************************************************
// Helper function prototype:
*******************************************************/

//void gotoRowCol(int thisRow, int thatCol);  //move into class?


#endif  // end header guard