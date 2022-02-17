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

// external libraries:
#include <ADS1X15.h>  //Arduino library for I2C ADC ADS1x15 devices https://github.com/RobTillaart/ADS1X15
#include <Arduino.h>  //platformio IDE requires Arduino framework to be explicitly included
#include <Bounce2.h>
#include <MemoryFree.h>  // https://github.com/maniacbug/MemoryFree & https://playground.arduino.cc/Code/AvailableMemory/
#include <NewEncoder.h>
#include <Wire.h>                           // must include before hd44780 libraries due to dependencies
#include <hd44780.h>                        // LCD library
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

// internal (user) libraries:
#include <lcd.h>           // lcd function tests, helper functions and custom characters
#include <periphials.h>    // serial monitor function tests and usuage routines
#include <press_type.h>    // wrapper library abstracting Yabl / Bounce2 routines
#include <smokerStates.h>  // cloudSmoker state machine functionality
#include <wrapEncoder.h>   //  encoder library including encoder object with min / max values that "wrap" around

// *****  Selective Debug Saffolding *****
// Set up selective debug scaffold; comment out appropriate lines below to disable debugging tests at pre-proccessor stage
//   Note: #ifdef preprocessor simply tests if the symbol's been defined; therefore don't use #ifdef 0
//   Ref: https://stackoverflow.com/questions/16245633/ifdef-debug-versus-if-debug
// *****************************************************************
//#define DEBUG_SERIAL 1  // uncomment to debug - Serial monitor function test
//#define DEBUG_LCD 1  // uncomment to debug - LCD function test
//#define DEBUG_PRESSTYPE  1  // uncomment to debug rotary encoder button press type function test
//#define DEBUG_LED  1       // uncomment to debug LED test of rotary encoder  **CHECK THIS MISSING??**
//#define DEBUG_FREEMEM 1  // uncomment to debug remaining free memory

// Pin set-up listed below are for nodeMCU ESP8266
//? Note to self:  constexp better than const for variable values that should be known at compile
//?    time -> more memory efficient.  Also better than simple #define
// constexpr int I2C_SCL = D1;      // optional as hd44780 set to auto-configure
// constexpr int I2C_SDA = D2;      // optional as hd44780 set to auto-configure
constexpr int ENCODER_DT = D4;   // pinB newEncoder lib config
constexpr int ENCODER_CLK = D5;  // pinA newEncoder lib config
constexpr int BUTTON_PIN = D3;   // KY40 SW (switch) pin (connected to Uno pin 4)

// Baudrate:  Recommend 74480 baud rate for ESP8266 devices to match ESP8266 fixed bootloader initialisation speed
//  (otherwise you will get startup gibberish characters on serial monitor before serial speed syncs)
//  https://forum.arduino.cc/t/serial-monitor-only-shows-strange-symbols-arduino-mega-with-esp8266/640490/5
//  note: may have to manually reset board after flashing for code to work correctly
#define SERIAL_MONITOR_SPEED 74880

// temperature variables- global; all temps stored in degF and converted on the fly as necessary for alternative units (eg DegC)
float meatDoneTemp = 203;    // default to usual brisket internal done temp 203degF
float pitTempTarget = 225;   // reasonable range around 225F long and slow target, pit temps can run 200 to 350 deg F
float currentMeatTemp = 40;  // current meat temp; default to refridgerator temp degF
float currentPitTemp = 225;  // current pit temp; default to long and slow brisket cooking (pit) temp=225degF
bool degCFlag = 0;           // temperature unit flag: 1 for Centigrade or 0 for Fahrenheit (default)
float batteryVoltage = 999;  // 999 as null proxy
float voltageFactor = 0;     // dependent on ADS1015 settings used

/* *****
TODO:  add 4-hour rule check and exception notification
The 4-hour rule is a general food safety guideline that suggests that the internal temperature of meat
should increase from 40°F to 140°F within 4 hours. This is due to foodborne bacteria growing much faster
within this temperature range (known as the “danger zone”).  Ref https://www.totallysmokin.com/4-hour-rule-smoking/
***** */

ADS1015 adc(0x48);  // instantiate ADS1015 ADC object at default address (ADDR conneced to GND)

// timing variables - global
// unsigned long currentMillis;

// run once flag to use in functions called in loop - global
bool hasRunFlag = 0;

// WrapEncoder globals - move to libary?
int16_t prevEncoderValue;
// int16_t currentEncoderValue;

// for testing - then can remove
// smokerState = splashScreen;

// debug code for testing - then can remove
//   int loopcounter = 0;

void setup() {
    Serial.begin(SERIAL_MONITOR_SPEED);
    lcd.initialiseLCD();
    lcd.initialiseCustomCharSet();  // creates eight custom lcd charaters
    encoder.initialise();
    delay(100);  //! *** TEST given blocking *** is delay necessary to clear serial buffer in encoder.initialise(); otherwise garbage characters

    // initialise button press type set-up code (pin, pullup mode, callback function)
    button.begin(BUTTON_PIN);

    smokerState = splashScreen;  // temporarily disable for testing

    // initialise and configure ads1015 ADC module
    adc.begin();
    if (!adc.isConnected()) {
        Serial.print(F(" **** ERROR*** ADS1x15 not connected! "));
    }
    adc.setGain(0);                   // PGA max voltage ±6.144V -> for 5V setup
    adc.setMode(1);                   // SINGLE SHOT MODE
    adc.setDataRate(4);               // 4 == default ADS1015 data rate of 1600 sps; available range 128-3300 sps
    voltageFactor = adc.toVoltage();  // voltage factor is dependent on ADS1015 settings
    adc.requestADC(A0);               // parameters not set in the device until an explicit read/request of the ADC (any read call will do)

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
    button.update();

    processState(lcd);  // temporarily disable for testing, as needed
    // encoder.getCount();  // need to enable this if line above is commented out for testing

    // asynchronous (non-blocking ADC single shot read)
    if (adc.isBusy() == false) {
        int16_t val_0 = adc.getValue();
        adc.requestADC(0);  // request a new one
        Serial.print("\tAnalog0: ");
        Serial.print(val_0);
        Serial.print('\t');
        Serial.println(val_0 * voltageFactor, 3);
    }

    /*  // debug code - serial print out prevEncoderValue every n loops
loopcounter = loopcounter + 1;
 if ((loopcounter % 10) == 0) {
     Serial.println();
     Serial.print(F("    >>>> Loop: -> prevEncoderValue = "));
     Serial.println(prevEncoderValue);
     Serial.println();
 } */

// *****  debug - free memory check  *****
#ifdef DEBUG_FREEMEM
    Serial.print(F("freeMemory()="));
    Serial.println(freeMemory());
    delay(1000);
#endif  // end DEBUG
    // *****  end debug free memory check *****

}  // end of loop