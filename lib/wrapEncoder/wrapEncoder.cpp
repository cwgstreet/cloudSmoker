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

#include "wrapEncoder.h"

#include <Arduino.h>
#include <NewEncoder.h>

#include "myConstants.h"  // all constants in one file

WrapEncoder encoder(ENCODER_CLK, ENCODER_DT, 0, 1, 0, FULL_PULSE);  // default encoder is 0,1 wrapping (for two-line LCD).  Args: pinA, pinB, min, max, start, full or half pulse
WrapEncoder::EncoderState currentEncoderState;                      // note EncoderState is a public struct datatype (currentEncoderState is global per extern declaration in wrapEncoder.h)

void ESP_ISR WrapEncoder::updateValue(uint8_t updatedState) {
    if ((updatedState & DELTA_MASK) == INCREMENT_DELTA) {
        liveState.currentClick = UpClick;
        liveState.currentValue++;
        if (liveState.currentValue > _maxValue) {
            liveState.currentValue = _minValue;
        }
    } else if ((updatedState & DELTA_MASK) == DECREMENT_DELTA) {
        liveState.currentClick = DownClick;
        liveState.currentValue--;
        if (liveState.currentValue < _minValue) {
            liveState.currentValue = _maxValue;
        }
    }
    stateChanged = true;
}

void WrapEncoder::initialise() {
    // delay(500);   //can remove if blocking - for asthetics
    Serial.println(F("Starting"));
    if (!encoder.begin()) {
        Serial.println(F("WrapEncoder::initialise -> Encoder Failed to Start. Check pin assignments and available interrupts. Aborting."));
        while (1) {
            yield();  // Do (almost) nothing -- yield allows ESP8266 background functions
        }
    } else {
        encoder.getState(currentEncoderState);
        prevEncoderValue = currentEncoderState.currentValue;
        Serial.print(F("WrapEncoder::initialise -> Encoder Successfully Started at value (prevEncoderValue = currentEncoderState.currentValue) = "));
        Serial.println(prevEncoderValue);
    }
}

int16_t WrapEncoder::getCount() {
    int16_t currentEncoderValue;

    if (encoder.getState(currentEncoderState)) {
        currentEncoderValue = currentEncoderState.currentValue;
        if (currentEncoderValue != prevEncoderValue) {
            Serial.println();
            Serial.print(F("WrapEncoder::getCount() Encoder: currentEncoderValue = "));
            Serial.print(currentEncoderValue);
            Serial.print(F(" / prevEncoderValue = "));
            Serial.println(prevEncoderValue);

            prevEncoderValue = currentEncoderValue;
            return currentEncoderValue;
        }
    }
    return 0;  // alleviates compiler warning: control reaches end of non-void function [-Wreturn-type]
}

bool WrapEncoder::moved() {
    int16_t currentEncoderValue;

    if (encoder.getState(currentEncoderState)) {  // checks for state change (movement)
        currentEncoderValue = currentEncoderState.currentValue;
        // debug statements:
        Serial.println();
        Serial.print(F("WrapEncoder::moved() -> prevEncoderValue: "));  // debug
        Serial.print(prevEncoderValue);
        Serial.print(F(" / currentEncoderValue = currentEncoderState.currentValue: "));  // debug
        Serial.println(currentEncoderValue);
        // end debug statements:
    }
    currentEncoderValue = currentEncoderState.currentValue;

    if (currentEncoderValue != prevEncoderValue) {
        prevEncoderValue = currentEncoderValue;
        // debug statements:
        Serial.println();
        Serial.println(F("   **WrapEncoder::moved() => Encoder Moved!  if (currentEncoderValue != prevEncoderValue)"));  // debug
        Serial.print(F("exiting WrapEncoder::moved() -> prevEncoderValue: "));                                           // debug
        Serial.print(prevEncoderValue);
        Serial.print(F(" / currentEncoderValue: "));  // debug
        Serial.println(currentEncoderValue);
        // end debug statements:
        return 1;  // true if encoder value changes
    } else {
        return 0;  // false if encoder value does not change
    }
}
