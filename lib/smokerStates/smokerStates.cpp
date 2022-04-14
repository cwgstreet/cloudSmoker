/* ***************************************************************
 * smokerStates.h - cloudSmoker state machine implementation routines
 *
 * See (github link tba) for cloudSmoker State Machine diagram
 *
 *  C W Greenstreet, Ver1, 16Dec21
 *    MIT Licence - Released into the public domain
 *
 ** ************************************************************* */

// set up debug scaffold; comment out following line if you want to "turn off" debugging to serial monitor
#define DEBUG_ADC_Temps 1  // uncomment to debug ADC1015 ADC readings

// include 3rd party libraries
#include <Arduino.h>
#include <NewEncoder.h>
#include <Wire.h>                           // i2C device communication
#include <hd44780.h>                        // LCD library
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

// incliude local libraries
#include "cwg_ads1x15.h"        // ADS1x15 I2C ADC device functionality
#include "cwg_steinhartHart.h"  // Thermistor steinhart hart temperature calcs
#include "helper_functions.h"
#include "lcd.h"
#include "press_type.h"
#include "secrets.h"
#include "smokerStates.h"
#include "wrapEncoder.h"

// always include thingspeak header file after other header files and custom macros
#include <ThingSpeak.h>

// entryStates is an enum variable type defined in menu.h header file (as extern); smokerState is global
entryStates_t smokerState;

extern float batteryVoltage_v;

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
                currentEncoderValue = currentEncoderState.currentValue;  // do I need this line?  Check
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
                yield();                                                               // Do (almost) nothing -- yield allows ESP8266 background functions
                Serial.println("-------------------Start Cook Now-----------------");  // debug
                smokerState = getTemp;                                                 // start bbq cook
                                                                                       // long unsigned startCookTime_ms = millis();  // capture cook start time; var defined as global (extern in helper_functions.h)
                startCookTime_ms = millis();                                           // capture cook start time; var defined as global (extern in helper_functions.h)

                Serial.print("startCookTime_ms = ");  // debug
                Serial.println(startCookTime_ms);     // debug
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

                // debug statements
                // Serial.println();
                //  Serial.print(F("processStates hasRunFlag block triggered (==0) -> prevEncoderValue: "));  //debug
                //  Serial.println(prevEncoderValue);
                //  end debug
            }

            if (button.triggered(SINGLE_TAP)) {
                // debug statements
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
                    hasRunFlag = 0;                 // reset flag to allow next encoder scale change

                    // debug statements
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
                    hasRunFlag = 0;                  // reset flag for next encoder scale change
                    break;
                }

                if (prevEncoderValue == 6) {
                    hasRunFlag = 0;              // reset flag for next encoder scale change
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
                currentEncoderValue = currentEncoderState.currentValue;  // TODO: do I need this line?  Check
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;

                hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
            }

            // Select & Tap to set meatDoneTemp setting
            if (button.triggered(SINGLE_TAP)) {
                if (prevEncoderValue == 1) {
                    hasRunFlag = 0;  // reset hasRunFlag
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
                            currentEncoderValue = currentEncoderState.currentValue;  // do I need this line?  Check
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
                                // smokerState = changeSettings;  // go up one level next loop

                                // debug
                                Serial.println();
                                Serial.print(F("New meatDoneTemp value set -> new meatDoneTemp = "));
                                Serial.println(meatDoneTemp);
                                // end debug

                                // reset encoder scale to match number of menu list items
                                if (hasRunFlag == 0) {
                                    Serial.println(F("Run Once! (return to changeSettings::hasRunFlag == 0); Changing Encoder Settings - setMeatDoneTemp 0,2,0"));
                                    encoder.newSettings(0, 2, 0, currentEncoderState);
                                    currentEncoderValue = currentEncoderState.currentValue;  // do I need this line?  Check
                                    Serial.println(currentEncoderValue);
                                    prevEncoderValue = currentEncoderValue;

                                    hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
                                }
                            }
                        }
                    }  // end While
                    Serial.println(F("while (adjustTempFlag) -> exited while loop"));
                }  // end if

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
            // lcd.printMenuLine("sm setpitTemp");  // temporary to confirm navigation branch
            lcd.showSetPitTempTargetMenu(prevEncoderValue);
            currentEncoderValue = encoder.getCount();

            // reset encoder scale to match number of menu list items
            if (hasRunFlag == 0) {
                Serial.println(F("Run Once! (setPitTempTarget::hasRunFlag == 0); Changing Encoder Settings 0,2,0 "));
                encoder.newSettings(0, 2, 0, currentEncoderState);
                currentEncoderValue = currentEncoderState.currentValue;  // do I need this line?  Check
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;

                hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
            }

            // Select & Tap to set meatDoneTemp setting
            if (button.triggered(SINGLE_TAP)) {
                if (prevEncoderValue == 1) {
                    hasRunFlag = 0;  // reset hasRunFlag
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
                            currentEncoderValue = currentEncoderState.currentValue;  // do I need this line?  Check
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
                                // smokerState = changeSettings;  // go up one level next loop

                                // debug
                                Serial.println();
                                Serial.print(F("New pitTempTarget value set -> new pitTempTarget = "));
                                Serial.println(pitTempTarget);
                                // end debug

                                // reset encoder scale to match number of menu list items
                                if (hasRunFlag == 0) {
                                    Serial.println(F("Run Once! (return to changeSettings::hasRunFlag == 0); Changing Encoder Settings - setPitTempTarget 0,2,0"));
                                    encoder.newSettings(0, 2, 0, currentEncoderState);
                                    currentEncoderValue = currentEncoderState.currentValue;  // do I need this line?  Check
                                    Serial.println(currentEncoderValue);
                                    prevEncoderValue = currentEncoderValue;

                                    hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
                                }
                            }
                        }
                    }  // end While
                    Serial.println(F("while (adjustTempFlag) -> exited while loop"));
                }  // end if

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
                currentEncoderValue = currentEncoderState.currentValue;  // do I need this line?  Check
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;

                hasRunFlag = 1;  // ensure settings are only changed once given this is part of loop()
            }

            lcd.showSetTempUnitsMenu(prevEncoderValue);

            if (button.triggered(SINGLE_TAP)) {
                if (prevEncoderValue == 1) {  // ensure termperature unit change line is selected
                    if (degCFlag == 0) {
                        degCFlag = 1;
                        Serial.println(F("degC Flag set - Temperature units now in degC!"));  // debug
                    } else {
                        degCFlag = 0;
                        Serial.println(F("degF Flag set - Temperature units now in degF!"));  // debug
                    }
                }
            }

            if (button.triggered(HOLD)) {
                smokerState = splashScreen;  // return to splashScreen
                hasRunFlag = 0;              // allow another reset of encoder scale range
            }
            break;

            case getTemp: {
                Serial.print("getTemp Case -> startCookTime_ms =");  // debug
                Serial.println(startCookTime_ms);                    // debug

                // hasRunFlag = 0;  // reset run-once flag

                // obtain 11 ADC readings from designated pin and return a median filtered value (variables are globals)
                double voltageVCC_medianFiltered_V = ads1015.getSensorValue_MedianFiltered_V(ADC_VCCsupplyPin, 11);
                batteryVoltage_v = voltageVCC_medianFiltered_V * 1;
                double voltagePit_medianFiltered_V = ads1015.getSensorValue_MedianFiltered_V(ADC_pitPin, 11);
                double voltageMeat_medianFiltered_V = ads1015.getSensorValue_MedianFiltered_V(ADC_meatPin, 11);

                yield();  // allow ESP8266 background functions to "play through" - avoid blocking

                // convert thermistor ADC voltage readings to temperature (degF; other units are just converted on fly for display)
                currentMeatTemp = sh_meatProbe.getTempFahrenheit(voltageVCC_medianFiltered_V, voltageMeat_medianFiltered_V);
                currentPitTemp = sh_pitProbe.getTempFahrenheit(voltageVCC_medianFiltered_V, voltagePit_medianFiltered_V);

#ifdef DEBUG_ADC_Temps  // *****  debug - ADS1015 ADC *****
                Serial.println();
                Serial.println(F("MF_VCC\t|\tMF_PIT\t|\tPITdegF\t|\tMF_MEAT\t|\tMEATdegF"));
                Serial.println(F("-------\t|\t-------\t|\t-------\t|\t-------\t|\t-------"));
                Serial.print(voltageVCC_medianFiltered_V, 4);
                Serial.print(F("\t|\t"));
                Serial.print(voltagePit_medianFiltered_V, 4);
                Serial.print(F("\t|\t"));
                Serial.print(currentPitTemp);
                Serial.print(F("\t|\t"));
                Serial.print(voltageMeat_medianFiltered_V, 4);
                Serial.print(F("\t|\t"));
                Serial.println(currentMeatTemp);
                Serial.println("leaving getTemps state now for txTemps");
                Serial.println();
#endif  // end DEBUG

                smokerState = txTemp;
            } break;

            case txTemp: {
                // implicit typecast from double to float as ThingSpeak.setField requires float (otherwise no match in method overlaod signature)
                float currentMeatTemp_float = currentMeatTemp;
                float currentpitTemp_float = currentPitTemp;

                ThingSpeak.setField(1, currentMeatTemp_float);
                ThingSpeak.setField(2, meatDoneTemp);
                ThingSpeak.setField(3, currentpitTemp_float);
                ThingSpeak.setField(4, pitTempTarget);
                ThingSpeak.setField(5, batteryVoltage_v);

                // ThingSpeak.writeFields(THNGSPK_CHANNEL_ID, THNGSPK_WRITE_API_KEY);

                smokerState = Sleep;  // go back to sleep
                hasRunFlag = 0;       // reset run-once flag
            // smokerState = launchPad;  // debug code.  Remove line once stack dump error found

            } break;

            case Sleep: {
                // TODO: explore putting ESP8266 to sleep (modem / light / deep sleep) between readings for power savings
                    Serial.println("Sleep case - if hasRunFlag==0");

                if (hasRunFlag == 0) {
                    // lcd.sleepScreen();  // disable (hide) pixels on display
                    Serial.print("*");
                    if (encoder.moved()) {
                    lcd.wakeScreen();                                      // enable pixels on display
                    lcd.printMenuLine_noArrow("Sleep state - waking up");  // debug
                    Serial.println("Sleep state - waking up");             // debug code
                    smokerState = bbqStatus;
                    hasRunFlag = 1;
                }

                    unsigned long currentMillis = millis();  // grab current time
                    unsigned long previousMillis = 0;        // millis() returns an unsigned long

                    unsigned long transmitInterval = 30000;  // wait time (30 seconds)
                    //! change to 60 seconds?

                    // check if "displayInterval" time has passed  and,
                    if ((unsigned long)(currentMillis - previousMillis) >= transmitInterval) {
                        smokerState = txTemp;                                                                     // when true, update ThingSpeak channel with new temperature readings
                        hasRunFlag = 1;                                                                           // set run-once flag
                        Serial.println("sleep transmit interval reached. smokerState = txTemp, hasRunFlag =1 ");  // debug
                    }
                }

                
                // smokerState = launchPad;  // debug code.  Remove line once stack dump error found
            }

        } break;

        case bbqStatus: {
            lcd.display();  // make sure screen is not blanked
            lcd.showBBQStatusScreen(degCFlag, currentMeatTemp, currentPitTemp);

            Serial.println("entered bbqStatus state");

            // non-blocking delay for BBQ Display before clearing screen / sleep
            unsigned long displayInterval = 7000;  // the time we need to wait (7 seconds)
            unsigned long previousMillis = 0;      // millis() returns an unsigned long

            unsigned long currentMillis = millis();  // grab current time

            // check if "displayInterval" time has passed  and,
            if ((unsigned long)(currentMillis - previousMillis) >= displayInterval) {
                smokerState = Sleep;  // when true, then go back to Sleep case (blank screen)
                hasRunFlag = 0;       // reset run-once flag
            }

            // smokerState = launchPad;  // debug code.  Remove line once stack dump error found

        } break;

        default: {
            smokerState = bbqStatus;
        } break;
    }
}
//}
