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
#include "ADS1X15.h"
#include "NewEncoder.h"
#include <Bounce2.h>

// 1. Conditionally instantiate the correct ADC class based on platformio.ini flags
#ifdef USE_ADS1015
  ADS1015 adc;
#elif defined(USE_ADS1115)
  ADS1115 adc;
#endif

// 2. Instantiate Encoder and Button
// NewEncoder parameters: A pin, B pin, min value, max value, starting value, type
NewEncoder encoder(PIN_ENCODER_A, PIN_ENCODER_B, -20, 20, 0, FULL_PULSE);
Bounce debouncer = Bounce(); 

int16_t currentEncoderValue = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- cloudSmoker Hardware Test ---");

  // Initialize I2C with the pins defined in platformio.ini
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  // Initialize ADC
  if (!adc.begin()) {
    Serial.println("Failed to initialize ADS1x15! Check wiring.");
  } else {
    Serial.println("ADS1x15 initialized successfully.");
    adc.setGain(0); // 6.144 volt range
  }

  // Initialize Encoder
  if (!encoder.begin()) {
    Serial.println("Encoder Failed to Start. Check Pins.");
  } else {
    Serial.println("Encoder initialized.");
  }

  // Initialize Button
  debouncer.attach(PIN_ENCODER_BTN, INPUT_PULLUP);
  debouncer.interval(25); // 25ms debounce
}

void loop() {
  // 1. Read Button
  debouncer.update();
  if (debouncer.fell()) {
    Serial.println("Encoder Button Pressed!");
  }

  // 2. Read Encoder
  int16_t newValue = encoder.getCount();
  if (newValue != currentEncoderValue) {
    Serial.print("Encoder Value: ");
    Serial.println(newValue);
    currentEncoderValue = newValue;
  }

  // 3. Read ADC (Channel 0) periodically
  static unsigned long lastAdcRead = 0;
  if (millis() - lastAdcRead > 2000) {
    lastAdcRead = millis();
    int16_t adc0 = adc.readADC(0);
    float voltage = adc.toVoltage(1); // convert raw reading to voltage
    
    Serial.print("ADC CH0 Raw: ");
    Serial.print(adc0);
    Serial.print("\tVoltage: ");
    Serial.println(adc0 * voltage, 4); 
  }
}