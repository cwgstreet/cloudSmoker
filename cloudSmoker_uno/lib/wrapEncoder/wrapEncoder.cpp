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

extern WrapEncoder encoder;
extern WrapEncoder::EncoderState state;
extern int16_t prevEncoderValue;

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
    WrapEncoder::EncoderState state;

    delay(2000);
    Serial.println("Starting");
    if (!encoder.begin()) {
        Serial.println("Encoder Failed to Start. Check pin assignments and available interrupts. Aborting.");
        while (1) {
            yield();
        }
    } else {
        encoder.getState(state);
        Serial.print("Encoder Successfully Started at value = ");
        prevEncoderValue = state.currentValue;
        Serial.println(prevEncoderValue);
    }
}

int16_t WrapEncoder::getCount(WrapEncoder::EncoderState &state) {
//int16_t WrapEncoder::getCount() {
    int16_t currentValue;
    //WrapEncoder::EncoderState currentEncoderState;

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
