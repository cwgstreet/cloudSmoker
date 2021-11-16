/* ***************************************************************
* periphials.h - library containing functions to test and use
*   cloudSmoker hardware periphials
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

//Recommend setting Serial Monitor speed to match ESP8266 fixed bootloader initialisation of 74880
//  (otherwise you will get startup gibberish characters on serial monitor before serial speed syncs)
#define SERIAL_MONITOR_SPEED 74880

//Yabl::Button button;                             //instantiate button object from Yabl library
hd44780_I2Cexp lcd;                                //instantiate lcd object: auto locate & auto config expander chip
//NewEncoder encoder(2, 3, -20, 20, 0, FULL_PULSE);  // instantiate encoder object
//int16_t prevEncoderValue;

//LCD constructor passing lcd rows and columns
CWG_LCD::CWG_LCD(const int lcdCols, const int lcdRows) {
    _numCols = lcdCols;
    _numRows = lcdRows;
}

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

// LCD.function test to determine if LCD is working properly.  Place function call in setup (not loop)
void CWG_LCD::functionTest() {
    int LCDstatus;
    LCDstatus = lcd.begin(_numCols, _numRows);  // begin() automatically turns on  backlight

    if (LCDstatus)  // non zero status means fatal error.  will blink onboard LED if possible
    {
        hd44780::fatalError(LCDstatus);  // does not return
    }
    // otherwise, initalization was successful, the backlight should be on now
    // Print a message to the LCD to show LCD successfully initialised
    lcd.print(F("G'Day cloudSmkr!"));  // F-Macro to save dynamic memory
    lcd.setCursor(0, 1);               //set cursor to first column of second row (forst position == 0)
    delay(3000);
    lcd.print(F("LCD test passed"));
    //  end LCD function test
}
