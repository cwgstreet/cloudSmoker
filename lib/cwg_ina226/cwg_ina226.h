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

#ifndef CWG_INA226_H
#define CWG_INA226_H

#include <INA226.h>  // Tillaart INA226 library, MIT licence
#include <Wire.h>

// INA226 wrapper for the +5 V supply measurement and power-bank coulomb counting.
// Shunt is 0.1 ohm on the USB-C input; VIN+ is the supply side, VIN- the load side.
class CWG_INA226 : public INA226 {
 public:
  CWG_INA226(uint8_t address = 0x40, TwoWire* wire = &Wire) : INA226(address, wire) {}
  virtual ~CWG_INA226() {}

  // Call after Wire.begin(). Returns true if the device answered and settings were accepted.
  bool initialise(float maxCurrent_A, float shunt_ohm);

  // true only if the device acknowledged on the bus during initialise()
  bool deviceFound() const { return deviceOk; }

  // +5 V rail voltage in volts, or NAN if the device is not present
  float getRailV();

 protected:
  bool deviceOk = false;  // set by initialise()
};

// ensure object is visible everywhere (global)
extern CWG_INA226 ina226;

#endif  // CWG_INA226_H