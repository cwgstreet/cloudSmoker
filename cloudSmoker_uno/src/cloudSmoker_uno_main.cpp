/* *************************************************************************************
 *    CloudSmoker          
 *    V1.0_UNO   (coded for UNO before porting to ESP8266)
 *    Carl Greenstreet, Aug 2021,   
 *    Licence: MIT (do what you want with this but no functionality guarantees!)
 *    
 *    Purpose:  CloudSmoker Barbecue Temperature Monitor 
 *                (LCD information display, menu, setup and operational routines)
 *    
 *    Project Repository:  https://github.com/cwgstreet/Cloud-Smoker-BBQ-Monitor
 *    Project Wiki:        https://github.com/cwgstreet/cloudSmoker/wiki 
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
 *         encoder switch plus differentiate long vs short button press 
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

//set up debug scaffold; comment out following line to "turn off" debugging routines at pre-proccessor stage
#define DEBUG 1

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

// pins below set up for Uno, not ESP8266
#define I2C_SCL A5  //optional as hd44780 set to auto-configure
#define I2C_SDA A4  //optional as hd44780 set to auto-configure
#define ENCODER_DT 2
#define ENCODER_CLK 3
#define BUTTON_PIN 4  //KY40 push switch SW == BUTTON_PIN

// Baudrate:  Use 74480 baud rate for ESP8266 devices to match ESP8266 fixed bootloader initialisation speed
//  (otherwise you will get startup gibberish characters on serial monitor before serial speed syncs)
//  https://forum.arduino.cc/t/serial-monitor-only-shows-strange-symbols-arduino-mega-with-esp8266/640490/5
//  note: must manually reset board after flashing for code to work correctly
#define SERIAL_MONITOR_SPEED 74880

// external libraries:
#include <Arduino.h>  //platformio requires Arduino framework to be explicitly included
#include <Wire.h>  // must include before hd44780 libraries due to dependencies
#include <hd44780.h>  // LCD library
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header -> required for my YwRobot 1602 LCD
#include <Bounce2.h>
#include <NewEncoder.h>

// internal libraries:
#include <periphials.h>  // contains function tests and usuage for periphials 
#include <press_type.h>   // wrapper library abstracting Yabl / Bounce2 routines

// define LCD geometry (YwRobot 1602 LCD)
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

// *******   surplus code?  *******
// LCD config - auto-locate (actual address is 0x27) and auto-config backpack expander chip for lcd object
// note: cannot declare the lcd object in both a library (ie debug.cpp) and main program or you will get 
//   linker errors "multiple definitions of lcd"; therfore comment out following line unless debug.cpp not included
//hd44780_I2Cexp lcd;
// *******   end surplus code? ******


// *******
// Object instantiation and set-up
// *******

// newEncoder object 
//NewEncoder encoder(ENCODER_CLK, ENCODER_DT, -20, 20, 0, FULL_PULSE);  //NewEncoder(aPin, bPin, minValue, maxValue, initalValue, FULL_PULSE)
//int16_t prevEncoderValue;

#ifdef DEBUG
// <<<<<<<  debug - define pin=internal LED for rotary encoder button press Function Test >>>>>>>>
#define LED_PIN LED_BUILTIN
#endif  //DEBUG

// Instantiate a Button Object from the Bounce2 namespace
//Bounce2::Button button = Bounce2::Button();
//bool longPress = false;

#ifdef DEBUG
// <<<<<<<  debug - button press Function Test >>>>>>>>
// SET A VARIABLE TO STORE THE LED STATE
int ledState = LOW;
#endif  //DEBUG

void setup() {

    NewEncoder::EncoderState state;
    CWG_LCD lcd(LCD_COLS, LCD_ROWS);  //instantiate lcd object from periphials library
    Serial.begin(SERIAL_MONITOR_SPEED);

// ********************** debug - periphial function tests ************************************

#ifdef DEBUG
    // Serial monitor terminal function test  
    Serial.begin(SERIAL_MONITOR_SPEED);
    CWG_SerialMonitor terminal;  //instantiate terminal object from periphials library
    terminal.functionTest();
#endif   

#ifdef DEBUG
    // LCD function test  
    lcd.functionTest();
#endif   




    // Bounce2 Library Button object set-up
    //button.attach(BUTTON_PIN, INPUT_PULLUP);  // Use internal pull-up
    //button.interval(5);                       //Debounce interval in milliseconds
    //button.setPressedState(LOW);              //Low state corresponds to physically pressing the button

/* #ifdef DEBUG
    // <<<<<<<  debug - setup button press Function Test >>>>>>>>
    // setup for LED toggle button function test
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, ledState);
#endif */

/*
#ifdef DEBUG
    // <<<<<<<  debug - rotary enoder Function Test >>>>>>>>
    Serial.println(F("Starting rotary encoder function test"));

    if (!encoder.begin()) {
        Serial.println(F("Encoder Failed to Start. Check pin assignments and available interrupts. Aborting."));
        while (1) {
            yield();
        }
    } else {
        encoder.getState(state);
        Serial.print(F("Encoder Successfully Started at value = "));
        prevEncoderValue = state.currentValue;
        Serial.println(prevEncoderValue);
    }
    //  end rotary encoder function test
#endif
*/

}  // end of setup


void loop() {

#ifdef DEBUG
    // rotary encoder function test  
    CWG_Encoder encoder;
    encoder.functionTest();
#endif 

    //button.update();  // update button press state; must call every loop
    //int debouncedState = button.read();  //get debounced state  note unused variable!

/* 
#ifdef DEBUG
    // <<<<<<<  debug - button press and duration type Function Test >>>>>>>>

    // <Button>.pressed() RETURNS true IF THE STATE CHANGED
    // AND THE CURRENT STATE MATCHES <Button>.setPressedState(<HIGH or LOW>);
    if (button.pressed()) {
        // Depreciated code in block, temporarily retaining during refactoring
      
      if (debouncedState == LOW && button.currentDuration() > LONG_PRESS_TIME_ms )
      {
        longPress = true;  //unused variable (warning - will use this in later code)
        Serial.println(F("Long button press!"));
      }
      else
      {
        Serial.println(F("Short button press!"));
        Serial.print(F("longPress = "));
        Serial.println(longPress);
      
      } 
      ******* end depreciated code snippet *********

        // TOGGLE THE LED STATE :
        ledState = !ledState;             // SET ledState TO THE OPPOSITE OF ledState
        digitalWrite(LED_PIN, ledState);  // WRITE THE NEW ledState
    }
#endif
 */

    // place loop code here

}  // end of loop