/* ***************************************************************
 * cwg_steinhartHart.cpp - Library calculates temperature from NTC thermistor using SteinHart-Hart equation
 *    Ver 1			24 Feb 2022  	MIT Licence
 *
 * key changes:  modified open-source code to be more generic and permit interface to ADS1x14 external ADCs
 *                all defaults from cloudSmoker Project (https://github.com/cwgstreet/cloudSmoker)
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

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#ifndef SteinhartHart_h
#define SteinhartHart_h

#include <inttypes.h>

#define V_IN 5.0  // Input voltage
#define K 0.6     // Dissipation factor (mW/°C)

class SteinhartHart {
   public:
    /**
     * Constructor: SteinhratHart( ADCpin, biasResistorValue_ohm, a, b, b)
     *
     * Only the ADC reading pin has to be specified in the constuctor.
     * If no other parameters are given default values will be used
     *   from a Mavrick-723 temperature probe and cloudSmoker project design
     */
    SteinhartHart(
        uint8_t ADCpin,
        double biasResistorValue_ohm = 75.0e3,
        double a = 0.62577364e-3,
        double b = 1.84225469e-4,
        double c = 0.69426546e-7) : _ADCpin(ADCpin),
                                    _biasResistance(biasResistorValue_ohm),
                                    _a(a),
                                    _b(b),
                                    _c(c){};

    double getTempKelvin();
    double getTempCelsius();
    double getTempFahrenheit();

   private:
    double steinhartHart(double);

    // Value of the bias resistor put in parallel
    double _biasResistance;

    uint8_t _ADCpin;

    // Manufacturing constants
    double _a;
    double _b;
    double _c;
};

#endif