/* ***************************************************************
 * cwg_ads1x15.h - library to extend ads1x15 library functionality
 *   Adds initialise(), a voltage factor, a median-filtered reading and deviceFound()
 *   to the RobTillaart ADS1X15 class, for the ADS1115 ADC.
 *
 *   C W Greenstreet, cloudSmoker2, 25Oct2026
 *   Licence (this file): GPLv3
 *
 *   Built on:
 *    1) RobTillaart/ADS1X15 library, licenced under MIT Licence
 *         reference:  https://github.com/RobTillaart/ADS1X15
 *    2) Median filter: cwg_MedianFilterLib2, derived from Luis Llamas/Arduino-MedianFilter,
 *         licenced under Apache Licence 2.0
 *         reference:
 * https://github-com.translate.goog/luisllamasbinaburo/Arduino-MedianFilter?_x_tr_sl=auto&_x_tr_tl=en&_x_tr_hl=en
 *    3) Gain table: taken from the Adafruit ADS1x15 library
 * ************************************************************* */

#ifndef CWG_ADS1x15_H
#define CWG_ADS1x15_H

#if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
#include <Arduino.h>
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block

/******************************************************
//   Required library
*******************************************************/
#include <ADS1X15.h>  // ADS1x15 I2C ADC device functionality

/******************************************************
// Class definition:
//   CWG_ADS1115 is child (derived) class of public library ADS1x15::ADS1115
******************************************************/

class CWG_ADS1115 : public ADS1115 {
 public:
  // constructor: device address, and optionally the Wire interface (defaults to &Wire)
  CWG_ADS1115(uint8_t Address = ADS1115_ADDRESS, TwoWire* wire = &Wire) : ADS1115(Address, wire) {}
  virtual ~CWG_ADS1115() {}

  // methods (function):
  void initialise(uint8_t gainSetting, uint8_t modeSetting, uint8_t dataRateSetting);
  float getVoltageFactor_V();
  float getSensorValue_MedianFiltered_V(uint8_t pin, int windowSize);

  // true only if the device acknowledged on the bus during initialise()
  bool deviceFound() const { return deviceOk; }

 protected:
  bool deviceOk = false;  // set by initialise()
};

// ensure objects are visible everywhere (global)
extern CWG_ADS1115 ads1115;

//==============================================================================================
// For reference:
//  ADS1X15 ADCs do not take an external reference voltage but rely on an internal full scale
//   voltage (fsv) that is a function of the PGA gain setting - see table below
//
//  The ADS1015 is a 12-bit ADC. The ADS1115 is a 16-bit ADC, so at the default
//   2/3x gain: 6.144V full scale / 32768 counts = 0.1875mV LSB
// -------------------------
// PGA gain settings, max Voltage and LSB (least significant bit)
//   taken from Adafruit ADS1x15 library, where bit = LSB
//                                                                ADS1015  ADS1115
//                                                                -------  -------
// ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
// ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
// ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
// ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
// ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
// ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
//==============================================================================================

#endif  // end header guard