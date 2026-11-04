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

// Returns the measured rail voltage used by the Steinhart-Hart calculation.
float calRailV() { return railVolts; }

// Validates, saves the rail voltage to NVS, and updates the RAM copy. Returns false if rejected.
bool calSetRailV(float volts) {
  if (!railOk(volts)) {
    return false;
  }
  // Checks each putFloat() return value: a failed flash write returns 0, not sizeof(float).
  // RAM is updated only after both writes succeed, so a failed save never reports success.
  if (prefs.putFloat("rail", volts) != sizeof(float)) {
    return false;
  }
  railVolts = volts;
  return true;
}

// ---- serial entry (interim until the encoder settings screen) ----

// Prints one probe's current gain and offset, for checking values after entry.
// Prints one probe's gain and offset, with the name padded so the values line up.
static void printCal(ProbeId id) {
  char name[8];
  snprintf(name, sizeof(name), "%-6s", probeNames[id]);  // left-justified, 6 characters
  Serial.print(name);
  Serial.print(F("  gain = "));
  Serial.print(calTable[id].gain, 6);
  Serial.print(F("  offset = "));
  Serial.println(calTable[id].offset, 4);
}

// Prints the serial command summary. Called once when the device leaves the splash screen,
// and whenever the user types "help".
void calPrintHelp() {
  Serial.println();
  Serial.println(F("================ cloudSmoker2 serial commands ================"));
  Serial.println(F("  help                          show this list"));
  Serial.println(F("  calShow                       show rail voltage and all probe factors"));
  Serial.println(F("  calReset                      reset all probes to gain 1.0, offset 0.0"));
  Serial.println(F("  cal <probe> <gain> <offset>   set one probe, e.g.  cal MEAT1 1.0526 -5.47"));
  Serial.println(F("                                  probes: PIT, MEAT1, MEAT2, MEAT3"));
  Serial.println(F("                                  gain 0.9 to 1.1, offset -9 to +9 degF"));
  Serial.println(F("  railV <volts>                 set measured +5 V rail, e.g.  railV 5.09"));
  Serial.println(F("                                  range 4.5 to 5.5 V"));
  Serial.println(F("=============================================================="));
  Serial.println(F("Type a command and press Enter. The device echoes what you type."));
  Serial.println();
}

// Parses one complete command line. Commands are case-sensitive except the probe name:
//   cal <probe> <gain> <offset>   e.g.  cal PIT 1.0526 -2.105
//   railV <volts>                 e.g.  railV 5.09
//   calShow                       print the rail and all four probes
//   calReset                      return all four probes to identity
static void handleCommand(String cmd) {
  cmd.trim();

  if (cmd == "help") {
    calPrintHelp();
    return;
  }

  if (cmd == "calShow") {
    Serial.print(F("RAIL  = "));
    Serial.print(railVolts, 4);
    Serial.println(F(" V"));
    for (uint8_t i = 0; i < PROBE_COUNT; i++) {
      printCal((ProbeId)i);
    }
    return;
  }

  if (cmd == "calReset") {
    for (uint8_t i = 0; i < PROBE_COUNT; i++) {
      calReset((ProbeId)i);
    }
    Serial.println(F("calibration reset to identity (gain 1, offset 0)"));
    return;
  }

  if (cmd.startsWith("railV ")) {
    float v = cmd.substring(6).toFloat();  // 0.0 for unparseable text, which fails the limits
    if (calSetRailV(v)) {
      Serial.print(F("rail saved: "));
      Serial.print(railVolts, 4);
      Serial.println(F(" V"));
    } else {
      Serial.println(F("rejected: rail must be 4.5 to 5.5 V"));
    }
    return;
  }

  if (cmd.startsWith("cal ")) {
    // Strip "cal " and split the rest into <probe> <gain> <offset>.
    String rest = cmd.substring(4);
    int sp1 = rest.indexOf(' ');
    if (sp1 < 0) {
      Serial.println(F("usage: cal <probe> <gain> <offset>"));
      return;
    }
    String name = rest.substring(0, sp1);
    String vals = rest.substring(sp1 + 1);
    int sp2 = vals.indexOf(' ');
    if (sp2 < 0) {
      Serial.println(F("usage: cal <probe> <gain> <offset>"));
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
      Serial.println(F("unknown probe: use PIT, MEAT1, MEAT2 or MEAT3"));
      return;
    }
    if (calSet(id, g, o)) {
      Serial.print(F("saved: "));
      printCal(id);
    } else {
      Serial.println(F("rejected: gain must be 0.9 to 1.1, offset -9 to +9 degF"));
    }
    return;
  }

  Serial.println(F("unknown command. Type help for the command list."));
}

// Called once per loop(). Reads any waiting serial characters and builds a line until
// the end-of-line character arrives. Lines are capped at 80 characters, so a runaway
// input can't grow the buffer without limit.
void calPollSerial() {
  static String line;  // persists between calls, so a line can arrive in pieces
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      Serial.println();         // move to a new line after the typed command
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