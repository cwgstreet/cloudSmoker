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
//#include <NewEncoder.h>
//#include <Wire.h>                           // i2C device communication
//#include <hd44780.h>                        // LCD library
//#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

//incliude local libraries
#include <periphials.h>

CWG_SerialMonitor SerialTerminal;  //instantiate terminal object from periphials library

// Serial Monitor function test to establish if monitor is working correctly.
//   Place function call in setup (not loop) to ensure single occurence of serial
//     monitor initialisation text rather than repeating scrolling text if placed in loop
void CWG_SerialMonitor::functionTest() {
    Serial.println(F("   "));  // blank line to make easier to read
    Serial.println(F("|-------------------------------------------|"));
    Serial.println(F("  Initialising serial monitor function test"));
    Serial.println(F("     Serial Monitor Test Passed"));
    Serial.println(F("         G'day cloudSmoker!"));
    Serial.println(F("|-------------------------------------------|"));
    Serial.println(F("   "));
}
