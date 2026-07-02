/* ***************************************************************
 * peripherals.h - library containing functions to test and use
 *   cloudSmoker hardware peripherals: LCD (TFT in Ver2) and Serial Monitor
 *
 *    C W Greenstreet, cloudSmoker2, 1Jul26
 *     Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public 
 *           distribution must also share the full source code under this same license.
 *
 ** ************************************************************ */

// include 3rd party libraries
#include <Arduino.h>
#include <NewEncoder.h>
#include <Wire.h>                           // i2C device communication
//#include <hd44780.h>                        // Depreciated - can remove LCD library
//#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

// include local libraries
#include "peripherals.h"
#include "helper_functions.h"

/******************************************************
// helper functions
*******************************************************/

// Degree F to Degree C conversion (all cloudSmoker temps stored as Deg F)
float convertDegFtoDegC(float tempDegF) {
    return (tempDegF - 32) * 5 / 9;
}


// get elapsed time from start of Cook in milliseconds
long unsigned getElapsedCookTime_ms() {
    return startCookTime_ms - millis();
}


