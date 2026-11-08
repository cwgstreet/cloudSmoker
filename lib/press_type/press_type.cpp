/* *************************************************************
 * press_type.cpp - Wrapper Library to determine button press type
 *   press_types:  short press, long press and double press
 *
 * C W Greenstreet, cloudSmoker2, 1Jul26
 * Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 * distribution must also share the full source code under this same license.
 *
 * Library encapsulates and integrates selected features from
 *    1) Yabl (Yet another button library for Arduino)
 *           https://github.com/yergin/Yabl
 *    2) Bounce2 (debouncng library)
 *           https://github.com/thomasfredericks/Bounce2
 *
 * ************************************************************ */

// set up debug scaffold; comment out following line if you want to "turn off" debugging to serial
// monitor
// #define DEBUG 1

#include "bleserial.h"
#include "press_type.h"

#include <Arduino.h>
#include <Yabl.h>

#include "myConstants.h"  // all constants in one file

// speed defined in myConstants.h, which is set to 115200 for ESP32 native speed
const long BAUD_RATE = SERIAL_MONITOR_SPEED;

Press_Type button(BUTTON_PIN);  // instantiate button object from press_type (YABL child) library

// Press_Type constructor attaching button switch
Press_Type::Press_Type(const int switchPin) { _pin = switchPin; }

// pressType_T is an enum type defined in press_type.h header file (as extern); pressEventCode is
// global
pressType_T pressEventCode;

// -------------------------
//   onButtonEvent() function purpose: perform switch-case, triggered by Yabl library callback
//   actions
//      depending on button press type
// --------------------------
void onButtonEvent(const EventInfo& info) {
  switch (info.event) {
    case SINGLE_TAP:
      pressEventCode = SHORT_PRESS;  // pressEventCode == 1
#if DEBUG
      Log.println();
      Log.print(F("Short Press!  pressEventCode = "));
      Log.println(pressEventCode);
#endif  // end DEBUG
      break;

    case DOUBLE_TAP:
      pressEventCode = DOUBLE_PRESS;  // pressEventCode == 2
#if DEBUG
      Log.println();
      Log.print(F("Double Tap Press!  pressEventCode = "));
      Log.println(pressEventCode);
#endif  // end DEBUG
      break;

    case HOLD:
      pressEventCode = LONG_PRESS;  // pressEventCode == 3
#if DEBUG
      Log.println();
      Log.print(F("Long Press!  pressEventCode = "));
      Log.println(pressEventCode);
#endif  // end DEBUG
      break;

    default:
      pressEventCode = NO_PRESS;  // pressEventCode == 0
  }
}

void Press_Type::begin(int _pin) {
  button.attach(_pin, INPUT_PULLUP);  // pin configured to pull-up mode
  button.callback(onButtonEvent);

#if DEBUG  // Debug scaffolding - set in main.cpp to #define DEBUG 1 to enable debug output to
           // serial monitor
  Log.println("   ");  // blank line for visual space
  Log.println("     Yabl Test follows");
  Log.println("==============================");
  Log.println();
#endif  // DEBUG
}

void Press_Type::functionTest() {
  if (pressEventCode == 1) {
    Log.print("*** Short Press! pressEventCode = ");
    Log.println(pressEventCode);
  }
  if (pressEventCode == 2) {
    Log.print("*** Long Press! pressEventCode = ");
    Log.println(pressEventCode);
  }
  if (pressEventCode == 3) {
    Log.print("*** Double Press! pressEventCode = ");
    Log.println(pressEventCode);
  }
}

// uneccessary code - can go stright to inherited button.update()
/*
void Press_Type::checkPress() {
    button.update();
}
 */