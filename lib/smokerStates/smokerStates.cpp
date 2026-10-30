/* ***************************************************************
 * smokerStates.cpp - cloudSmoker state machine implementation routines
 *
 * See cloudSmoker State Machine diagram
 *  https://github.com/cwgstreet/cloudSmoker/wiki/V2-SM_structure
 *
 * C W Greenstreet, cloudSmoker2, 1Jul26
 * Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 * distribution must also share the full source code under this same license.
 *
 ** ************************************************************* */

// #define DEBUG_ADC_Temps 1  // uncomment to debug ADC1015 ADC readings

// include 3rd party libraries
#include <Arduino.h>
#include <NewEncoder.h>
#include <Wire.h>  // i2C device communication

// include local libraries
#include "cwg_ads1x15.h"        // ADS1x15 I2C ADC device functionality
#include "cwg_steinhartHart.h"  // Thermistor steinhart hart temperature calcs
#include "helper_functions.h"
#include "lcd.h"
#include "press_type.h"
#include "secrets.h"  // Ensure this file exists in your 'include/' or 'src/' directory
#include "smokerStates.h"
#include "wrapEncoder.h"

// always include thingspeak header file after other header files and custom macros
#include <ThingSpeak.h>

// include WiFi and HTTPClient for ntfy.sh notifications
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// include APIs for ntfy.sh notifications
#include "secrets.h"

// entryStates is an enum variable type defined in menu.h header file (as extern); smokerState is
// global
entryStates_t smokerState;

// External globals from main.cpp mapped with exact matching types
extern float meatDoneTemp;
extern float pitTempTarget;
extern float currentPitTemp;
extern float currentMeat1Temp;
extern float currentMeat2Temp;
extern float currentMeat3Temp;
extern bool degCFlag;
extern int16_t prevEncoderValue;

// Analog telemetry globals (matching the double precision in main)
extern double voltagePit_medianFiltered_V;
extern double voltageMeat1_medianFiltered_V;
extern double voltageMeat2_medianFiltered_V;
extern double voltageMeat3_medianFiltered_V;
extern float batteryVoltage_v;

// Done decision for one probe. NaN (no probe) is checked first, so it can never
//    mistakenly become DONE.
enum DoneStatus { UNKNOWN, NO_DATA, NOT_DONE, DONE };

// Per-probe DON state. Flags are set once per cook and cleared when a new cook starts.
DoneStatus meat1LastStatus = UNKNOWN;
DoneStatus meat2LastStatus = UNKNOWN;
DoneStatus meat3LastStatus = UNKNOWN;
bool meat1DoneFlag = false;
bool meat2DoneFlag = false;
bool meat3DoneFlag = false;

// ntfy.sh alert sent flags for each probe. These are set once per cook
//  when the alert is sent, and cleared when a new cook starts.
bool meat1SentFlag = false;  // set once the MEAT1 alert has been delivered this cook
bool meat2SentFlag = false;
bool meat3SentFlag = false;

DoneStatus probeDoneStatus(float tempF, float targetF) {
  if (isnan(tempF)) {
    return NO_DATA;
  }
  return (tempF >= targetF) ? DONE : NOT_DONE;
}

// Sends one message to the ntfy topic over HTTPS. Returns true only on HTTP 200.
bool sendNtfyAlert(const String& message) {
  WiFiClientSecure client;
  client.setInsecure();  // HTTPS encryption without certificate check (accepted risk)
  HTTPClient http;
  String url = String("https://ntfy.sh/") + NTFY_TOPIC;
  http.begin(client, url);
  http.setTimeout(5000);
  int code = http.POST(message);
  http.end();
  Serial.print(F("ntfy HTTP code: "));
  Serial.println(code);
  return code == 200;
}

// Reports done transitions for one probe and sends the alert once per cook.
// lastStatus and doneFlag are references, so the helper updates each probe's own variables.
void checkProbeDone(const char* name, float tempF, DoneStatus& lastStatus, bool& doneFlag,
                    bool& sentFlag) {
  DoneStatus status = probeDoneStatus(tempF, meatDoneTemp);

  if (status != lastStatus) {
    Serial.print(name);
    Serial.print(F(" done status -> "));
    if (status == DONE) {
      Serial.println(F("DONE"));
    } else if (status == NOT_DONE) {
      Serial.println(F("NOT DONE"));
    } else {
      Serial.println(F("NO DATA - Check Probe"));
    }
    lastStatus = status;
  }

  if (status == DONE && !doneFlag) {
    doneFlag = true;
    Serial.print(name);
    Serial.println(F(" DONE flag set: alert due (first time this cook)"));
  }

  // Alert: send once per cook, retried each cycle until it succeeds
  if (doneFlag && !sentFlag && WiFi.status() == WL_CONNECTED) {
    String msg = String(name) + " reached " + String(tempF, 0) + "F (target " +
                 String(meatDoneTemp, 0) + "F)";
    if (sendNtfyAlert(msg)) {
      sentFlag = true;
      Serial.print(name);
      Serial.println(F(" alert sent"));
    } else {
      Serial.print(name);
      Serial.println(F(" alert not sent, will retry"));
    }
  }
}

void processState(CWG_LCD& lcd) {
  // Declare this at the top of the function to avoid switch-case initialization bypass errors
  int16_t currentEncoderValue;

  // Tracks whether we've *just* transitioned into this state this pass —
  // lets each case draw its display screen once on entry instead on every loop (flickering)
  static entryStates_t previousState = (entryStates_t)0;
  bool enteringState = (smokerState != previousState);
  previousState = smokerState;

  switch (smokerState) {
    case splashScreen: {
      if (enteringState) {  // only run once when entering this state
        lcd.showSplashScreen(degCFlag, meatDoneTemp, pitTempTarget);
      }
      if (encoder.moved()) {
        smokerState = launchPad;
      }
    } break;

    case launchPad: {
      if (hasRunFlag == 0) {
        Serial.println(
            F("Run Once! (launchPad::hasRunFlag == 0); Changing Encoder Settings - setTempUnits "
              "0,1,0"));
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
        Serial.println("\n-------------------Start Cook Now-----------------");

        // New cook: clear per-probe done state so each probe alerts once for this cook
        meat1DoneFlag = false;
        meat2DoneFlag = false;
        meat3DoneFlag = false;
        meat1SentFlag = false;
        meat2SentFlag = false;
        meat3SentFlag = false;
        meat1LastStatus = UNKNOWN;
        meat2LastStatus = UNKNOWN;
        meat3LastStatus = UNKNOWN;

        smokerState = getTemp;
        startCookTime_ms = millis();
      }
    } break;

    case changeSettings: {
      static int16_t lastDrawnSelection = -1;
      currentEncoderValue = encoder.getCount();

      if (hasRunFlag == 0) {
        encoder.newSettings(1, MENU_COUNT, 1, currentEncoderState);
        prevEncoderValue = currentEncoderState.currentValue;
        hasRunFlag = 1;
        lastDrawnSelection = -1;
      }

      if (prevEncoderValue != lastDrawnSelection) {
        lcd.showSettingsMenu(prevEncoderValue);
        lastDrawnSelection = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
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
            smokerState = launchPad;
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
      static int16_t lastDrawnSelection = -1;
      currentEncoderValue = encoder.getCount();

      if (hasRunFlag == 0) {
        encoder.newSettings(1, 2, 1, currentEncoderState);  // 1=Edit, 2=Back
        currentEncoderValue = currentEncoderState.currentValue;
        prevEncoderValue = currentEncoderValue;
        hasRunFlag = 1;
        lastDrawnSelection = -1;
      }

      if (prevEncoderValue != lastDrawnSelection) {
        lcd.showSetMeatDoneTempMenu(prevEncoderValue, meatDoneTemp, degCFlag);
        lastDrawnSelection = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
        if (prevEncoderValue == 1) {
          hasRunFlag = 0;
          bool adjustTempFlag = 1;
          float temporaryTemperatureTarget;
          int16_t lastDrawnAdjustValue = -1;

          while (adjustTempFlag) {
            if (hasRunFlag == 0) {
              if (degCFlag) {
                encoder.newSettings(90, 105, 95, currentEncoderState);
              } else {
                encoder.newSettings(122, 220, 203, currentEncoderState);
              }
              currentEncoderValue = currentEncoderState.currentValue;
              prevEncoderValue = currentEncoderValue;
              hasRunFlag = 1;
              lastDrawnAdjustValue = -1;
            }
            bool meatTargetFlag = 1;
            currentEncoderValue = encoder.getCount();
            temporaryTemperatureTarget = prevEncoderValue;

            yield();

            if ((int16_t)temporaryTemperatureTarget != lastDrawnAdjustValue) {
              lcd.showTemperatureTargetAdjustment(temporaryTemperatureTarget, meatTargetFlag);
              lastDrawnAdjustValue = (int16_t)temporaryTemperatureTarget;
            }

            if (button.update()) {
              if (button.triggered(SINGLE_TAP)) {
                adjustTempFlag = 0;
                meatDoneTemp = temporaryTemperatureTarget;
                hasRunFlag = 0;

                Serial.print(F("New meatDoneTemp value set -> new meatDoneTemp = "));
                Serial.println(meatDoneTemp);

                encoder.newSettings(1, 2, 1, currentEncoderState);
                currentEncoderValue = currentEncoderState.currentValue;
                prevEncoderValue = currentEncoderValue;
                hasRunFlag = 1;
                lastDrawnSelection = -1;  // force settings screen to redraw on return
              }
            }
          }
        } else if (prevEncoderValue == 2) {
          smokerState = changeSettings;
          hasRunFlag = 0;
        }
        button.update();
      }

      if (button.triggered(HOLD)) {
        smokerState = changeSettings;  // up one level, not all the way to splashScreen
        hasRunFlag = 0;
      }
    } break;

    case setPitTempTarget: {
      static int16_t lastDrawnSelection = -1;
      currentEncoderValue = encoder.getCount();

      if (hasRunFlag == 0) {
        encoder.newSettings(1, 2, 1, currentEncoderState);  // 1=Edit, 2=Back
        currentEncoderValue = currentEncoderState.currentValue;
        prevEncoderValue = currentEncoderValue;
        hasRunFlag = 1;
        lastDrawnSelection = -1;
      }

      if (prevEncoderValue != lastDrawnSelection) {
        lcd.showSetPitTempTargetMenu(prevEncoderValue, pitTempTarget, degCFlag);
        lastDrawnSelection = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
        if (prevEncoderValue == 1) {
          hasRunFlag = 0;
          bool adjustTempFlag = 1;
          float temporaryTemperatureTarget;
          int16_t lastDrawnAdjustValue = -1;

          while (adjustTempFlag) {
            if (hasRunFlag == 0) {
              if (degCFlag) {
                encoder.newSettings(100, 177, 107, currentEncoderState);
              } else {
                encoder.newSettings(212, 350, 225, currentEncoderState);
              }
              currentEncoderValue = currentEncoderState.currentValue;
              prevEncoderValue = currentEncoderValue;
              hasRunFlag = 1;
              lastDrawnAdjustValue = -1;
            }
            bool meatTargetFlag = 0;
            currentEncoderValue = encoder.getCount();
            temporaryTemperatureTarget = prevEncoderValue;

            yield();

            if ((int16_t)temporaryTemperatureTarget != lastDrawnAdjustValue) {
              lcd.showTemperatureTargetAdjustment(temporaryTemperatureTarget, meatTargetFlag);
              lastDrawnAdjustValue = (int16_t)temporaryTemperatureTarget;
            }

            if (button.update()) {
              if (button.triggered(SINGLE_TAP)) {
                adjustTempFlag = 0;
                pitTempTarget = temporaryTemperatureTarget;
                hasRunFlag = 0;

                Serial.print(F("New pitTempTarget value set -> new pitTempTarget = "));
                Serial.println(pitTempTarget);

                encoder.newSettings(1, 2, 1, currentEncoderState);
                currentEncoderValue = currentEncoderState.currentValue;
                prevEncoderValue = currentEncoderValue;
                hasRunFlag = 1;
                lastDrawnSelection = -1;
              }
            }
          }
        } else if (prevEncoderValue == 2) {
          smokerState = changeSettings;
          hasRunFlag = 0;
        }
        button.update();
      }

      if (button.triggered(HOLD)) {
        smokerState = changeSettings;
        hasRunFlag = 0;
      }
    } break;

    case setTempUnits: {
      static int16_t lastDrawnSelection = -1;
      currentEncoderValue = encoder.getCount();

      if (hasRunFlag == 0) {
        encoder.newSettings(1, 2, 1, currentEncoderState);  // 1=Toggle, 2=Back
        currentEncoderValue = currentEncoderState.currentValue;
        prevEncoderValue = currentEncoderValue;
        hasRunFlag = 1;
        lastDrawnSelection = -1;
      }

      if (prevEncoderValue != lastDrawnSelection) {
        lcd.showSetTempUnitsMenu(prevEncoderValue, degCFlag);
        lastDrawnSelection = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
        if (prevEncoderValue == 1) {
          degCFlag = !degCFlag;
          Serial.println(degCFlag ? F("degC Flag set - Temperature units now in degC!")
                                  : F("degF Flag set - Temperature units now in degF!"));
          lastDrawnSelection = -1;  // force redraw so "Current: degX" updates immediately
        } else if (prevEncoderValue == 2) {
          smokerState = changeSettings;
          hasRunFlag = 0;
        }
      }

      if (button.triggered(HOLD)) {
        smokerState = changeSettings;
        hasRunFlag = 0;
      }
    } break;

    case getTemp: {
      Serial.print("getTemp Case -> startCookTime_ms =");
      Serial.println(startCookTime_ms);

      // TEMPORARY: bias supply reference is a fixed 5.0 V until the INA226 bus voltage replaces it
      double voltageVCC_medianFiltered_V = ADC_biasSupplyInterim_V;
      batteryVoltage_v = voltageVCC_medianFiltered_V;  // placeholder: not a measurement yet

      voltagePit_medianFiltered_V = ads1115.getSensorValue_MedianFiltered_V(ADC_pitPin, 11);
      voltageMeat1_medianFiltered_V = ads1115.getSensorValue_MedianFiltered_V(ADC_meat1Pin, 11);
      voltageMeat2_medianFiltered_V = ads1115.getSensorValue_MedianFiltered_V(ADC_meat2Pin, 11);
      voltageMeat3_medianFiltered_V = ads1115.getSensorValue_MedianFiltered_V(ADC_meat3Pin, 11);

      yield();

      currentPitTemp =
          sh_pitProbe.getTempFahrenheit(voltageVCC_medianFiltered_V, voltagePit_medianFiltered_V);

      currentMeat1Temp = sh_meat1Probe.getTempFahrenheit(voltageVCC_medianFiltered_V,
                                                         voltageMeat1_medianFiltered_V);
      currentMeat2Temp = sh_meat2Probe.getTempFahrenheit(voltageVCC_medianFiltered_V,
                                                         voltageMeat2_medianFiltered_V);
      currentMeat3Temp = sh_meat3Probe.getTempFahrenheit(voltageVCC_medianFiltered_V,
                                                         voltageMeat3_medianFiltered_V);

      // Done decisions for each meat probe
      checkProbeDone("MEAT1", currentMeat1Temp, meat1LastStatus, meat1DoneFlag, meat1SentFlag);
      checkProbeDone("MEAT2", currentMeat2Temp, meat2LastStatus, meat2DoneFlag, meat2SentFlag);
      checkProbeDone("MEAT3", currentMeat3Temp, meat3LastStatus, meat3DoneFlag, meat3SentFlag);

#ifdef DEBUG_ADC_Temps
      Serial.println();
      Serial.println(
          F("REF_V\t|\tPIT_V\t|\tPIT_F\t|\tMEAT1_V\t|\tMEAT1_F\t|\tMEAT2_V\t|\tMEAT2_F\t|\tMEAT3_"
            "V\t|\tMEAT3_F"));
      Serial.print(voltageVCC_medianFiltered_V, 4);
      Serial.print(F("\t|\t"));
      Serial.print(voltagePit_medianFiltered_V, 4);
      Serial.print(F("\t|\t"));
      Serial.print(currentPitTemp);
      Serial.print(F("\t|\t"));
      Serial.print(voltageMeat1_medianFiltered_V, 4);
      Serial.print(F("\t|\t"));
      Serial.print(currentMeat1Temp);
      Serial.print(F("\t|\t"));
      Serial.print(voltageMeat2_medianFiltered_V, 4);
      Serial.print(F("\t|\t"));
      Serial.print(currentMeat2Temp);
      Serial.print(F("\t|\t"));
      Serial.print(voltageMeat3_medianFiltered_V, 4);
      Serial.print(F("\t|\t"));
      Serial.println(currentMeat3Temp);
#endif

      smokerState = txTemp;
    } break;

    case txTemp: {
      Serial.println("entering txTemp case -> transmitting data");
      WiFiClient client;
      ThingSpeak.begin(client);

      float currentMeatTemp_float = currentMeat1Temp;
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
          Serial.println(
              "sleep transmit duration interval reached. smokerState = getTemp, reset hasRunFlag "
              "=1 ");
        }
      }
    } break;

    case bbqStatus: {
      lcd.display();

      // TODO: gating this draw on enteringState only paints the rings/digits once,
      // on the moment we arrive at this state. It won't update again while we sit
      // here, even though currentMeat1Temp/currentPitTemp keep changing underneath it.
      // Fine for now (unblocks compiling + visual check on the bench), but before this
      // screen is trusted for a real cook, it needs a periodic-refresh trigger instead
      // of a one-shot draw - e.g. redraw every N seconds, or only when a temp value
      // has moved by some threshold - rather than either "draw once" or "draw every loop".
      if (enteringState) {
        lcd.showBBQStatusScreen(degCFlag, currentMeat1Temp, meatDoneTemp, currentPitTemp,
                                pitTempTarget);
      }

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