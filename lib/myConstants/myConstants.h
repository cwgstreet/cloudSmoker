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

// TODO:  Consider moving these constants to platformio.ini build_flags for easier configuration of
// different hardware setups
/* ******************************************************
 *   Pin-out Summaries
 *     See github cloudSmoker wiki for schematic and breadboard hookup picture
 * *******************************************************/

/*  NodeMCU V1.0 (ESP12E) microcontroller pinout setup:
 *   Pin        Function  Comment
 *   -------    --------  -----------------------------------------------
 *   D0          LED      NodeMCU built-in LED
 *   D1 (GPIO5)  SCL      YwRobot LCD (via 4.7k ohm pullup resistor)  green
 *   D2 (GPIO4)  SDA      YwRobot LCD (via 4.7k ohm pullup resistor)  white
 *   D3          SW       KY40 Push switch    white
 *   D4 (GPIO2)  DT       KY40 (PinB)         yellow
 *   D5 (GPI014) CLK      KY40 (PinA)         green
 */

/* ADS1015 ADS pinout setup
 *   Pin        Function  Comment
 *   -------    --------  -----------------------------------------------
 *   VDD        ~5V       red //TODO Investigate benefits of driving off 3.3V
 *   GND        GND       black
 *   SCL        D1        green (via 4.7k ohm pullup resistor)
 *   SCA        D2        white (via 4.7k ohm pullup resistor)
 *   ADDR       GND       sets address at hex 0x48; configurable to other addresses
 *   ALRT       float     not used (float per datasheet)
 *   A0         VDD       monitor battery voltage and use for VDD in Steinhart-Hart calcs
 *   A1         n/a       no measurement - floating (per datasheet)
 *   A2         MEAT      Meat probe thermistor jack (measured 9.09k ohm 1% bias resistor - vs
 * 1% 9.09k ohm design) A3         PIT       Pit probe thermistor jack  (measured 75.0k ohm 1% bias
 * resistor - vs 1% 75k ohm design)
 *  */

//******************************************************
//? Constants
//*******************************************************/

// ---------------------------------------------------------
// Pin set-up mapping to platformio.ini build_flags
//
//? Allows different build environments to be set up for different hardware configurations
//?   (e.g. ESP32 vs ESP8266, or different pinouts for different breadboard layouts)
// ---------------------------------------------------------
//? Note to self:  constexp better than const for variable values that should be known at compile
//?    time -> more memory efficient.  Also better than simple #define
//! cannot use "extern constexp", must use "const" instead, as with constexp "...it must be
//! immediately constructed or assigned a value"
// ---------------------------------------------------------
// ---------------------------------------------------------
constexpr int I2C_SCL = PIN_I2C_SCL;
constexpr int I2C_SDA = PIN_I2C_SDA;
constexpr int ENCODER_DT = PIN_ENCODER_B;
constexpr int ENCODER_CLK = PIN_ENCODER_A;
constexpr int BUTTON_PIN = PIN_ENCODER_BTN;

// ---------------------------------------------------------
//? Baudrate: Updated for ESP32 native speed (consistent with platformio.ini build_flags)
// ---------------------------------------------------------
constexpr int SERIAL_MONITOR_SPEED = 115200;

// ---------------------------------------------------------
// ADS1015 ADC pin constants
// ---------------------------------------------------------
constexpr int ADC_VCCsupplyPin = 0;
// pin1 left floating (per datasheet) - no measurement
constexpr int ADC_meatPin = 2;
constexpr int ADC_pitPin = 3;

// *******************************************************
//   Thermistor Hardware setup
// *******************************************************
//
//         Bias Resistor     NTC Thermistor
//                ____           ___/_
//   +V_IN o-----|____|----+----|__/__|----o GND
//  (~5 V)                 |      /
//                Rbias    | Rthermistor
//      (75E3 / 120E3 ohm) |  (10E6 ohm R25)
//                         |
//        ADC PIN          |
//         (A0) o----------+
//                       Vout
//  TODO:  confirm ADS1115 ADC PIN
// *******************************************************
/// Per cloudSmoker2 redesign, single bias resistor value shared by all thermistor
//    channels (pit + meat), ref: schematic PR1-PR4: 120k, 0.1% thin-film.
//    cloudSmoker1 used separate bias resistors per probe type (meat=75k, pit=9.1k);
//    cloudSmoker2 consolidated to one shared value. Same 120k used on the breadboard test rig.
//
// These are 0.1% precision resistors - use the nominal value as-is. A typical DMM's
//    own measurement accuracy (~0.5-1%) is worse than the resistor's guaranteed tolerance,
//    so measuring in-circuit would add error rather than remove it.
constexpr double BIAS_RESISTOR_Ohm = 120.0e3;

// *******************************************************
//   Other constants
// *******************************************************

//! NOT USED:  facility to add correction factors to Steinhart-Hart derrived temps
// TODO:  confirm these values with actual measurements and update as needed
// empirically determined temperature correction factors - applied in cwg_steinhartHart lib
// constexpr double PIT_TEMP_OFFSET_DEGF = 8.0;
// constexpr double MEAT_TEMP_OFFSET_DEGF = 2.2;

// TODO:  confirm these values with actual measurements and update as needed
// empirical temperature offset correction factor
// constexpr double MEAT_PROBE_OFFSET_DEGF = 2.0;
// constexpr double PIT_PROBE_OFFSET_DEGF = 28.0;

#endif  // end header guard