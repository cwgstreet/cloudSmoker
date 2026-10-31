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

// Handle to the "cal" namespace in NVS
static Preferences prefs;

// Plausibility limits: a sensible probe needs only a small correction.
// gain 0.9 to 1.1 is +/-10 %; offset -9 to +9 degF (about +/-5 degC). Values outside
// these are rejected, so a mistyped number can't wreck every temperature reading.
static const float GAIN_MIN = 0.9f;
static const float GAIN_MAX = 1.1f;
static const float OFFSET_MIN = -9.0f;  // degF (about 5 degC)
static const float OFFSET_MAX = 9.0f;

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

// Opens NVS and loads all four probes. Missing or invalid entries fall back to identity
// (gain 1, offset 0), so an uncalibrated probe reads its raw Steinhart-Hart temperature.
void calInit() {
  prefs.begin("cal", false);  // namespace "cal", read-write
  for (uint8_t i = 0; i < PROBE_COUNT; i++) {
    char kg[16], ko[16];
    keyFor((ProbeId)i, 'g', kg, sizeof(kg));
    keyFor((ProbeId)i, 'o', ko, sizeof(ko));
    float g = prefs.getFloat(kg, 1.0f);  // default if the key doesn't exist yet
    float o = prefs.getFloat(ko, 0.0f);
    calTable[i] = valuesOk(g, o) ? ProbeCal{g, o} : ProbeCal{1.0f, 0.0f};
  }
}

// Returns one probe to the uncorrected reading (gain 1, offset 0).
void calReset(ProbeId id) { calSet(id, 1.0f, 0.0f); }

// Validates, saves to NVS, and updates the RAM copy. Returns false if rejected.
bool calSet(ProbeId id, float gain, float offset) {
  if (id >= PROBE_COUNT || !valuesOk(gain, offset)) {
    return false;  // nothing is written to flash for a rejected value
  }
  char kg[16], ko[16];
  keyFor(id, 'g', kg, sizeof(kg));
  keyFor(id, 'o', ko, sizeof(ko));
  prefs.putFloat(kg, gain);  // writes to flash; keep calls rare, not per cycle
  prefs.putFloat(ko, offset);
  calTable[id] = ProbeCal{gain, offset};
  return true;
}

// Applies the correction to a temperature in degF.
// NaN (no probe) passes through unchanged, so the done logic still sees "no data".
float calApplyF(ProbeId id, float tempF) {
  if (isnan(tempF)) {
    return tempF;
  }
  return calTable[id].gain * tempF + calTable[id].offset;
}

// ---- serial entry (interim until the encoder settings screen) ----

// Prints one probe's current gain and offset, for checking values after entry.
static void printCal(ProbeId id) {
  Serial.print(probeNames[id]);
  Serial.print(F("  gain = "));
  Serial.print(calTable[id].gain, 6);
  Serial.print(F("  offset = "));
  Serial.println(calTable[id].offset, 4);
}

// Parses one complete command line. Commands are case-sensitive except the probe name:
//   cal <probe> <gain> <offset>   e.g.  cal PIT 1.0526 -2.105
//   calShow                       print all four probes
//   calReset                      return all four probes to identity
static void handleCommand(String cmd) {
  cmd.trim();

  if (cmd == "calShow") {
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

  Serial.println(F("commands: cal <probe> <gain> <offset> | calShow | calReset"));
}

// Called once per loop(). Reads any waiting serial characters and builds a line until
// the end-of-line character arrives. Lines are capped at 80 characters, so a runaway
// input can't grow the buffer without limit.
void calPollSerial() {
  static String line;  // persists between calls, so a line can arrive in pieces
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (line.length() > 0) {  // ignore the blank line from "\r\n" endings
        handleCommand(line);
        line = "";
      }
    } else if (line.length() < 80) {
      line += c;
    }
  }
}