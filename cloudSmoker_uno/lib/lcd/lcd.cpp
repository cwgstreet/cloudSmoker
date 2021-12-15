/* ***************************************************************
* lcd.h - library containing functions to use lcd, including 
*   custom character set
* 
*  C W Greenstreet, Ver1, 4Dec21
*    MIT Licence - Released into the public domain
*
** ************************************************************* */

// include 3rd party libraries
#include <Arduino.h>
#include <NewEncoder.h>
#include <Wire.h>                           // i2C device communication
#include <hd44780.h>                        // LCD library
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

//incliude local libraries
#include <helper_functions.h>
#include <lcd.h>

hd44780_I2Cexp lcd;  //instantiate lcd object: auto locate & auto config expander chip

//LCD constructor passing lcd rows and columns
CWG_LCD::CWG_LCD(const int lcdCols, const int lcdRows) {
    _numCols = lcdCols;
    _numRows = lcdRows;
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
    delay(3000);
    lcd.clear();
    lcd.print(F("LCD test passed"));
    lcd.setCursor(0, 1);               //set cursor to first column of second row (forst position == 0)
    lcd.print(F("G'Day cloudSmkr!"));  // F-Macro to save dynamic memory
    delay(2000);
}  //  end LCD function test

//function LCD_goto_row_col() to place the LCD cursor at a specific position, and a
// function LCD_write_data(),  o send a data byte to the LCD, you can put the special character
// on the screen like this:
//LCD_goto_row_col(thisrow, thatcol);
//LCD_write_data(ActiveRowIndicator);

////////////////////////////////////////////////////////////////////////////////
// Custom character data - symbols mostly borrowed from Prusa Mk3 firmware, unless noted
//   Ref: https://github.com/prusa3d/Prusa-Firmware/blob/MK3/Firmware/lcd.cpp
//
////////////////////////////////////////////////////////////////////////////////

const uint8_t lcd_custChar_degree[8] PROGMEM = {  //need to refer to element 0 in array as "/010", binary 8
    B01100,                                       // alternative degree symbol is in character set "/337"
    B10010,
    B10010,
    B01100,
    B00000,
    B00000,
    B00000,
    B00000};  // Source: Prusa

// uint8_t degreeC[8]     = {0x18,0x18,0x03,0x04,0x04,0x04,0x03,0x00};
const uint8_t lcd_custChar_degreeC[8] PROGMEM = {
    B11000,
    B11000,
    B00011,
    B00100,
    B00100,
    B00100,
    B00011,
    B00000};  //source: https://github.com/duinoWitchery/hd44780/blob/master/examples/ioClass/hd44780_I2Cexp/LCDCustomChars/LCDCustomChars.ino

// uint8_t degreeF[8]     = {0x18,0x18,0x07,0x04,0x07,0x04,0x04,0x00};
const uint8_t lcd_custChar_degreeF[8] PROGMEM = {
    B11000,
    B11000,
    B00111,
    B00100,
    B00111,
    B00100,
    B00100,
    B00000};  //source: https://github.com/duinoWitchery/hd44780/blob/master/examples/ioClass/hd44780_I2Cexp/LCDCustomChars/LCDCustomChars.ino

const uint8_t lcd_custChar_thermometer[8] PROGMEM = {
    B00100,
    B01010,
    B01010,
    B01010,
    B01010,
    B10001,
    B10001,
    B01110};  // Source: Prusa

const uint8_t lcd_custChar_uplevel[8] PROGMEM = {
    B00100,
    B01110,
    B11111,
    B00100,
    B11100,
    B00000,
    B00000,
    B00000};  // Source: Prusa

const uint8_t lcd_custChar_arrup[8] PROGMEM = {
    B00100,
    B01110,
    B11111,
    B00100,
    B00100,
    B00000,
    B00000,
    B00000};  // Source: Prusa

const uint8_t lcd_custChar_arrdown[8] PROGMEM = {
    B00000,
    B00000,
    B00000,
    B00100,
    B00100,
    B11111,
    B01110,
    B00100};  // Source: Prusa

const uint8_t lcd_custChar_selectarr[8] PROGMEM = {
    B01000,
    B01100,
    B01110,
    B01111,
    B01110,
    B01100,
    B01000,
    B00000};  // custom made

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
}

// format: dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
// sprintf(line0, "Temp: %-7sC", float_str); // %6s right pads the string
// *********************************************************************************************
void CWG_LCD::showSplashScreen(bool degCFlag, float meatDoneTemp, float pitTemp) {
    lcd.clear();
    char msg[17];          // space for 16 charcaters + null termination
    char meatFloatStr[4];  // empty array to hold convert float meat temp + null
    char pitFloatStr[4];

    if (degCFlag == 1) {
        float meatDoneTempC = convertDegFtoDegC(meatDoneTemp);
        float pitTempC = convertDegFtoDegC(pitTemp);
        dtostrf(meatDoneTempC, 3, 0, meatFloatStr);  // (float var to convert, width==3, 0==no digits after decimal, char arra for output)
        dtostrf(pitTempC, 3, 0, pitFloatStr);
        sprintf(msg, "Meat%s\001 Pit%s\001", meatFloatStr, pitFloatStr);

        lcd.print("BBQ set points: ");
        lcd.setCursor(0, 1);
        lcd.print(msg);

    } else {
        dtostrf(meatDoneTemp, 3, 0, meatFloatStr);  // width==3, no digits after decimal
        dtostrf(pitTemp, 3, 0, pitFloatStr);        // width==3, no digits after decimal sprintf(msg, "Meat%s\002 Pit%s\002", meatFloatStr, pitFloatStr);
        sprintf(msg, "Meat%s\002 Pit%s\002", meatFloatStr, pitFloatStr);

        lcd.print("BBQ set points: ");
        lcd.setCursor(0, 1);
        lcd.print(msg);
    }
}
// ******* end showSplashScreen()

// *********************************************************************************************
void CWG_LCD::showInstructionsScreen() {
    lcd.clear();
    char msgLine0[17] = {"Cook: LongPress "};
    char msgLine1[17] = {"Config: DblPress"};
    lcd.print(msgLine0);
    lcd.setCursor(0, 1);
    lcd.print(msgLine1);
}
// ******* end showInstructionsScreen()

// *********************************************************************************************
void CWG_LCD::showSettingsMenu() {
    lcd.clear();
    char msg0[17] = {" <Press to set> "};
    char msg1[17] = {" <Hold to exit> "};
    char msg2[17] = {" Meat done[xxx] "};
    char msg3[17] = {" Min Pit [xxx]  "};
    char msg4[17] = {" Max Pit [xxx]  "};
    char msg5[17] = {" Units [F] / C  "};
    // *** function not finished; need to bring arguments in for various temps and use dtosdrf and 
    //    sprintf to get formated string lines -> return formated line so put into one big array
}
// ******* end void CWG_LCD::showSettingsMenu() 

