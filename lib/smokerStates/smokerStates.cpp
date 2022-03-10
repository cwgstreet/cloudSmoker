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
            // if neccessary, reset encoder scale to match number of menu list items
            if (hasRunFlag == 0) {
                Serial.println(F("Run Once! (launchPad::hasRunFlag == 0); Changing Encoder Settings - setTempUnits 0,1,0"));
                encoder.newSettings(0, 1, 1, currentEncoderState);
                currentEncoderValue = currentEncoderState.currentValue;  //do I need this line?  Check
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;

                hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
            }

            lcd.showLaunchPad();
            if (button.triggered(DOUBLE_TAP)) {
                yield();         // Do (almost) nothing -- yield allows ESP8266 background functions
                hasRunFlag = 0;  // reset flag for next encoder scale change

                smokerState = changeSettings;  // enter config menu
            }
            if (button.triggered(SINGLE_TAP)) {
                yield();                                   // Do (almost) nothing -- yield allows ESP8266 background functions
                smokerState = getTemp;                     // start bbq cook
                long unsigned startCookTimeMS = millis();  // capture cook start time; var defined as global (extern in helper_functions.h)
            }
        } break;

        case changeSettings: {
            lcd.showSettingsMenu(prevEncoderValue);
            currentEncoderValue = encoder.getCount();

            // firstly, reset encoder scale to match number of Settings menu items (1 to 7)
            if (hasRunFlag == 0) {
                Serial.println(F("Run Once! (changeSettings::hasRunFlag == 0); Changing Encoder Settings 1,8,1"));
                encoder.newSettings(1, 8, 1, currentEncoderState);
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;

                hasRunFlag = 1;  // ensure settings are only changed once within a loop

                //debug statements
                //Serial.println();
                // Serial.print(F("processStates hasRunFlag block triggered (==0) -> prevEncoderValue: "));  //debug
                // Serial.println(prevEncoderValue);
                // end debug
            }

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
                    hasRunFlag = 0;                 //reset flag to allow next encoder scale change

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
                    hasRunFlag = 0;                  //reset flag for next encoder scale change
                    break;
                }

                if (prevEncoderValue == 6) {
                    hasRunFlag = 0;              //reset flag for next encoder scale change
                    smokerState = setTempUnits;  // enter sub-menu to set meat done temperature target
                    break;
                }
            }

            if (button.triggered(HOLD)) {
                smokerState = splashScreen;  // return to splashScreen
                hasRunFlag = 0;              // allow another reset of encoder scale range
            }
        } break;

        case setMeatDoneTemp: {
            lcd.showSetMeatDoneTempMenu(prevEncoderValue);
            currentEncoderValue = encoder.getCount();

            // reset encoder scale to match number of menu list items
            if (hasRunFlag == 0) {
                Serial.println(F("Run Once! (setMeatDoneTemp::hasRunFlag == 0); Changing Encoder Settings 0,2,0 "));
                encoder.newSettings(0, 2, 0, currentEncoderState);
                currentEncoderValue = currentEncoderState.currentValue;  //TODO: do I need this line?  Check
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;

                hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
            }

            // Select & Tap to set meatDoneTemp setting
            if (button.triggered(SINGLE_TAP)) {
                if (prevEncoderValue == 1) {
                    hasRunFlag = 0;  //reset hasRunFlag
                    bool adjustTempFlag = 1;
                    float temporaryTemperatureTarget;  // temporary variable while desired meat done temperature is being adjusted

                    while (adjustTempFlag) {
                        // reset encoder scale to match number of menu list items
                        if (hasRunFlag == 0) {
                            Serial.println(F("Run Once! (setMeatDoneTemp inside block::hasRunFlag == 0); Changing Encoder Settings 122,220,203 or 90,105,95 "));
                            if (degCFlag) {
                                encoder.newSettings(90, 105, 95, currentEncoderState);
                            } else {
                                encoder.newSettings(122, 220, 203, currentEncoderState);
                            }
                            currentEncoderValue = currentEncoderState.currentValue;  //do I need this line?  Check
                            Serial.println(currentEncoderValue);
                            prevEncoderValue = currentEncoderValue;
                            hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
                        }
                        bool meatTargetFlag = 1;
                        currentEncoderValue = encoder.getCount();
                        temporaryTemperatureTarget = prevEncoderValue;  // implicit conversion of int16_t prevEncoderValue to float temporaryTemperatureTarget

                        yield();  // Do (almost) nothing -- yield allows ESP8266 background functions
                        lcd.showTemeratureTargetAdjustment(temporaryTemperatureTarget, meatTargetFlag);

                        // exit temperature adjustment loop upon next button press
                        if (button.update()) {
                            if (button.triggered(SINGLE_TAP)) {
                                adjustTempFlag = 0;
                                meatDoneTemp = temporaryTemperatureTarget;
                                hasRunFlag = 0;  // allow another reset of encoder scale range
                                //smokerState = changeSettings;  // go up one level next loop

                                //debug
                                Serial.println();
                                Serial.print(F("New meatDoneTemp value set -> new meatDoneTemp = "));
                                Serial.println(meatDoneTemp);
                                // end debug

                                // reset encoder scale to match number of menu list items
                                if (hasRunFlag == 0) {
                                    Serial.println(F("Run Once! (return to changeSettings::hasRunFlag == 0); Changing Encoder Settings - setMeatDoneTemp 0,2,0"));
                                    encoder.newSettings(0, 2, 0, currentEncoderState);
                                    currentEncoderValue = currentEncoderState.currentValue;  //do I need this line?  Check
                                    Serial.println(currentEncoderValue);
                                    prevEncoderValue = currentEncoderValue;

                                    hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
                                }
                            }
                        }
                    }  // end While
                    Serial.println(F("while (adjustTempFlag) -> exited while loop"));
                }  //end if

                button.update();
            }

            // hold button to exit
            if (button.triggered(HOLD)) {
                smokerState = splashScreen;  // return to launchPad menu (one level up)
                Serial.print(F("Hold press - going up one level; smokerState = "));
                Serial.println(smokerState);
                hasRunFlag = 0;  // allow another reset of encoder scale range
            }

        } break;

        case setPitTempTarget: {
            //lcd.printMenuLine("sm setpitTemp");  // temporary to confirm navigation branch
            lcd.showSetPitTempTargetMenu(prevEncoderValue);
            currentEncoderValue = encoder.getCount();

            // reset encoder scale to match number of menu list items
            if (hasRunFlag == 0) {
                Serial.println(F("Run Once! (setPitTempTarget::hasRunFlag == 0); Changing Encoder Settings 0,2,0 "));
                encoder.newSettings(0, 2, 0, currentEncoderState);
                currentEncoderValue = currentEncoderState.currentValue;  //do I need this line?  Check
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;

                hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
            }

            // Select & Tap to set meatDoneTemp setting
            if (button.triggered(SINGLE_TAP)) {
                if (prevEncoderValue == 1) {
                    hasRunFlag = 0;  //reset hasRunFlag
                    bool adjustTempFlag = 1;
                    float temporaryTemperatureTarget;  // temporary variable while desired meat done temperature is being adjusted

                    while (adjustTempFlag) {
                        // reset encoder scale to match number of menu list items
                        if (hasRunFlag == 0) {
                            Serial.println(F("Run Once! (setPitTempTarget inside block::hasRunFlag == 0); Changing Encoder Settings degF:211,350,225 or degC:100,177,107  "));
                            if (degCFlag) {
                                encoder.newSettings(100, 177, 107, currentEncoderState);
                            } else {
                                encoder.newSettings(212, 350, 225, currentEncoderState);
                            }
                            currentEncoderValue = currentEncoderState.currentValue;  //do I need this line?  Check
                            Serial.println(currentEncoderValue);
                            prevEncoderValue = currentEncoderValue;
                            hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
                        }
                        bool meatTargetFlag = 0;  // 0 for pitTempTarget
                        currentEncoderValue = encoder.getCount();
                        temporaryTemperatureTarget = prevEncoderValue;  // implicit conversion of int16_t prevEncoderValue to float temporaryTemperatureTarget

                        yield();  // Do (almost) nothing -- yield allows ESP8266 background functions
                        lcd.showTemeratureTargetAdjustment(temporaryTemperatureTarget, meatTargetFlag);

                        // exit temperature adjustment loop upon next button press
                        if (button.update()) {
                            if (button.triggered(SINGLE_TAP)) {
                                adjustTempFlag = 0;
                                pitTempTarget = temporaryTemperatureTarget;
                                hasRunFlag = 0;  // allow another reset of encoder scale range
                                //smokerState = changeSettings;  // go up one level next loop

                                //debug
                                Serial.println();
                                Serial.print(F("New pitTempTarget value set -> new pitTempTarget = "));
                                Serial.println(pitTempTarget);
                                // end debug

                                // reset encoder scale to match number of menu list items
                                if (hasRunFlag == 0) {
                                    Serial.println(F("Run Once! (return to changeSettings::hasRunFlag == 0); Changing Encoder Settings - setPitTempTarget 0,2,0"));
                                    encoder.newSettings(0, 2, 0, currentEncoderState);
                                    currentEncoderValue = currentEncoderState.currentValue;  //do I need this line?  Check
                                    Serial.println(currentEncoderValue);
                                    prevEncoderValue = currentEncoderValue;

                                    hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
                                }
                            }
                        }
                    }  // end While
                    Serial.println(F("while (adjustTempFlag) -> exited while loop"));
                }  //end if

                button.update();
            }

            // hold button to exit
            if (button.triggered(HOLD)) {
                smokerState = splashScreen;  // return to launchPad menu (one level up)
                Serial.print(F("Hold press - going back to splashScreen; smokerState = "));
                Serial.println(smokerState);
                hasRunFlag = 0;  // allow another reset of encoder scale range
            }

        } break;

        case setTempUnits: {
            currentEncoderValue = encoder.getCount();

            // reset encoder scale to match number of menu list items
            if (hasRunFlag == 0) {
                Serial.println(F("Run Once! (setTempUnits::hasRunFlag == 0); Changing Encoder Settings 0,2,0"));
                encoder.newSettings(0, 2, 0, currentEncoderState);
                currentEncoderValue = currentEncoderState.currentValue;  //do I need this line?  Check
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;

                hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
            }

            lcd.showSetTempUnitsMenu(prevEncoderValue);

            if (button.triggered(SINGLE_TAP)) {
                if (prevEncoderValue == 1) {  //ensure termperature unit change line is selected
                    if (degCFlag == 0) {
                        degCFlag = 1;
                        Serial.println(F("degC Flag set - Temperature units now in degC!"));  //debug
                    } else {
                        degCFlag = 0;
                        Serial.println(F("degF Flag set - Temperature units now in degF!"));  //debug
                    }
                }
            }

            if (button.triggered(HOLD)) {
                smokerState = splashScreen;  // return to splashScreen
                hasRunFlag = 0;              // allow another reset of encoder scale range
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
