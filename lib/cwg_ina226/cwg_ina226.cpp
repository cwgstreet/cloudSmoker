/* ***************************************************************
 * cwg_ina226.cpp - library to extend ina226 library functionality
 *   includes
 *    1) RobTillaart/INA226 library, licenced under MIT Licence
 *         reference:  https://github.com/RobTillaart/INA226
 *
 *  Purpose of INA226 is
 *    a) 5V rail measurement for Steinhart-Hart temperature calculations, and b) coulomb counting
 * for battery charge used b) coulomb counting for power bank battery charge used
 *
 *  C W Greenstreet, Ver2, 4Nov2026
 *    GPLv3 Licence
 *
 ** ************************************************************* */

#include "bleserial.h"
#include "cwg_ina226.h"

#include <math.h>  // NAN

CWG_INA226 ina226(0x40);  // A0 and A1 tied to GND

/* ----------------------------------------
 *  initialise() method: check the device and set the current range and averaging.
 *    maxCurrent_A: 0.79 A gives 79 mV at full scale on 0.1 ohm, under the 80 mV library limit
 *    shunt_ohm:    0.1 ohm, the 2512 shunt on the board
 ** --------------------------------------*/
bool CWG_INA226::initialise(float maxCurrent_A, float shunt_ohm) {
  deviceOk = begin();  // needs Wire.begin() to have run first
  if (!deviceOk) {
    Log.println(F(" **ERROR** INA226 not connected! "));
    return false;
  }

  int err = setMaxCurrentShunt(maxCurrent_A, shunt_ohm);
  if (err != INA226_ERR_NONE) {
    Log.print(F(" **ERROR** INA226 range rejected, code 0x"));
    Log.println(err, HEX);
    deviceOk = false;
    return false;
  }

  setAverage(2);  // 2 = 16 samples per reading, smooths short radio bursts
  return true;
}

/* ----------------------------------------
 *  getRailV() method: bus voltage at VIN- (load side), which is the +5 V rail
 ** --------------------------------------*/
float CWG_INA226::getRailV() {
  if (!deviceOk) {
    return NAN;
  }
  return getBusVoltage();
}

/* ----------------------------------------
 *  updateCharge() method: integrate current over time to count charge (coulomb counting).
 *  Call once per loop() pass. Does nothing until 1 s has passed since the last sample.
 *  Uses micros() so the interval is the real elapsed time, even when a blocking call
 *  (ADC reads, ThingSpeak upload) has stretched a loop pass.
 ** --------------------------------------*/
void CWG_INA226::updateCharge() {
  if (!deviceOk) {
    return;
  }

  unsigned long nowUs = micros();

  if (!haveSample) {  // first call only starts the clock
    lastSampleUs = nowUs;
    haveSample = true;
    return;
  }

  // unsigned subtraction stays correct when micros() wraps (about every 71 minutes)
  unsigned long elapsedUs = nowUs - lastSampleUs;
  if (elapsedUs < 1000000UL) {
    return;  // less than 1 s since the last sample
  }

  double dt_s = (double)elapsedUs / 1e6;
  lastSampleUs = nowUs;

  float amps = getCurrent();  // positive = current flowing from the power bank to the board
  chargeUsed_As += (double)amps * dt_s;
}