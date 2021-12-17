/* *************************************************************
* press_type.h - Header for Wrapper Library to determine button press type 
*   press_types:  short press, long press and double press
* 
*  C W Greenstreet, Ver1, 7Sep21
*    MIT Licence - Released into the public domain
*
* This wrapper library encapsulates and integrates selected fetures from 
*    1) Yabl (Yet another button library for Arduino)
*           https://github.com/yergin/Yabl 
*    2) Bounce2 (debouncng library)
*           https://github.com/thomasfredericks/Bounce2 
*
* ************************************************************ */

#ifndef PRESS_TYPE_H
#define PRESS_TYPE_H

#if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
#include <Arduino.h>
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block

#include <Yabl.h>

enum pressType_T { NO_PRESS,
                 SHORT_PRESS,
                 LONG_PRESS,
                 DOUBLE_PRESS };

// pressEventCode_T defined in implementation press_type.cpp, hence extern keyword
extern pressType_T pressEventCode;

extern const int BUTTON_PIN;

// Press_Type class - wrapper class to determine press type; inherited from Yabl::Button class
class Press_Type : public Button {
    public:
    Press_Type(const int switchPin);  //constructor - will initialise switchPin
                                      
    //prototype functions - see *.cpp for method code
    void begin(const int _pin);
    void checkPress();

   private:
    int _pin;
};

//extern Press_Type button;  // ensure button object is visable everywhere


/*****************************************************************
*  Note: unsuccessful at declaring onButtonEventfunction prototype below within Press_Type class 
*     above as a member function without compiler giving "invalid use of non-static member function" error
*
*  see discussion: https://arduino.stackexchange.com/questions/33795/error-invalid-use-of-non-static-member-function-while-calling-a-function-from
*****************************************************************/
void onButtonEvent(const EventInfo& info);

#endif  // end header guard