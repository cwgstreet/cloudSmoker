/* *************************************************************************************
 *    main_breadboard-test.cpp
 *     purpose:  test breadboard components & configuration 
 *    
 *    Carl Greenstreet, Jun 2026,
 *    Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public 
 *           distribution must also share the full source code under this same license.
)
 *
 *    Purpose:  cloudSmoker Barbecue Temperature Monitor
 *
 *    Project Repository:  https://github.com/cwgstreet/cloudSmoker
 *    Project Wiki:        https://github.com/cwgstreet/cloudSmoker/wiki
 *
 * ************************************************************************************
 */

#include <Arduino.h>
#include <Wire.h>
#include "myConstants.h"
#include "cwg_ads1x15.h"
#include "wrapEncoder.h"
#include "press_type.h"

// Note:  wrappers (ads1015, encoder, button) are already instantiated 
// as extern globals in their respective library .cpp files!

// Globals required by wrapEncoder.h
bool hasRunFlag = 0;

void setup() {
    Serial.begin(SERIAL_MONITOR_SPEED);
    delay(1000);
    Serial.println(F("\n--- cloudSmoker2 Phase 1: ESP32 Hardware Test ---"));

    // 1. Initialize I2C using the pins from myConstants (which pull from platformio.ini)
    Wire.begin(I2C_SDA, I2C_SCL);

    // 2. Initialize Custom ADC Wrapper
    // gainSetting=0 (6.144V), modeSetting=1 (Single Shot), dataRateSetting=4 (1600sps)
    ads1015.initialise(0, 1, 4);
    Serial.println(F("CWG_ADS1015 Initialized."));

    // 3. Initialize Custom Encoder Wrapper
    encoder.initialise();

    // 4. Initialize Custom Button Wrapper (Yabl)
    button.begin(BUTTON_PIN);
    Serial.println(F("Press_Type Button Initialized."));
}

void loop() {
    // 1. Test Yabl Button Logic
    button.update();
    if (pressEventCode != NO_PRESS) {
        button.functionTest(); // Prints "Short Press!", "Long Press!", etc.
        pressEventCode = NO_PRESS; // Reset the global flag after handling
    }

    // 2. Test WrapEncoder Logic
    if (encoder.moved()) {
        int16_t currentVal = encoder.getCount();
        Serial.print(F("Encoder Value Updated: "));
        Serial.println(currentVal);
    }

    // 3. Test ADC & Median Filter Logic (Non-blocking every 2 seconds)
    static unsigned long lastAdcRead = 0;
    if (millis() - lastAdcRead > 2000) {
        lastAdcRead = millis();
        
        // Use custom median filter method (11 samples)
        float meatVoltage = ads1015.getSensorValue_MedianFiltered_V(ADC_meatPin, 11);
        
        Serial.print(F("Meat Probe (A2) Median Filtered Voltage: "));
        Serial.println(meatVoltage, 4);
    }
}