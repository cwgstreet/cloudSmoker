/* ***************************************************************
 * cwg_steinhartHart.h - library calculates temperature from an NTC thermistor
 *   using the Steinhart-Hart equation
 *
 *   C W Greenstreet, cloudSmoker2, 16Nov2026
 *   Licence (this file): GPLv3. The upstream notice (GPL v2 or any later
 *   version) is kept in full below.
 *
 *   Origin:
 *    1) SteinhartHart library by Andreas Tacke (fiende), GPL v2 or any later version
 *         https://github.com/fiendie/SteinhartHart
 *    2) cloudSmoker1 (two probe channels), modified by C W Greenstreet. Key changes:
 *         made the code more generic and added an interface to external ADS1x14 ADCs.
 *         Defaults come from the cloudSmoker project
 *         (https://github.com/cwgstreet/cloudSmoker).
 *         The upstream library has an error in the self-heating temperature
 *         compensation (k-factor); see https://github.com/fiendie/SteinhartHart/issues/3
 *    3) cloudSmoker2 (four probe channels): extended with AI assistance (Claude).
 *         Under review by the author.
 *
 *   Thermistor datasheet (Maverick ET-732/733 probes, Semitec 105GT-2 NTC,
 *   R25 = 1M ohm):
 *     https://github.com/cwgstreet/cloudSmoker/blob/main/Hardware/Thermistor%20datasheet/Maverick%20ET-732%20-%20733%20(Semitec%20105GT-2%20Thermistor)%20datasheet.pdf
 *
 *   Tool to get Steinhart-Hart coefficients:
 *     https://www.thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html
 *
 *   This version is distributed under GPLv3, which the upstream 'or any later
 *   version' wording allows.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * *************************************************************** */

// *******************************************************
//   Thermistor Hardware setup
// *******************************************************
//
//         Bias Resistor     NTC Thermistor
//                ____           ___/_
//   +V_IN o-----|____|----+----|__/__|----o GND
//  (~5 V)                 |      /
//             Rbias       |   Rthermistor
//      (120E3 ohm) |  (1E6 ohm R25)
//                         |
//        ADC PIN          |
//         (A0) o----------+
//                       Vout
//
// *******************************************************
#ifndef SteinhartHart_h
#define SteinhartHart_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block

#include <inttypes.h>  // standardised library of integer types

#include "myConstants.h"

class SteinhartHart {
 public:
  /**
   * Constructor: SteinhartHart(biasResistorValue_ohm, a, b, c)
   *
   * Only the bias resistor value has to be specified in the constructor.
   * If no other parameters are given default values will be used based
   *   on a Maverick ET-732/733 probe (Semitec 105GT-2 NTC, R25 = 1M ohm) and cloudSmoker project design parameters
   */

  SteinhartHart(double biasResistorValue_ohm, double a = 0.0006308217444, double b = 0.000183627837,
                double c = 0.00000007052387818)
      : _biasResistance(biasResistorValue_ohm), _a(a), _b(b), _c(c) {};

  //  public member functions:
  double getTempKelvin(double ADCmeasuredVCC_volts, double ADCmeasuredThermistor_volts);
  double getTempCelsius(double ADCmeasuredVCC_volts, double ADCmeasuredThermistor_volts);
  double getTempFahrenheit(double ADCmeasuredVCC_volts, double ADCmeasuredThermistor_volts);

 private:
  // private member function
  double steinhartHart(double rThermistor_ohm);

  //  Thermistor voltage divider variables
  double _biasResistance;  //  bias resistor value
  double _vAdc_V;            //  probe voltage measured by ADC (typically median filtered)
  double _vSupply_V;         //  supply voltage to voltage divider
  double _rThermistor_ohm;         //  NTC thermistor resistance

  // Manufacturing constants
  double _a;
  double _b;
  double _c;
};

// ensure objects are visible everywhere (global)
extern SteinhartHart sh_pitProbe;    // pit thermometer (AIN0)
extern SteinhartHart sh_meat1Probe;  // meat thermometer, MEAT1 (AIN1)
extern SteinhartHart sh_meat2Probe;  // meat thermometer, MEAT2 (AIN2)
extern SteinhartHart sh_meat3Probe;  // meat thermometer, MEAT3 (AIN3)

// make external voltage reading variables declared elsewhere visible in this library
extern double voltageMeat1_medianFiltered_V;
extern double voltagePit_medianFiltered_V;
extern double voltageMeat2_medianFiltered_V;
extern double voltageMeat3_medianFiltered_V;

#endif  // end header guard