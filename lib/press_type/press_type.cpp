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

// set up debug scaffold; comment out following line if you want to "turn off" debugging to serial monitor 
#define DEBUG 1

#include "press_type.h"

#include <Arduino.h>
#include <Yabl.h>

#include "myConstants.h"  // all constants in one file

const long BAUD_RATE = 74880;  // match native ESP8266 bootup baud rate to view bootup info, otherwise gibberish

Press_Type button(BUTTON_PIN);  // instantiate button object from press_type (YABL child) library

// Press_Type constructor attaching button switch
Press_Type::Press_Type(const int switchPin) {
    _pin = switchPin;
}

// pressType_T is an enum type defined in press_type.h header file (as extern); pressEventCode is global
pressType_T pressEventCode;

// -------------------------
//   onButtonEvent() function purpose: perform switch-case, triggered by Yabl library callback actions
//      depending on button press type
// --------------------------
void onButtonEvent(const EventInfo& info) {
    switch (info.event) {
        case SINGLE_TAP:
            pressEventCode = SHORT_PRESS;  // pressEventCode == 1
#if DEBUG
            Serial.println();
            Serial.print(F("Short Press!  pressEventCode = "));
            Serial.println(pressEventCode);
#endif  // end DEBUG
            break;

        case DOUBLE_TAP:
            pressEventCode = DOUBLE_PRESS;  // pressEventCode == 2
#if DEBUG
            Serial.println();
            Serial.print(F("Double Tap Press!  pressEventCode = "));
            Serial.println(pressEventCode);
#endif  // end DEBUG
            break;

        case HOLD:
            pressEventCode = LONG_PRESS;  // pressEventCode == 3
#if DEBUG
            Serial.println();
            Serial.print(F("Long Press!  pressEventCode = "));
            Serial.println(pressEventCode);
#endif  // end DEBUG
            break;

        default:
            pressEventCode = NO_PRESS;  // pressEventCode == 0
    }
}

void Press_Type::begin(int _pin) {
    button.attach(_pin, INPUT_PULLUP);  // pin configured to pull-up mode
    button.callback(onButtonEvent);

    Serial.begin(BAUD_RATE);

#if DEBUG
    Serial.println("   ");  // blank line for visual space
    Serial.println("     Yabl Test follows");
    Serial.println("==============================");
    Serial.println();
#endif  // DEBUG
}

void Press_Type::functionTest() {
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
}

// uneccessary code - can go stright to inherited button.update()
/*
void Press_Type::checkPress() {
    button.update();
}
 */