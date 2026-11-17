/* ***************************************************************
 * wrapEncoder.h - rotary encoder class extending the NewEncoder library
 *   Creates an encoder object with a minimum and maximum value. The value wraps from
 *   the maximum back to the minimum (and the reverse) by default. newRange() can turn
 *   wrapping off, giving hard stops at the limits.
 *
 *   C W Greenstreet, cloudSmoker2, 17Nov2026
 *   Licence (this file): GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any
 *     public distribution must also share the full source code under this same license.)
 *
 *   Built on:
 *     NewEncoder by gfvalvo, https://github.com/gfvalvo/NewEncoder
 *     The upstream repository does not state a licence. Its borrowed PJRC files
 *     (direct_pin_read.h, interrupt_pins.h) carry PJRC.COM, LLC - Paul Stoffregen copyright.
 *     Concepts inspired by buxtronix, http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
 *     Design reference: CustomEncoder example sketch, https://github.com/gfvalvo/NewEncoder/blob/master/examples/CustomEncoder/CustomEncoder.ino
 *     (no code copied from the example)
 * *************************************************************** */

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
// NewEncoder library, which this class extends:
*****************/

// EC11 Rotary Encoder (cloudSmoker2) - encoder CW / CCW counts
#include <NewEncoder.h>

/*****************
// Class definition:
//   WrapEncoder is child (derived) class of public library NewEncoder
//   Value wraps from max to min (and back) by default. newRange() can turn wrapping off,
//   giving hard stops at the limits.
*****************/

class WrapEncoder : public NewEncoder {
 public:
  WrapEncoder() : NewEncoder() {}
  WrapEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initialValue,
              uint8_t type = FULL_PULSE)
      : NewEncoder(aPin, bPin, minValue, maxValue, initialValue, type) {}
  virtual ~WrapEncoder() {}

  void initialise();
  int16_t getCount();
  bool moved();

  // Sets a new range and whether the value wraps at its ends. wrap = false gives hard stops.
  // Returns false if the range is invalid (same as NewEncoder::newSettings).
  bool newRange(int16_t newMin, int16_t newMax, int16_t newCurrent, bool wrap,
                NewEncoder::EncoderState& state);

 protected:
  virtual void updateValue(uint8_t updatedState);

  // Read inside the interrupt, so volatile. true keeps the original wrap behaviour.
  volatile bool wrapEnabled = true;
};

// ensure objects are visible everywhere (global)
extern WrapEncoder encoder;
extern WrapEncoder::EncoderState currentEncoderState;

#endif  // end header guard