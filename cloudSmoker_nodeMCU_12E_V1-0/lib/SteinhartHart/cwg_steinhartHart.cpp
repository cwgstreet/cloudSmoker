/* ***************************************************************
 * cwg_steinhartHart.cpp - Library calculates temperature from NTC thermistor using SteinHart-Hart equation
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
//   Hardware setup
// *******************************************************
//
//         Bias Resistor     NTC Thermistor
//                ____           ____
//   +V_IN o-----|____|----+----|____|----o GND
//  (~5 V)        Rbias    | Rthermistor
//             (10E6 ohm)  |  (75E3 ohm)
//    ADC PIN              |
//      (A0) o-------------+
//                       Vout
//
// *******************************************************

//? *************************************************************************
//? CWG additions / notes
//? ----------------------
//? Voltage divider equation for thermistor configuration above
//? Vo = Rt / (Rt + Rb) * V_IN
//?   ->Vo = Voltage Output at resistor divider (measured by ADC), Rth = thermistor resistance,
//?       Rb=bias resistor, V_IN = power supply voltage to resistor divider
//?
//?  rearrange to solve for Thermistor Resistance, Rth
//?   Rth = -[(Vo * Rb) / (Vo - V_IN)]   -> don't miss the negative sign!
//? *************************************************************************

#include "cwg_steinhartHart.h"

#include <math.h>
#include <myConstants.h>  // all constants together in one file

#include "cwg_ads1x15.h"

// --------------------------
//   steinhartHart() function purpose: Returns the temperature in degrees kelvin for the given thermistor resistance
//      value using the Steinhart-Hart polynomial relationship
// --------------------------
double SteinhartHart::steinhartHart(double _Rth_ohm) {
    double log_r = log(_Rth_ohm);  // log operation in math.h is log(base e) or ln, not log(base 10)
    double log_r3 = log_r * log_r * log_r;

    return 1.0 / (_a + _b * log_r + _c * log_r3);  // Steinhart-Hart poloynomial relationship, returns temp in deg K
}

//?  rearrange to solve for Thermistor Resistance, Rth
//?   Rth = -(Vo * Rb)/(Vo - Vcc)   -> don't miss the negative sign!


// --------------------------
//   getTempKelvin() function purpose: Returns the temperature in degrees kelvin for the given thermistor resistance
//      value using the Steinhart-Hart polynomial relationship
// --------------------------
double SteinhartHart::getTempKelvin(double VmeasuredADC_V) {
    _Rth_ohm = - ( (VmeasuredADC_V * _biasResistance) / (VmeasuredADC_V - _Vin) );
    return steinhartHart(_Rth_ohm);
}

double SteinhartHart::getTempCelsius() {
    return getTempKelvin() - 273.15;
}

double SteinhartHart::getTempFahrenheit() {
    return getTempCelsius() * 9 / 5 + 32;
}
