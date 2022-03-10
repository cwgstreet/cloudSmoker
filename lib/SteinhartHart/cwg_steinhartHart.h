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
     * Constructor: SteinhratHart(ADCpin, biasResistorValue_ohm, a, b, c)
     *
     * Only the ADC reading pin and bias resistor values have to be specified in the constuctor.
     * If no other parameters are given default values will be used
     *   from a Mavrick-723 temperature probe and cloudSmoker project design
     */
    SteinhartHart(
        uint8_t ADCpin,
        double biasResistorValue_ohm,
        double a = 0.625773640e-3,
        double b = 1.842254690e-4,
        double c = 6.94265460e-8) : _ADCpin(ADCpin),
                                    _Vin(V_IN_Volt),
                                    _biasResistance(biasResistorValue_ohm),
                                    _a(a),
                                    _b(b),
                                    _c(c){};

    double getTempKelvin(double VmeasuredADC_V);
    double getTempCelsius(double VmeasuredADC_V);
    double getTempFahrenheit(double VmeasuredADC_V);

   private:
    double steinhartHart(double _Rth_ohm);

    // Constructor prrivate members - Thermistor voltage divider
    uint8_t _ADCpin;         //  ADS1015 ADC Pin (0 to 3)
    double _Vin;             //  supply voltage to voltage divider
    double _biasResistance;  //  bias resistor value
    double _Rth_ohm;         //  NTC thermistor resistance

    double _Vadc;  //  voltage measured by ADC (typically median filtered)

    // Manufacturing constants
    double _a;
    double _b;
    double _c;
};

// ensure objects are visable everywhere (global)
extern SteinhartHart sh_meatProbe;  // meat thermometer
extern SteinhartHart sh_pitProbe;   // meat thermometer

#endif  // end header guard