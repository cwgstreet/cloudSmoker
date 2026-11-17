/* ***************************************************************
 * helper_functions.h - general-use helper functions for cloudSmoker2
 *
 *   C W Greenstreet, cloudSmoker2, 25Oct2026
 *   Licence (this file): GPLv3
 *
 *   Written with AI assistance (Claude). Under review by the author.
 * ************************************************************* */

#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
#include <Arduino.h>
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block

extern long unsigned startCookTime_ms;

/******************************************************
// helper function prototypes
*******************************************************/
float convertDegFtoDegC(float tempDegF);
long unsigned getElapsedCookTime_ms();

#endif  // end header guard