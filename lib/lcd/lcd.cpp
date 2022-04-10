/* ***************************************************************
 * lcd.h - library containing functions to use lcd, including
 *   custom character set
 *
 *  C W Greenstreet, Ver1, 4Dec21
 *    MIT Licence - Released into the public domain
 *
 ** ************************************************************* */

// set up debug scaffold; comment out following line if you want to "turn off" serial monitor debugging
#define DEBUG 1

// include 3rd party libraries
#include <Arduino.h>
#include <NewEncoder.h>                     // do I need this here anymore?  use is in smokerStates now?
#include <Wire.h>                           // i2C device communication
#include <hd44780.h>                        // LCD library
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

// incliude local libraries
#include "helper_functions.h"  // do I need this here anymore?  use is in smokerStates now?
#include "lcd.h"
#include "press_type.h"
#include "smokerStates.h"
#include "wrapEncoder.h"

// define LCD geometry (YwRobot 1602 LCD)
constexpr int LCD_COLS = 16;
constexpr int LCD_ROWS = 2;

// LCD display varibles
byte menuPrintLine = 0;
byte menuSelectLine = 0;
char messageBuffer[17];  // empty buffer array; 16 char LCD screen + terminating null char

// LCD constructor passing lcd rows and columns
CWG_LCD::CWG_LCD(const int LCD_COLS, const int LCD_ROWS) {
    _numCols = LCD_COLS;
    _numRows = LCD_ROWS;
}

CWG_LCD lcd(LCD_COLS, LCD_ROWS);  // instantiate lcd object: auto locate & auto config expander chip

// LCD initialisation.  Tests if LCD is working properly.  Place function call in setup (not loop)
void CWG_LCD::initialiseLCD() {
    int LCDstatus;
    LCDstatus = lcd.begin(_numCols, _numRows);  // begin() automatically turns on  backlight

    if (LCDstatus)  // non zero status means fatal error.  will blink onboard LED if possible
    {
        hd44780::fatalError(LCDstatus);  // does not return
    }
    // otherwise, initalization was successful, the backlight should be on now
    // Print a message to the LCD to show LCD successfully initialised
    lcd.print(F("G'Day cloudSmkr!"));  // F-Macro to save dynamic memory
    delay(1000);
    lcd.clear();
    lcd.print(F("LCD test passed"));
    lcd.setCursor(0, 1);               // set cursor to first column of second row (first position == 0)
    lcd.print(F("G'Day cloudSmkr!"));  // F-Macro to save dynamic memory
    delay(1000);
}  //  end LCD initialisation

/////////////////////////////////////////////////////////////////////////////////////////
// Custom character data - symbols mostly borrowed from Prusa Mk3 firmware, unless noted
//   Ref: https://github.com/prusa3d/Prusa-Firmware/blob/MK3/Firmware/lcd.cpp
//
////////////////////////////////////////////////////////////////////////////////////////

const uint8_t lcd_custChar_degree[8] PROGMEM = {  // need to refer to element 0 in array as "/010", binary 8
    B01100,                                       //   **     note: alternative degree symbol is in character set "/337"
    B10010,                                       //  *  *
    B10010,                                       //  *  *
    B01100,                                       //   **
    B00000,
    B00000,
    B00000,
    B00000};  // Source: Prusa

// uint8_t degreeC[8]     = {0x18,0x18,0x03,0x04,0x04,0x04,0x03,0x00};
const uint8_t lcd_custChar_degreeC[8] PROGMEM = {
    B11000,   //  **
    B11000,   //  **
    B00011,   //     **
    B00100,   //    *
    B00100,   //    *
    B00100,   //    *
    B00011,   //     **
    B00000};  // source: https://github.com/duinoWitchery/hd44780/blob/master/examples/ioClass/hd44780_I2Cexp/LCDCustomChars/LCDCustomChars.ino

// uint8_t degreeF[8]     = {0x18,0x18,0x07,0x04,0x07,0x04,0x04,0x00};
const uint8_t lcd_custChar_degreeF[8] PROGMEM = {
    B11000,   //  **
    B11000,   //  **
    B00111,   //    ***
    B00100,   //    *
    B00111,   //    ***
    B00100,   //    *
    B00100,   //    *
    B00000};  // source: https://github.com/duinoWitchery/hd44780/blob/master/examples/ioClass/hd44780_I2Cexp/LCDCustomChars/LCDCustomChars.ino

const uint8_t lcd_custChar_thermometer[8] PROGMEM = {
    B00100,   //    *
    B01010,   //   * *
    B01010,   //   * *
    B01010,   //   * *
    B01010,   //   * *
    B10001,   //  *   *
    B10001,   //  *   *
    B01110};  //   ***       Source: Prusa

const uint8_t lcd_custChar_uplevel[8] PROGMEM = {
    B00100,   //    *
    B01110,   //   ***
    B11111,   //  *****
    B00100,   //    *
    B11100,   //  ***
    B00000,   //
    B00000,   //
    B00000};  //    Source: Prusa

const uint8_t lcd_custChar_arrup[8] PROGMEM = {
    B00100,   //    *
    B01110,   //   ***
    B11111,   //  *****
    B00100,   //    *
    B00100,   //    *
    B00000,   //
    B00000,   //
    B00000};  //         Source: Prusa

// Surplus symbol - swapped out as not used and limited to 8 special characters
/*
const uint8_t lcd_custChar_arrdown[8] PROGMEM = {
    B00000,   //
    B00000,   //
    B00000,   //
    B00100,   //    *
    B00100,   //    *
    B11111,   //  *****
    B01110,   //   ***
    B00100};  //    *    Source: Prusa
 */

const uint8_t lcd_custChar_selectArrLeft[8] PROGMEM = {
    B00010,   //      *
    B00110,   //     **
    B01110,   //    ***
    B11110,   //   ****
    B01110,   //    ***
    B00110,   //     **
    B00010,   //      *
    B00000};  //         custom made

// const uint8_t lcd_custChar_selectArrRight[8] PROGMEM = {
const uint8_t lcd_custChar_selectArrRight[8] PROGMEM = {
    B01000,   //   *
    B01100,   //   **
    B01110,   //   ***
    B01111,   //   ****
    B01110,   //   ***
    B01100,   //   **
    B01000,   //   *
    B00000};  //         custom made

// creates eight custom charcters to use in menu or status screens
void CWG_LCD::initialiseCustomCharSet() {
    lcd.createChar(0, lcd_custChar_degree);  // use \010 (binary for 8 as \000 is null termination and will cause problems
    lcd.createChar(1, lcd_custChar_degreeC);
    lcd.createChar(2, lcd_custChar_degreeF);
    lcd.createChar(3, lcd_custChar_thermometer);
    lcd.createChar(4, lcd_custChar_uplevel);
    lcd.createChar(5, lcd_custChar_arrup);
    // lcd.createChar(6, lcd_custChar_arrdown); // substituted out for symbol below
    lcd.createChar(6, lcd_custChar_selectArrLeft);
    lcd.createChar(7, lcd_custChar_selectArrRight);
}

// confirming functionality of lcd display of special characters
void CWG_LCD::displayTest() {
    lcd.clear();
    // lcd.print("\008" "\001" "\002" "\003" "\004" "\005" "\006" "\007");
    lcd.setCursor(0, 0);
    lcd.print(
        "\010"
        "\001"
        "\002"
        "\003"
        "\004"
        "\005"
        "\006"
        "\007"
        "\337");
    lcd.setCursor(0, 1);  // display second line on lcd
    lcd.print(
        "temp is 210"
        "\002");
    delay(2000);
}

// *********************************************************************************************
//   printMenuLine function adapted from Open Vapors project, MIT Licence.
//     Ref: https://github.com/baldengineer/Open-Vapors
// *********************************************************************************************

void CWG_LCD::printMenuLine(const char *c) {
    unsigned int lineWidth = LCD_COLS;
    lcd.setCursor(0, menuPrintLine);
    if (menuPrintLine == menuSelectLine) {
        lcd.print((char)0x7e);  // 0x7e is arrow in LCD character set
        lineWidth = lineWidth - 1;
    } else {
        lcd.print(F(" "));
        lineWidth = lineWidth - 1;
    }
    // test for character overflow error; causes blank LCD line if printing more characters than display width
    if (strlen(c) > lineWidth) {
        lcd.print(F("Overflow error!"));
    } else {
        lcd.print(c);
        for (unsigned int x = 0; x < (lineWidth - strlen(c)); x++) {
            lcd.print(F(" "));
        }
    }
    menuPrintLine++;
    if (menuPrintLine > 1) menuPrintLine = 0;
}

//! ************************************************************************************
/* ----------------------------------------------------------------------------------
// printElapsedTime() - convert millis based count to human readable time HH:MM:SS and prints it
//adapted from "jurs" at https://forum.arduino.cc/t/converting-milliseconds-to-time/423887/5
//  ----------------------------------------------------------------------------------
void printElapsedTime(unsigned long millisTime_ms) {

unsigned long allSeconds = millisTime_ms / 1000;
int elapsedHours = allSeconds / 3600;
int secondsRemaining = allSeconds % 3600;
int elapsedMinutes = secondsRemaining / 60;
int elapsedSeconds = secondsRemaining %60;

char elapsedTimebuffer[21];
sprintf(elapsedTimebuffer,"Runtime%02d:%02d:%02d", elapsedHours, elapsedMinutes, elapsedSeconds);
Serial.println(elapsedTimebuffer);
//delay(1000);

}
*/
//! ************************************************************************************




void CWG_LCD::printMenuLine_noArrow(const char *c) {
    unsigned int lineWidth = LCD_COLS;
    lcd.setCursor(0, menuPrintLine);

    // test for character overflow error; causes blank LCD line if printing more characters than display width
    if (strlen(c) > LCD_COLS) {
        lcd.print(F("Overflow error!"));
    } else {
        lcd.print(c);
        for (unsigned int x = 0; x < (lineWidth - strlen(c)); x++) {
            lcd.print(F(" "));
        }
    }
    menuPrintLine++;
    if (menuPrintLine > 1) {
        menuPrintLine = 0;
    }
}

// format: dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
// sprintf(line0, "Temp: %-7sC", float_str); // %6s right pads the string
// *********************************************************************************************
void CWG_LCD::showSplashScreen(bool degCFlag, float meatDoneTemp, float pitTempTarget) {
    // lcd.printMenuLine("test works1");         //test code - REMOVE
    //  lcd.printMenuLine("test works234567");   //test code - REMOVE
    char msg[17];                // space for 16 charcaters + null termination
    char currentMeatTempStr[4];  // empty array to hold converted (to string) float meat temp + null
    char currentPitTempStr[4];
    lcd.setCursor(0, 0);

    if (degCFlag == 1) {
        float meatDoneTempC = convertDegFtoDegC(meatDoneTemp);
        float pitTempTargetC = convertDegFtoDegC(pitTempTarget);
        dtostrf(meatDoneTempC, 3, 0, currentMeatTempStr);  // (float var to convert, width==3, 0==no digits after decimal, char arra for output)
        dtostrf(pitTempTargetC, 3, 0, currentPitTempStr);
        snprintf(msg, sizeof(msg), "Meat%s\001 Pit%s\001", currentMeatTempStr, currentPitTempStr);

        lcd.printMenuLine_noArrow("BBQ set points:");
        lcd.printMenuLine_noArrow(msg);

    } else {
        dtostrf(meatDoneTemp, 3, 0, currentMeatTempStr);  // width==3, no digits after decimal
        dtostrf(pitTempTarget, 3, 0, currentPitTempStr);  // width==3, no digits after decimal sprintf(msg, "Meat%s\002 Pit%s\002", currentMeatTempStr, currentPitTempStr);
        snprintf(msg, sizeof(msg), "Meat%s\002 Pit%s\002", currentMeatTempStr, currentPitTempStr);

        lcd.printMenuLine_noArrow("BBQ set points:");
        lcd.printMenuLine_noArrow(msg);
    }
}

void CWG_LCD::showLaunchPad() {
    lcd.printMenuLine_noArrow("Cook: ShortPress");
    lcd.printMenuLine_noArrow("Config: DblPress");
}

void CWG_LCD::showSettingsMenu(int16_t currentEncoderValue) {
    switch (currentEncoderValue) {
        case 1:  // first line instructions
            menuSelectLine = 1;
            lcd.printMenuLine_noArrow("<Turn to scroll>");
            lcd.printMenuLine("<Press to set");
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("CWG_LCD::showSettingsMenu Case 1: -> prevEncoderValue:  "));
                Serial.print(prevEncoderValue);
                Serial.print(F(" / currentEncoderValue: "));  // debug
                Serial.println(currentEncoderValue);
            }
#endif
            break;

        case 2:  // next line instructions
            menuSelectLine = 0;
            lcd.printMenuLine("<Press to set>");
            lcd.printMenuLine("<Hold to exit>");
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println(F("entered showSettingsMenu "));
                Serial.println();
                Serial.print(F("CWG_LCD::showSettingsMenu Case 2: -> prevEncoderValue:  "));
                Serial.print(prevEncoderValue);
                Serial.print(F(" / currentEncoderValue: "));  // debug
                Serial.println(currentEncoderValue);
            }

#endif
            break;

        case 3:  // 3rd line instructions
            menuSelectLine = 1;
            lcd.printMenuLine("<Press to set>");
            lcd.printMenuLine("<Hold to exit>");
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("CWG_LCD::showSettingsMenu Case 3: -> prevEncoderValue:  "));
                Serial.print(prevEncoderValue);
                Serial.print(F(" / currentEncoderValue: "));  // debug
                Serial.println(currentEncoderValue);
            }
#endif
            break;

        case 4: {  // meatTemperatureTarget setting
            menuSelectLine = 1;
            lcd.printMenuLine("<Hold to exit>");
            bool meatTargetFlag = 1;
            bool adjTempFlag = 0;
            getTargetTemperatureMsg(messageBuffer, degCFlag, meatDoneTemp, meatTargetFlag, adjTempFlag);
            lcd.printMenuLine(messageBuffer);
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("CWG_LCD::showSettingsMenu Case 4: -> prevEncoderValue:  "));
                Serial.print(prevEncoderValue);
                Serial.print(F(" / currentEncoderValue: "));  // debug
                Serial.println(currentEncoderValue);
            }
#endif
        } break;

        case 5: {  // pitTemperatureTarget setting
            menuSelectLine = 1;
            bool meatTargetFlag = 1;
            bool adjTempFlag = 0;
            getTargetTemperatureMsg(messageBuffer, degCFlag, meatDoneTemp, meatTargetFlag, adjTempFlag);
            lcd.printMenuLine(messageBuffer);
            meatTargetFlag = 0;
            adjTempFlag = 0;
            getTargetTemperatureMsg(messageBuffer, degCFlag, pitTempTarget, meatTargetFlag, adjTempFlag);
            lcd.printMenuLine(messageBuffer);
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("CWG_LCD::showSettingsMenu Case 5: -> prevEncoderValue:  "));
                Serial.print(prevEncoderValue);
                Serial.print(F(" / currentEncoderValue: "));  // debug
                Serial.println(currentEncoderValue);
            }
#endif
            /* if (pressEventCode == SHORT_PRESS) {
                smokerState = setPitTempTarget;  // enter sub-menu to set pit temperature target
                Serial.print(F("CWG_LCD::showSettingsMenu smokerState = "));
                Serial.println(smokerState);
            } */
        } break;

        case 6: {
            menuSelectLine = 1;  // Temperature Units setting
            bool meatTargetFlag = 0;
            bool adjTempFlag = 0;
            getTargetTemperatureMsg(messageBuffer, degCFlag, pitTempTarget, meatTargetFlag, adjTempFlag);
            lcd.printMenuLine(messageBuffer);
            if (degCFlag) {
                lcd.printMenuLine("Units F/[C]");
            } else {
                lcd.printMenuLine("Units [F]/C");
            }
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("CWG_LCD::showSettingsMenu Case 6: -> prevEncoderValue:  "));
                Serial.print(prevEncoderValue);
                Serial.print(F(" / currentEncoderValue: "));  // debug
                Serial.println(currentEncoderValue);
            }
#endif
            /* if (pressEventCode == SHORT_PRESS) {
                smokerState = setTempUnits;  // enter sub-menu to set temperature units
                Serial.print(F("CWG_LCD::showSettingsMenu smokerState = "));
                Serial.println(smokerState);
            } */
        } break;

        case 7: {  // instruction to go up level
            menuSelectLine = 1;
            if (degCFlag) {
                lcd.printMenuLine("Units F/[C]");
            } else {
                lcd.printMenuLine("Units [F]/C");
            }
            lcd.printMenuLine("Hold to Return\004");
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("CWG_LCD::showSettingsMenu Case 7: -> prevEncoderValue:  "));
                Serial.print(prevEncoderValue);
                Serial.print(F(" / currentEncoderValue: "));  // debug
                Serial.println(currentEncoderValue);
            }
#endif
        } break;

        case 8: {  // wrap to beginning
            menuSelectLine = 1;
            lcd.printMenuLine("Hold to Return\004");
            lcd.printMenuLine_noArrow("<Turn to scroll>");
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("CWG_LCD::showSettingsMenu Case 7: -> prevEncoderValue:  "));
                Serial.print(prevEncoderValue);
                Serial.print(F(" / currentEncoderValue: "));  // debug
                Serial.println(currentEncoderValue);
            }
#endif
        } break;
    }
}

// getTargetTemperatureMsg(messageBuffer, degCFlag, temporaryTemperatureTarget, meatTargetFlag, adjTempFlag);
void CWG_LCD::getTargetTemperatureMsg(char (&messageBuffer)[17], bool degCFlag, float targetTemperature, bool meatTargetFlag, bool adjTempFlag) {
    // Serial.println(F("entered getTargetTemperatureMsg"));

    char TemperatureStr[4];  // empty array to hold converted (to string) float meat temp + null

    // TODO: array out of bounds error checking
    if (degCFlag == 1) {  // degC branch
        float targetTemperatureC = convertDegFtoDegC(targetTemperature);
        dtostrf(targetTemperatureC, 3, 0, TemperatureStr);  // (float var to convert, width==3, 0==no digits after decimal, char arra for output)
        // sprintf:  15 characters + null; Octal escape chars: \x5B = [  \x5D = ], /001 = degC custom char,  /007 /008 (/x7 or /x8) are custom character arrows
        if (meatTargetFlag) {  // focus: meatDoneTemp
            snprintf(messageBuffer, sizeof(messageBuffer), "Meat done\x5B%s\x5D\001", TemperatureStr);
            if (adjTempFlag) {
                snprintf(messageBuffer, sizeof(messageBuffer), "Meat done\x7%s\x6\001", TemperatureStr);
            }
        } else {  // else focus: pitTemp
            snprintf(messageBuffer, sizeof(messageBuffer), "Pit temp\x5B%s\x5D\001", TemperatureStr);
            if (adjTempFlag) {
                snprintf(messageBuffer, sizeof(messageBuffer), "Pit temp\x7%s\x6\001", TemperatureStr);
            }
        }
    } else {
        // else degF branch
        dtostrf(targetTemperature, 3, 0, TemperatureStr);  // width==3, no digits after decimal
        // sprintf:  15 characters + null; Octal escape chars: \x5B = [  \x5D = ], /001 = degC custom char
        if (meatTargetFlag) {
            // focus: meatDoneTemp branch
            snprintf(messageBuffer, sizeof(messageBuffer), "Meat done\x5B%s\x5D\002", TemperatureStr);
            if (adjTempFlag) {
                snprintf(messageBuffer, sizeof(messageBuffer), "Meat done\x6%s\x7\002", TemperatureStr);  // PROBLEM: \x6 not working -> displaying as \x7
            }
        } else {
            // else focus: pitTemp
            snprintf(messageBuffer, sizeof(messageBuffer), "Pit temp\x5B%s\x5D\002", TemperatureStr);
            if (adjTempFlag) {
                snprintf(messageBuffer, sizeof(messageBuffer), "Pit temp\x6%s\x7\002", TemperatureStr);
            }
        }
    }
}

void CWG_LCD::showSetMeatDoneTempMenu(int16_t prevEncoderValue) {
    switch (prevEncoderValue) {
        case 0: {
            menuSelectLine = 0;
            lcd.printMenuLine_noArrow(" Select&Tap\007 set");
            // lcd.printMenuLine("Tap to set temp");
            bool meatTargetFlag = 1;
            bool adjTempFlag = 0;
            getTargetTemperatureMsg(messageBuffer, degCFlag, meatDoneTemp, meatTargetFlag, adjTempFlag);
            lcd.printMenuLine(messageBuffer);
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("Case 0 showSetMeatDoneTempMenu -> prevEncoderValue = "));
                Serial.println(prevEncoderValue);
            }
#endif
        } break;

        case 1: {
            menuSelectLine = 1;
            lcd.printMenuLine("Select&Tap\007 set");
            // lcd.printMenuLine("Tap to set temp");
            bool meatTargetFlag = 1;
            bool adjTempFlag = 0;
            getTargetTemperatureMsg(messageBuffer, degCFlag, meatDoneTemp, meatTargetFlag, adjTempFlag);
            lcd.printMenuLine(messageBuffer);
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("Case 1 showSetMeatDoneTempMenu -> prevEncoderValue = "));
                Serial.println(prevEncoderValue);
            }
#endif
        } break;

        case 2: {
            menuSelectLine = 1;
            bool meatTargetFlag = 1;
            bool adjTempFlag = 0;
            getTargetTemperatureMsg(messageBuffer, degCFlag, meatDoneTemp, meatTargetFlag, adjTempFlag);
            lcd.printMenuLine(messageBuffer);
            lcd.printMenuLine("Hold to Return\004");
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("Case 2 showSetMeatDoneTempMenu -> prevEncoderValue = "));
                Serial.println(prevEncoderValue);
            }
#endif
        } break;
    }
}

void CWG_LCD::showSetPitTempTargetMenu(int16_t prevEncoderValue) {
    switch (prevEncoderValue) {
        case 0: {
            menuSelectLine = 0;
            lcd.printMenuLine_noArrow(" Select&Tap\007 set");
            // lcd.printMenuLine("Tap to set temp");
            bool meatTargetFlag = 0;  // 0 for pitTempTarget
            bool adjTempFlag = 0;
            getTargetTemperatureMsg(messageBuffer, degCFlag, pitTempTarget, meatTargetFlag, adjTempFlag);
            lcd.printMenuLine(messageBuffer);
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("Case 0 showSetPitTempTargetMenu -> prevEncoderValue = "));
                Serial.println(prevEncoderValue);
            }
#endif
        } break;

        case 1: {
            menuSelectLine = 1;
            lcd.printMenuLine("Select&Tap\007 set");
            // lcd.printMenuLine("Tap to set temp");
            bool meatTargetFlag = 0;  // 0 for pitTempTarget
            bool adjTempFlag = 0;
            getTargetTemperatureMsg(messageBuffer, degCFlag, pitTempTarget, meatTargetFlag, adjTempFlag);
            lcd.printMenuLine(messageBuffer);
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("Case 1 showSetPitTempTargetMenu -> prevEncoderValue = "));
                Serial.println(prevEncoderValue);
            }
#endif
        } break;

        case 2: {
            menuSelectLine = 1;
            bool meatTargetFlag = 0;  // 0 for pitTempTarget
            bool adjTempFlag = 0;
            getTargetTemperatureMsg(messageBuffer, degCFlag, pitTempTarget, meatTargetFlag, adjTempFlag);
            lcd.printMenuLine(messageBuffer);
            lcd.printMenuLine("Hold to Return\004");
#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("Case 2 showSetPitTempTargetMenu -> prevEncoderValue = "));
                Serial.println(prevEncoderValue);
            }
#endif
        } break;
    }
}

void CWG_LCD::showSetTempUnitsMenu(int16_t currentEncoderValue) {
    switch (currentEncoderValue) {
        case 0:
            // Serial.println("case 0 SetTempUnits");
            menuSelectLine = 0;
            if (degCFlag == 0) {
                lcd.printMenuLine("Tap to set unit");
                lcd.printMenuLine("Units [F] / C");  // degF are selected temperature units
            } else {
                lcd.printMenuLine("Tap to set unit");
                lcd.printMenuLine("Units  F /[C]");  // degC are selected temperature units
            }

#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("CWG_LCD::showSetTempUnitsMenu Case 0: -> prevEncoderValue:  "));
                Serial.print(prevEncoderValue);
                Serial.print(F(" / currentEncoderValue: "));  // debug
                Serial.println(currentEncoderValue);
            }
#endif
            break;

        case 1:
            // Serial.println("case 1 SetTempUnits");
            menuSelectLine = 1;
            if (degCFlag == 0) {
                lcd.printMenuLine("Tap to set unit");
                lcd.printMenuLine("Units [F] / C");
            } else {
                lcd.printMenuLine("Tap to set unit");
                lcd.printMenuLine("Units  F /[C]");
            }

#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("CWG_LCD::showSetTempUnitsMenu Case 1: -> prevEncoderValue:  "));
                Serial.print(prevEncoderValue);
                Serial.print(F(" / currentEncoderValue: "));  // debug
                Serial.println(currentEncoderValue);
            }
#endif
            break;

        case 2:
            // Serial.println("case 2 SetTempUnits");
            menuSelectLine = 1;
            if (degCFlag == 0) {
                lcd.printMenuLine("Units [F] / C");
                lcd.printMenuLine("Hold to Return\004");
            } else {
                lcd.printMenuLine("Units  F /[C]");
                lcd.printMenuLine("Hold to Return\004");
            }

#ifdef DEBUG
            if (encoder.moved()) {
                Serial.println();
                Serial.print(F("CWG_LCD::showSetTempUnitsMenu Case 2: -> prevEncoderValue:  "));
                Serial.print(prevEncoderValue);
                Serial.print(F(" / currentEncoderValue: "));  // debug
                Serial.println(currentEncoderValue);
            }
#endif
            break;
    }
}

void CWG_LCD::showTemeratureTargetAdjustment(float temporaryTemperatureTarget, bool meatTargetFlag) {
    bool adjTempFlag = 1;
    /*
    //debug
    if (encoder.moved()) {
        Serial.println();
        Serial.print(F("showTemeratureTargetAdjustment -> temporaryTemperatureTarget = "));
        Serial.print(temporaryTemperatureTarget);
        Serial.print(F(" / meatTargetFlag = "));
        Serial.print(meatTargetFlag);
        Serial.print(F(" / adjTempFlag = "));
        Serial.println(adjTempFlag);
    }
    // end debug
 */
    menuSelectLine = 1;
    lcd.printMenuLine("Set: Rotate/tap");
    getTargetTemperatureMsg(messageBuffer, degCFlag, temporaryTemperatureTarget, meatTargetFlag, adjTempFlag);
    lcd.printMenuLine(messageBuffer);
}
