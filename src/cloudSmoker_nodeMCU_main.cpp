/* *************************************************************************************
 *    cloudSmoker_nodeMCU_main
 *    V2.NodeMCU   (ported from UNO Rev3 to NodeMCU V1.0 (v2) (ESP12E based) )
 *    Carl Greenstreet, Aug 2021,
 *    Licence: MIT (do what you want with this code but no functionality guarantees!)
 *
 *    Purpose:  cloudSmoker Barbecue Temperature Monitor
 *
 *    Project Repository:  https://github.com/cwgstreet/cloudSmoker
 *    Project Wiki:        https://github.com/cwgstreet/cloudSmoker/wiki
 *
 *    Code intended to work with the following hardware
 *      ESP8266
 *         (initially developed on Uno Rev3 (while waiting on a replacement NodeMCU
 *           after letting the magic smoke out!)
 *         NodeMCU V1.0 (ESP12) development board (will port over to bare bones ESP8266
 *          once up, running and debugged)
 *      YwRobot 1602 LCD with i2c i/o exapander backpack (PCF8574 or MCP23008)
 *      KY40 Rotary Encoder
 *      ADS1015 - 12 bit ADC; I2C addressible
 *      Maverick brand ET-723/733 (NPN  thermistor) temperature probes
 *
 *    3rd Party Libraries / Credits:
 *      NewEncoder Library https://github.com/gfvalvo/NewEncoder - best rotary
 *        encoder library I've come across; uses State Machine approach to software
 *        "debounce" encoder
 *      Bounce2 Library  https://github.com/thomasfredericks/Bounce2  use for debouncing rotary
 *         encoder switch
 *      Yabl (Yet another button library for Arduino) - https://github.com/yergin/Yabl
 *          extends Bounce2 library by adding easy functionality to determine press
 *          types (long, short and double press) plus trigger callback functions
 *      hd44780 Libary https://github.com/duinoWitchery/hd44780 - extensible
 *         Arduino LCD library for hd44780 based LCD displays
 *      state machine aduino demo -
 *         https://bitbucket.org/pierrerossel/state-machine-arduino-demo/src/master/
 *         public domain C++ State Machine demo code by Pierre Rossel
 *
 *    Revisions:
 *      2021.12.31    V1 partially complete but ran into Uno 2kb SRAM memory limitation wall
 *      2022.01.02    V2 ported code to ESP8266 (NodeMCU V1.0 ESP12E 80kb!)
 *
 * ************************************************************************************
 */

/* ******************************************************
 *   Pin-out Summaries
 *     Refer to myConstants.lib for pin-out table plus also see github cloudSmoker wiki
 *       for device schematic and breadboard hookup picture
 * *******************************************************/

// external libraries:
#include <ADS1X15.h>  //Arduino library for I2C ADC ADS1x15 devices https://github.com/RobTillaart/ADS1X15
#include <Arduino.h>  //platformio IDE requires Arduino framework to be explicitly included
#include <Bounce2.h>
#include <ESP8266WiFi.h>
#include <MemoryFree.h>  // https://github.com/maniacbug/MemoryFree & https://playground.arduino.cc/Code/AvailableMemory/
#include <NewEncoder.h>
#include <WiFiManager.h>                    // https://github.com/tzapu/WiFiManager
#include <Wire.h>                           // must include before hd44780 libraries due to dependencies
#include <hd44780.h>                        // LCD library
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

#include "ThingSpeak.h"  // always include thingspeak header file after other header files and custom macros

// internal (user) libraries:
#include "cwg_ads1x15.h"        // ADS1x15 I2C ADC device functionality
#include "cwg_steinhartHart.h"  // Thermistor steinhart hart temperature calcs
#include "cwg_wifi.hpp"
#include "lcd.h"           // lcd function tests, helper functions and custom characters
#include "myConstants.h"   // all constants in one file
#include "periphials.h"    // serial monitor function tests and usuage routines
#include "press_type.h"    // wrapper library abstracting Yabl / Bounce2 routines
#include "secrets.h"       //  confidential informtion - usernames, passwords, API keys etc
#include "smokerStates.h"  // cloudSmoker state machine functionality
#include "wrapEncoder.h"   //  encoder library including encoder object with min / max values that "wrap" around

//? **************  Selective Debug Scaffolding *********************
// Set up selective debug scaffold; comment out appropriate lines below to disable debugging tests at pre-proccessor stage
//   Note: #ifdef preprocessor simply tests if the symbol's been defined; therefore don't use #ifdef 0
//   Ref: https://stackoverflow.com/questions/16245633/ifdef-debug-versus-if-debug
//? *****************************************************************
//#define DEBUG_SERIAL 1        // uncomment to debug - Serial monitor function test
//#define DEBUG_LCD 1           // uncomment to debug - LCD function test
//#define DEBUG_PRESSTYPE  1    // uncomment to debug rotary encoder button press type function test
//#define DEBUG_LED  1          // uncomment to debug LED test of rotary encoder  **CHECK THIS MISSING??**
//#define DEBUG_FREEMEM 1       // uncomment to debug remaining free memory
//?  end Selective Debug Scaffolding *********************************


//? *******************************************************************
//?   Global variables
//? *******************************************************************

// temperature variables- global; all temps stored in degF and converted on the fly as necessary for alternative units (eg DegC)
float meatDoneTemp = 203;    // default to usual brisket internal done temp 203degF
float pitTempTarget = 225;   // reasonable range around 225F long and slow target, pit temps can run 200 to 350 deg F
float currentMeatTemp = 40;  // current meat temp; default to refridgerator temp degF
float currentPitTemp = 60;  // current pit temp; default to cold pit
bool degCFlag = 0;           // temperature unit flag: 1 for Centigrade or 0 for Fahrenheit (default)

// other Globals
// float batteryVoltage = 999;  // 999 as null proxy
float voltageFactor = 0;  // dependent on ADS1015 settings used
double voltageVCC_medianFiltered_V;
double voltagePit_medianFiltered_V;
double voltageMeat_medianFiltered_V;

/* *****
TODO:  add 4-hour rule check and exception notification
The 4-hour rule is a general food safety guideline that suggests that the internal temperature of meat
should increase from 40°F to 140°F within 4 hours. This is due to foodborne bacteria growing much faster
within this temperature range (known as the “danger zone”).  Ref https://www.totallysmokin.com/4-hour-rule-smoking/
***** */

// timing variables - global
//unsigned long nowMillis_ms;
long unsigned startCookTime_ms;

// run-once flag to use in functions called in loop - global
bool hasRunFlag = 0;

// WrapEncoder globals - move to libary?
int16_t prevEncoderValue;
// int16_t currentEncoderValue;

// debug code for testing - then can remove
//   int loopcounter = 0;

void setup() {
    Serial.begin(SERIAL_MONITOR_SPEED);
    lcd.initialiseLCD();
    lcd.initialiseCustomCharSet();  // creates eight custom lcd charaters
    encoder.initialise();
    // delay(100);  //! *** TEST given blocking *** is delay necessary to clear serial buffer in encoder.initialise(); otherwise garbage characters
    WifiManager_initialise();

    // initialise button press_type set-up code (pin, pullup mode, callback function)
    button.begin(BUTTON_PIN);

    smokerState = splashScreen;  // initial state for state machine; temporarily disable during testing

    // configure ADS1015 I2C ADC
    const uint8_t GAIN_SETTING = 0;       // 2/3x gain, max (ful scale) voltage: ±6.144V, 1 bit(LSB) = 3mV
    const uint8_t MODE_SETTING = 1;       // single shot
    const uint8_t DATA_RATE_SETTING = 4;  // default 1660 samples/sec
    ads1015.initialise(GAIN_SETTING, MODE_SETTING, DATA_RATE_SETTING);

    Serial.println();
    Serial.print(F("VCC A0\t|\tPIT A2\t|\tPitdegF\t|\tMEAT A3\t|\tMeatdegF"));
    Serial.println();

// ***************************
// ** Debug - function tests
// ***************************
// *****  debug - Serial monitor periphial function tests *****
#ifdef DEBUG_SERIAL
    SerialTerminal.functionTest();
#endif  // *****  end debug periphial function tests *****

#ifdef DEBUG_LCD  // *****  debug - LCD function tests *****
    // LCD special character function test
    lcd.displayTest();
#endif  // *****  end debug LCD function tests *****

#ifdef DEBUG_PRESSTYPE  // *****  debug - button press_type function tests *****
    button.functionTest();
#endif  // *****  end button press_type function tests *****


}  // end of setup

void loop() {
    yield();
    button.update();

    processState(lcd);  // temporarily disable for testing, as needed
    // encoder.getCount();  // need to enable this if line above is commented out for testing

/*
#ifdef DEBUG_ADC  // *****  debug - memory *****

        // not working - but once debugged, should show memory stats to determine if there is a leak, etc.
        static uint32_t myfree;
        static uint16_t mymax;
        static uint8_t myfrag;
        ESP.getHeapStats(&myfree, &mymax, &myfrag);


#endif  // end DEBUG
*/

/*
// debug code - serial print out prevEncoderValue every n loops
loopcounter = loopcounter + 1;
 if ((loopcounter % 10) == 0) {
     Serial.println();
     Serial.print(F("    >>>> Loop: -> prevEncoderValue = "));
     Serial.println(prevEncoderValue);
     Serial.println();
 }
 */

// *****  debug - free memory check  *****
#ifdef DEBUG_FREEMEM
    Serial.print(F("freeMemory()="));
    Serial.println(freeMemory());
    delay(1000);
#endif  // end DEBUG
    // *****  end debug free memory check *****

}  // end of loop