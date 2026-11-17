/* ***************************************************************
 * helper_functions.cpp - implementation of helper_functions.h
 *
 *   C W Greenstreet, cloudSmoker2, 17Nov2026
 *   Licence (this file): GPLv3
 *
 *   Written with AI assistance (Claude). Under review by the author.
 * ************************************************************* */

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
    return millis() - startCookTime_ms;
}


