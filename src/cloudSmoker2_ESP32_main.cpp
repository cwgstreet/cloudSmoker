/* *************************************************************************************
 *   cloudSmoker2_ESP32_main
 *   V2.ESP32   (Ported from NodeMCU ESP8266 to Firebeetle ESP32 V4)
 *
 *  C W Greenstreet, cloudSmoker2, 1Jul26
 * Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *   distribution must also share the full source code under this same license.
 *
 *   Purpose:  cloudSmoker Barbecue Temperature Monitor (Upgraded with 1.28" TFT Display)
 *               using breadboard as a programming test jig
 *
 *    Project Repository:  https://github.com/cwgstreet/cloudSmoker
 *    Project Wiki:        https://github.com/cwgstreet/cloudSmoker/wiki
 *
 *   Code intended to work with the following hardware:
 *   --------------------------------------------------------
 *   ESP32 (Firebeetle32 V4)
 *   1.28" GC9A01 Round TFT Display (via TFT_eSPI)
 *   EC11 Rotary Encoder Module with Push Button
 *   ADS1015 - 12 bit ADC; I2C addressable
 *   Maverick brand ET-723/733 (NPN thermistor) temperature probes; R25 = 10K ohm, Beta = 3950K
 *
 * ************************************************************************************
 */

// external libraries:
#include <ADS1X15.h>
#include <Arduino.h>  // PlatformIO IDE requirement
#include <Bounce2.h>
#include <NewEncoder.h>
#include <SPI.h>   // Master SPI bus management (Required for GC9A01 TFT)
#include <WiFi.h>  // UPGRADED: Replaced <ESP8266WiFi.h> for ESP32 compatibility
#include <WiFiManager.h>
#include <Wire.h>  // Master I2C bus management

// internal (user) libraries:
#include "cwg_ads1x15.h"  // ADS1x15 I2C ADC functionality
#include "cwg_steinhartHart.h"
#include "cwg_wifi.hpp"
#include "lcd.h"          // UPGRADED: with new TFT_eSPI adapter class framework
#include "myConstants.h"  // All pin mappings and constants
#include "peripherals.h"
#include "press_type.h"    // Wrapper library abstracting Yabl / Bounce2 routines
#include "smokerStates.h"  // cloudSmoker state machine functionality
#include "wrapEncoder.h"

// Always include ThingSpeak header file after other header files and custom macros
#include <ThingSpeak.h>

//! BEGIN DEBUG CODE BLOCK
#include <esp_system.h>

char bootResetReasonStr[24] = "";  // set once in setup(), shown on the splash screen for diagnosis

//? ************** Selective Debug Scaffolding *********************
// #define DEBUG_SERIAL 1
// #define DEBUG_LCD 1           // Configured for local TFT verification testing
// #define DEBUG_PRESSTYPE  1
// #define DEBUG_FREEMEM 1       // Configured for ESP32 internal Heap diagnostics
//?  end Selective Debug Scaffolding *********************************

//? *******************************************************************
//?   Global variables (Maintained as 2-Channel Stable for current phase)
//? *******************************************************************

// Temperature variables- global; all temps stored in degF and converted to DegC on the fly
float meatDoneTemp = 203;    // Default brisket target internal temp
float pitTempTarget = 225;   // Traditional low-and-slow cooking target temperature
float currentMeatTemp = 40;  // Initial baseline temperature
float currentPitTemp = 60;
bool degCFlag = 0;  // Temperature unit flag: 1 for Centigrade, 0 for Fahrenheit

// Analog telemetry globals
float voltageFactor = 0;
float batteryVoltage_v;
double voltagePit_medianFiltered_V;
double voltageMeat_medianFiltered_V;

// Timing metrics - global
long unsigned startCookTime_ms;
unsigned long previousMillis = 0;
unsigned long transmitInterval = 60000;  // Cloud data push rate
unsigned long previousDisplayMillis = 0;
unsigned long displayInterval = 15000;  // Time threshold for active status panel screens

// Operational flags
bool hasRunFlag = 0;
int16_t prevEncoderValue;

// Instantiate your global hardware instances explicitly
CWG_LCD lcd;

void setup() {
  Serial.begin(SERIAL_MONITOR_SPEED);

  esp_reset_reason_t resetReason = esp_reset_reason();
  switch (resetReason) {
    case ESP_RST_POWERON:
      strcpy(bootResetReasonStr, "RST:POWERON");
      break;
    case ESP_RST_EXT:
      strcpy(bootResetReasonStr, "RST:EXT_PIN");
      break;
    case ESP_RST_SW:
      strcpy(bootResetReasonStr, "RST:SW");
      break;
    case ESP_RST_PANIC:
      strcpy(bootResetReasonStr, "RST:PANIC");
      break;
    case ESP_RST_WDT:
      strcpy(bootResetReasonStr, "RST:WDT");
      break;
    case ESP_RST_BROWNOUT:
      strcpy(bootResetReasonStr, "RST:BROWNOUT");
      break;
    default:
      snprintf(bootResetReasonStr, sizeof(bootResetReasonStr), "RST:OTHER(%d)", resetReason);
      break;
  }
  Serial.print(F("Reset reason: "));
  Serial.println(bootResetReasonStr);

  delay(1000);
  Serial.println("\n--- cloudSmoker 2 System Booting (ESP32) ---");

  // debugging statements follow
  Serial.println("Calling lcd.begin()");  // debug
  lcd.begin();
  Serial.println("LCD Initialised");  // debug
  lcd.clearScreen();

  Serial.println("Before Encoder Init");
  encoder.initialise();
  Serial.println("After Encoder Init");  // If this message is not seen, it's hung in initialise()

  WifiManager_initialise();
  Serial.println("Skipped WiFi, entering loop setup");

  // Initialise button press context tracking
  button.begin(BUTTON_PIN);

  // Enter initial splash sequence handling
  smokerState = splashScreen;

  // Configure ADS1015 I2C ADC hardware interface lines
  const uint8_t GAIN_SETTING = 0;       // 2/3x gain, max scale voltage: ±6.144V
  const uint8_t MODE_SETTING = 1;       // Single shot operation
  const uint8_t DATA_RATE_SETTING = 4;  // Baseline sampling configuration
  ads1015.initialise(GAIN_SETTING, MODE_SETTING, DATA_RATE_SETTING);

  // Execution hooks for debug routines
#ifdef DEBUG_SERIAL
  SerialTerminal.functionTest();
#endif

#ifdef DEBUG_LCD
  // Optional: Call a primitive test string loop pattern if needed
  lcd.showSplashScreen(degCFlag, meatDoneTemp, pitTempTarget);
#endif

#ifdef DEBUG_PRESSTYPE
  button.functionTest();
#endif
}

void loop() {
  // Keep processor threads stable on underlying FreeRTOS environments
  yield();

  // Check hardware input triggers
  button.update();

  // Handle primary state machine transitions passing the updated display reference
  processState(lcd);

  // ***** debug - ESP32 Native Free Heap Memory Checking *****
#ifdef DEBUG_FREEMEM
  static unsigned long lastMemCheck = 0;
  if (millis() - lastMemCheck > 5000) {
    lastMemCheck = millis();
    // UPGRADED: Replaced old AVR <MemoryFree.h> call with native ESP32 Heap APIs
    Serial.printf("[SYSTEM MEMORY] Free Heap: %d Bytes\n", ESP.getFreeHeap());
  }
#endif
}