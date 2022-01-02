/* ***************************************************************
* helper_functions.h - library containing genral use helper functions 
* 
*  C W Greenstreet, Ver1, 27Sep21
*    MIT Licence - Released into the public domain
*
** ************************************************************* */

#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
#include <Arduino.h>
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block


extern long unsigned startCookTimeMS; 

/******************************************************
// helper function prototypes
*******************************************************/
float convertDegFtoDegC(float tempDegF);
float getElapsedCookTime(long unsigned startCookTimeMS);
float getCountMS(float tempMS);

#endif  // end header guard