/* ***************************************************************
*  menu.h - cloudSmoker state maching functionality including 
*     lcd menu 
* 
*  C W Greenstreet, Ver1, 4Dec21
*    MIT Licence - Released into the public domain
*
** ************************************************************* */

#ifndef MENU_H
#define MENU_H

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

// lcd
//#include <lcd.h>

// Prepare menu entry case names for switch-case menu stucture
enum entryStates { bbqStatus = 1,  //set enum 1 to 4 rather than default 0 for first element
                   Menu2_setMeatDoneTemp,
                   Menu3_setPitMinTemp,
                   Menu4_setPitMaxTemp,
                   Menu5_setTempUnits };

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
    void displayStatusScreen();

   private:
    int _numCols;
    int _numRows;
};

/******************************************************
// Helper function prototype:
*******************************************************/

//void gotoRowCol(int thisRow, int thatCol);  //move into class?

#endif  // end header guard