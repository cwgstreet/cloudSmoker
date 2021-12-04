/* ***************************************************************
*  menu.h - cloudSmoker lcd menu library 
* 
*  C W Greenstreet, Ver1, 4Dec21
*    MIT Licence - Released into the public domain
*
** ************************************************************* */

/* 
// include 3rd party libraries
#include <Arduino.h>
#include <NewEncoder.h>
#include <Wire.h>                           // i2C device communication
#include <hd44780.h>                        // LCD library
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD
 */
//incliude local libraries
#include <lcd.h>
#include <menu.h>

// entryStates is an enum defined in menu.h header file (as extern); menuState is global
entryStates menuState;

/*
    bbqStatus = 1,  //set enum 1 to 4 rather than default 0 for first element
    Menu2_setMeatDoneTemp 
    Menu3_setPitMinTemp,
    Menu4_setPitMaxTemp,
    Menu5_setTempUnits
*/

void CloudSmoker_Menu::processMenu() {
    switch (menuState) {
        case bbqStatus:
            Serial.print("menuState =");
            Serial.println(menuState);
            encoderCountValue = encoder.getCount(currentEncoderState);
            break;

        case Menu2_setMeatDoneTemp:
            Serial.print("menuState =");
            Serial.println(menuState);
            encoderCountValue = encoder.getCount(currentEncoderState);
            break;

        case Menu3_setPitMinTemp:
            encoderCountValue = encoder.getCount(currentEncoderState);
            if (encoderCountValue > 209) {
                Serial.println("Changing Encoder Settings.");
                encoder.newSettings(-5, 15, 0, currentEncoderState);  //previous 180, 210, 203
                prevEncoderValue = currentEncoderState.currentValue;
                Serial.print("main Starting Value: ");
                Serial.println(prevEncoderValue);
            }
            break;

        case Menu5_setTempUnits:
            Serial.print("menuState =");
            Serial.println(menuState);
            break;

        default:
            menuState = bbqStatus;
    }
}

void CloudSmoker_Menu::showSplashScreen() {
    //splash screen lcd display code here
}