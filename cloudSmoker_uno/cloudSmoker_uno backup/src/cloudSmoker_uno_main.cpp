/* *************************************************************************************
 *    CloudSmoker_uno          
 *    V1.0_UNO   (coded for UNO Rev3 before porting to ESP8266)
 *    Carl Greenstreet, Aug 2021,   
 *    Licence: MIT (do what you want with this code but no functionality guarantees!)
 *    
 *    Purpose:  CloudSmoker Barbecue Temperature Monitor 
 *                (LCD information display, menu, setup and operational routines)
 *    
 *    Project Repository:  https://github.com/cwgstreet/cloudSmoker 
 *    Project Wiki:        https://github.com/cwgstreet/cloudSmoker/wiki 
 *    
 *    Code intended to work with the following hardware
 *      ESP8266-V07
 *         (initially developed on Uno Rev3 (while waiting on a replacement NodeMCU  
 *           after letting the magic smoke out!)
 *         NodeMCU V1 (ESP12) development board (will port over to bare bones ESP8266-07 
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
 *      2021.8.15    fixed blah blah blah  
 *      
 * ************************************************************************************   
 */

/*  Uno pinout setup:    
 *   
 *   Pin      Function  Comment
 *   -------  --------  -----------------------------------------------
 *   A5        SCL      YwRobot LCD (via 4.7k ohm pullup resistor)
 *   A4        SDA      YwRobot LCD (via 4.7k ohm pullup resistor)
 *   4         SW       KY40 Push switch    grey 
 *   3         DT       KY40 (PinB)         brown
 *   2         CLK      KY40 (PinA)         white
*/

// external libraries:
#include <Arduino.h>  //platformio IDE requires Arduino framework to be explicitly included
#include <Bounce2.h>
#include <MemoryFree.h>  // https://github.com/maniacbug/MemoryFree & https://playground.arduino.cc/Code/AvailableMemory/
#include <NewEncoder.h>
#include <Wire.h>                           // must include before hd44780 libraries due to dependencies
#include <hd44780.h>                        // LCD library
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

// internal (user) libraries:
#include <periphials.h>    // serial monitor function tests and usuage routines
#include <lcd.h>  // lcd function tests, helper functions and custom characters
#include <press_type.h>    // wrapper library abstracting Yabl / Bounce2 routines
#include <wrapEncoder.h>   //  encoder library including encoder object with min / max values that "wrap" around
#include <smokerStates.h>  // cloudSmoker state machine functionality

// **************  Selective Debug Saffolding ***********************
// Set up selective debug scaffold; comment out appropriate lines below to disable debugging tests at pre-proccessor stage
//   Note: #ifdef preprocessor simply tests if the symbol's been defined; therefore don't use #ifdef 0
//    Ref: https://stackoverflow.com/questions/16245633/ifdef-debug-versus-if-debug
// *****************************************************************
#define DEBUG_SERIAL  1      // uncomment to debug - Serial monitor function test
#define DEBUG_LCD 1  // uncomment to debug - LCD function test
#define DEBUG_PRESSTYPE  1  // uncomment to debug - Rotary encoder button press type function test
#define DEBUG_LED  1       // uncomment to debug LED test of rotary encoder
//#define DEBUG_FREEMEM 1  // uncomment to debug remaining free memory

// pins set-up listed below is for Uno, not ESP8266
//  Note to self:  constexp  better than const for variable values that should be known at compile 
//     time -> more memory efficient.  Also better than simple #define
constexpr int I2C_SCL = A5;     //optional as hd44780 set to auto-configure
constexpr int I2C_SDA = A4;     //optional as hd44780 set to auto-configure
constexpr int ENCODER_DT = 2;   // pinA newEncode
constexpr int ENCODER_CLK = 3;  //pinB newEncode
constexpr int BUTTON_PIN = 4;   // KY40 SW (switch) pin connected to Uno pin 4

// Baudrate:  Recommend 74480 baud rate for ESP8266 devices to match ESP8266 fixed bootloader initialisation speed
//  (otherwise you will get startup gibberish characters on serial monitor before serial speed syncs)
//  https://forum.arduino.cc/t/serial-monitor-only-shows-strange-symbols-arduino-mega-with-esp8266/640490/5
//  note: may have to manually reset board after flashing for code to work correctly
#define SERIAL_MONITOR_SPEED 74880

// temperature variables- global; all temps stored in degF and converted on the fly as necessary for alternative units (eg DegC)
float meatDoneTemp = 203;    // default to usual brisket internal done temp 203degF
float pitTempTarget = 210;   // reasonable range around 225F long and slow target, pit temps can run 200 to 350 deg F
float currentMeatTemp = 40;  // current meat temp; default to refridgerator temp degF
float currentPitTemp = 225;  // current pit temp; default to long and slow brisket cooking (pit) temp=225degF
bool degCFlag = 0;           // temperature unit flag: 1 for Centigrade or 0 for Fahrenheit

/* *********
To do:  add 4-hour rule check and exception notification
The 4-hour rule is a general food safety guideline that suggests that the internal temperature of meat 
should increase from 40°F to 140°F within 4 hours. This is due to foodborne bacteria growing much faster 
within this temperature range (known as the “danger zone”).  Ref https://www.totallysmokin.com/4-hour-rule-smoking/
********** */

// timing variables - global
//unsigned long currentMillis;

// *********************
// Object instantiation
// *********************

// WrapEncoder globals - move to libary?
//  delete this line? //WrapEncoder encoder(2, 3, 180, 210, 203, FULL_PULSE);  //default meatProbe; briskett usually done at 195-203F internal meat temp
int16_t prevEncoderValue;
int16_t encoderCountValue;

// KY40 button
//Press_Type buttonPress(BUTTON_PIN);

// for testing - then can remove
//smokerState = splashScreen;

void setup() {
    Serial.begin(SERIAL_MONITOR_SPEED);

    //CWG_LCD lcd(LCD_COLS, LCD_ROWS);  //instantiate lcd object from periphials library
    //lcd.initialiseCustomCharSet();    //creates eight custom lcd charaters

// **********  debug - Serial monitor periphial function tests **************************
#ifdef DEBUG_SERIAL
    SerialTerminal.functionTest();
#endif  // **********  end debug periphial function tests *************************

#ifdef DEBUG_LCD // **********  debug - LCD function tests **************************
    // LCD function test
    lcd.functionTest();
    lcd.displayTest();
#endif  // **********  end debug LCD function tests *************************

#ifdef DEBUG_PRESSTYPE // **********  debug - button press_type function tests **************************
    button.functionTest();
#endif  // **********  end button press_type function tests *************************


    lcd.initialiseCustomCharSet();  //creates eight custom lcd charaters, see lcd.cpp

    encoder.initialise();
    delay(100); // delay necessary to clear serial buffer in encoder.initialise(); otherwise garbage characters

    // initialise button press type set-up code (pin, pullup mode, callback function)
    button.begin(BUTTON_PIN);

}  // end of setup

void loop() {
    button.checkPress();
    encoder.getCount();
    processState(lcd); disable temporarily



/* // **********  debug - press Type function test  **************************
//  pressEventCode: Short Press == 1, Long Press == 2, Double Press == 3, No Press == 0
#ifdef DEBUG_PRESSTYPE
    if (pressEventCode == 1) {
        Serial.print("*** Short Press! pressEventCode = ");
        Serial.println(pressEventCode);
    }
    if (pressEventCode == 2) {
        Serial.print("*** Long Press! pressEventCode = ");
        Serial.println(pressEventCode);
    }
    if (pressEventCode == 3) {
        Serial.print("*** Double Press! pressEventCode = ");
        Serial.println(pressEventCode);
    }
#endif  // end DEBUG
    // **********  end debug press Type function tests *************************
 */
 //   WrapEncoder::EncoderState currentEncoderState;

// **********  debug - free memory check  **************************
#ifdef DEBUG_FREEMEM
    Serial.print("freeMemory()=");
    Serial.println(freeMemory());
    delay(1000);
#endif  // end DEBUG
    // **********  end debug free memory check *************************

}  // end of loop