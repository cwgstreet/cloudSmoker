/* ***************************************************************
 * cwg_ina226.h - library to extend ina226 library functionality
 *   includes
 *    1) RobTillaart/INA226 library, licenced under MIT Licence
 *         reference:  https://github.com/RobTillaart/INA226
 *
 *  C W Greenstreet, Ver2, 4Nov2026
 *    GPLv3 Licence
 *
 ** ************************************************************* */

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
    Serial.println(F(" **ERROR** INA226 not connected! "));
    return false;
  }

  int err = setMaxCurrentShunt(maxCurrent_A, shunt_ohm);
  if (err != INA226_ERR_NONE) {
    Serial.print(F(" **ERROR** INA226 range rejected, code 0x"));
    Serial.println(err, HEX);
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