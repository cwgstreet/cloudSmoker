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
     * Constructor: SteinhratHart(biasResistorValue_ohm, voltageProbe_volt, a, b, c)
     *
     * Only the ADC reading pin and bias resistor values have to be specified in the constuctor.
     * If no other parameters are given default values will be used
     *   from a Mavrick-723 temperature probe and cloudSmoker project design
     */

    SteinhartHart() = delete; // forbid the default constructor   

    SteinhartHart(
        double biasResistorValue_ohm,
        double voltageProbe_volt,
        // double V_IN_Volt,
        double a = 0.625773640e-3,
        double b = 1.842254690e-4,
        double c = 6.94265460e-8) : _biasResistance{biasResistorValue_ohm},
                                    _Vadc{voltageProbe_volt},
                                    //_Vin{V_IN_Volt},
                                    _a{a},
                                    _b{b},
                                    _c{c} {
        // debug code
        Serial.println();
        Serial.println(F("=========SteinhartHart constructor applied ============="));
        /* Serial.println(F(" _Rth_ohm \t _Vadc \t _biasResistance \t _Vin"));
        Serial.print(_Rth_ohm);
        Serial.print(F("\t"));
        Serial.print(_Vadc);
        Serial.print(F("\t"));
        Serial.print(_biasResistance);
        Serial.print(F("\t"));
        Serial.println(_Vin); */
    };

    double getTempKelvin();
    double getTempCelsius();
    double getTempFahrenheit();

   private:
    double steinhartHart(double _Rth_ohm);

    //  Thermistor voltage divider
    double _biasResistance;  //  bias resistor value
    double _Vadc;            //  probe voltage measured by ADC (typically median filtered)
    double _Vin{V_IN_Volt};             //  supply voltage to voltage divider
    double _Rth_ohm;         //  NTC thermistor resistance

    // Manufacturing constants
    double _a;
    double _b;
    double _c;
};

// ensure objects are visable everywhere (global)
extern SteinhartHart sh_meatProbe;  // meat thermometer
extern SteinhartHart sh_pitProbe;   // meat thermometer

// make external voltage readings variables visible in library
extern double voltageMeat_medianFiltered_V;
extern double voltagePit_medianFiltered_V;

#endif  // end header guard