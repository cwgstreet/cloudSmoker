/* *************************************************************
* press_type.cpp - Wrapper Library to  determine button press type 
*   press_types:  short press, long press and double press
* 
*  C W Greenstreet, Ver1, 7Sep21
*    MIT Licence - Released into the public domain
*
* Library encapsulates and integrates selected fetures from 
*    1) Yabl (Yet another button library for Arduino)
*           https://github.com/yergin/Yabl 
*    2) Bounce2 (debouncng library)
*           https://github.com/thomasfredericks/Bounce2 
*
* ************************************************************ */

//set up debug scaffold; comment out following line if you want to "turn off" serial monitor debugging
#define DEBUG 1

#include <Arduino.h>
#include <Yabl.h>
#include <press_type.h>

const long BAUD_RATE = 74880;  // match native ESP8266 bootup baud rate to view bootup info, otherwise gibberish

Yabl::Button button;  //instantiate button object from Yabl library

//Press_Type constructor attaching button switch
Press_Type::Press_Type(const int switchPin) {
    _pin = switchPin;
}

// set up name flags for pressEvents (later assigned SHORT_PRESS == 0, LONG_PRESS == 1, DOUBLE_PRESS == 2,)
enum pressType { SHORT_PRESS,
                 LONG_PRESS,
                 DOUBLE_PRESS };

enum pressType pressEventCode;

// -------------------------
//   onButtonEvent() function purpose: perform switch-case, triggered by Yabl library callback actions
//      depending on button press type
// --------------------------
void onButtonEvent(const EventInfo& info) {
    switch (info.event) {
        case SINGLE_TAP:
            pressEventCode = SHORT_PRESS;
#ifdef DEBUG
            Serial.print("Short Press!  pressEventCode = ");
            Serial.println(pressEventCode);
#endif  // end DEBUG
            break;

        case DOUBLE_TAP:
            pressEventCode = DOUBLE_PRESS;
#ifdef DEBUG
            Serial.print("Double Tap Press!  pressEventCode = ");
            Serial.println(pressEventCode);
#endif  // end DEBUG
            break;

        case HOLD:
            pressEventCode = LONG_PRESS;
#ifdef DEBUG
            Serial.print("Long Press!  pressEventCode = ");
            Serial.println(pressEventCode);
#endif  // end DEBUG
            break;
    }
}

//void Press_Type::begin(int switchPin) {
void Press_Type::begin(int _pin) {
    //button.attach(switchPin, INPUT_PULLUP);  // pin configured to pull-up mode
    button.attach(_pin, INPUT_PULLUP);  // pin configured to pull-up mode
    button.callback(onButtonEvent);
#ifdef DEBUG
    Serial.begin(BAUD_RATE);
    Serial.println("   ");
    Serial.println("     Yabl Test follows");
    Serial.println("==============================");
#endif  //DEBUG
}

void Press_Type::checkPress() {
    button.update();
}
