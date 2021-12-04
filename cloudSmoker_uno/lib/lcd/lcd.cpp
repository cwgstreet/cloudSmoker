/* ***************************************************************
* lcd.h - library containing functions to use lcd, including 
*   custom characters
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
    //  end LCD function test
}

//function LCD_goto_row_col() to place the LCD cursor at a specific position, and a
// function LCD_write_data(),  o send a data byte to the LCD, you can put the special character
// on the screen like this:
//LCD_goto_row_col(thisrow, thatcol);
//LCD_write_data(ActiveRowIndicator);

////////////////////////////////////////////////////////////////////////////////
// Custom character data - symbols mostly borrowed from Prusa Mk3 firmware, unless noted
//   Ref: https://github.com/prusa3d/Prusa-Firmware/blob/MK3/Firmware/lcd.cpp
////////////////////////////////////////////////////////////////////////////////

const uint8_t lcd_custChar_degree[8] PROGMEM = {
    B01100,
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
    B00000,
    B00000,
    B00000,
    B00000,
    B00000};  // Source: Prusa

const uint8_t lcd_custChar_arrdown[8] PROGMEM = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B10001,
    B01010,
    B00100};  // Source: Prusa

const uint8_t lcd_custChar_selectarr[8] PROGMEM = {
    B01000,
    B00100,
    B00010,
    B00001,
    B00010,
    B00100,
    B01000,
    B00000};  // custom made

void CWG_LCD::initialiseCustomCharSet() {
    lcd.createChar(0, lcd_custChar_degree);
    lcd.createChar(1, lcd_custChar_degreeC);
    lcd.createChar(2, lcd_custChar_degreeF);
    lcd.createChar(3, lcd_custChar_thermometer);
    lcd.createChar(4, lcd_custChar_uplevel);
    lcd.createChar(5, lcd_custChar_arrup);
    lcd.createChar(6, lcd_custChar_arrdown);
    lcd.createChar(7, lcd_custChar_selectarr);
}

/* 
Note that you might want to embed a special character in a "string" to write to the LCD (
for example: using "\x01", for special character number 1).  That would work, fine and dandy, 
for special characters 1 through 7, but you can not (that's not) embed special character zero 
in a "string," since \x00 will terminate the "string."

For character zero, use "\x08" since all 8 custom characters are mapped to 0x08 to 0x0F as well
 */