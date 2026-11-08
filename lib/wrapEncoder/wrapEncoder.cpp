/* ***************************************************************
 * wrapEncoder.cpp - library extending newEncoder Library functionality
 *   creates encoder object with min / max values that "wrap" (optional)
 *
 *   based on newEncoder example sketch
 *     https://github.com/gfvalvo/NewEncoder/blob/master/examples/CustomEncoder/CustomEncoder.ino
 *
 *  C W Greenstreet, cloudSmoker2, 1Jul26
 * Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *   distribution must also share the full source code under this same license.
 *
 ** ************************************************************ */

#include "bleserial.h"
#include "wrapEncoder.h"

#include <Arduino.h>
#include <NewEncoder.h>

#include "myConstants.h"  // all constants in one file

WrapEncoder encoder(ENCODER_CLK, ENCODER_DT, 0, 1, 0,
                    FULL_PULSE);  // default encoder is 0,1 wrapping (for two-line LCD).  Args:
                                  // pinA, pinB, min, max, start, full or half pulse
WrapEncoder::EncoderState
    currentEncoderState;  // note EncoderState is a public struct datatype (currentEncoderState is
                          // global per extern declaration in wrapEncoder.h)

// Interrupt handler: moves the value one step per click.
// At an end of the range, wraps to the other end if wrapEnabled is true, otherwise stops.
void ESP_ISR WrapEncoder::updateValue(uint8_t updatedState) {
  if ((updatedState & DELTA_MASK) == DECREMENT_DELTA) {
    liveState.currentClick = UpClick;
    if (liveState.currentValue < _maxValue) {
      liveState.currentValue++;
    } else if (wrapEnabled) {
      liveState.currentValue = _minValue;
    }
  } else if ((updatedState & DELTA_MASK) == INCREMENT_DELTA) {
    liveState.currentClick = DownClick;
    if (liveState.currentValue > _minValue) {
      liveState.currentValue--;
    } else if (wrapEnabled) {
      liveState.currentValue = _maxValue;
    }
  }
  stateChanged = true;
}

// Sets a new range, and whether the value wraps at its ends.
// wrap = true keeps the original wrap behaviour; wrap = false gives hard stops.
// The flag is set before the range, so a click arriving during the change can't wrap
// across the new limits. Returns false if the range is invalid (same as newSettings).
bool WrapEncoder::newRange(int16_t newMin, int16_t newMax, int16_t newCurrent, bool wrap,
                           NewEncoder::EncoderState& state) {
  wrapEnabled = wrap;
  return newSettings(newMin, newMax, newCurrent, state);
}

void WrapEncoder::initialise() {
  // delay(500);   //can remove if blocking - for aesthetics
  Log.println(F("Starting"));
  if (!encoder.begin()) {
    Log.println(
        F("WrapEncoder::initialise -> Encoder Failed to Start. Check pin assignments and available "
          "interrupts. Aborting."));
    while (1) {
      yield();  // Do (almost) nothing -- yield allows ESP8266 background functions
    }
  } else {
    encoder.getState(currentEncoderState);
    prevEncoderValue = currentEncoderState.currentValue;
    Log.print(
        F("WrapEncoder::initialise -> Encoder Successfully Started at value (prevEncoderValue = "
          "currentEncoderState.currentValue) = "));
    Log.println(prevEncoderValue);
  }
}

int16_t WrapEncoder::getCount() {
  int16_t currentEncoderValue;

  if (encoder.getState(currentEncoderState)) {
    currentEncoderValue = currentEncoderState.currentValue;
    if (currentEncoderValue != prevEncoderValue) {
      Log.println();
      Log.print(F("WrapEncoder::getCount() Encoder: currentEncoderValue = "));
      Log.print(currentEncoderValue);
      Log.print(F(" / prevEncoderValue = "));
      Log.println(prevEncoderValue);

      prevEncoderValue = currentEncoderValue;
      return currentEncoderValue;
    }
  }
  return 0;  // alleviates compiler warning: control reaches end of non-void function
             // [-Wreturn-type]
}

bool WrapEncoder::moved() {
  int16_t currentEncoderValue;

  if (encoder.getState(currentEncoderState)) {  // checks for state change (movement)
    currentEncoderValue = currentEncoderState.currentValue;
    // debug statements:
    Log.println();
    Log.print(F("WrapEncoder::moved() -> prevEncoderValue: "));  // debug
    Log.print(prevEncoderValue);
    Log.print(F(" / currentEncoderValue = currentEncoderState.currentValue: "));  // debug
    Log.println(currentEncoderValue);
    // end debug statements:
  }
  currentEncoderValue = currentEncoderState.currentValue;

  if (currentEncoderValue != prevEncoderValue) {
    prevEncoderValue = currentEncoderValue;
    // debug statements:
    Log.println();
    Log.println(
        F("   **WrapEncoder::moved() => Encoder Moved!  if (currentEncoderValue != "
          "prevEncoderValue)"));                                            // debug
    Log.print(F("exiting WrapEncoder::moved() -> prevEncoderValue: "));  // debug
    Log.print(prevEncoderValue);
    Log.print(F(" / currentEncoderValue: "));  // debug
    Log.println(currentEncoderValue);
    // end debug statements:
    return 1;  // true if encoder value changes
  } else {
    return 0;  // false if encoder value does not change
  }
}