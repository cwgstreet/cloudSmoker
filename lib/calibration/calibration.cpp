/* ***************************************************************
 * calibration.cpp - library to manage calibration coefficients of thermistor probes
 *
 *  per-probe two-point calibration (gain and offset) for cloudSmoker2
 *
 * Each probe's Steinhart-Hart calculated temperature (in degF, the firmware's working unit)
 *  is corrected with:
 *    T_corrected = gain * T_measured + offset
 *
 *  Values are stored in ESP32 NVS (flash), so they survive reboots and power loss.
 *
 *  Probes are identified by their labels (PIT, MEAT1, MEAT2, MEAT3), which match the
 *   jack numbers 1 to 4 on the enclosure.
 *
 *  The bias rail voltage is used by the Steinhart-Hart calculation, so it must be
 *  measured, not assumed.

 *  Ref: https://github.com/cwgstreet/cloudSmoker/wiki/Calibration
 *
 *  C W Greenstreet, Ver1, 31Oct2026
 *    GPLv3 Licence
 *
 ** ************************************************************* */

#/* ***************************************************************
 * calibration.cpp - per-probe calibration (gain and offset) and measured rail voltage
 *
 *  Each probe's temperature (degF) is corrected with:
 *    T_corrected = gain * T_measured + offset
 *
 *  The bias rail voltage is used by the Steinhart-Hart calculation, so it must be
 *  measured, not assumed. Values are stored in ESP32 NVS (flash).
 *
 *  Ref: https://github.com/cwgstreet/cloudSmoker/wiki/Calibration
 *
 *  C W Greenstreet, Ver1, 31Oct2026
 *    GPLv3 Licence
 *
 ** ************************************************************* */

#include "calibration.h"

#include <Preferences.h>  // ESP32 NVS key/value storage, part of the Arduino core
#include <math.h>         // isnan(), isfinite()

#include "bleserial.h"
#include "cookstore.h"   // stored cook state, for the cookShow and cookEnd bench commands
#include "cwg_ina226.h"  // live +5 V rail reading, used by calRailV()
#include "devconfig.h"   // devBlePasskey()

// Display names, used in both the NVS keys and the serial commands
static const char* const probeNames[PROBE_COUNT] = {"PIT", "MEAT1", "MEAT2", "MEAT3"};

// Calibration values held in RAM. Loaded from NVS by calInit().
static ProbeCal calTable[PROBE_COUNT];

// Measured rail voltage in RAM. Loaded from NVS by calInit().
static float railVolts = 5.0f;

// Handle to the "cal" namespace in NVS
static Preferences prefs;

// Plausibility limits: a sensible probe needs only a small correction.
// gain 0.9 to 1.1 is +/-10 %; offset -9 to +9 degF. Values outside these are rejected.
static const float GAIN_MIN = 0.9f;
static const float GAIN_MAX = 1.1f;
static const float OFFSET_MIN = -9.0f;  // degF (about 5 degC)
static const float OFFSET_MAX = 9.0f;

// Rail limits: the +5 V supply should sit within about 10 % of nominal.
static const float RAIL_MIN = 4.5f;
static const float RAIL_MAX = 5.5f;
static const float RAIL_DEFAULT = 5.0f;

// Builds an NVS key such as "MEAT1_g" (gain) or "MEAT1_o" (offset).
// NVS keys are limited to 15 characters, so these short names are deliberate.
static void keyFor(ProbeId id, char suffix, char* out, size_t len) {
  snprintf(out, len, "%s_%c", probeNames[id], suffix);
}

// Returns true if both values are finite numbers within the plausibility limits.
static bool valuesOk(float gain, float offset) {
  return isfinite(gain) && gain >= GAIN_MIN && gain <= GAIN_MAX && isfinite(offset) &&
         offset >= OFFSET_MIN && offset <= OFFSET_MAX;
}

// Returns true if the rail voltage is a finite number within the limits.
static bool railOk(float volts) {
  return isfinite(volts) && volts >= RAIL_MIN && volts <= RAIL_MAX;
}

// Opens NVS and loads the rail and all four probes. Missing or invalid entries fall back
// to defaults (rail 5.00 V, gain 1, offset 0).
void calInit() {
  prefs.begin("cal", false);  // namespace "cal", read-write

  float r = prefs.getFloat("rail", RAIL_DEFAULT);
  railVolts = railOk(r) ? r : RAIL_DEFAULT;

  for (uint8_t i = 0; i < PROBE_COUNT; i++) {
    char kg[16], ko[16];
    keyFor((ProbeId)i, 'g', kg, sizeof(kg));
    keyFor((ProbeId)i, 'o', ko, sizeof(ko));
    float g = prefs.getFloat(kg, 1.0f);
    float o = prefs.getFloat(ko, 0.0f);
    calTable[i] = valuesOk(g, o) ? ProbeCal{g, o} : ProbeCal{1.0f, 0.0f};
  }
}

// Validates, saves to NVS, and updates the RAM copy. Returns false if rejected.
bool calSet(ProbeId id, float gain, float offset) {
  if (id >= PROBE_COUNT || !valuesOk(gain, offset)) {
    return false;  // nothing is written to flash for a rejected value
  }
  char kg[16], ko[16];
  keyFor(id, 'g', kg, sizeof(kg));
  keyFor(id, 'o', ko, sizeof(ko));
  if (prefs.putFloat(kg, gain) != sizeof(float) || prefs.putFloat(ko, offset) != sizeof(float)) {
    return false;  // write failed: RAM copy left unchanged
  }
  calTable[id] = ProbeCal{gain, offset};
  return true;
}

// Returns one probe to the uncorrected reading (gain 1, offset 0).
void calReset(ProbeId id) { calSet(id, 1.0f, 0.0f); }

// Applies the correction to a temperature in degF.
// NaN (no probe) passes through unchanged, so the done logic still sees "no data".
float calApplyF(ProbeId id, float tempF) {
  if (isnan(tempF)) {
    return tempF;
  }
  return calTable[id].gain * tempF + calTable[id].offset;
}

/// Returns the rail voltage required by the Steinhart-Hart calculation.
// Uses the live INA226 reading when it's present and within the rail limits.
// Otherwise falls back to the stored value set with railV.
float calRailV() {
  float live = ina226.getRailV();  // NaN if the INA226 is not present
  if (railOk(live)) {
    return live;
  }
  return railVolts;
}

// Validates, saves the rail voltage to NVS, and updates the RAM copy. Returns false if rejected.
bool calSetRailV(float volts) {
  if (!railOk(volts)) {
    return false;
  }
  if (prefs.putFloat("rail", volts) != sizeof(float)) {
    return false;  // write failed: RAM copy left unchanged
  }
  railVolts = volts;
  return true;
}

// Prints one probe's current gain and offset, for checking values after entry.
// Prints one probe's gain and offset, with the name padded so the values line up.
static void printCal(ProbeId id) {
  char name[8];
  snprintf(name, sizeof(name), "%-6s", probeNames[id]);  // left-justified, 6 characters
  Log.print(name);
  Log.print(F("  gain = "));
  Log.print(calTable[id].gain, 6);
  Log.print(F("  offset = "));
  Log.println(calTable[id].offset, 4);
}

// Prints the serial command summary. Called once when the device leaves the splash screen,
// and whenever the user types "help".
void calPrintHelp() {
  Log.println();
  Log.println(F("================ cloudSmoker2 serial commands ================"));
  Log.println(F("  help                            show this list"));
  Log.println(F("  calShow                         show rail voltage and all probe factors"));
  Log.println(F("  calReset                        reset all probes to gain 1.0, offset 0.0"));
  Log.println(F("  calReset <probe>                reset one probe, e.g.  calReset MEAT1"));
  Log.println(F("  cal <probe> <gain> <offset>     set one probe, e.g.  cal MEAT1 1.0526 -5.47"));
  Log.println(F("                                  probes: PIT, MEAT1, MEAT2, MEAT3"));
  Log.println(F("                                  gain 0.9 to 1.1, offset -9 to +9 degF"));
  Log.println(F("  railV <volts>                   set measured +5 V rail, e.g.  railV 5.09"));
  Log.println(F("                                  range 4.5 to 5.5 V"));
  Log.println(F("  charge                          show charge drawn from the power bank"));
  Log.println(F("  chargeReset                     restart the charge count at 0"));
  Log.println(F("  passkey                         show the BLE pair code"));
#ifdef DEBUG_COOK_CMDS
  Log.println(F("  cookShow                        debug: show whether a cook is stored"));
  Log.println(F("  cookEnd                         debug: clear the stored cook"));
  Log.println(F("  cookInfo                        debug: stored cook, charge and lastSeen age"));
  Log.println(F("  cookAge <hours>                 debug: move stored lastSeen back (stale test)"));
#endif
  Log.println(F("=============================================================="));
  Log.println(F("Type a command and press Enter. The device echoes what you type."));
  Log.println();
}

// Parses one complete command line. Commands are case-sensitive except the probe name:
//   cal <probe> <gain> <offset>   e.g.  cal PIT 1.0526 -2.105
//   calReset                      return all four probes to identity
//   calReset <probe>              return one probe to identity, e.g.  calReset MEAT1
//   railV <volts>                 e.g.  railV 5.09
//   calShow                       print the rail and all four probes
static void handleCommand(String cmd) {
  cmd.trim();

  if (cmd == "help") {
    calPrintHelp();
    return;
  }

  if (cmd == "passkey") {
    Log.printf("pair code %06lu\n", (unsigned long)devBlePasskey());
    return;
  }

  if (cmd == "calShow") {
    Log.print(F("RAIL  = "));
    Log.print(calRailV(), 4);  // value used by the temperature calculations
    Log.print(F(" V used   stored = "));
    Log.print(railVolts, 4);  // fallback value set with railV
    Log.println(F(" V"));
    for (uint8_t i = 0; i < PROBE_COUNT; i++) {
      printCal((ProbeId)i);
    }
    return;
  }

  if (cmd == "calReset") {
    for (uint8_t i = 0; i < PROBE_COUNT; i++) {
      calReset((ProbeId)i);
    }
    Log.println(F("calibration reset to identity (gain 1, offset 0)"));
    return;
  }

  if (cmd.startsWith("calReset ")) {
    String name = cmd.substring(9);
    name.trim();
    ProbeId id = PROBE_COUNT;  // PROBE_COUNT means "not found"
    for (uint8_t i = 0; i < PROBE_COUNT; i++) {
      if (name.equalsIgnoreCase(probeNames[i])) {
        id = (ProbeId)i;
      }
    }
    if (id == PROBE_COUNT) {
      Log.println(F("unknown probe: use PIT, MEAT1, MEAT2 or MEAT3"));
      return;
    }
    calReset(id);
    Log.print(F("reset: "));
    printCal(id);
    return;
  }

  if (cmd.startsWith("railV ")) {
    float v = cmd.substring(6).toFloat();  // 0.0 for unparseable text, which fails the limits
    if (calSetRailV(v)) {
      Log.print(F("rail saved: "));
      Log.print(railVolts, 4);
      Log.println(F(" V"));
    } else {
      Log.println(F("rejected: rail must be 4.5 to 5.5 V"));
    }
    return;
  }

  if (cmd.startsWith("cal ")) {
    // Strip "cal " and split the rest into <probe> <gain> <offset>.
    String rest = cmd.substring(4);
    int sp1 = rest.indexOf(' ');
    if (sp1 < 0) {
      Log.println(F("usage: cal <probe> <gain> <offset>"));
      return;
    }
    String name = rest.substring(0, sp1);
    String vals = rest.substring(sp1 + 1);
    int sp2 = vals.indexOf(' ');
    if (sp2 < 0) {
      Log.println(F("usage: cal <probe> <gain> <offset>"));
      return;
    }
    // toFloat() returns 0.0 for text it can't parse. That's safe here, because 0 is
    // outside the gain limits, so a typo gets rejected rather than saved.
    float g = vals.substring(0, sp2).toFloat();
    float o = vals.substring(sp2 + 1).toFloat();

    // Match the probe name case-insensitively against the table.
    ProbeId id = PROBE_COUNT;  // PROBE_COUNT means "not found"
    for (uint8_t i = 0; i < PROBE_COUNT; i++) {
      if (name.equalsIgnoreCase(probeNames[i])) {
        id = (ProbeId)i;
      }
    }
    if (id == PROBE_COUNT) {
      Log.println(F("unknown probe: use PIT, MEAT1, MEAT2 or MEAT3"));
      return;
    }
    if (calSet(id, g, o)) {
      Log.print(F("saved: "));
      printCal(id);
    } else {
      Log.println(F("rejected: gain must be 0.9 to 1.1, offset -9 to +9 degF"));
    }
    return;
  }

  if (cmd == "charge") {
    Log.print(F("charge used = "));
    Log.print(ina226.getChargeUsed_mAh(), 2);
    Log.println(F(" mAh at 5 V"));
    return;
  }

  if (cmd == "chargeReset") {
    ina226.resetCharge();
    Log.println(F("charge count reset to 0"));
    return;
  }

#ifdef DEBUG_COOK_CMDS
  if (cmd == "cookShow") {
    Log.print(F("stored cook active: "));
    Log.println(cookStoreActive() ? F("yes") : F("no"));
    return;
  }

  if (cmd == "cookEnd") {
    cookSaveEnd();  // clears the stored cook flag only; charge count and targets untouched
    Log.println(F("stored cook cleared"));
    return;
  }
#endif

#ifdef DEBUG_COOK_CMDS
  if (cmd == "cookInfo") {
    float meatT[3], pitMinT, pitMaxT, charge;
    bool degC;
    uint8_t flags;
    Log.print(F("stored cook active: "));
    Log.println(cookStoreActive() ? F("yes") : F("no"));
    if (cookLoad(meatT, pitMinT, pitMaxT, degC, charge, flags)) {
      Log.print(F("  charge stored: "));
      Log.print(charge, 1);
      Log.println(F(" mAh"));
      Log.print(F("  meat targets F: "));
      Log.print(meatT[0], 0);
      Log.print(F(" / "));
      Log.print(meatT[1], 0);
      Log.print(F(" / "));
      Log.println(meatT[2], 0);
      Log.print(F("  PIT min / max F: "));
      if (isnan(pitMinT)) {
        Log.print(F("OFF"));
      } else {
        Log.print(pitMinT, 0);
      }
      Log.print(F(" / "));
      Log.println(pitMaxT, 0);
    }
    Log.print(F("  clock valid: "));
    Log.println(clockValid() ? F("yes") : F("no"));
    float ageH = cookLastSeenAgeHours();
    Log.print(F("  lastSeen age: "));
    if (ageH < 0.0f) {
      Log.println(F("unknown (clock not set or cook started offline)"));
    } else {
      Log.print(ageH, 2);
      Log.println(F(" h"));
    }
    return;
  }
#endif

#ifdef DEBUG_COOK_CMDS
  if (cmd.startsWith("cookAge ")) {
    float hours = cmd.substring(8).toFloat();
    if (cookSetLastSeenAgeHours(hours)) {
      Log.print(F("stored lastSeen moved back "));
      Log.print(hours, 2);
      Log.println(F(" h; reset the board now to test the stale rule"));
    } else {
      Log.println(F("cookAge needs a stored cook and a valid clock"));
    }
    return;
  }
#endif

  Log.println(F("unknown command. Type help for the command list."));
}

// Called once per loop(). Reads any waiting serial characters and builds a line until
// the end-of-line character arrives. Lines are capped at 80 characters, so a runaway
// input can't grow the buffer without limit.
void calPollSerial() {
  static String line;  // persists between calls, so a line can arrive in pieces
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      Log.println();            // move to a new line after the typed command
      if (line.length() > 0) {  // ignore the blank line from "\r\n" endings
        handleCommand(line);
        line = "";
      }
    } else if (c == 8 || c == 127) {  // backspace or delete: remove the last character
      if (line.length() > 0) {
        line.remove(line.length() - 1);
        Serial.write(8);    // move the cursor back
        Serial.write(' ');  // blank out the old character
        Serial.write(8);
      }
    } else if (line.length() < 80) {
      line += c;
      Serial.write(c);  // echo the typed character
    }
  }
}
// Runs one command line from any source (USB or BLE), exactly as the USB port does
void calRunCommand(const String& cmd) { handleCommand(cmd); }
