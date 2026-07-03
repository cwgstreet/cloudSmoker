/* ***************************************************************
 * smokerStates.cpp - cloudSmoker state machine implementation routines
 *
 * See (github link tba) for cloudSmoker State Machine diagram
 *
 * C W Greenstreet, cloudSmoker2, 1Jul26
 * Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public 
 * distribution must also share the full source code under this same license.
 *
 ** ************************************************************* */

//#define DEBUG_ADC_Temps 1  // uncomment to debug ADC1015 ADC readings

// include 3rd party libraries
#include <Arduino.h>
#include <NewEncoder.h>
#include <Wire.h>                           // i2C device communication

// include local libraries
#include "cwg_ads1x15.h"        // ADS1x15 I2C ADC device functionality
#include "cwg_steinhartHart.h"  // Thermistor steinhart hart temperature calcs
#include "helper_functions.h"
#include "lcd.h"
#include "press_type.h"
#include "secrets.h"            // Ensure this file exists in your 'include/' or 'src/' directory
#include "smokerStates.h"
#include "wrapEncoder.h"

// always include thingspeak header file after other header files and custom macros
#include <ThingSpeak.h>

// entryStates is an enum variable type defined in menu.h header file (as extern); smokerState is global
entryStates_t smokerState;

// External globals from main.cpp mapped with exact matching types
extern float meatDoneTemp;
extern float pitTempTarget;
extern float currentMeatTemp;
extern float currentPitTemp;
extern bool degCFlag;
extern int16_t prevEncoderValue;

// Analog telemetry globals (matching the double precision in main)
extern double voltagePit_medianFiltered_V;
extern double voltageMeat_medianFiltered_V;
extern float batteryVoltage_v;

void processState(CWG_LCD &lcd) {
    // Declare this at the top of the function to avoid switch-case initialization bypass errors
    int16_t currentEncoderValue; 

    // Tracks whether we've *just* transitioned into this state this pass —
    // lets each case draw its display screen once on entry instead on every loop (flickering)
    static entryStates_t previousState = (entryStates_t)0;
    bool enteringState = (smokerState != previousState);
    previousState = smokerState;

    switch (smokerState) {

   case splashScreen: {
            if (enteringState) {  //only run once when entering this state
                lcd.showSplashScreen(degCFlag, meatDoneTemp, pitTempTarget);
            }
            if (encoder.moved()) {
                smokerState = launchPad;
            }
        } break;
     
        case launchPad: {
            if (hasRunFlag == 0) {
                Serial.println(F("Run Once! (launchPad::hasRunFlag == 0); Changing Encoder Settings - setTempUnits 0,1,0"));
                encoder.newSettings(0, 1, 1, currentEncoderState);
                currentEncoderValue = currentEncoderState.currentValue;
                prevEncoderValue = currentEncoderValue;
                hasRunFlag = 1;
            }

            if (enteringState) {
                lcd.showLaunchPad();
            }

            if (button.triggered(DOUBLE_TAP)) {
                yield();
                hasRunFlag = 0;
                smokerState = changeSettings;
            }
            if (button.triggered(SINGLE_TAP)) {
                yield();
                Serial.println("-------------------Start Cook Now-----------------");
                smokerState = getTemp;
                startCookTime_ms = millis();
            }
        } break; 

    

case changeSettings: {
            lcd.showSettingsMenu(prevEncoderValue);
            currentEncoderValue = encoder.getCount();

            if (hasRunFlag == 0) {
                // We use MENU_COUNT to ensure the encoder matches our menu size
                encoder.newSettings(1, MENU_COUNT, 1, currentEncoderState);
                prevEncoderValue = currentEncoderState.currentValue;
                hasRunFlag = 1;  
            }

            if (button.triggered(SINGLE_TAP)) {
                // Use the enum constants instead of raw numbers
                switch (prevEncoderValue) {
                    case MENU_MEAT_TEMP:
                        smokerState = setMeatDoneTemp;
                        hasRunFlag = 0;
                        break;
                    case MENU_PIT_TEMP:
                        smokerState = setPitTempTarget;
                        hasRunFlag = 0;
                        break;
                    case MENU_UNITS:
                        smokerState = setTempUnits;
                        hasRunFlag = 0;
                        break;
                    case MENU_BACK:
                        smokerState = launchPad; // Or wherever "Back" should take you
                        hasRunFlag = 0;
                        break;
                }
            }

            if (button.triggered(HOLD)) {
                smokerState = splashScreen;  
                hasRunFlag = 0;              
            }
        } break;

        case setMeatDoneTemp: {
            lcd.showSetMeatDoneTempMenu(prevEncoderValue);
            currentEncoderValue = encoder.getCount();

            if (hasRunFlag == 0) {
                Serial.println(F("Run Once! (setMeatDoneTemp::hasRunFlag == 0); Changing Encoder Settings 0,2,0 "));
                encoder.newSettings(0, 2, 0, currentEncoderState);
                currentEncoderValue = currentEncoderState.currentValue;  
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;
                hasRunFlag = 1;  
            }

            if (button.triggered(SINGLE_TAP)) {
                if (prevEncoderValue == 1) {
                    hasRunFlag = 0;  
                    bool adjustTempFlag = 1;
                    float temporaryTemperatureTarget;  

                    while (adjustTempFlag) {
                        if (hasRunFlag == 0) {
                            Serial.println(F("Run Once! (setMeatDoneTemp inside block::hasRunFlag == 0); Changing Encoder Settings 122,220,203 or 90,105,95 "));
                            if (degCFlag) {
                                encoder.newSettings(90, 105, 95, currentEncoderState);
                            } else {
                                encoder.newSettings(122, 220, 203, currentEncoderState);
                            }
                            currentEncoderValue = currentEncoderState.currentValue;  
                            Serial.println(currentEncoderValue);
                            prevEncoderValue = currentEncoderValue;
                            hasRunFlag = 1;  
                        }
                        bool meatTargetFlag = 1;
                        currentEncoderValue = encoder.getCount();
                        temporaryTemperatureTarget = prevEncoderValue;  

                        yield();  
                        lcd.showTemperatureTargetAdjustment(temporaryTemperatureTarget, meatTargetFlag);

                        if (button.update()) {
                            if (button.triggered(SINGLE_TAP)) {
                                adjustTempFlag = 0;
                                meatDoneTemp = temporaryTemperatureTarget;
                                hasRunFlag = 0;  

                                Serial.println();
                                Serial.print(F("New meatDoneTemp value set -> new meatDoneTemp = "));
                                Serial.println(meatDoneTemp);

                                if (hasRunFlag == 0) {
                                    Serial.println(F("Run Once! (return to changeSettings::hasRunFlag == 0); Changing Encoder Settings - setMeatDoneTemp 0,2,0"));
                                    encoder.newSettings(0, 2, 0, currentEncoderState);
                                    currentEncoderValue = currentEncoderState.currentValue;  
                                    Serial.println(currentEncoderValue);
                                    prevEncoderValue = currentEncoderValue;
                                    hasRunFlag = 1;  
                                }
                            }
                        }
                    }  
                    Serial.println(F("while (adjustTempFlag) -> exited while loop"));
                }  
                button.update();
            }

            if (button.triggered(HOLD)) {
                smokerState = splashScreen;  
                Serial.print(F("Hold press - going up one level; smokerState = "));
                Serial.println(smokerState);
                hasRunFlag = 0;  
            }
        } break;

        case setPitTempTarget: {
            lcd.showSetPitTempTargetMenu(prevEncoderValue);
            currentEncoderValue = encoder.getCount();

            if (hasRunFlag == 0) {
                Serial.println(F("Run Once! (setPitTempTarget::hasRunFlag == 0); Changing Encoder Settings 0,2,0 "));
                encoder.newSettings(0, 2, 0, currentEncoderState);
                currentEncoderValue = currentEncoderState.currentValue;  
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;
                hasRunFlag = 1;  
            }

            if (button.triggered(SINGLE_TAP)) {
                if (prevEncoderValue == 1) {
                    hasRunFlag = 0;  
                    bool adjustTempFlag = 1;
                    float temporaryTemperatureTarget;  

                    while (adjustTempFlag) {
                        if (hasRunFlag == 0) {
                            Serial.println(F("Run Once! (setPitTempTarget inside block::hasRunFlag == 0); Changing Encoder Settings degF:211,350,225 or degC:100,177,107  "));
                            if (degCFlag) {
                                encoder.newSettings(100, 177, 107, currentEncoderState);
                            } else {
                                encoder.newSettings(212, 350, 225, currentEncoderState);
                            }
                            currentEncoderValue = currentEncoderState.currentValue;  
                            Serial.println(currentEncoderValue);
                            prevEncoderValue = currentEncoderValue;
                            hasRunFlag = 1;  
                        }
                        bool meatTargetFlag = 0;  
                        currentEncoderValue = encoder.getCount();
                        temporaryTemperatureTarget = prevEncoderValue;  

                        yield();  
                        lcd.showTemperatureTargetAdjustment(temporaryTemperatureTarget, meatTargetFlag);

                        if (button.update()) {
                            if (button.triggered(SINGLE_TAP)) {
                                adjustTempFlag = 0;
                                pitTempTarget = temporaryTemperatureTarget;
                                hasRunFlag = 0;  

                                Serial.println();
                                Serial.print(F("New pitTempTarget value set -> new pitTempTarget = "));
                                Serial.println(pitTempTarget);

                                if (hasRunFlag == 0) {
                                    Serial.println(F("Run Once! (return to changeSettings::hasRunFlag == 0); Changing Encoder Settings - setPitTempTarget 0,2,0"));
                                    encoder.newSettings(0, 2, 0, currentEncoderState);
                                    currentEncoderValue = currentEncoderState.currentValue;  
                                    Serial.println(currentEncoderValue);
                                    prevEncoderValue = currentEncoderValue;
                                    hasRunFlag = 1;  
                                }
                            }
                        }
                    }  
                    Serial.println(F("while (adjustTempFlag) -> exited while loop"));
                }  
                button.update();
            }

            if (button.triggered(HOLD)) {
                smokerState = splashScreen;  
                Serial.print(F("Hold press - going back to splashScreen; smokerState = "));
                Serial.println(smokerState);
                hasRunFlag = 0;  
            }
        } break;

        case setTempUnits: {
            currentEncoderValue = encoder.getCount();

            if (hasRunFlag == 0) {
                Serial.println(F("Run Once! (setTempUnits::hasRunFlag == 0); Changing Encoder Settings 0,2,0"));
                encoder.newSettings(0, 2, 0, currentEncoderState);
                currentEncoderValue = currentEncoderState.currentValue;  
                Serial.println(currentEncoderValue);
                prevEncoderValue = currentEncoderValue;
                hasRunFlag = 1;  
            }

            lcd.showSetTempUnitsMenu(prevEncoderValue);

            if (button.triggered(SINGLE_TAP)) {
                if (prevEncoderValue == 1) {  
                    if (degCFlag == 0) {
                        degCFlag = 1;
                        Serial.println(F("degC Flag set - Temperature units now in degC!"));  
                    } else {
                        degCFlag = 0;
                        Serial.println(F("degF Flag set - Temperature units now in degF!"));  
                    }
                }
            }

            if (button.triggered(HOLD)) {
                smokerState = splashScreen;  
                hasRunFlag = 0;              
            }
        } break; // FIXED: Closed the brace and terminated the block properly here

        case getTemp: {
            Serial.print("getTemp Case -> startCookTime_ms =");  
            Serial.println(startCookTime_ms);                    

            double voltageVCC_medianFiltered_V = ads1015.getSensorValue_MedianFiltered_V(ADC_VCCsupplyPin, 11);
            batteryVoltage_v = voltageVCC_medianFiltered_V * 1;
            
            // FIXED: Removed "double" declaration redefinition so values populate the global extern variables
            voltagePit_medianFiltered_V = ads1015.getSensorValue_MedianFiltered_V(ADC_pitPin, 11);
            voltageMeat_medianFiltered_V = ads1015.getSensorValue_MedianFiltered_V(ADC_meatPin, 11);

            yield();  

            currentMeatTemp = sh_meatProbe.getTempFahrenheit(voltageVCC_medianFiltered_V, voltageMeat_medianFiltered_V);
            currentPitTemp = sh_pitProbe.getTempFahrenheit(voltageVCC_medianFiltered_V, voltagePit_medianFiltered_V);

#ifdef DEBUG_ADC_Temps  
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
#endif  

            smokerState = txTemp;
        } break;

        case txTemp: {
            Serial.println("entering txTemp case -> transmitting data");  
            WiFiClient client;
            ThingSpeak.begin(client);

            float currentMeatTemp_float = currentMeatTemp;
            float currentpitTemp_float = currentPitTemp;

            ThingSpeak.setField(1, currentMeatTemp_float);
            ThingSpeak.setField(2, meatDoneTemp);

            if (currentpitTemp_float > 500) {
                ThingSpeak.setField(3, currentpitTemp_float);
            }

            ThingSpeak.setField(4, pitTempTarget);
            ThingSpeak.setField(5, batteryVoltage_v);

            ThingSpeak.writeFields(THNGSPK_CHANNEL_ID, THNGSPK_WRITE_API_KEY);

            hasRunFlag = 0;       
            smokerState = Sleep;  
            Serial.println("leaving for Sleep case");
        } break;

        case Sleep: {
            if (encoder.moved()) {
                lcd.display();                                              
                Serial.println("Sleep state, encoder moved -> waking up");  
                smokerState = bbqStatus;
                hasRunFlag = 1;
                Serial.println("leaving for bbqStatus state");
            }

            if (hasRunFlag == 0) {
                lcd.noDisplay();  

                unsigned long currentMillis = millis();  

                if ((unsigned long)(currentMillis - previousMillis) >= transmitInterval) {
                    previousMillis = millis();
                    smokerState = getTemp;                                                                                   
                    hasRunFlag = 1;                                                                                          
                    Serial.println("sleep transmit duration interval reached. smokerState = getTemp, reset hasRunFlag =1 ");  
                }
            }
        } break;

        case bbqStatus: {
            lcd.display();  
            lcd.showBBQStatusScreen(degCFlag, currentMeatTemp, currentPitTemp);

            unsigned long currentMillis = millis();  

            if ((unsigned long)(currentMillis - previousDisplayMillis) >= displayInterval) {
                previousDisplayMillis = millis();
                smokerState = Sleep;  
                hasRunFlag = 0;       
            }
        } break;

        default: {
            smokerState = bbqStatus;
        } break;
    }
}