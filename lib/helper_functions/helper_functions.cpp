/* ***************************************************************
* periphials.h - library containing functions to test and use
*   cloudSmoker hardware periphials: LCD and Serial Monitor
* 
*  C W Greenstreet, Ver1, 27Sep21
*    MIT Licence - Released into the public domain
*
** ************************************************************ */

// include 3rd party libraries
#include <Arduino.h>
#include <NewEncoder.h>
#include <Wire.h>                           // i2C device communication
#include <hd44780.h>                        // LCD library
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

//incliude local libraries
#include <periphials.h>

/******************************************************
// helper functions
*******************************************************/

// Deg F to Deg C conversion (all temps stored as Deg F)
float convertDegFtoDegC(float tempDegF){
    float tempDegC = (tempDegF - 32) * 5/9;
    return tempDegC;
}

