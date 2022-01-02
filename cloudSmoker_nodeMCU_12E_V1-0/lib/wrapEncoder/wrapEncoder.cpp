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

WrapEncoder encoder(2, 3, 0, 1, 0, FULL_PULSE);  // default encoder is 0,1 wrapping (for two-line LCD)
WrapEncoder::EncoderState state;                 // EncoderState is a public struct datatype
WrapEncoder::EncoderState currentEncoderState;

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
    //delay(500);   //can remove if blocking - for asthetics
    Serial.println("Starting");
    if (!encoder.begin()) {
        Serial.println("1-Encoder Failed to Start. Check pin assignments and available interrupts. Aborting.");
        while (1) {
            yield();
        }
    } else {
        encoder.getState(state);
        Serial.print("2-Encoder Successfully Started at value = ");
        prevEncoderValue = state.currentValue;
        Serial.println(prevEncoderValue);
    }
}

int16_t WrapEncoder::getCount() {
    int16_t currentValue;

    if (encoder.getState(state)) {
        currentValue = state.currentValue;
        if (currentValue != prevEncoderValue) {
            Serial.print("Encoder: ");
            Serial.println(currentValue);
            prevEncoderValue = currentValue;
            return currentValue;
        }
    }
    return 0;  // alleviates compiler warning: control reaches end of non-void function [-Wreturn-type]
}

bool WrapEncoder::moved() {
    int prevEncoderCount = 0;
    int currentEncoderCount = encoder.getCount();
    if (currentEncoderCount != prevEncoderCount) {
        return 1;  // true if encoder value changes
    } else {
        return 0;  // false if encoder value does not change
    }
}
/* 
int16_t WrapEncoder::newScale() {
    int16_t currentEncoderValue = encoder.getCount();
    Serial.println("Changing Encoder Settings.");
    encoder.newSettings(0, 4, 0, currentEncoderState);
    prevEncoderValue = currentEncoderState.currentValue;
    Serial.print("Starting Value: ");
    Serial.println(prevEncoderValue);
}
 */