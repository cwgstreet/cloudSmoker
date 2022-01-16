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
        int16_t currentEncoderValue;

        case splashScreen: {  // note: case splashScreen == 1 explicitly per enum assignment in smokerState.
            lcd.showSplashScreen(degCFlag, meatDoneTemp, pitTempTarget);
            if (encoder.moved()) {
                smokerState = launchPad;
            }
        } break;

        case launchPad: {
            lcd.showLaunchPad();
            if (pressEventCode == DOUBLE_PRESS) {
                yield();  // Do (almost) nothing -- yield allows ESP8266 background functions

                smokerState = changeSettings;  // enter config menu
            }
            if (pressEventCode == LONG_PRESS) {
                yield();                                   // Do (almost) nothing -- yield allows ESP8266 background functions
                smokerState = getTemp;                     // start bbq cook
                long unsigned startCookTimeMS = millis();  // capture cook start time; var defined as global (extern in helper_functions.h)
            }
        } break;

        case changeSettings: {
            lcd.showSettingsMenu(prevEncoderValue);
            //int16_t currentEncoderValue = encoder.getCount();
            currentEncoderValue = encoder.getCount();

            // firstly, reset encoder scale to match number of Settings menu items (1 to 7)
            if (hasRunFlag == 0) {
                Serial.println(F("Run Once! (hasRunFlag == 0); Changing Encoder Settings."));
                encoder.newSettings(1, 7, 1, currentEncoderState);
                //currentEncoderState.currentValue;  //do I need this line?  Check
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;

                hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()

                //debug statements
                Serial.println();
                Serial.print(F("processStates hasRunFlag block triggered (==0) -> prevEncoderValue: "));  //debug
                Serial.println(prevEncoderValue);
                // end debug
            }

            // DELETE FOLLOWING - NOT NEEDED
            /* 
                if (encoder.moved()) {
                    lcd.showSettingsMenu(currentEncoderValue);
                }
            */

            if (button.triggered(SINGLE_TAP)) {
                //debug statements
                Serial.println();
                Serial.print(F(" !!!  Button Triggered !!! "));
                Serial.print(F("processStates smokerState = "));
                Serial.println(smokerState);
                Serial.print(F("processStates -> Encoder: currentEncoderValue = "));
                Serial.print(currentEncoderValue);
                Serial.print(F(" / prevEncoderValue = "));
                Serial.println(prevEncoderValue);
                // end debug

                if (prevEncoderValue == 4) {
                    smokerState = setMeatDoneTemp;  // enter sub-menu to set meat done temperature target

                    //debug statements
                    Serial.println();
                    Serial.print(F("processStates -> [if (prevEncoderValue == 4)] smokerState = "));
                    Serial.println(smokerState);
                    Serial.print(F("processStates -> [if (prevEncoderValue == 4)]: currentEncoderValue = "));
                    Serial.print(currentEncoderValue);
                    Serial.print(F(" / prevEncoderValue = "));
                    Serial.println(prevEncoderValue);
                    // end debug
                    break;
                }
                if (prevEncoderValue == 5) {
                    smokerState = setPitTempTarget;  // enter sub-menu to set meat done temperature target
                    break;
                }

                if (prevEncoderValue == 6) {
                    hasRunFlag = 0;              //reset flag for next encoder scale change
                    smokerState = setTempUnits;  // enter sub-menu to set meat done temperature target
                    break;
                }
            }

            /* if (encoder.moved()) {
                currentEncoderValue = currentEncoderState.currentValue;

                Serial.print(F("processStates -> [if (encoder.moved() Ln118]: currentEncoderValue = "));
                Serial.print(currentEncoderValue);
                Serial.print(F(" / prevEncoderValue = "));
                Serial.println(prevEncoderValue);
                // end debug

                prevEncoderValue = currentEncoderValue;
            } */
        } break;

        case setMeatDoneTemp: {
            lcd.printMenuLine("sm MeatDoneTemp");  // temporary to confirm navigation branch
            // code here
        } break;

        case setPitTempTarget: {
            lcd.printMenuLine("sm setpitTemp");  // temporary to confirm navigation branch
            // code here
        } break;

        case setTempUnits: {
            //lcd.printMenuLine("sm setTempUnits");  // temporary to confirm navigation branch
            currentEncoderValue = encoder.getCount();
            //lcd.showSetTempUnitsMenu(currentEncoderValue);

            // reset encoder scale to match number of menu list items
            if (hasRunFlag == 0) {
                Serial.println(F("Run Once! (hasRunFlag == 0); Changing Encoder Settings - setTempUnits."));
                encoder.newSettings(0, 1, 0, currentEncoderState);
                currentEncoderValue = currentEncoderState.currentValue;  //do I need this line?  Check
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;

                hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
            }
            //if (encoder.moved()) {
                lcd.showSetTempUnitsMenu(prevEncoderValue);
                //prevEncoderValue = currentEncoderValue;
            //}

            if (button.triggered(SINGLE_TAP)) {
                if (prevEncoderValue == 1) {
                    degCFlag = 1;
                    Serial.println(F("degC Flag set - units now in degC!"));  //debug
                }
            }

            break;

            case getTemp: {
                lcd.printMenuLine("get Temp");  // temporary to confirm navigation branch
                // code here
            } break;

            case txTemp: {
                lcd.printMenuLine("txTemp");  // temporary to confirm navigation branch
                // code here
            } break;

            case modemSleep: {
                lcd.printMenuLine("modemSleep");  // temporary to confirm navigation branch
                // code here
            } break;

            case bbqStatus: {
                lcd.printMenuLine("bbqStatus");  // temporary to confirm navigation branch
                // code here
            } break;

            default: {
                smokerState = bbqStatus;
            } break;
        }
    }
}

/* void CloudSmoker_State::showSplashScreen() {
    //splash screen lcd display code here
}

void CloudSmoker_State::showStatusScreen() {
    //status screen lcd display code here
} */
