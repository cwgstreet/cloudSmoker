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
#include "bleserial.h"
#include <Arduino.h>
#include <NewEncoder.h>
#include <Wire.h>  // i2C device communication

// include local libraries
#include "calibration.h"
#include "cookstore.h"
#include "devconfig.h"
void WifiManager_initialise(bool openPortalNow);
bool portalIsOpen();
void portalOpen();
bool portalPump();
void portalClose();
#include "cwg_ads1x15.h"        // ADS1x15 I2C ADC device functionality
#include "cwg_ina226.h"         // INA226 rail voltage and coulomb counting
#include "cwg_steinhartHart.h"  // Thermistor steinhart hart temperature calcs
#include "helper_functions.h"
#include "lcd.h"
#include "press_type.h"
#include "smokerStates.h"
#include "wrapEncoder.h"

// always include thingspeak header file after other header files and custom macros
#include <ThingSpeak.h>

// include WiFi and HTTPClient for ntfy.sh notifications
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// include APIs for ntfy.sh notifications

// Debug timing: prints how long a state's block took when the case block exits.
// With DEBUG_TIMING undefined, TIME_BLOCK() expands to nothing.
#ifdef DEBUG_TIMING
struct ScopeTimer {
  const char* name;
  unsigned long startUs;
  ScopeTimer(const char* n) : name(n), startUs(micros()) {}
  ~ScopeTimer() { Log.printf("[TIMING] %s took %lu us\n", name, micros() - startUs); }
};
#define TIME_BLOCK(name) ScopeTimer timeBlock_(name)
#else
#define TIME_BLOCK(name)
#endif

// entryStates is an enum variable type defined in menu.h header file (as extern); smokerState is
// global
entryStates_t smokerState;

// External globals from main.cpp mapped with exact matching types
extern float meatTarget[3];

// Power bank: a rough energy factor from a 3.7 V cell to the 5 V output
static const float BANK_USABLE_FACTOR = 0.74f;
float bankCapacity_mAh = 10000.0f;  // nominal capacity from the box, kept across cooks
uint8_t bankStartPct = 100;         // charge when plugged in, reset to 100 for each new cook
static uint8_t bankEditIndex = 0;   // 0 = capacity, 1 = charge
bool bleSerialEnabled = true;  // BLE serial on or off, saved to NVS
static bool bankEditFromCook = false;  // true while the bank editor is run from the cook menu
static float bankBaseMah = 0.0f;  // usable charge at the start of this cook, mAh at 5 V
static uint32_t readingsSerial = 0;  // bumps after each getTemp, so the cook screen redraws on new readings
static bool bankAlertSent = false;     // forecast alert sent this cook (saved as flag bit 6)
static uint8_t bankAlertAttempts = 0;  // retry cap, as for the PIT alert
extern float pitMax;
extern float pitMin;
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

// Packs the done and sent flags for all three probes into one byte.
// bit 0 MEAT1 done, 1 MEAT1 sent, 2 MEAT2 done, 3 MEAT2 sent, 4 MEAT3 done, 5 MEAT3 sent
static uint8_t packCookFlags() {
  return (meat1DoneFlag ? 1 : 0) | (meat1SentFlag ? 2 : 0) | (meat2DoneFlag ? 4 : 0) |
         (meat2SentFlag ? 8 : 0) | (meat3DoneFlag ? 16 : 0) | (meat3SentFlag ? 32 : 0) | (bankAlertSent ? 64 : 0);
}

// Picks the meat probe for the cook screen: the first of MEAT1, MEAT2, MEAT3 with data.
static void pickCookMeat(float& temp, const char*& label, float& target) {
  for (uint8_t i = 0; i < 3; i++) {
    float t = (i == 0) ? currentMeat1Temp : (i == 1) ? currentMeat2Temp : currentMeat3Temp;
    if (!isnan(t)) {
      temp = t;
      label = (i == 0) ? "MEAT1" : (i == 1) ? "MEAT2" : "MEAT3";
      target = meatTarget[i];
      return;
    }
  }
  temp = NAN;
  label = "MEAT";
  target = meatTarget[0];
}

// Set on resume so the cook screen appears once the first readings are in
static bool showCookOnNextSleep = false;
static bool hintActive = false;       // cook hint screen is up
static unsigned long hintEndMs = 0;   // when the hint screen comes down

// Set at boot when the resume prompt is shown; it tells the prompt whether time is unverified
static bool resumeUnverified = false;



// Called from setup() after the time sync. Returns true if an interrupted cook needs the
// resume prompt. A stale cook (12 h or more) is closed here and is never prompted.

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
  if (devNtfyTopic().length() == 0) {
    return false;  // phone alerts off
  }
  WiFiClientSecure client;
  client.setInsecure();  // HTTPS encryption without certificate check (accepted risk)
  HTTPClient http;
  String url = String("https://ntfy.sh/") + devNtfyTopic();
  http.begin(client, url);
  http.setTimeout(5000);
  int code = http.POST(message);
  http.end();
  Log.print(F("ntfy HTTP code: "));
  Log.println(code);
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
  if (devThingSpeakKey().length() == 0) {
    return false;  // cloud logging off
  }
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  String body = String("api_key=") + devThingSpeakKey();
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
  Log.print(F("ThingSpeak HTTP "));
  Log.print(code);
  Log.print(F(" entry "));
  Log.println(entryId);
  return code == 200 && entryId > 0;
}

// Reports done transitions for one probe and sends the alert once per cook.
// lastStatus and doneFlag are references, so the helper updates each probe's own variables.
void checkProbeDone(const char* name, float tempF, DoneStatus& lastStatus, bool& doneFlag,
                    bool& sentFlag, uint8_t& attempts, float targetF) {
  DoneStatus status = probeDoneStatus(tempF, targetF);

  if (status != lastStatus) {
    Log.print(name);
    Log.print(F(" done status -> "));
    if (status == DONE) {
      Log.println(F("DONE"));
    } else if (status == NOT_DONE) {
      Log.println(F("NOT DONE"));
    } else {
      Log.println(F("NO DATA - Check Probe"));
    }
    lastStatus = status;
  }

  if (status == DONE && !doneFlag) {
    doneFlag = true;
    Log.print(name);
    Log.println(F(" DONE flag set: alert due (first time this cook)"));
  }

  // Alert: one send per cycle while online, up to NTFY_MAX_ATTEMPTS. Offline cycles don't count.
  if (doneFlag && !sentFlag && attempts < NTFY_MAX_ATTEMPTS && WiFi.status() == WL_CONNECTED) {
    attempts++;
    String msg =
        String(name) + " reached " + String(tempF, 0) + "F (target " + String(targetF, 0) + "F)";
    if (sendNtfyAlert(msg)) {
      sentFlag = true;
      Log.print(name);
      Log.println(F(" alert sent"));
    } else {
      Log.print(name);
      Log.print(F(" alert not sent, attempt "));
      Log.print(attempts);
      Log.print(F(" of "));
      Log.println(NTFY_MAX_ATTEMPTS);
      if (attempts >= NTFY_MAX_ATTEMPTS) {
        Log.print(name);
        Log.println(F(" retry limit reached, no further attempts this cook"));
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

// Stored PIT limits for the sub-menu editor: 0 = Min, 1 = Max
static uint8_t editPitIndex = 0;

// PIT band alert: one ntfy per excursion, sent only after PIT stays outside its band for 5 min
static const unsigned long PIT_EXCURSION_MS = 5UL * 60UL * 1000UL;
static unsigned long pitOutSinceMs = 0;  // 0 = PIT inside its band
static bool pitAlertSent = false;
static uint8_t pitAlertAttempts = 0;

// Defaults for the next cook. Called when a cook ends, so Min Off and other per-cook changes
// don't carry over. Units are a device setting and are kept.
static void resetCookSettingsToDefaults() {
  for (uint8_t i = 0; i < 3; i++) {
    meatTarget[i] = 203.0f;
  }
  pitMin = 225.0f;
  pitMax = 250.0f;
  bankStartPct = 100;
  bankChargeSave(100);
  Log.println(F("settings reset to defaults for next cook"));
}



// Called once per getTemp pass. A missing PIT reading never counts as out of band.
// A Min of NAN means Min is Off: only the Max limit applies.
static void checkPitBand(float pitF, float minF, float maxF) {
  bool belowMin = !isnan(minF) && pitF < minF;
  bool aboveMax = pitF > maxF;
  bool outside = !isnan(pitF) && (belowMin || aboveMax);

  if (!outside) {
    if (pitOutSinceMs != 0) {
      Log.println(F("PIT back inside band, excursion timer cleared"));
    }
    pitOutSinceMs = 0;
    pitAlertSent = false;
    pitAlertAttempts = 0;
    return;
  }

  if (pitOutSinceMs == 0) {
    pitOutSinceMs = millis();
    Log.println(F("PIT outside band, 5 min timer started"));
  }
  if (pitAlertSent || pitAlertAttempts >= NTFY_MAX_ATTEMPTS) {
    return;
  }
  if ((unsigned long)(millis() - pitOutSinceMs) < PIT_EXCURSION_MS) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    return;  // offline cycles don't count as attempts
  }

  pitAlertAttempts++;
  String band = isnan(minF) ? String("below ") + String(maxF, 0) + "F"
                            : String(minF, 0) + "-" + String(maxF, 0) + "F";
  String msg = String("PIT out of range: ") + String(pitF, 0) + "F (band " + band +
               " for 5 min)";
  if (sendNtfyAlert(msg)) {
    pitAlertSent = true;
    Log.println(F("PIT alert sent"));
  } else {
    Log.print(F("PIT alert not sent, attempt "));
    Log.print(pitAlertAttempts);
    Log.print(F(" of "));
    Log.println(NTFY_MAX_ATTEMPTS);
  }
}

// Set on resume: the splash shows restored targets until this time, then the cook starts
static unsigned long resumeSplashEndMs = 0;  // 0 = no timed splash pending

// Power bank forecast. The draw is averaged over the last 30 minutes, from a RAM history of
// (time, charge) samples taken once per reading cycle.
static const uint8_t RATE_SAMPLES = 40;
static unsigned long rateMs[RATE_SAMPLES];
static float rateMah[RATE_SAMPLES];
static uint8_t rateHead = 0;
static uint8_t rateCount = 0;

// Clears the history and the alert state. Called at Start Cook, and on resume before the
// saved alert flag is restored.
static void resetBankForecast() {
  rateHead = 0;
  rateCount = 0;
  bankAlertSent = false;
  bankAlertAttempts = 0;
}

static void recordChargeSample() {
  rateMs[rateHead] = millis();
  rateMah[rateHead] = ina226.getChargeUsed_mAh();
  rateHead = (rateHead + 1) % RATE_SAMPLES;
  if (rateCount < RATE_SAMPLES) {
    rateCount++;
  }
}

// Average draw in mA over up to the last 30 minutes. Returns 0 until 5 minutes of data exist.
static float averageDrawMa() {
  if (rateCount < 2) {
    return 0.0f;
  }
  uint8_t newest = (rateHead + RATE_SAMPLES - 1) % RATE_SAMPLES;
  unsigned long nowMs = rateMs[newest];
  uint8_t oldest = newest;
  for (uint8_t k = 1; k < rateCount; k++) {
    uint8_t idx = (newest + RATE_SAMPLES - k) % RATE_SAMPLES;
    if ((unsigned long)(nowMs - rateMs[idx]) > 30UL * 60UL * 1000UL) {
      break;
    }
    oldest = idx;
  }
  unsigned long spanMs = nowMs - rateMs[oldest];
  if (spanMs < 5UL * 60UL * 1000UL) {
    return 0.0f;
  }
  float mah = rateMah[newest] - rateMah[oldest];
  return mah / ((float)spanMs / 3600000.0f);  // mAh per hour is mA
}

// Warns once per cook if the bank looks short for the 14 h planning target. Waits 10 minutes
// into the cook so the average settles. Offline cycles don't count as attempts.
static void checkBankForecast() {
  const float PLANNING_HOURS = 14.0f;
  if (bankAlertSent || bankAlertAttempts >= NTFY_MAX_ATTEMPTS || !cookStoreActive() ||
      bankBaseMah <= 0.0f) {
    return;
  }
  if ((unsigned long)(millis() - startCookTime_ms) < 10UL * 60UL * 1000UL) {
    return;
  }
  float draw = averageDrawMa();
  if (draw < 5.0f) {
    return;
  }
  float remainingMah = bankBaseMah - ina226.getChargeUsed_mAh();
  float hoursLeft = remainingMah / draw;
  if (hoursLeft >= PLANNING_HOURS) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  bankAlertAttempts++;
  String msg = "Power bank may not last a 14 h cook: estimated " + String(hoursLeft, 0) +
               " h left at " + String(draw, 0) + " mA";
  if (sendNtfyAlert(msg)) {
    bankAlertSent = true;
    cookSaveFlags(packCookFlags());  // saves the sent flag so a resume doesn't repeat it
    Log.println(F("bank forecast alert sent"));
  } else {
    Log.print(F("bank forecast alert not sent, attempt "));
    Log.print(bankAlertAttempts);
    Log.print(F(" of "));
    Log.println(NTFY_MAX_ATTEMPTS);
  }
}

// Cool-down alerts for done meat probes. A probe is watched only after it has been above 150F
// since its done alert, so a rare beef pull that never gets that hot gets no cool-down alerts.
static const float COOL_WARN_F = 150.0f;    // approaching danger zone
static const float COOL_DANGER_F = 140.0f;  // danger zone: the timer starts
static const float COOL_CLEAR_F = 141.0f;   // back above this after danger = reheated
#ifdef DEBUG_COOLDOWN_SHORT
static const uint32_t DANGER_HOLD_MIN = 2;  // bench test only: 2 minutes instead of 2 hours
#else
static const uint32_t DANGER_HOLD_MIN = 120;
#endif

// Per-probe state. dangerAt: 0 = not in danger, 1 = started while time not synced, else epoch.
// coolBits layout, for probe i = 0..2:
//   bit i       armed (has been above 150F since done)
//   bit 3+i     150F warning sent
//   bit 6+i     140F danger alert sent
//   bit 9+i     2-hour discard alert sent
static uint32_t dangerAt[3] = {0, 0, 0};
static uint16_t coolBits = 0;

static void resetCooldown() {
  for (uint8_t i = 0; i < 3; i++) {
    dangerAt[i] = 0;
  }
  coolBits = 0;
  cookSaveCooldown(dangerAt, coolBits);
}

static void formatTemp(float tF, char* out, size_t len) {
  if (degCFlag) {
    snprintf(out, len, "%.0fC", convertDegFtoDegC(tF));
  } else {
    snprintf(out, len, "%.0fF", tF);
  }
}

static void checkCooldown() {
  static const char* const names[3] = {"MEAT1", "MEAT2", "MEAT3"};
  const float temps[3] = {currentMeat1Temp, currentMeat2Temp, currentMeat3Temp};
  const bool done[3] = {meat1DoneFlag, meat2DoneFlag, meat3DoneFlag};
  bool online = (WiFi.status() == WL_CONNECTED);
  bool changed = false;
  uint32_t nowS = clockValid() ? (uint32_t)time(nullptr) : 0;

  for (uint8_t i = 0; i < 3; i++) {
    if (!done[i] || isnan(temps[i])) {
      continue;
    }
    float t = temps[i];
    uint16_t armedBit = (uint16_t)(1u << i);
    uint16_t warnBit = (uint16_t)(1u << (3 + i));
    uint16_t dangerBit = (uint16_t)(1u << (6 + i));
    uint16_t discardBit = (uint16_t)(1u << (9 + i));
    char tStr[12], dStr[12];
    formatTemp(t, tStr, sizeof(tStr));
    formatTemp(COOL_DANGER_F, dStr, sizeof(dStr));

    // Above 150F: arm the probe and re-arm the 150F warning for the next cool-down
    if (t >= COOL_WARN_F) {
      if (!(coolBits & armedBit)) {
        coolBits |= armedBit;
        changed = true;
      }
      if (coolBits & warnBit) {
        coolBits &= (uint16_t)~warnBit;
        changed = true;
      }
      continue;
    }

    if (!(coolBits & armedBit)) {
      continue;  // never cooled from above 150F after done, so no cool-down alerts
    }

    // Reheated: back above 141F after the danger zone clears the timer and its alerts
    if (dangerAt[i] != 0 && t >= COOL_CLEAR_F) {
      dangerAt[i] = 0;
      coolBits &= (uint16_t)~(dangerBit | discardBit);
      changed = true;
      Log.print(F("cooldown: "));
      Log.print(names[i]);
      Log.println(F(" reheated above danger zone, timer cleared"));
    }

    // 150F: approaching danger zone, sent once per cool-down
    if (!(coolBits & warnBit) && online) {
      String msg = String(names[i]) + " at " + tStr +
                   ": approaching danger zone. Act now: wrap, cool, then store or reheat.";
      if (sendNtfyAlert(msg)) {
        coolBits |= warnBit;
        changed = true;
        Log.print(F("cooldown warning sent: "));
        Log.println(msg);
      }
    }

    // Below 140F for the first time: start the timer
    if (t < COOL_DANGER_F && dangerAt[i] == 0) {
      dangerAt[i] = clockValid() ? nowS : 1;
      changed = true;
    }

    if (dangerAt[i] == 0) {
      continue;
    }

    bool timed = (dangerAt[i] > 1 && clockValid());
    uint32_t elapsedMin = timed ? (nowS - dangerAt[i]) / 60 : 0;
    uint32_t leftMin = (timed && elapsedMin < DANGER_HOLD_MIN) ? DANGER_HOLD_MIN - elapsedMin : 0;

    // 140F: danger zone reached, one alert, with elapsed and remaining minutes
    if (!(coolBits & dangerBit) && online) {
      String msg;
      if (timed) {
        msg = String(names[i]) + " fell below " + dStr + " " + String(elapsedMin) +
              " minutes ago. Store or reheat within " + String(leftMin) + " minutes.";
      } else {
        msg = String(names[i]) + " fell below " + dStr +
              ". Time not synced: store or reheat now, discard if in doubt.";
      }
      if (sendNtfyAlert(msg)) {
        coolBits |= dangerBit;
        changed = true;
        Log.print(F("cooldown danger alert sent: "));
        Log.println(msg);
      }
    }

    // Two hours below 140F: discard, sent once, only while still in the danger zone
    if (timed && elapsedMin >= DANGER_HOLD_MIN && t < COOL_DANGER_F && !(coolBits & discardBit) &&
        online) {
      String msg = String(names[i]) + " has been below " + dStr +
                   " for over 2 hours. USDA recommends discarding it due to food safety concerns.";
      if (sendNtfyAlert(msg)) {
        coolBits |= discardBit;
        changed = true;
        Log.print(F("cooldown discard alert sent: "));
        Log.println(msg);
      }
    }
  }

  if (changed) {
    cookSaveCooldown(dangerAt, coolBits);
  }
}

// Probe lost mid-cook: one alert after two readings with no data from a probe that had data
// during this cook. Re-arms when the probe reads again. Index: 0 PIT, 1 MEAT1, 2 MEAT2, 3 MEAT3.
static bool probeSeen[4] = {false, false, false, false};
static uint8_t probeLostCount[4] = {0, 0, 0, 0};
static bool probeLostAlerted[4] = {false, false, false, false};

static void resetProbeLoss() {
  for (uint8_t i = 0; i < 4; i++) {
    probeSeen[i] = false;
    probeLostCount[i] = 0;
    probeLostAlerted[i] = false;
  }
}

static void checkProbeLoss() {
  static const char* const probeNames[4] = {"PIT", "MEAT1", "MEAT2", "MEAT3"};
  const float temps[4] = {currentPitTemp, currentMeat1Temp, currentMeat2Temp, currentMeat3Temp};
  bool online = (WiFi.status() == WL_CONNECTED);

  for (uint8_t i = 0; i < 4; i++) {
    if (!isnan(temps[i])) {
      probeSeen[i] = true;
      probeLostCount[i] = 0;
      probeLostAlerted[i] = false;
      continue;
    }
    if (!probeSeen[i]) {
      continue;  // never connected during this cook, so nothing has been lost
    }
    if (probeLostCount[i] < 255) {
      probeLostCount[i]++;
    }
    if (probeLostCount[i] >= 2 && !probeLostAlerted[i] && online) {
      String msg = String(probeNames[i]) +
                   " probe lost during cook: no reading for 2 checks. Check the probe and jack.";
      if (sendNtfyAlert(msg)) {
        probeLostAlerted[i] = true;
        Log.print(F("probe lost alert sent: "));
        Log.println(msg);
      }
    }
  }
}

// Sent when the resume prompt appears after a restart with a cook in progress
static void notifyRebootDuringCook() {
  if (WiFi.status() != WL_CONNECTED) {
    Log.println(F("restart during cook: offline, no phone alert sent"));
    return;
  }
  sendNtfyAlert(String("cloudSmoker2 restarted during a cook. Resume prompt is waiting on the device: tap to resume, hold to restart."));
}

// Restores the stored cook into the running state and starts it.
static void applyResume() {
  float meatT[3], pitMinT, pitMaxT, charge;
  bool degC;
  uint8_t flags;
  if (!cookLoad(meatT, pitMinT, pitMaxT, degC, charge, flags)) {
    return;
  }

  for (uint8_t i = 0; i < 3; i++) {
    meatTarget[i] = meatT[i];
  }
  pitMin = pitMinT;
  pitMax = pitMaxT;
  degCFlag = degC;
  ina226.setChargeUsed_mAh(charge);
  bankBaseMah = cookLoadBank();
  cookLoadCooldown(dangerAt, coolBits);

  meat1DoneFlag = (flags & 1) != 0;
  meat1SentFlag = (flags & 2) != 0;
  meat2DoneFlag = (flags & 4) != 0;
  meat2SentFlag = (flags & 8) != 0;
  meat3DoneFlag = (flags & 16) != 0;
  meat3SentFlag = (flags & 32) != 0;
  resetBankForecast();                 // fresh history for the restored charge
  bankAlertSent = (flags & 64) != 0;  // a resumed cook doesn't repeat its alert

  startCookTime_ms = millis();
  showCookOnNextSleep = true;
  resumeSplashEndMs = millis() + 3000;  // timed splash with restored targets
  smokerState = splashScreen;
}

bool cookApplyOnBoot() {
  static const char* const bootNames[] = {"none", "resume prompt",
                                          "resume prompt (time unverified)",
                                          "finished (stale), starting at setup"};
  bankCapacity_mAh = bankCapacityLoad();  // capacity persists across cooks
  bankStartPct = bankChargeLoad();
  CookBoot_t boot = cookBootCheck();
  Log.print(F("cook boot check: "));
  Log.println(bootNames[boot]);

  if (boot == COOK_NONE) {
    return false;
  }

  float meatT[3], pitMinT, pitMaxT, charge;
  bool degC;
  uint8_t flags;
  if (!cookLoad(meatT, pitMinT, pitMaxT, degC, charge, flags)) {
    return false;
  }

  // Targets always load: they are the setup values if the user restarts or the cook is stale
  for (uint8_t i = 0; i < 3; i++) {
    meatTarget[i] = meatT[i];
  }
  pitMin = pitMinT;
  pitMax = pitMaxT;
  degCFlag = degC;

  if (boot == COOK_STALE) {
    cookSaveEnd();
    resetCookSettingsToDefaults();
    return false;
  }

  resumeUnverified = (boot == COOK_RESUME_UNVERIFIED);
  return true;
}

// Meat Temp menu: which probes are present, rebuilt each time the menu opens
static const ProbeId meatProbeIds[3] = {PROBE_MEAT1, PROBE_MEAT2, PROBE_MEAT3};
static const uint8_t MEAT_MENU_MAX = 4;       // up to three probes plus Back
static uint8_t meatMenuProbe[MEAT_MENU_MAX];  // probe index per entry, 255 = Back
static char meatMenuLabel[MEAT_MENU_MAX][8];
static char meatMenuDetail[MEAT_MENU_MAX][16];
static uint8_t meatMenuCount = 0;
static uint8_t editMeatIndex = 0;

// Takes a fresh reading of each meat jack and lists the ones with a probe. Targets are shown in
// the display units, but are always stored in degF.
static void buildMeatMenu() {
  meatMenuCount = 0;
  for (uint8_t i = 0; i < 3; i++) {
    float t = rawProbeF(meatProbeIds[i]);
    if (!isnan(t)) {
      meatMenuProbe[meatMenuCount] = i;
      snprintf(meatMenuLabel[meatMenuCount], sizeof(meatMenuLabel[0]), "MEAT%u", i + 1);
      float shown = degCFlag ? convertDegFtoDegC(meatTarget[i]) : meatTarget[i];
      snprintf(meatMenuDetail[meatMenuCount], sizeof(meatMenuDetail[0]), "%.0f%c", shown,
               degCFlag ? 'C' : 'F');
      meatMenuCount++;
    }
  }
  meatMenuProbe[meatMenuCount] = 255;  // Back
  snprintf(meatMenuLabel[meatMenuCount], sizeof(meatMenuLabel[0]), "Back");
  meatMenuDetail[meatMenuCount][0] = '\0';
  meatMenuCount++;
}


// Fraction of the PIT band: 0 at Min (or 50F when Min is Off), 1 at Max
static float pitFractionFor(float pitF) {
  float low = isnan(pitMin) ? 50.0f : pitMin;
  float span = pitMax - low;
  if (isnan(pitF) || span <= 0.0f) {
    return 0.0f;
  }
  return (pitF - low) / span;
}

// Fraction of a meat target: 0 at ambient, 1 at target
static float meatFractionFor(float meatF, float targetF) {
  if (isnan(meatF) || targetF <= 0.0f) {
    return 0.0f;
  }
  return meatF / targetF;
}




// Remaining power bank charge as a percentage of its full usable capacity (capacity x 0.74).
// A cook started at 50% reads 50% at the start, and 25% after half of that is used.
// Returns -1 if no cook base is known.
static int bankRemainingPct() {
  float fullUsable = bankCapacity_mAh * BANK_USABLE_FACTOR;
  if (fullUsable <= 0.0f || bankBaseMah <= 0.0f) {
    return -1;
  }
  float remaining = bankBaseMah - ina226.getChargeUsed_mAh();
  return (int)constrain(100.0f * remaining / fullUsable, 0.0f, 100.0f);
}

// Draws one cook page: 0 = overview, 1 = PIT over MEAT1, 2 = MEAT2 over MEAT3
static void drawCookPage(CWG_LCD& lcd, uint8_t page) {
  if (page == 3) {
    lcd.showCookHint();
    return;
  }
  if (page == 0) {
    lcd.showCookOverview(degCFlag, currentPitTemp, currentMeat1Temp, currentMeat2Temp,
                         currentMeat3Temp);
    lcd.showBankLine(bankRemainingPct());
  } else if (page == 1) {
    lcd.showZoomPage(degCFlag, "PIT", TFT_ORANGE, currentPitTemp, pitFractionFor(currentPitTemp),
                     "MEAT1", TFT_CYAN, currentMeat1Temp,
                     meatFractionFor(currentMeat1Temp, meatTarget[0]));
  } else {
    lcd.showZoomPage(degCFlag, "MEAT2", TFT_GREEN, currentMeat2Temp,
                     meatFractionFor(currentMeat2Temp, meatTarget[1]), "MEAT3", TFT_MAGENTA,
                     currentMeat3Temp, meatFractionFor(currentMeat3Temp, meatTarget[2]));
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
        lcd.showSplashScreen(degCFlag, meatTarget[0], pitMin, pitMax, bankCapacity_mAh, bankStartPct);
      }

      // After a resume, show the restored targets for a few seconds, then start the cook
      if (resumeSplashEndMs != 0) {
        if ((long)(millis() - resumeSplashEndMs) >= 0) {
          resumeSplashEndMs = 0;
          smokerState = getTemp;
        }
        break;
      }

      if (encoder.moved()) {
        calPrintHelp();  // one-time command summary on first encoder move
        smokerState = launchPad;
      }
    } break;

    case launchPad: {
      if (hasRunFlag == 0) {
        Log.println(
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
        Log.println("\n-------------------Start Cook Now-----------------");

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

        ina226.resetCharge();
        cookSaveStart(meatTarget, pitMin, pitMax, degCFlag, ina226.getChargeUsed_mAh());
        cookSaveBank(bankCapacity_mAh * BANK_USABLE_FACTOR * bankStartPct / 100.0f);
        showCookOnNextSleep = true;  // first sleep after the cook starts shows the hint
        bankBaseMah = bankCapacity_mAh * BANK_USABLE_FACTOR * bankStartPct / 100.0f;
        resetBankForecast();
        resetProbeLoss();
        resetCooldown();

        pitOutSinceMs = 0; pitAlertSent = false; pitAlertAttempts = 0;
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
            smokerState = meatTempMenu;
            hasRunFlag = 0;
            break;
          case MENU_PIT_TEMP:
            smokerState = pitTempMenu;
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
          case MENU_RAIL:
            smokerState = railStatus;
            hasRunFlag = 0;
            break;
          case MENU_BANK:
            smokerState = bankMenu;
            hasRunFlag = 0;
            break;
          case MENU_BLE:
            smokerState = bleMenu;
            hasRunFlag = 0;
            break;
          case MENU_SETUP:
            smokerState = setupPortal;
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
          Log.println(degCFlag ? F("degC Flag set - Temperature units now in degC!")
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
      TIME_BLOCK("getTemp");
      // TIME_BLOCK only active when DEBUG_TIMING is defined, otherwise expands to nothing
      Log.print("getTemp Case -> startCookTime_ms =");
      Log.println(startCookTime_ms);

      // TEMPORARY: bias supply reference is a fixed 5.0 V until the INA226 bus voltage replaces
      // it
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
                     meat1AlertAttempts, meatTarget[0]);
      checkProbeDone("MEAT2", currentMeat2Temp, meat2LastStatus, meat2DoneFlag, meat2SentFlag,
                     meat2AlertAttempts, meatTarget[1]);
      checkProbeDone("MEAT3", currentMeat3Temp, meat3LastStatus, meat3DoneFlag, meat3SentFlag,
                     meat3AlertAttempts, meatTarget[2]);
      cookSaveFlags(packCookFlags());
      checkPitBand(currentPitTemp, pitMin, pitMax);
      checkCooldown();
      checkProbeLoss();
      readingsSerial++;
      recordChargeSample();
      checkBankForecast();
#ifdef DEBUG_BANK_DIAG
      Log.print(F("bank diag: draw_mA=")); Log.print(averageDrawMa(), 1); Log.print(F(" rem_mAh=")); Log.print(bankBaseMah - ina226.getChargeUsed_mAh(), 1); Log.print(F(" samples=")); Log.print(rateCount); Log.print(F(" sent=")); Log.print(bankAlertSent); Log.print(F(" wifi=")); Log.println(WiFi.status() == WL_CONNECTED);
#endif

#ifdef DEBUG_ADC_Temps
      Log.println();
      Log.println(
          F("REF_V\t|\tPIT_V\t|\tPIT_F\t|\tMEAT1_V\t|\tMEAT1_F\t|\tMEAT2_V\t|\tMEAT2_F\t|\tMEAT3_"
            "V\t|\tMEAT3_F"));
      Log.print(voltageVCC_medianFiltered_V, 4);
      Log.print(F("\t|\t"));
      Log.print(voltagePit_medianFiltered_V, 4);
      Log.print(F("\t|\t"));
      Log.print(currentPitTemp);
      Log.print(F("\t|\t"));
      Log.print(voltageMeat1_medianFiltered_V, 4);
      Log.print(F("\t|\t"));
      Log.print(currentMeat1Temp);
      Log.print(F("\t|\t"));
      Log.print(voltageMeat2_medianFiltered_V, 4);
      Log.print(F("\t|\t"));
      Log.print(currentMeat2Temp);
      Log.print(F("\t|\t"));
      Log.print(voltageMeat3_medianFiltered_V, 4);
      Log.print(F("\t|\t"));
      Log.println(currentMeat3Temp);
#endif

      smokerState = txTemp;
    } break;

    case txTemp: {
      TIME_BLOCK("txTemp");
      Log.println("entering txTemp case -> transmitting data");
      // TODO: Battery field is NAN until the INA226 bus voltage replaces the placeholder
      sendThingSpeak(currentMeat1Temp, meatTarget[0], currentPitTemp, pitMax, NAN);

      Log.println("leaving for Sleep case");
      smokerState = Sleep;
      hasRunFlag = 0;  // Sleep only runs its timer when hasRunFlag is 0
    } break;

    case resumePrompt: {
      if (enteringState) {
        lcd.showResumePrompt(resumeUnverified);
        notifyRebootDuringCook();
      }

      if (button.triggered(SINGLE_TAP)) {
        applyResume();  // carries on with the stored cook; sets smokerState to getTemp
        hasRunFlag = 0;
      } else if (button.triggered(HOLD)) {
        cookSaveEnd();
        resetCookSettingsToDefaults();  // a restarted cook always starts from defaults
        Log.println(F("cook restarted by user -> setup"));
        smokerState = splashScreen;
        hasRunFlag = 0;
      }
    } break;

    case Sleep: {
      // Hint screen: shown for 3 seconds after a cook starts or resumes, then the cook screen
      if (hintActive) {
        if ((long)(millis() - hintEndMs) >= 0) {
          hintActive = false;
          smokerState = bbqStatus;
          hasRunFlag = 1;
        }
        break;
      }

      if (showCookOnNextSleep) {
        showCookOnNextSleep = false;
        hintActive = true;
        hintEndMs = millis() + 3000;
        lcd.display();
        lcd.showCookHint();
        Log.println("cook running: hold for cook options");
        break;
      }

      // Encoder move or any button press wakes the cook screen
      if (encoder.moved() || button.triggered(SINGLE_TAP) || button.triggered(DOUBLE_TAP) ||
          button.triggered(HOLD)) {
        lcd.display();
        Log.println("Sleep state, input -> waking up");
        smokerState = bbqStatus;
        hasRunFlag = 1;
        Log.println("leaving for bbqStatus state");
      }

      if (hasRunFlag == 0) {
        lcd.noDisplay();

        unsigned long currentMillis = millis();

        if ((unsigned long)(currentMillis - previousMillis) >= transmitInterval) {
          previousMillis = millis();
          smokerState = getTemp;
          hasRunFlag = 1;
          Log.println(
              "sleep transmit duration interval reached. smokerState = getTemp, reset hasRunFlag "
              "=1 ");
        }
      }
    } break;

    case bbqStatus: {
      static int16_t lastPageValue = -1;
      static uint32_t lastSerial = 0;

      lcd.display();

      if (enteringState) {
        encoder.newRange(1, 4, 1, true, currentEncoderState);  // four pages, wraps
        prevEncoderValue = currentEncoderState.currentValue;
        lastPageValue = -1;
        lastSerial = readingsSerial;
        previousDisplayMillis = millis();  // start the display timer on entry, not on last timeout
      }

      currentEncoderValue = encoder.getCount();
      uint8_t page = (uint8_t)((prevEncoderValue - 1) % 4);  // 0 overview, 1 PIT/MEAT1, 2 MEAT2/MEAT3
      bool pageChanged = (prevEncoderValue != lastPageValue);
      bool refreshDue = (readingsSerial != lastSerial);  // redraw only when readings change
      if (refreshDue) {
        lastSerial = readingsSerial;
      }
      if (pageChanged || (refreshDue && page != 3)) {
        lastPageValue = prevEncoderValue;
        drawCookPage(lcd, page);
      }
      if (pageChanged && !enteringState) {
        previousDisplayMillis = millis();  // turning a page keeps the screen up
      }

      // Hold opens the cook menu
      if (button.triggered(HOLD)) {
        smokerState = cookMenu;
        hasRunFlag = 0;
        break;
      }

      if ((unsigned long)(millis() - previousDisplayMillis) >= displayInterval) {
        previousDisplayMillis = millis();
        smokerState = Sleep;
        hasRunFlag = 0;
      }
    } break;

    case cookMenu: {
      static int16_t lastDrawnSelection = -1;
      static const char* const menuLabels[3] = {"End cook", "New bank", "Cancel"};
      static const char* const menuDetails[3] = {"back to setup", "reset charge", "keep cooking"};
      currentEncoderValue = encoder.getCount();

      if (hasRunFlag == 0) {
        encoder.newRange(1, 3, 1, true, currentEncoderState);  // menus wrap
        prevEncoderValue = currentEncoderState.currentValue;
        hasRunFlag = 1;
        lastDrawnSelection = -1;
      }

      if (prevEncoderValue != lastDrawnSelection) {
        lcd.showListMenu("COOK", menuLabels, menuDetails, 3, (uint8_t)prevEncoderValue);
        lastDrawnSelection = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
        if (prevEncoderValue == 1) {  // End cook: clears the stored cook, back to setup
          cookSaveEnd();
          resetCookSettingsToDefaults();
          Log.println(F("cook ended by user -> setup"));
          smokerState = splashScreen;
          hasRunFlag = 0;
        } else if (prevEncoderValue == 2) {  // New bank: capacity, then charge, then reset
          bankEditFromCook = true;
          bankEditIndex = 0;
          smokerState = editBank;
          hasRunFlag = 0;
        } else {  // Cancel
          smokerState = bbqStatus;
          hasRunFlag = 0;
        }
      }

      if (button.triggered(HOLD)) {
        smokerState = bbqStatus;
        hasRunFlag = 0;
      }
    } break;

    case bleMenu: {
      static int16_t lastDrawn = -1;
      currentEncoderValue = encoder.getCount();

      if (hasRunFlag == 0) {
        encoder.newRange(1, 2, bleSerialEnabled ? 2 : 1, true, currentEncoderState);  // wraps
        prevEncoderValue = currentEncoderState.currentValue;
        hasRunFlag = 1;
        lastDrawn = -1;
      }

      if (prevEncoderValue != lastDrawn) {
        lcd.showBleToggle((uint8_t)prevEncoderValue, devBlePasskey());
        lastDrawn = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
        bleSerialEnabled = (prevEncoderValue == 2);
        bleSerialSave(bleSerialEnabled);
        bleSerialApply(bleSerialEnabled);
        Log.print(F("BLE serial set -> "));
        Log.println(bleSerialEnabled ? F("on")
                                        : F("off"));
        smokerState = changeSettings;
        hasRunFlag = 0;
      }

      if (button.triggered(HOLD)) {
        smokerState = changeSettings;
        hasRunFlag = 0;
      }
    } break;

    case meatTempMenu: {
      static int16_t lastDrawnSelection = -1;
      currentEncoderValue = encoder.getCount();

      if (hasRunFlag == 0) {
        buildMeatMenu();
        encoder.newRange(1, (int16_t)meatMenuCount, 1, true, currentEncoderState);  // menus wrap
        prevEncoderValue = currentEncoderState.currentValue;
        hasRunFlag = 1;
        lastDrawnSelection = -1;
      }

      if (prevEncoderValue != lastDrawnSelection) {
        const char* labels[MEAT_MENU_MAX];
        const char* details[MEAT_MENU_MAX];
        for (uint8_t i = 0; i < meatMenuCount; i++) {
          labels[i] = meatMenuLabel[i];
          details[i] = meatMenuDetail[i];
        }
        lcd.showListMenu("MEAT TEMP", labels, details, meatMenuCount, (uint8_t)prevEncoderValue);
        lastDrawnSelection = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
        uint8_t entry = (uint8_t)prevEncoderValue - 1;
        if (meatMenuProbe[entry] == 255) {  // Back
          smokerState = changeSettings;
          hasRunFlag = 0;
        } else {
          editMeatIndex = meatMenuProbe[entry];
          smokerState = editMeatTarget;
          hasRunFlag = 0;
        }
      }

      if (button.triggered(HOLD)) {
        smokerState = changeSettings;
        hasRunFlag = 0;
      }
    } break;

    case editMeatTarget: {
      static int16_t lastDrawnValue = -1000;
      currentEncoderValue = encoder.getCount();

      const char* probeName = (editMeatIndex == 0) ? "MEAT1" : (editMeatIndex == 1) ? "MEAT2" : "MEAT3";

      if (hasRunFlag == 0) {
        // The encoder counts in the display units. Limits: 46 to 104 C, or 115 to 220 F.
        float currentF = meatTarget[editMeatIndex];
        int16_t lo = degCFlag ? 46 : 115;
        int16_t hi = degCFlag ? 104 : 220;
        float currentDisplay = degCFlag ? convertDegFtoDegC(currentF) : currentF;
        int16_t start = (int16_t)(currentDisplay + 0.5f);
        encoder.newRange(lo, hi, constrain(start, lo, hi), true, currentEncoderState);
        prevEncoderValue = currentEncoderState.currentValue;
        hasRunFlag = 1;
        lastDrawnValue = -1000;
      }

      // Work out the value in degF once, for the safety check and for storage
      float enteredF = degCFlag ? ((float)prevEncoderValue * 9.0f / 5.0f + 32.0f)
                                : (float)prevEncoderValue;
      bool belowSafe = enteredF < 145.0f;  // USDA minimum for whole cuts of beef, pork, lamb

      if (prevEncoderValue != lastDrawnValue) {
        lcd.showMeatTargetEdit(probeName, prevEncoderValue, degCFlag, belowSafe);
        lastDrawnValue = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
        meatTarget[editMeatIndex] = enteredF;  // stored as degF
        Log.print(F("meat target set: "));
        Log.print(probeName);
        Log.print(F(" -> "));
        Log.print(prevEncoderValue);
        Log.print(degCFlag ? F(" C (stored as ") : F(" F (stored as "));
        Log.print(enteredF, 1);
        Log.println(F(" F)"));
        if (belowSafe) {
          Log.print(F("WARNING: "));
          Log.print(probeName);
          Log.println(F(" target below USDA 145F minimum for whole cuts"));
        }
        smokerState = meatTempMenu;
        hasRunFlag = 0;
      }

      if (button.triggered(HOLD)) {
        smokerState = meatTempMenu;
        hasRunFlag = 0;
      }
    } break;

    case pitTempMenu: {
      static int16_t lastDrawnSelection = -1;
      static char pitDetail[2][16];
      static const char* const pitLabels[3] = {"Min", "Max", "Back"};
      static const char* pitDetails[3] = {pitDetail[0], pitDetail[1], ""};
      currentEncoderValue = encoder.getCount();

      if (hasRunFlag == 0) {
        encoder.newRange(1, 3, 1, true, currentEncoderState);  // menus wrap
        prevEncoderValue = currentEncoderState.currentValue;
        hasRunFlag = 1;
        lastDrawnSelection = -1;
      }

      if (prevEncoderValue != lastDrawnSelection) {
        char unit = degCFlag ? 'C' : 'F';
        if (isnan(pitMin)) {
          snprintf(pitDetail[0], sizeof(pitDetail[0]), "Off");
        } else {
          float minShown = degCFlag ? convertDegFtoDegC(pitMin) : pitMin;
          snprintf(pitDetail[0], sizeof(pitDetail[0]), "%.0f%c", minShown, unit);
        }
        float maxShown = degCFlag ? convertDegFtoDegC(pitMax) : pitMax;
        snprintf(pitDetail[1], sizeof(pitDetail[1]), "%.0f%c", maxShown, unit);
        lcd.showListMenu("PIT TEMP", pitLabels, pitDetails, 3, (uint8_t)prevEncoderValue);
        lastDrawnSelection = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
        if (prevEncoderValue == 3) {  // Back
          smokerState = changeSettings;
          hasRunFlag = 0;
        } else {
          editPitIndex = (uint8_t)(prevEncoderValue - 1);  // 0 = Min, 1 = Max
          smokerState = editPitLimit;
          hasRunFlag = 0;
        }
      }

      if (button.triggered(HOLD)) {
        smokerState = changeSettings;
        hasRunFlag = 0;
      }
    } break;

    case editPitLimit: {
      static int16_t lastDrawnValue = -1000;
      currentEncoderValue = encoder.getCount();
      const char* label = (editPitIndex == 0) ? "PIT MIN" : "PIT MAX";

      // Min has one extra position below its lowest value: that position means Off
      const int16_t offValue = degCFlag ? 9 : 49;

      if (hasRunFlag == 0) {
        // Min runs from 50F (10C) up to 5F under Max. Max runs from 5F over Min (or 50F
        // when Min is Off) up to 572F (300C).
        float loF = (editPitIndex == 0) ? 50.0f : (isnan(pitMin) ? 50.0f : pitMin + 5.0f);
        float hiF = (editPitIndex == 0) ? pitMax - 5.0f : 572.0f;
        int16_t lo = degCFlag ? (int16_t)ceilf((loF - 32.0f) * 5.0f / 9.0f) : (int16_t)ceilf(loF);
        int16_t hi = degCFlag ? (int16_t)floorf((hiF - 32.0f) * 5.0f / 9.0f) : (int16_t)floorf(hiF);
        int16_t rangeLo = (editPitIndex == 0) ? offValue : lo;

        int16_t start;
        if (editPitIndex == 0 && isnan(pitMin)) {
          start = offValue;
        } else {
          float currentF = (editPitIndex == 0) ? pitMin : pitMax;
          float currentDisplay = degCFlag ? convertDegFtoDegC(currentF) : currentF;
          start = (int16_t)(currentDisplay + 0.5f);
        }

        encoder.newRange(rangeLo, hi, constrain(start, rangeLo, hi), true, currentEncoderState);
        prevEncoderValue = currentEncoderState.currentValue;
        hasRunFlag = 1;
        lastDrawnValue = -1000;
      }

      bool isOff = (editPitIndex == 0 && prevEncoderValue == offValue);

      if (prevEncoderValue != lastDrawnValue) {
        lcd.showLimitEdit(label, prevEncoderValue, degCFlag, isOff);
        lastDrawnValue = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
        if (isOff) {
          pitMin = NAN;  // stored as NAN: Min Off, no low-temperature alert
          Log.println(F("PIT min -> Off (no low alert)"));
        } else {
          float enteredF = degCFlag ? ((float)prevEncoderValue * 9.0f / 5.0f + 32.0f)
                                    : (float)prevEncoderValue;
          if (editPitIndex == 0) {
            pitMin = enteredF;
          } else {
            pitMax = enteredF;
          }
          Log.print(F("PIT "));
          Log.print(editPitIndex == 0 ? F("min") : F("max"));
          Log.print(F(" -> "));
          Log.print(prevEncoderValue);
          Log.print(degCFlag ? F(" C (stored as ") : F(" F (stored as "));
          Log.print(enteredF, 1);
          Log.println(F(" F)"));
        }
        smokerState = pitTempMenu;
        hasRunFlag = 0;
      }

      if (button.triggered(HOLD)) {
        smokerState = pitTempMenu;
        hasRunFlag = 0;
      }
    } break;

    case bankMenu: {
      static int16_t lastDrawnSelection = -1;
      static char bankDetail[2][16];
      static const char* const bankLabels[3] = {"Capacity", "Charge", "Back"};
      static const char* bankDetails[3] = {bankDetail[0], bankDetail[1], ""};
      currentEncoderValue = encoder.getCount();

      if (hasRunFlag == 0) {
        encoder.newRange(1, 3, 1, true, currentEncoderState);  // menus wrap
        prevEncoderValue = currentEncoderState.currentValue;
        hasRunFlag = 1;
        lastDrawnSelection = -1;
      }

      if (prevEncoderValue != lastDrawnSelection) {
        snprintf(bankDetail[0], sizeof(bankDetail[0]), "%.0f mAh", bankCapacity_mAh);
        snprintf(bankDetail[1], sizeof(bankDetail[1]), "%d%%", bankStartPct);
        lcd.showListMenu("POWER BANK", bankLabels, bankDetails, 3, (uint8_t)prevEncoderValue);
        lastDrawnSelection = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
        if (prevEncoderValue == 3) {  // Back
          smokerState = changeSettings;
          hasRunFlag = 0;
        } else {
          bankEditIndex = (uint8_t)(prevEncoderValue - 1);  // 0 = capacity, 1 = charge
          smokerState = editBank;
          hasRunFlag = 0;
        }
      }

      if (button.triggered(HOLD)) {
        smokerState = changeSettings;
        hasRunFlag = 0;
      }
    } break;

    case editBank: {
      static int16_t lastDrawnValue = -1000;
      currentEncoderValue = encoder.getCount();
      bool isCapacity = (bankEditIndex == 0);

      // Capacity: 500 mAh steps from 1000 to 30000. Charge: 10% steps from 10 to 100.
      int16_t lo = isCapacity ? 2 : 1;
      int16_t hi = isCapacity ? 60 : 10;

      if (hasRunFlag == 0) {
        int16_t start = isCapacity ? (int16_t)(bankCapacity_mAh / 500.0f + 0.5f)
                                   : (int16_t)(bankStartPct / 10);
        encoder.newRange(lo, hi, constrain(start, lo, hi), false, currentEncoderState);
        prevEncoderValue = currentEncoderState.currentValue;
        hasRunFlag = 1;
        lastDrawnValue = -1000;
      }

      if (prevEncoderValue != lastDrawnValue) {
        char text[16];
        if (isCapacity) {
          snprintf(text, sizeof(text), "%d mAh", prevEncoderValue * 500);
        } else {
          snprintf(text, sizeof(text), "%d%%", prevEncoderValue * 10);
        }
        lcd.showValueEdit(isCapacity ? "CAPACITY" : "CHARGE", text);
        lastDrawnValue = prevEncoderValue;
      }

      if (button.triggered(SINGLE_TAP)) {
        if (isCapacity) {
          bankCapacity_mAh = prevEncoderValue * 500.0f;
          bankCapacitySave(bankCapacity_mAh);
          Log.print(F("power bank capacity set -> "));
          Log.print(bankCapacity_mAh, 0);
          Log.println(F(" mAh"));
        } else {
          bankStartPct = (uint8_t)(prevEncoderValue * 10);
          bankChargeSave(bankStartPct);
          Log.print(F("power bank charge set -> "));
          Log.print(bankStartPct);
          Log.println(F("%"));
        }

        if (bankEditFromCook) {
          if (isCapacity) {
            bankEditIndex = 1;  // next, ask for the charge of the new bank
            smokerState = editBank;
            hasRunFlag = 0;
          } else {
            // New bank in place: restart the count and forecast; targets and done flags stay
            ina226.resetCharge();
            bankBaseMah = bankCapacity_mAh * BANK_USABLE_FACTOR * bankStartPct / 100.0f;
            cookSaveBank(bankBaseMah);
            cookForceCheckpoint(ina226.getChargeUsed_mAh());  // save the reset count now
            resetBankForecast();
            bankEditFromCook = false;
            Log.println(F("new bank: charge count reset, forecast re-armed"));
            smokerState = bbqStatus;
            hasRunFlag = 0;
          }
        } else {
          smokerState = bankMenu;
          hasRunFlag = 0;
        }
      }

      if (button.triggered(HOLD)) {
        smokerState = bankEditFromCook ? bbqStatus : bankMenu;
        bankEditFromCook = false;
        hasRunFlag = 0;
      }
    } break;

    case setupPortal: {
      if (hasRunFlag == 0) {
        portalOpen();
        lcd.display();
        lcd.showPortalScreen();
        hasRunFlag = 1;
      }

      // Stays open for three minutes, or until Hold. Submitting the form saves at once.
      if (!portalPump() || button.triggered(HOLD)) {
        portalClose();
        smokerState = changeSettings;
        hasRunFlag = 0;
      }
    } break;

    case railStatus: {
      static unsigned long lastDrawMs = 0;
      static bool firstDraw = true;

      if (hasRunFlag == 0) {
        hasRunFlag = 1;
        firstDraw = true;
      }

      if (firstDraw || millis() - lastDrawMs >= 1000) {
        firstDraw = false;
        lastDrawMs = millis();
        lcd.showRailStatus(ina226.getRailV(), calRailV());
      }

      if (button.triggered(HOLD)) {
        smokerState = changeSettings;
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
        Log.print(F("calibration reset for "));
        Log.println(calNames[id]);
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
                Log.print(F("ice point captured for "));
                Log.print(calNames[calSelectedProbe]);
                Log.print(F(": "));
                Log.print(calIceReading_F, 2);
                Log.println(F(" degF"));
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
          Log.print(F("boil reference set to "));
          Log.println(calBoilRef_F);
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
              Log.print(F("boil point captured for "));
              Log.print(calNames[calSelectedProbe]);
              Log.print(F(": "));
              Log.print(calBoilReading_F, 2);
              Log.println(F(" degF"));
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
          Log.print(F("calibration saved for "));
          Log.println(calNames[calSelectedProbe]);
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