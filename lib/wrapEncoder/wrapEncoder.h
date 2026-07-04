/* ***************************************************************
 * wrapEncoder.h - library extending newEncoder Library functionality
 *   creates encoder object with min / max values that "wrap"
 *
 *   based on newEncoder example sketch
 *     https://github.com/gfvalvo/NewEncoder/blob/master/examples/CustomEncoder/CustomEncoder.ino
 *
 *  C W Greenstreet, cloudSmoker2, 1Jul26
 * Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *   distribution must also share the full source code under this same license.
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
// extern int16_t currentEncoderValue;

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
  WrapEncoder() : NewEncoder() {}
  WrapEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue,
              uint8_t type = FULL_PULSE)
      : NewEncoder(aPin, bPin, minValue, maxValue, initalValue, type) {}
  virtual ~WrapEncoder() {}

  void initialise();
  int16_t getCount();
  bool moved();

 protected:
  virtual void updateValue(uint8_t updatedState);
};

// ensure objects are visable everywhere (global)
extern WrapEncoder encoder;
extern WrapEncoder::EncoderState currentEncoderState;

// globals - must define in main
extern bool hasRunFlag;
extern const int ENCODER_DT;
extern const int ENCODER_CLK;

#endif  // end header guard