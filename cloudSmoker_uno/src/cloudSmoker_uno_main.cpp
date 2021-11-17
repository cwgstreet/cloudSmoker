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
 *    Project Wiki:        hhttps://github.com/cwgstreet/cloudSmoker/wiki
 *    
 *    Code intended to work with the following hardware
 *      ESP8266-V07
 *         (initially developed on Uno Rev3 (waiting on a replacement NodeMCU after 
 *          letting the magic smoke out!)
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
#include <NewEncoder.h>
#include <Wire.h>                           // must include before hd44780 libraries due to dependencies
#include <hd44780.h>                        // LCD library
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD

// internal libraries:
#include <periphials.h>   // contains function tests and usuage for periphials
#include <press_type.h>   // wrapper library abstracting Yabl / Bounce2 routines
#include <wrapEncoder.h>  //creates encoder object with min / max values that "wrap" around

//set up debug scaffold; comment out following line to "turn off" debugging routines at pre-proccessor stage
//   #ifdef preprocessor simply tests if the symbol's been defined; therefore don't use #ifdef 0
//     ref: https://stackoverflow.com/questions/16245633/ifdef-debug-versus-if-debug
#define DEBUG 1 

// pins set up below is for Uno, not ESP8266
#define I2C_SCL A5     //optional as hd44780 set to auto-configure
#define I2C_SDA A4     //optional as hd44780 set to auto-configure
#define ENCODER_DT 2   // pinA newEncode
#define ENCODER_CLK 3  //pinB newEncode
#define BUTTON_PIN 4   //KY40 push switch SW == BUTTON_PIN

// Baudrate:  Recommend 74480 baud rate for ESP8266 devices to match ESP8266 fixed bootloader initialisation speed
//  (otherwise you will get startup gibberish characters on serial monitor before serial speed syncs)
//  https://forum.arduino.cc/t/serial-monitor-only-shows-strange-symbols-arduino-mega-with-esp8266/640490/5
//  note: must manually reset board after flashing for code to work correctly
#define SERIAL_MONITOR_SPEED 74880

// define LCD geometry (YwRobot 1602 LCD)
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

// **************************************************
// periphial function testing set-up for debug tests
// **************************************************

#ifdef DEBUG
// <<<<<<<  debug - define pin = internal LED for rotary encoder button press Function Test >>>>>>>>
#define LED_PIN LED_BUILTIN
#endif  //DEBUG

#ifdef DEBUG
// <<<<<<<  debug - button press Function Test >>>>>>>>
// SET A VARIABLE TO STORE THE LED STATE
int ledState = LOW;
#endif  //DEBUG

// *********************
// Object instantiation
// *********************

// WrapEncoder object
WrapEncoder encoder(2, 3, 180, 210, 203, FULL_PULSE);  //briskett usually done at 195-203F meat temp
int16_t prevEncoderValue;
int16_t encoderCountValue;

// KY40 button
Press_Type buttonPress(BUTTON_PIN);

// Prepare menu entry case names for switch-case menu stucture
enum entryStates {
    Menu1_meatDone = 1,  //set enum 1 to 4 rather than default 0 for first element
    Menu2_pitMin,
    Menu3_pitMax,
    Menu4_unitsTemp
};

enum entryStates menuState;

void setup() {
    CWG_LCD lcd(LCD_COLS, LCD_ROWS);  //instantiate lcd object from periphials library
    Serial.begin(SERIAL_MONITOR_SPEED);

// **********************  debug - periphial function tests ************************************
#ifdef DEBUG
    // Serial monitor terminal function test
    CWG_SerialMonitor terminal;  //instantiate terminal object from periphials library
    terminal.functionTest();

    // LCD function test
    lcd.functionTest();
#endif
    // **********************  end debug periphial function tests ************************************

    encoder.initialise();

    // initialise button press type set-up code (pin, pullup mode, callback function)
    buttonPress.begin(BUTTON_PIN);

 

}  // end of setup

void loop() {
// **********************  debug - periphial function tests ************************************
#ifdef DEBUG
    //rotary encoder function test
    //rotary_encoder.functionTest();
#endif
    // **********************  end debug periphial function tests ************************************

    buttonPress.checkPress();

    WrapEncoder::EncoderState currentEncoderState;
    //int16_t currentValue;

    menuState = Menu2_pitMin;  //test out various cases by defining test case

    switch (menuState) {
        case Menu1_meatDone:
            Serial.print("menuState =");
            Serial.println(menuState);
            encoderCountValue = encoder.getCount(currentEncoderState);
            break;

        case Menu2_pitMin:
            encoderCountValue = encoder.getCount(currentEncoderState);
            if (encoderCountValue > 209) {
                Serial.println("Changing Encoder Settings.");
                encoder.newSettings(-5, 15, 0, currentEncoderState);  //previous 180, 210, 203
                prevEncoderValue = currentEncoderState.currentValue;
                Serial.print("Starting Value: ");
                Serial.println(prevEncoderValue);
            }
            break;

        case Menu3_pitMax:
            Serial.print("menuState =");
            Serial.println(menuState);
            break;

        case Menu4_unitsTemp:
            Serial.print("menuState =");
            Serial.println(menuState);
            break;

        default:
            Serial.print("default case!  menuState =");
            Serial.println(menuState);

            break;
    }

}  // end of loop