/* *************************************************************
* press_type.cpp - Wrapper Library to determine button press type 
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

// pressType is an enum defined in press_type.h header file (as extern); pressEventCode is global
pressType pressEventCode;

// -------------------------
//   onButtonEvent() function purpose: perform switch-case, triggered by Yabl library callback actions
//      depending on button press type
// --------------------------
void onButtonEvent(const EventInfo& info) {
    switch (info.event) {
        case SINGLE_TAP:
            pressEventCode = SHORT_PRESS;  // pressEventCode == 1
#ifdef DEBUG
            Serial.print("Short Press!  pressEventCode = ");
            Serial.println(pressEventCode);
#endif  // end DEBUG
            break;

        case DOUBLE_TAP:
            pressEventCode = DOUBLE_PRESS;  // pressEventCode == 2
#ifdef DEBUG
            Serial.print("Double Tap Press!  pressEventCode = ");
            Serial.println(pressEventCode);
#endif  // end DEBUG
            break;

        case HOLD:
            pressEventCode = LONG_PRESS;  // pressEventCode == 3
#ifdef DEBUG
            Serial.print("Long Press!  pressEventCode = ");
            Serial.println(pressEventCode);
#endif  // end DEBUG
            break;

        default:
            pressEventCode = NO_PRESS;  // pressEventCode == 0
    }
}

void Press_Type::begin(int switchPin) {
    button.attach(switchPin, INPUT_PULLUP);  // pin configured to pull-up mode
    button.callback(onButtonEvent);


Serial.begin(BAUD_RATE);

#ifdef DEBUG
    Serial.println("   ");  //blank line for visual space
    Serial.println("     Yabl Test follows");
    Serial.println("==============================");
#endif  //DEBUG
}

void Press_Type::checkPress() {
    button.update();
}
