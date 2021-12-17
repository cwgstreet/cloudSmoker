/* ***************************************************************
* smokerStates.h - cloudSmoker state machine implementation routines
*
* See (github link tba) for cloudSmoker State Machine diagram
* 
*  C W Greenstreet, Ver1, 16Dec21
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

// entryStates is an enum variable type defined in menu.h header file (as extern); smokerState is global
entryStates_t smokerState;

void processState(CWG_LCD &lcd) {
    smokerState = splashScreen;  //test line - remove

    switch (smokerState) {
        case splashScreen:  // note: case splashScreen == 1 explicitly per enum assignment in smokerState.h
            //extern void showSplashScreen(bool degCFlag, float meatDoneTemp, float pitTemp);  // don't think extern is necessary?
            lcd.showSplashScreen(degCFlag, meatDoneTemp, pitTempTarget);
            // initialise encoder?  or pass encoder object by reference?
            // test for encoderstate change, if encoder state change, then smokerState = launchPad;
            break;

        case launchPad:
            void showLaunchPad();
            break;

        case changeSettings:
            // code here
            break;

        case setMeatDoneTemp:
            // code here
            break;

        case setPitTempTarget:
            /* encoderCountValue = encoder.getCount(currentEncoderState);
            if (encoderCountValue > 209) {
                Serial.println("Changing Encoder Settings.");
                encoder.newSettings(-5, 15, 0, currentEncoderState);  //previous 180, 210, 203
                prevEncoderValue = currentEncoderState.currentValue; 
                Serial.print("main Starting Value: ");
                //Serial.println(prevEncoderValue);
            }*/
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

        case modemSleep:
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

/* void CloudSmoker_State::showSplashScreen() {
    //splash screen lcd display code here
}

void CloudSmoker_State::showStatusScreen() {
    //status screen lcd display code here
} */
