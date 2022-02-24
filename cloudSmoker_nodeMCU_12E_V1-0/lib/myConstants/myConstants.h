/* ***************************************************************
 * myConstants.h - library listing all user-defined constants
 *
 *  C W Greenstreet, Ver1, 27Sep21
 *    MIT Licence - Released into the public domain
 *
 ** ************************************************************* */

#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

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
 *   VDD        5V        red //TODO Investigate benefits of driving off 3.3V
 *   GND        GND       black
 *   SCL        D1        green (via 4.7k ohm pullup resistor)
 *   SCA        D2        white (via 4.7k ohm pullup resistor)
 *   ADDR       GND       sets address at hex 0x48; configurable to other addresses
 *   ALRT       float     not used (float per datasheet)
 *   A0         5V        monitor battery voltage
 *   A1         GND
 *   A2         PIT       Pit probe thermistor jack  (measured 81.2 kohm 5% bias resistor - vs 1% 75kohm design)
 *   A3         MEAT      Meat probe thermistor jack (measured 9.96 kohm 1% bias resistor - vs 1% 9.09kohm design)
 */


/******************************************************
// constants
*******************************************************/

// Pin set-up listed below are for nodeMCU ESP8266
//? Note to self:  constexp better than const for variable values that should be known at compile
//?    time -> more memory efficient.  Also better than simple #define
//! not allowed extern constexp, must use const as with constexp " it must be immediately constructed or assigned a value"

constexpr int I2C_SCL = D1;  // optional for LCD as hd44780 auto-configures but needed for ADS1015
constexpr int I2C_SDA = D2;  // optional for LCD as hd44780 auto-configures but needed for ADS1015

constexpr int ENCODER_DT = D4;   // pinB newEncoder lib config
constexpr int ENCODER_CLK = D5;  // pinA newEncoder lib config
constexpr int BUTTON_PIN = D3;   // KY40 SW (switch) pin (connected to Uno pin 4)

// Baudrate:  Recommend 74480 baud rate for ESP8266 devices to match ESP8266 fixed bootloader initialisation speed
//  (otherwise you will get startup gibberish characters on serial monitor before serial speed syncs)
//  https://forum.arduino.cc/t/serial-monitor-only-shows-strange-symbols-arduino-mega-with-esp8266/640490/5
//  note: may have to manually reset board after flashing for code to work correctly
#define SERIAL_MONITOR_SPEED 74880

#endif  // end header guard