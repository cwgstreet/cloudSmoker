/* ***************************************************************
 * cwg_steinhartHart.cpp - Library calculates temperature from NTC thermistor using SteinHart-Hart equation
 *    Ver 1			24 Feb 2022    MIT Licence
 *
 * key changes:  modified open-source code to be more generic and permit interface to ADS1x14 external ADCs
 *                all defaults from cloudSmoker Project (https://github.com/cwgstreet/cloudSmoker)
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
//         Bias Resistor   NTC Thermistor
//               ____           ____
//   +VCC o-----|____|----+----|____|----o GND
//   (5 V)       Rbias    | Rthermistor
//            (10E6 ohm)  |  (75E3 ohm)
//    ADC PIN             |
//      (A0) o------------+
//                      Vout
//
// *******************************************************

//? *************************************************************************
//? CWG additions / notes
//? ----------------------
//? Voltage divider
//? Vo = Rt / (Rt + Rb) * Vcc
//?   ->Vo=Voltage Output at resistor divider, Rt = thermistor resistance, Rb=bias resistor, Vcc = power supply voltage
//?  rearrange for Thermistor Resitance
//?   Rth = -(Vo - Rb)/(Vo - Vcc)
//?    in this program's variables:  r = - (voltage - Rbias)/(voltage -V_IN)
//?
//? ADC value = Rt / (Rt + Rb) * Vcc * ADB-bits / Varef  ->where Rt = NTC thermistor resistance, Rb=bias resistor resistance
//?  For ADS1015 with default PGA setting, max voltage = 6.144V, LSB = 3mV (6.144/2048)
//? *************************************************************************

#include "cwg_steinhartHart.h"

#include <math.h>
#include "cwg_ads1x15.h"

/**
 * Returns the temperature in kelvin for the given thermistor resistance value
 * using the Steinhart-Hart polynomial relationship.
 *   note: log in math.h is log base e or ln, not log(base 10) commonly written as log   
 */
double SteinhartHart::steinhartHart(double thermistorResistance_ohms) {
    double log_r = log(thermistorResistance_ohms);
    double log_r3 = log_r * log_r * log_r;

    return 1.0 / (_a + _b * log_r + _c * log_r3);  //Steinhart-Hart poloynomial relationship
}

double SteinhartHart::getTempKelvin() {
    double adc_raw = analogRead(_ADCpin);  
    double voltage = adc_raw / 1024 * V_IN;
    double resistance = ((1024 * _biasResistance / adc_raw) - _biasResistance);

    // Account for dissipation factor K
    return steinhartHart(resistance) - voltage * voltage / (K * _biasResistance);
}

double SteinhartHart::getTempCelsius() {
    return getTempKelvin() - 273.15;
}

double SteinhartHart::getTempFahrenheit() {
    return getTempCelsius() * 9 / 5 + 32;
}
