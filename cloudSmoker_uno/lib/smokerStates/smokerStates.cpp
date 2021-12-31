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
#include <helper_functions.h>
#include <lcd.h>
#include <press_type.h>
#include <smokerStates.h>
#include <wrapEncoder.h>

// entryStates is an enum variable type defined in menu.h header file (as extern); smokerState is global
entryStates_t smokerState;

void processState(CWG_LCD &lcd) {
    switch (smokerState) {
        case splashScreen:  // note: case splashScreen == 1 explicitly per enum assignment in smokerState.
        {
            lcd.showSplashScreen(degCFlag, meatDoneTemp, pitTempTarget);
            delay(1000);

            if (encoder.moved()) {
                smokerState = launchPad;
            }

        } break;

        case launchPad: {
            lcd.showLaunchPad();

            if (pressEventCode == DOUBLE_PRESS) {
                smokerState = changeSettings;  // enter config screens
            }

            if (pressEventCode == LONG_PRESS) {
                smokerState = getTemp;                     // start bbq cook
                long unsigned startCookTimeMS = millis();  // start cook time; var defined as global (extern in helper_functions.h)
            }
        } break;

        case changeSettings: {
            //lcd.printMenuLine("changeSettings");  // temporary to confirm navigation branch

            // set new wrap encoder range for 5-row menu
            encoder.newSettings(0, 4, 0, currentEncoderState);
            int16_t currentEncoderValue = encoder.getCount();
            
            //lcd.showSettingsMenu(currentEncoderValue); 

            
        } break;

        case setMeatDoneTemp:
            lcd.printMenuLine("MeatDoneTemp");  // temporary to confirm navigation branch
            // code here
            break;

        case setPitTempTarget:
            lcd.printMenuLine("set pitTemp");  // temporary to confirm navigation branch
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
            lcd.printMenuLine("set pitTemp");  // temporary to confirm navigation branch
            break;

        case getTemp:
            lcd.printMenuLine("get Temp");  // temporary to confirm navigation branch
            // code here
            break;

        case txTemp:
            lcd.printMenuLine("txTemp");  // temporary to confirm navigation branch

            // code here
            break;

        case modemSleep:
            lcd.printMenuLine("modemSleep");  // temporary to confirm navigation branch
            // code here
            break;

        case bbqStatus:
            lcd.printMenuLine("bbqStatus");  // temporary to confirm navigation branch
            // code here
            break;

        default:
            smokerState = bbqStatus;
            break;
    }
}

/* void CloudSmoker_State::showSplashScreen() {
    //splash screen lcd display code here
}

void CloudSmoker_State::showStatusScreen() {
    //status screen lcd display code here
} */
