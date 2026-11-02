/* ***************************************************************
 * calibration.h - library to manage calibration coefficients of thermistor probes
 *
 *  Ref: https://github.com/cwgstreet/cloudSmoker/wiki/Calibration
 *
 *  C W Greenstreet, Ver1, 31Oct2026
 *    GPLv3 Licence
 *
 ** ************************************************************* */

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Arduino.h>

// Probe identities follow the labels: jack 1 = PIT, 2 = MEAT1, 3 = MEAT2, 4 = MEAT3
enum ProbeId : uint8_t { PROBE_PIT = 0, PROBE_MEAT1, PROBE_MEAT2, PROBE_MEAT3, PROBE_COUNT };

struct ProbeCal {
  float gain;    // a
  float offset;  // b, degrees F
};

void calInit();                                     // load from NVS, defaults if absent
bool calSet(ProbeId id, float gain, float offset);  // validate, save to NVS, update RAM
void calReset(ProbeId id);                          // back to identity (a = 1, b = 0)

// Applies the correction to a temperature in degF.
// NaN (no probe) passes through unchanged, so the done logic still sees "no data".
float calApplyF(ProbeId id, float tempF);
void calPollSerial();  // calShow / calReset / cal / railV commands

// formatted serial terminal command summary, printed on first encoder move and by "help"
void calPrintHelp();

// Measured bias supply: the +5 V rail feeding the thermistor dividers, in volts.
// Defaults to 5.00 V until set. Stored in NVS.
float calRailV();
bool calSetRailV(float volts);

#endif  // CALIBRATION_H