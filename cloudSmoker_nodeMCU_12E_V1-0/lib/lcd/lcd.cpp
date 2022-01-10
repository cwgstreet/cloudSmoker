/* ***************************************************************
* lcd.h - library containing functions to use lcd, including 
*   custom character set
* 
*  C W Greenstreet, Ver1, 4Dec21
*    MIT Licence - Released into the public domain
*
** ************************************************************* */

//set up debug scaffold; comment out following line if you want to "turn off" serial monitor debugging
#define DEBUG 1

// include 3rd party libraries
#include <Arduino.h>
#include <NewEncoder.h>                     // do I need this here anymore?  use is in smokerStates now?
#include <Wire.h>                           // i2C device communication
#include <hd44780.h>                        // LCD library
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

//incliude local libraries
#include <helper_functions.h>  // do I need this here anymore?  use is in smokerStates now?
#include <lcd.h>
#include <press_type.h>
#include <smokerStates.h>
#include <wrapEncoder.h>

// define LCD geometry (YwRobot 1602 LCD)
constexpr int LCD_COLS = 16;
constexpr int LCD_ROWS = 2;

// LCD display varibles
byte menuPrintLine = 0;
byte menuSelectLine = 0;
char messageBuffer[17];  // empty buffer array; 16 char LCD screen + terminating null char

//LCD constructor passing lcd rows and columns
CWG_LCD::CWG_LCD(const int LCD_COLS, const int LCD_ROWS) {
    _numCols = LCD_COLS;
    _numRows = LCD_ROWS;
}

CWG_LCD lcd(LCD_COLS, LCD_ROWS);  //instantiate lcd object: auto locate & auto config expander chip

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
    lcd.setCursor(0, 1);               //set cursor to first column of second row (forst position == 0)
    lcd.print(F("G'Day cloudSmkr!"));  // F-Macro to save dynamic memory
    delay(1000);
}  //  end LCD initialisation

/////////////////////////////////////////////////////////////////////////////////////////
// Custom character data - symbols mostly borrowed from Prusa Mk3 firmware, unless noted
//   Ref: https://github.com/prusa3d/Prusa-Firmware/blob/MK3/Firmware/lcd.cpp
//
////////////////////////////////////////////////////////////////////////////////////////

const uint8_t lcd_custChar_degree[8] PROGMEM = {  //need to refer to element 0 in array as "/010", binary 8
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
    B00000};  //source: https://github.com/duinoWitchery/hd44780/blob/master/examples/ioClass/hd44780_I2Cexp/LCDCustomChars/LCDCustomChars.ino

// uint8_t degreeF[8]     = {0x18,0x18,0x07,0x04,0x07,0x04,0x04,0x00};
const uint8_t lcd_custChar_degreeF[8] PROGMEM = {
    B11000,   //  **
    B11000,   //  **
    B00111,   //    ***
    B00100,   //    *
    B00111,   //    ***
    B00100,   //    *
    B00100,   //    *
    B00000};  //source: https://github.com/duinoWitchery/hd44780/blob/master/examples/ioClass/hd44780_I2Cexp/LCDCustomChars/LCDCustomChars.ino

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

const uint8_t lcd_custChar_arrdown[8] PROGMEM = {
    B00000,   //
    B00000,   //
    B00000,   //
    B00100,   //    *
    B00100,   //    *
    B11111,   //  *****
    B01110,   //   ***
    B00100};  //    *    Source: Prusa

//const uint8_t lcd_custChar_selectarr[8] PROGMEM = {
const char lcd_custChar_selectarr[8] PROGMEM = {
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
    lcd.createChar(0, lcd_custChar_degree);  //use \010 (binary for 8 as \000 is null termination and will cause problems
    lcd.createChar(1, lcd_custChar_degreeC);
    lcd.createChar(2, lcd_custChar_degreeF);
    lcd.createChar(3, lcd_custChar_thermometer);
    lcd.createChar(4, lcd_custChar_uplevel);
    lcd.createChar(5, lcd_custChar_arrup);
    lcd.createChar(6, lcd_custChar_arrdown);
    lcd.createChar(7, lcd_custChar_selectarr);
}

// confirming functionality of lcd display of special characters
void CWG_LCD::displayTest() {
    lcd.clear();
    //lcd.print("\008" "\001" "\002" "\003" "\004" "\005" "\006" "\007");
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
    //lcd.printMenuLine("test works1");         //test code - REMOVE
    // lcd.printMenuLine("test works234567");   //test code - REMOVE
    char msg[17];                // space for 16 charcaters + null termination
    char currentMeatTempStr[4];  // empty array to hold converted (to string) float meat temp + null
    char currentPitTempStr[4];
    lcd.setCursor(0, 0);

    if (degCFlag == 1) {
        float meatDoneTempC = convertDegFtoDegC(meatDoneTemp);
        float pitTempTargetC = convertDegFtoDegC(pitTempTarget);
        dtostrf(meatDoneTempC, 3, 0, currentMeatTempStr);  // (float var to convert, width==3, 0==no digits after decimal, char arra for output)
        dtostrf(pitTempTargetC, 3, 0, currentPitTempStr);
        sprintf(msg, "Meat%s\001 Pit%s\001", currentMeatTempStr, currentPitTempStr);

        lcd.printMenuLine_noArrow("BBQ set points:");
        lcd.printMenuLine_noArrow(msg);

    } else {
        dtostrf(meatDoneTemp, 3, 0, currentMeatTempStr);  // width==3, no digits after decimal
        dtostrf(pitTempTarget, 3, 0, currentPitTempStr);  // width==3, no digits after decimal sprintf(msg, "Meat%s\002 Pit%s\002", currentMeatTempStr, currentPitTempStr);
        sprintf(msg, "Meat%s\002 Pit%s\002", currentMeatTempStr, currentPitTempStr);

        lcd.printMenuLine_noArrow("BBQ set points:");
        lcd.printMenuLine_noArrow(msg);
    }
}

void CWG_LCD::showLaunchPad() {
    lcd.printMenuLine_noArrow("Cook: LongPress");
    lcd.printMenuLine_noArrow("Config: DblPress");
}

void CWG_LCD::showSettingsMenu(int16_t currentEncoderValue) {
    //Serial.println(F("entered showSettingsMenu "));

    switch (currentEncoderValue) {
        case 1:
            menuSelectLine = 0;
            lcd.printMenuLine_noArrow("<Turn to scroll>");
            lcd.printMenuLine("<Press to set");
#ifdef DEBUG
            Serial.print(F("CWG_LCD::showSettingsMenu Case 1: currentEncoderValue = "));
            Serial.println(currentEncoderValue);
            Serial.println();
#endif
            break;

        case 2:
            menuSelectLine = 0;
            lcd.printMenuLine("<Press to set>");
            lcd.printMenuLine("<Hold to exit>");
#ifdef DEBUG
            Serial.print(F("CWG_LCD::showSettingsMenu Case 2: currentEncoderValue = "));
            Serial.println(currentEncoderValue);
            Serial.println();
#endif
            break;

        case 3:
            menuSelectLine = 1;
            lcd.printMenuLine("<Press to set>");
            lcd.printMenuLine("<Hold to exit>");
#ifdef DEBUG
            Serial.print(F("CWG_LCD::showSettingsMenu Case 3: currentEncoderValue = "));
            Serial.println(currentEncoderValue);
            Serial.println();
#endif
            break;

        case 4:
            menuSelectLine = 1;
            lcd.printMenuLine("<Hold to exit>");
            getMeatDoneTempMsg(messageBuffer, degCFlag, meatDoneTemp);
            lcd.printMenuLine(messageBuffer);
#ifdef DEBUG
            Serial.println();
            Serial.print(F("CWG_LCD::showSettingsMenu Case 4: currentEncoderValue = "));
            Serial.println(currentEncoderValue);
#endif
            break;

        case 5:
            menuSelectLine = 1;
            getMeatDoneTempMsg(messageBuffer, degCFlag, meatDoneTemp);
            lcd.printMenuLine(messageBuffer);
            getPitTempTargetMsg(messageBuffer, degCFlag, pitTempTarget);
            lcd.printMenuLine(messageBuffer);
#ifdef DEBUG
            Serial.print(F("CWG_LCD::showSettingsMenu Case 5: currentEncoderValue = "));
            Serial.println(currentEncoderValue);
            Serial.println();
#endif
            /* if (pressEventCode == SHORT_PRESS) {
                smokerState = setPitTempTarget;  // enter sub-menu to set pit temperature target
                Serial.print(F("CWG_LCD::showSettingsMenu smokerState = "));
                Serial.println(smokerState);
            } */
            break;

        case 6:
            menuSelectLine = 1;
            getPitTempTargetMsg(messageBuffer, degCFlag, pitTempTarget);
            lcd.printMenuLine(messageBuffer);
            lcd.printMenuLine("Units [F]/C");

#ifdef DEBUG
            Serial.print(F("CWG_LCD::showSettingsMenu Case 6: currentEncoderValue = "));
            Serial.println(currentEncoderValue);
            Serial.println();
#endif
            /* if (pressEventCode == SHORT_PRESS) {
                smokerState = setTempUnits;  // enter sub-menu to set temperature units
                Serial.print(F("CWG_LCD::showSettingsMenu smokerState = "));
                Serial.println(smokerState);
            } */
            break;

        case 7:
            menuSelectLine = 1;
            lcd.printMenuLine("Units [F]/C");
            lcd.printMenuLine_noArrow("<Turn to scroll>");
#ifdef DEBUG
            Serial.print(F("CWG_LCD::showSettingsMenu Case 7: currentEncoderValue = "));
            Serial.println(currentEncoderValue);
            Serial.println();
#endif
            break;
    }
}

void CWG_LCD::getMeatDoneTempMsg(char (&messageBuffer)[17], bool degCFlag, float meatDoneTemp) {
    char meatDoneTempStr[4];  // empty array to hold converted (to string) float meat temp + null

    // TO-DO: array out of bounds error checking
    if (degCFlag == 1) {
        float meatDoneTempC = convertDegFtoDegC(meatDoneTemp);
        dtostrf(meatDoneTempC, 3, 0, meatDoneTempStr);  // (float var to convert, width==3, 0==no digits after decimal, char arra for output)
        // sprintf:  15 characters + null; Octal escape chars: \x5B = [  \x5D = ], /001 = degC custom char
        sprintf(messageBuffer, "Meat done\x5B%s\x5D\001", meatDoneTempStr);
    } else {
        dtostrf(meatDoneTemp, 3, 0, meatDoneTempStr);  // width==3, no digits after decimal
        // sprintf:  15 characters + null; Octal escape chars: \x5B = [  \x5D = ], /001 = degC custom char
        sprintf(messageBuffer, "Meat done\x5B%s\x5D\002", meatDoneTempStr);
    }
}

void CWG_LCD::getPitTempTargetMsg(char (&messageBuffer)[17], bool degCFlag, float pitTempTarget) {
    char pitTempTargetStr[4];  // empty array to hold converted (to string) float meat temp + null

    // TO-DO: array out of bounds error checking
    if (degCFlag == 1) {
        float pitTempTargetC = convertDegFtoDegC(pitTempTarget);
        dtostrf(pitTempTargetC, 3, 0, pitTempTargetStr);  // (float var to convert, width==3, 0==no digits after decimal, char arra for output)
        // sprintf:  15 characters + null; Octal escape chars: \x5B = [  \x5D = ], /001 = degC custom char
        sprintf(messageBuffer, "Pit temp\x5B%s\x5D\001", pitTempTargetStr);
    } else {
        dtostrf(pitTempTarget, 3, 0, pitTempTargetStr);  // width==3, no digits after decimal
        // sprintf:  15 characters + null; Octal escape chars: \x5B = [  \x5D = ], /001 = degF custom char
        sprintf(messageBuffer, "Pit temp\x5B%s\x5D\002", pitTempTargetStr);
    }
}
/* 
// TO-DO geraricise function - combine these into a single function.  Tricky part is text in message
void CWG_LCD::getTempMsg(char (&messageBuffer)[17], bool degCFlag, float tempVariable) {
    char temperatureVariableStr[4];  // empty array to hold converted (to string) float meat temp + null

    // TO-DO: array out of bounds error checking
    if (degCFlag == 1) {
        float tempVariableC = convertDegFtoDegC(tempVariable);
        dtostrf(tempVariableC, 3, 0, temperatureVariableStr);  // (float var to convert, width==3, 0==no digits after decimal, char arra for output)
        // sprintf:  15 characters + null; Octal escape chars: \x5B = [  \x5D = ], /001 = degC custom char
        sprintf(messageBuffer, "Meat done\x5B%s\x5D\001", temperatureVariableStr);
    } else {
        dtostrf(tempVariable, 3, 0, temperatureVariableStr);  // width==3, no digits after decimal
        // sprintf:  15 characters + null; Octal escape chars: \x5B = [  \x5D = ], /001 = degF custom char
        sprintf(messageBuffer, "Meat done\x5B%s\x5D\002", temperatureVariableStr);
    }
} */
