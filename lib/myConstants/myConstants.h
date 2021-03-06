/* ***************************************************************
 * myConstants.h - library listing all user-defined constants
 *
 *  C W Greenstreet, Ver1, 27Sep21
 *    MIT Licence - Released into the public domain
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
 *   A2         MEAT      Meat probe thermistor jack (measured 9.09k ohm 1% bias resistor - vs 1% 9.09k ohm design)
 *   A3         PIT       Pit probe thermistor jack  (measured 75.0k ohm 1% bias resistor - vs 1% 75k ohm design)
 *  */


//******************************************************
//? Constants
//*******************************************************/

// ---------------------------------------------------------
// Pin set-up listed below are for nodeMCU ESP8266
//? Note to self:  constexp better than const for variable values that should be known at compile
//?    time -> more memory efficient.  Also better than simple #define
//! cannog use "extern constexp", must use "const" instead, as with constexp "...it must be immediately constructed or assigned a value"
// ---------------------------------------------------------
constexpr int I2C_SCL = D1;      // optional for LCD as hd44780 auto-configures but needed for ADS1015
constexpr int I2C_SDA = D2;      // optional for LCD as hd44780 auto-configures but needed for ADS1015
constexpr int ENCODER_DT = D4;   // pinB newEncoder lib config
constexpr int ENCODER_CLK = D5;  // pinA newEncoder lib config
constexpr int BUTTON_PIN = D3;   // KY40 SW (switch) pin (connected to Uno pin 4)

// ---------------------------------------------------------
// ADS1015 ADC pin constants
// ---------------------------------------------------------
constexpr int ADC_VCCsupplyPin = 0;
//pin1 left floating (per datasheet) - no measurement
constexpr int ADC_meatPin = 2;
constexpr int ADC_pitPin = 3;


// ---------------------------------------------------------
// Baudrate:  Recommend 74480 baud rate for ESP8266 devices to match ESP8266 fixed bootloader initialisation speed
//  (otherwise you will get startup gibberish characters on serial monitor before serial speed syncs)
//  https://forum.arduino.cc/t/serial-monitor-only-shows-strange-symbols-arduino-mega-with-esp8266/640490/5
//  note: may have to manually reset board after flashing for code to work correctly
//#define SERIAL_MONITOR_SPEED 74880  // change to constexpr
constexpr int SERIAL_MONITOR_SPEED = 74880;

// *******************************************************
//   Thermistor Hardware setup
// *******************************************************
//
//         Bias Resistor     NTC Thermistor
//                ____           ___/_
//   +V_IN o-----|____|----+----|__/__|----o GND
//  (~5 V)                 |      /
//                Rbias    | Rthermistor
//      (75E3 / 9.1E3 ohm) |  (10E6 ohm R25)
//                         |
//        ADC PIN          |
//         (A0) o----------+
//                       Vout
//
// *******************************************************

constexpr double MEAT_BIAS_RESISTOR_Ohm = 75.0e3;  // replace with DMM measured actual value for improved accuracy
constexpr double PIT_BIAS_RESISTOR_Ohm = 9.1e3;

// *******************************************************
//   Other constants
// *******************************************************

// empirically determined temperature correction factors - applied in cwg_steinhartHart lib
constexpr double PIT_TEMP_OFFSET_DEGF = 8.0;
constexpr double MEAT_TEMP_OFFSET_DEGF = 2.2;

//empirical termperature offset correction factor
constexpr double MEAT_PROBE_OFFSET_degF = 2.0;
constexpr double PIT_PROBE_OFFSET_degF = 28.0;


#endif  // end header guard