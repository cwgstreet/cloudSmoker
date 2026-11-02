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
#include "calibration.h"
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

constexpr int16_t CAL_MENU_COUNT = 5;  // PIT, MEAT1, MEAT2, MEAT3, BACK
ProbeId calSelectedProbe = PROBE_PIT;  // probe currently being calibrated
static const char* const calNames[] = {"PIT", "MEAT1", "MEAT2", "MEAT3"};  // index = ProbeId
constexpr unsigned long CAL_ICE_TIMEOUT_MS = 120000;  // give up after two minutes
float calIceReading_F = NAN;   // captured ice-point reading (degF), used by the boil step
int16_t calBoilRef_F = 212;    // boiling reference entered on the encoder (degF)
float calBoilReading_F = NAN;  // captured boil-point reading (degF)

// alert send attempts per probe per cook
constexpr uint8_t NTFY_MAX_ATTEMPTS = 10;
uint8_t meat1AlertAttempts = 0;
uint8_t meat2AlertAttempts = 0;
uint8_t meat3AlertAttempts = 0;

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

// Adds one field to the form body, skipping NaN so missing probes aren't uploaded.
void appendThingSpeakField(String& body, int field, float value) {
  if (isnan(value)) {
    return;
  }
  body += "&field" + String(field) + "=" + String(value, 2);
}

// Sends one update to ThingSpeak over HTTPS. Returns true only if ThingSpeak accepts it.
bool sendThingSpeak(float meat1F, float meatTargetF, float pitF, float pitTargetF, float batteryV) {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  String body = String("api_key=") + THNGSPK_WRITE_API_KEY;
  appendThingSpeakField(body, 1, meat1F);
  appendThingSpeakField(body, 2, meatTargetF);
  appendThingSpeakField(body, 3, pitF);
  appendThingSpeakField(body, 4, pitTargetF);
  appendThingSpeakField(body, 5, batteryV);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, "https://api.thingspeak.com/update");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.setTimeout(5000);
  int code = http.POST(body);
  String response = http.getString();
  http.end();

  // ThingSpeak returns the entry number on success, or 0 if it rejected the update
  long entryId = response.toInt();
  Serial.print(F("ThingSpeak HTTP "));
  Serial.print(code);
  Serial.print(F(" entry "));
  Serial.println(entryId);
  return code == 200 && entryId > 0;
}

// Reports done transitions for one probe and sends the alert once per cook.
// lastStatus and doneFlag are references, so the helper updates each probe's own variables.
void checkProbeDone(const char* name, float tempF, DoneStatus& lastStatus, bool& doneFlag,
                    bool& sentFlag, uint8_t& attempts) {
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

  // Alert: one send per cycle while online, up to NTFY_MAX_ATTEMPTS. Offline cycles don't count.
  if (doneFlag && !sentFlag && attempts < NTFY_MAX_ATTEMPTS && WiFi.status() == WL_CONNECTED) {
    attempts++;
    String msg = String(name) + " reached " + String(tempF, 0) + "F (target " +
                 String(meatDoneTemp, 0) + "F)";
    if (sendNtfyAlert(msg)) {
      sentFlag = true;
      Serial.print(name);
      Serial.println(F(" alert sent"));
    } else {
      Serial.print(name);
      Serial.print(F(" alert not sent, attempt "));
      Serial.print(attempts);
      Serial.print(F(" of "));
      Serial.println(NTFY_MAX_ATTEMPTS);
      if (attempts >= NTFY_MAX_ATTEMPTS) {
        Serial.print(name);
        Serial.println(F(" retry limit reached, no further attempts this cook"));
      }
    }
  }
}

// Corrected probe temperature in degF: Steinhart-Hart result, then the per-probe calibration
static float probeF(SteinhartHart& sh, ProbeId id, float vccV, float vadcV) {
  return calApplyF(id, sh.getTempFahrenheit(vccV, vadcV));
}

// Raw Steinhart-Hart reading in degF for one probe, with no calibration applied.
// Prints the measured divider voltage each time, for diagnosis during calibration.
// Returns NAN for an unplugged or open probe, so a capture can be refused.
static float rawProbeF(ProbeId id) {
  float v = NAN;
  SteinhartHart* sh = nullptr;

  switch (id) {
    case PROBE_PIT:
      sh = &sh_pitProbe;
      v = ads1115.getSensorValue_MedianFiltered_V(ADC_pitPin, 11);
      break;
    case PROBE_MEAT1:
      sh = &sh_meat1Probe;
      v = ads1115.getSensorValue_MedianFiltered_V(ADC_meat1Pin, 11);
      break;
    case PROBE_MEAT2:
      sh = &sh_meat2Probe;
      v = ads1115.getSensorValue_MedianFiltered_V(ADC_meat2Pin, 11);
      break;
    case PROBE_MEAT3:
      sh = &sh_meat3Probe;
      v = ads1115.getSensorValue_MedianFiltered_V(ADC_meat3Pin, 11);
      break;
    default:
      return NAN;
  }

  Serial.print(F("raw "));
  Serial.print(calNames[id]);
  Serial.print(F(" V = "));
  Serial.println(v, 4);

  return sh->getTempFahrenheit(calRailV(), v);
}

// Settling test: feed one reading per second. Returns true when the last 10 readings span
// less than 0.5 degF, and sets out to their average. A NaN reading restarts the window.
static bool settleStep(float reading, float* window, uint8_t& count, float& out) {
  if (isnan(reading)) {
    count = 0;
    return false;
  }
  if (count < 10) {
    window[count++] = reading;
  } else {  // slide the window: drop the oldest, add the newest
    for (uint8_t i = 1; i < 10; i++) {
      window[i - 1] = window[i];
    }
    window[9] = reading;
  }
  if (count < 10) {
    return false;
  }

  float lo = window[0];
  float hi = window[0];
  float sum = 0.0f;
  for (uint8_t i = 0; i < 10; i++) {
    if (window[i] < lo) lo = window[i];
    if (window[i] > hi) hi = window[i];
    sum += window[i];
  }
  if (hi - lo < 0.5f) {
    out = sum / 10.0f;
    return true;
  }
  return false;
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

        meat1AlertAttempts = 0;
        meat2AlertAttempts = 0;
        meat3AlertAttempts = 0;

        constexpr uint8_t NTFY_MAX_ATTEMPTS = 10;  // alert send attempts per probe per cook
        uint8_t meat1AlertAttempts = 0;
        uint8_t meat2AlertAttempts = 0;
        uint8_t meat3AlertAttempts = 0;

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
        encoder.newRange(1, MENU_COUNT, 1, true, currentEncoderState);  // menus always wrap
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
          case MENU_CALIBRATION:
            smokerState = calProbeMenu;
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
      double voltageVCC_medianFiltered_V = calRailV();
      batteryVoltage_v = voltageVCC_medianFiltered_V;  // placeholder: not a measurement yet

      voltagePit_medianFiltered_V = ads1115.getSensorValue_MedianFiltered_V(ADC_pitPin, 11);
      voltageMeat1_medianFiltered_V = ads1115.getSensorValue_MedianFiltered_V(ADC_meat1Pin, 11);
      voltageMeat2_medianFiltered_V = ads1115.getSensorValue_MedianFiltered_V(ADC_meat2Pin, 11);
      voltageMeat3_medianFiltered_V = ads1115.getSensorValue_MedianFiltered_V(ADC_meat3Pin, 11);

      yield();

      currentPitTemp =
          probeF(sh_pitProbe, PROBE_PIT, voltageVCC_medianFiltered_V, voltagePit_medianFiltered_V);
      currentMeat1Temp = probeF(sh_meat1Probe, PROBE_MEAT1, voltageVCC_medianFiltered_V,
                                voltageMeat1_medianFiltered_V);
      currentMeat2Temp = probeF(sh_meat2Probe, PROBE_MEAT2, voltageVCC_medianFiltered_V,
                                voltageMeat2_medianFiltered_V);
      currentMeat3Temp = probeF(sh_meat3Probe, PROBE_MEAT3, voltageVCC_medianFiltered_V,
                                voltageMeat3_medianFiltered_V);
      // Done decisions for each meat probe
      checkProbeDone("MEAT1", currentMeat1Temp, meat1LastStatus, meat1DoneFlag, meat1SentFlag,
                     meat1AlertAttempts);
      checkProbeDone("MEAT2", currentMeat2Temp, meat2LastStatus, meat2DoneFlag, meat2SentFlag,
                     meat2AlertAttempts);
      checkProbeDone("MEAT3", currentMeat3Temp, meat3LastStatus, meat3DoneFlag, meat3SentFlag,
                     meat3AlertAttempts);

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
      // TODO: Battery field is NAN until the INA226 bus voltage replaces the placeholder
      sendThingSpeak(currentMeat1Temp, meatDoneTemp, currentPitTemp, pitTempTarget, NAN);

      Serial.println("leaving for Sleep case");
      smokerState = Sleep;
      hasRunFlag = 0;  // Sleep only runs its timer when hasRunFlag is 0
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
    case calProbeMenu: {
      static int16_t lastDrawnSelection = -1;
      currentEncoderValue = encoder.getCount();

      if (hasRunFlag == 0) {
        encoder.newRange(1, CAL_MENU_COUNT, 1, true, currentEncoderState);  // hard stops
        prevEncoderValue = currentEncoderState.currentValue;
        hasRunFlag = 1;
        lastDrawnSelection = -1;
      }

      if (prevEncoderValue != lastDrawnSelection) {
        lcd.showCalProbeMenu(prevEncoderValue);
        lastDrawnSelection = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
        if (prevEncoderValue == CAL_MENU_COUNT) {  // Back
          smokerState = changeSettings;
          hasRunFlag = 0;
        } else {
          calSelectedProbe = (ProbeId)(prevEncoderValue - 1);
          smokerState = calWaitIce;  // not written yet, see note below
          hasRunFlag = 0;
        }
      }

      if (button.triggered(DOUBLE_TAP) && prevEncoderValue != CAL_MENU_COUNT) {
        ProbeId id = (ProbeId)(prevEncoderValue - 1);
        calReset(id);
        Serial.print(F("calibration reset for "));
        Serial.println(calNames[id]);
        lastDrawnSelection = -1;  // redraw the menu
      }

      if (button.triggered(HOLD)) {
        smokerState = changeSettings;
        hasRunFlag = 0;
      }
    } break;

    case calWaitIce: {
      static float iceSamples[10];
      static uint8_t sampleCount = 0;
      static unsigned long captureStart = 0;
      static unsigned long lastSample = 0;
      static float lastReading = NAN;
      static bool drawNeeded = true;
      static bool captured = false;
      static bool timedOut = false;

      if (hasRunFlag == 0) {
        sampleCount = 0;
        captured = false;
        timedOut = false;
        lastReading = NAN;
        captureStart = millis();
        lastSample = millis() - 1000;  // take the first sample straight away
        drawNeeded = true;
        calIceReading_F = NAN;  // no stale ice value from an earlier probe
        hasRunFlag = 1;
      }

      if (button.triggered(HOLD)) {  // cancel at any point
        smokerState = calProbeMenu;
        hasRunFlag = 0;
        break;
      }

      // Sample once per second. Capture when the last 10 readings span under 0.5 degF.
      if (!captured && !timedOut) {
        if ((unsigned long)(millis() - captureStart) >= CAL_ICE_TIMEOUT_MS) {
          timedOut = true;
          drawNeeded = true;
        } else if ((unsigned long)(millis() - lastSample) >= 1000) {
          lastSample = millis();
          lastReading = rawProbeF(calSelectedProbe);
          drawNeeded = true;

          if (isnan(lastReading)) {
            sampleCount = 0;  // unplugged or open: restart the settling window
          } else {
            if (sampleCount < 10) {
              iceSamples[sampleCount++] = lastReading;
            } else {  // slide the window: drop the oldest, add the newest
              for (uint8_t i = 1; i < 10; i++) {
                iceSamples[i - 1] = iceSamples[i];
              }
              iceSamples[9] = lastReading;
            }

            if (sampleCount == 10) {
              float lo = iceSamples[0];
              float hi = iceSamples[0];
              float sum = 0.0f;
              for (uint8_t i = 0; i < 10; i++) {
                if (iceSamples[i] < lo) lo = iceSamples[i];
                if (iceSamples[i] > hi) hi = iceSamples[i];
                sum += iceSamples[i];
              }
              if (hi - lo < 0.5f) {
                calIceReading_F = sum / 10.0f;  // average of the settled window
                captured = true;
                Serial.print(F("ice point captured for "));
                Serial.print(calNames[calSelectedProbe]);
                Serial.print(F(": "));
                Serial.print(calIceReading_F, 2);
                Serial.println(F(" degF"));
              }
            }
          }
        }
      }

      if (drawNeeded) {
        drawNeeded = false;
        if (timedOut) {
          lcd.showCalCapture("ICE POINT", "Timed out", NAN, false, "Press to return");
        } else if (captured) {
          lcd.showCalCapture("ICE POINT", "Captured", calIceReading_F, true, "Press to continue");
        } else if (isnan(lastReading)) {
          lcd.showCalCapture("ICE POINT", "Check probe", NAN, false, "Hold to cancel");
        } else {
          lcd.showCalCapture("ICE POINT", "Settling...", lastReading, true, "Keep ice in bath");
        }
      }

      if (timedOut && button.triggered(SINGLE_TAP)) {
        smokerState = calProbeMenu;
        hasRunFlag = 0;
      }

      if (captured && button.triggered(SINGLE_TAP)) {
        smokerState = calWaitBoil;
        hasRunFlag = 0;
      }

    } break;

    case calWaitBoil: {
      static bool capturing = false;  // false: setting the reference, true: capturing
      static float boilWindow[10];
      static uint8_t boilCount = 0;
      static unsigned long boilStart = 0;
      static unsigned long boilLastSample = 0;
      static float boilLast = NAN;
      static bool boilDone = false;
      static bool boilTimedOut = false;
      static bool drawNeeded = true;
      static int16_t lastDrawnRef = -1;

      if (hasRunFlag == 0) {
        encoder.newRange(185, 215, 212, false, currentEncoderState);  // hard stops
        prevEncoderValue = currentEncoderState.currentValue;
        capturing = false;
        boilCount = 0;
        boilDone = false;
        boilTimedOut = false;
        boilLast = NAN;
        calBoilReading_F = NAN;
        lastDrawnRef = -1;
        drawNeeded = true;
        hasRunFlag = 1;
      }

      if (button.triggered(HOLD)) {  // cancel at any point, nothing saved
        smokerState = calProbeMenu;
        hasRunFlag = 0;
        break;
      }

      if (!capturing) {
        // Setting the reference: the encoder moves the value, the button starts capture
        currentEncoderValue = encoder.getCount();
        if (prevEncoderValue != lastDrawnRef) {
          lcd.showCalCapture("BOIL REFERENCE", "Set boil temp", prevEncoderValue, true,
                             "Press when in boil");
          lastDrawnRef = prevEncoderValue;
        }
        if (button.triggered(SINGLE_TAP)) {
          calBoilRef_F = prevEncoderValue;
          Serial.print(F("boil reference set to "));
          Serial.println(calBoilRef_F);
          capturing = true;
          boilCount = 0;
          boilStart = millis();
          boilLastSample = millis() - 1000;  // first sample straight away
          drawNeeded = true;
        }
      } else {
        // Capturing: same settling test as the ice point
        if (!boilDone && !boilTimedOut) {
          if ((unsigned long)(millis() - boilStart) >= CAL_ICE_TIMEOUT_MS) {
            boilTimedOut = true;
            drawNeeded = true;
          } else if ((unsigned long)(millis() - boilLastSample) >= 1000) {
            boilLastSample = millis();
            boilLast = rawProbeF(calSelectedProbe);
            if (settleStep(boilLast, boilWindow, boilCount, calBoilReading_F)) {
              boilDone = true;
              Serial.print(F("boil point captured for "));
              Serial.print(calNames[calSelectedProbe]);
              Serial.print(F(": "));
              Serial.print(calBoilReading_F, 2);
              Serial.println(F(" degF"));
            }
            drawNeeded = true;
          }
        }

        if (drawNeeded) {
          drawNeeded = false;
          if (boilTimedOut) {
            lcd.showCalCapture("BOIL POINT", "Timed out", NAN, false, "Press to return");
          } else if (boilDone) {
            lcd.showCalCapture("BOIL POINT", "Captured", calBoilReading_F, true,
                               "Press to continue");
          } else if (isnan(boilLast)) {
            lcd.showCalCapture("BOIL POINT", "Check probe", NAN, false, "Hold to cancel");
          } else {
            lcd.showCalCapture("BOIL POINT", "Settling...", boilLast, true, "Keep probe in boil");
          }
        }

        if (boilDone && button.triggered(SINGLE_TAP)) {
          smokerState = calResult;
          hasRunFlag = 0;
        }
        if (boilTimedOut && button.triggered(SINGLE_TAP)) {
          smokerState = calProbeMenu;
          hasRunFlag = 0;
        }
      }
    } break;

    case calResult: {
      static float gainCalc = 1.0f;
      static float offsetCalc = 0.0f;
      static bool valid = false;
      static bool rejected = false;
      static bool drawNeeded = true;

      if (hasRunFlag == 0) {
        float m1 = calIceReading_F;      // ice point reading, degF
        float m2 = calBoilReading_F;     // boil point reading, degF
        float t2 = (float)calBoilRef_F;  // reference boil temperature, degF
        valid = !isnan(m1) && !isnan(m2) && (m2 - m1) >= 100.0f;
        if (valid) {
          // Two-point fit: gain maps M1 to 32 and M2 to T2; offset follows from the ice point
          gainCalc = (t2 - 32.0f) / (m2 - m1);
          offsetCalc = 32.0f - gainCalc * m1;
        }
        rejected = false;
        drawNeeded = true;
        hasRunFlag = 1;
      }

      if (button.triggered(HOLD)) {  // cancel: nothing saved
        smokerState = calProbeMenu;
        hasRunFlag = 0;
        break;
      }

      if (drawNeeded) {
        drawNeeded = false;
        if (!valid) {
          lcd.showCalResult(calNames[calSelectedProbe], 0.0f, 0.0f, false, "Readings too close",
                            "Hold: return");
        } else if (rejected) {
          lcd.showCalResult(calNames[calSelectedProbe], gainCalc, offsetCalc, true,
                            "Outside limits", "Hold: return");
        } else {
          lcd.showCalResult(calNames[calSelectedProbe], gainCalc, offsetCalc, true, "",
                            "Press: save  Hold: cancel");
        }
      }

      if (valid && !rejected && button.triggered(SINGLE_TAP)) {
        if (calSet(calSelectedProbe, gainCalc, offsetCalc)) {
          Serial.print(F("calibration saved for "));
          Serial.println(calNames[calSelectedProbe]);
          smokerState = calProbeMenu;
          hasRunFlag = 0;
        } else {
          rejected = true;
          drawNeeded = true;
        }
      }
    } break;

    default: {
      smokerState = bbqStatus;
    } break;
  }
}