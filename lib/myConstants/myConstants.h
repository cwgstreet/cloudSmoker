/* ***************************************************************
 * myConstants.h - library listing all user-defined constants
 *
 *  C W Greenstreet, cloudSmoker2 (Ver2), 27Jun26
 *    Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *           distribution must also share the full source code under this same license.
 *
 ** ************************************************************* */

#ifndef MY_CONSTANTS_H
#define MY_CONSTANTS_H

#if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
#include <Arduino.h>
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block

//******************************************************
//? Constants
//*******************************************************/

// ---------------------------------------------------------
//! Pin set-up mapping moved to platformio.ini build_flags
//
//? Allows different build environments to be set up for different hardware configurations
//?   (e.g. ESP32 vs ESP8266, or different pinouts for different breadboard layouts)
// ---------------------------------------------------------
//? Note to self:  constexp better than const for variable values that should be known at compile
//?    time -> more memory efficient.  Also better than simple #define
//! Caveat: cannot use "extern constexp", must use "const" instead, as with constexp
//!   "...it must be immediately constructed or assigned a value"
// ---------------------------------------------------------
// ---------------------------------------------------------
constexpr int I2C_SCL = PIN_I2C_SCL;
constexpr int I2C_SDA = PIN_I2C_SDA;
constexpr int ENCODER_DT = PIN_ENC_DT;
constexpr int ENCODER_CLK = PIN_ENC_CLK;
constexpr int BUTTON_PIN = PIN_ENC_SW;

// ---------------------------------------------------------
//? Baudrate: Updated for ESP32 native speed (consistent with platformio.ini build_flags)
// ---------------------------------------------------------
constexpr int SERIAL_MONITOR_SPEED = 115200;

// ---------------------------------------------------------
// ADS1115 ADC channel constants (schematic: PIT=AIN0, MEAT1=AIN1, MEAT2=AIN2, MEAT3=AIN3)
//  TODO: move ADC pins to platformio.ini?
// ---------------------------------------------------------
constexpr uint8_t ADC_pitPin = 0;    // AIN0
constexpr uint8_t ADC_meat1Pin = 1;  // AIN1
constexpr uint8_t ADC_meat2Pin = 2;  // AIN2
constexpr uint8_t ADC_meat3Pin = 3;  // AIN3
// VCC supply measurement removed: obsolete from Ver1, supply reference now from INA226 bus voltage
//
//? ADC Measurement limits to avoid erroneous readings:
//    Usable divider range as a fraction of the bias supply (Vadc / Vcc)
//    Below MIN: unplugged, shorted or noise. Above MAX: open circuit.
constexpr double ADC_PROBE_MIN_FRACTION = 0.01;  // about 0.05 V on a 5 V rail
constexpr double ADC_PROBE_MAX_FRACTION = 0.99;  // about 4.95 V on a 5 V rail

//! TEMPORARY: fixed bias-supply reference to 5V until the INA226 bus-voltage reading replaces it
constexpr double ADC_biasSupplyInterim_V = 5.0;

// *******************************************************
//   Thermistor Hardware setup
// *******************************************************
//
//         Bias Resistor     NTC Thermistor
//                ____           ___/_
//   +V_IN o-----|____|----+----|__/__|----o GND
//  (~5 V)                 |      /
//                Rbias    | Rthermistor
//       (120E3 ohm  0.1%) |  (10E6 ohm R25)
//                         |
//        ADC PIN          |
//         (A0) o----------+
//                       Vout
//  TODO:  confirm ADS1115 ADC PIN
// *******************************************************
/// Per cloudSmoker2 redesign, single bias resistor value shared by all thermistor
//    channels (pit + meat), ref: schematic PR1-PR4: 120k, 0.1% thin-film.
//    cloudSmoker1 used separate bias resistors per probe type (meat=75k ohm, pit=9.1k ohm);
//    cloudSmoker2 consolidated to single shared value. Same 120k used on the breadboard test rig.
//
//! These are 0.1% precision resistors - use the nominal value as-is. A typical DMM's
//!   own measurement accuracy (~0.5-1%) is worse than the resistor's guaranteed tolerance,
//!   so measuring in-circuit would add error rather than remove it.
constexpr double BIAS_RESISTOR_Ohm = 120.0e3;

// *******************************************************
//   Other constants
// *******************************************************

//! NOT YET USED:  facility to add correction factors to Steinhart-Hart derrived temps
//   will implement in cloudSmoker2
// TODO:  confirm these values with actual measurements and update as needed
// empirically determined temperature correction factors - applied in cwg_steinhartHart lib
// constexpr double PIT_TEMP_OFFSET_DEGF = 8.0;
// constexpr double MEAT_TEMP_OFFSET_DEGF = 2.2;

// TODO:  confirm these values with actual measurements and update as needed
// empirical temperature offset correction factor
// constexpr double MEAT_PROBE_OFFSET_DEGF = 2.0;
// constexpr double PIT_PROBE_OFFSET_DEGF = 28.0;

#endif  // end header guard