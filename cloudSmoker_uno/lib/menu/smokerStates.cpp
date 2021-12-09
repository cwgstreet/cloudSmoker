/* ***************************************************************
*  menu.h - cloudSmoker state maching functionality including 
*     lcd menu 
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
#include <smokerStates.h>

// entryStates is an enum defined in menu.h header file (as extern); menuState is global
entryStates smokerState;

void CloudSmoker_State::processState() {
    switch (smokerState) {
        case splashScreen:
            showSplashScreen();
            break;

        case startCook:
            //code here;
            break;

        case setUp:
            // code here
            break;

        case setMeatDoneTemp:
            // code here
            break;

        case setPitMinTemp:
            /* encoderCountValue = encoder.getCount(currentEncoderState);
            if (encoderCountValue > 209) {
                Serial.println("Changing Encoder Settings.");
                encoder.newSettings(-5, 15, 0, currentEncoderState);  //previous 180, 210, 203
                prevEncoderValue = currentEncoderState.currentValue; 
                Serial.print("main Starting Value: ");
                //Serial.println(prevEncoderValue);
            }*/
            break;

        case setPitMaxTemp:
            // code here
            break;

        case setTempUnits:
            Serial.print("menuState =");
            
            break;

        case getTemp:
            // code here
            break;

        case txTemp:
            // code here
            break;

        case lightSleep:
            // code here
            break;

        case bbqStatus:
            // code here
            break;

        default:
            break;
            //menuState = bbqStatus;
    }
}

void CloudSmoker_State::showSplashScreen() {
    //splash screen lcd display code here
}

void CloudSmoker_State::showStatusScreen() {
    //status screen lcd display code here
}
