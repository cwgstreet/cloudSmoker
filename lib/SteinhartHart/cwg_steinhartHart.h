/* ***************************************************************
 * cwg_steinhartHart.h - Library calculates temperature from NTC thermistor using SteinHart-Hart equation
 *    Ver 1			24 Feb 2022    MIT Licence
 *
 * key changes:  modified open-source code to be more generic and permit interface to ADS1x14 external ADCs
 *                all defaults from cloudSmoker Project (https://github.com/cwgstreet/cloudSmoker)
 *    Note that original library has error in temperature compensation for self-heating using k-factor
 *     see https://github.com/fiendie/SteinhartHart/issues/3
 *
 *  Tool to get SHH coefficients: https://www.thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html
 *
 * -----------------------------------------------------------------------------
 * Modified from SteinhartHart.h - Library for interacting with NTC thermistors
 *   Author: Andreas Tacke (fiende)
 *   Ref: https://github.com/fiendie/SteinhartHart
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
 *  * -----------------------------------------------------------------------------
 */

// *******************************************************
//   Thermistor Hardware setup
// *******************************************************
//
//         Bias Resistor     NTC Thermistor
//                ____           ___/_
//   +V_IN o-----|____|----+----|__/__|----o GND
//  (~5 V)                 |      /
//             Rbias       |   Rthermistor
//      (75E3 / 9.1E3 ohm) |  (10E6 ohm R25)
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

#include <inttypes.h>  // stabdardised library of integer types

#include "myConstants.h"

class SteinhartHart {
   public:
    /**
     * Constructor: SteinhratHart(biasResistorValue_ohm, a, b, c)
     *
     * Only the bias resistor value has to be specified in the constuctor.
     * If no other parameters are given default values will be used based
     *   on a Mavrick-723 temperature probe and cloudSmoker project design parmeters
     */

    SteinhartHart(
        double biasResistorValue_ohm,
        double a = 6.308217444e-4,
        double b = 1.836278370e-4,
        double c = 7052387818e-8) : _biasResistance{biasResistorValue_ohm},
                                    _a{a},
                                    _b{b},
                                    _c{c} {};

    //  public member functions:
    double getTempKelvin(double ADCmeasuredVCC_volts, double ADCmeasuredThermistor_volts);
    double getTempCelsius(double ADCmeasuredVCC_volts, double ADCmeasuredThermistor_volts);
    double getTempFahrenheit(double ADCmeasuredVCC_volts, double ADCmeasuredThermistor_volts);

   private:
    double steinhartHart(double _Rth_ohm);

    //  Thermistor voltage divider
    double _biasResistance;  //  bias resistor value
    double _Vadc;            //  probe voltage measured by ADC (typically median filtered)
    double _Vsupply;  //  supply voltage to voltage divider
    double _Rth_ohm;         //  NTC thermistor resistance

    // Manufacturing constants
    double _a;
    double _b;
    double _c;
};

// ensure objects are visable everywhere (global)
extern SteinhartHart sh_meatProbe;  // meat thermometer
extern SteinhartHart sh_pitProbe;   // meat thermometer

// make external voltage reading variables declared elsewhere visible in this library
extern double voltageMeat_medianFiltered_V;
extern double voltagePit_medianFiltered_V;
extern double voltageVCC_medianFiltered_V; 

#endif  // end header guard