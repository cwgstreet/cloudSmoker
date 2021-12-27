/* ***************************************************************
* wrapEncoder.h - library extending newEncoder Library fuctionality
*   creates encoder object with min / max values that "wrap"
*
*   based on newEncoder example sketch
*     https://github.com/gfvalvo/NewEncoder/blob/master/examples/CustomEncoder/CustomEncoder.ino
* 
*  C W Greenstreet, Ver1, 27Sep21
*    MIT Licence - Released into the public domain
*
** ************************************************************ */

#ifndef WRAP_ENCODER_H
#define WRAP_ENCODER_H

#if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
#include <Arduino.h>
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block

extern int16_t prevEncoderValue;

/*****************
// Libraries required for periphial function tests:
*****************/

// KY40 Rotary Encoder - encoder CW / CCW counts
#include <NewEncoder.h>

/*****************L
// Class definition:
//   WrapEncoder is child (derrived) class of public library NewEncoder
*****************/

class WrapEncoder : public NewEncoder {
   public:
    WrapEncoder() : NewEncoder() {
    }
    WrapEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type = FULL_PULSE) : NewEncoder(aPin, bPin, minValue, maxValue, initalValue, type) {
    }
    virtual ~WrapEncoder() {
    } 

    //virtual ~WrapEncoder() ;  //prob area?

    void initialise();
    // int16_t getCount(WrapEncoder::EncoderState &state);  old
    int16_t getCount();


   protected:
    virtual void updateValue(uint8_t updatedState);
};

// ensure objects are visable everywhere (global)
extern WrapEncoder encoder;  
extern WrapEncoder::EncoderState state;
extern  WrapEncoder::EncoderState currentEncoderState;


#endif  // end header guard