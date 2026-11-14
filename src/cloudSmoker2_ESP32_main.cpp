/* *************************************************************************************
 *   cloudSmoker2_ESP32_main
 *   V2.ESP32   (Ported from NodeMCU ESP8266 to Firebeetle ESP32 V4)
 *
 *  C W Greenstreet, cloudSmoker2, 1Jul26
 *     Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *     distribution must also share the full source code under this same license.
 *
 *   Purpose:  cloudSmoker Barbecue Temperature Monitor (Upgraded with 1.28" TFT Display)
 *               using breadboard as a programming test jig
 *
 *    Project Repository:  https://github.com/cwgstreet/cloudSmoker
 *    Project Wiki:        https://github.com/cwgstreet/cloudSmoker/wiki
 *
 *   cloudSmoker2 (Ver2) Code intended to work with the following hardware:
 *   --------------------------------------------------------
 *   ESP32-C3 (Firebeetle32 V4)
 *   1.28" GC9A01 Round TFT Display (via TFT_eSPI)
 *   EC11 Rotary Encoder Module with Push Button
 *   ADS1115 - 12 bit ADC; I2C addressable
 *   Maverick brand ET-723/733 (NPN thermistor) temperature probes; R25 = 10K ohm, Beta = 3950K
 *   Ref: https://github.com/cwgstreet/cloudSmoker/wiki/V2-Parts-List
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

#include "bleserial.h"

// internal (user) libraries:
#include "calibration.h"  // per-probe two-point calibration (gain and offset) constants
#include "cookstore.h"
#include "cwg_ads1x15.h"  // ADS1x15 I2C ADC functionality
#include "cwg_ina226.h"
#include "cwg_steinhartHart.h"
#include "cwg_wifi.hpp"
#include "devconfig.h"
#include "lcd.h"          // UPGRADED: with new TFT_eSPI adapter class framework
#include "myConstants.h"  // All pin mappings and constants
#include "ota.h"          // OTA (Over-the-Air) firmware update functionality
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
// #define DEBUG_FREEMEM 1  // Configured for ESP32 internal Heap diagnostics; good heartbeat test
//?  end Selective Debug Scaffolding *********************************

//? *******************************************************************
//?   Global variables (Maintained as 2-Channel Stable for current phase)
//? *******************************************************************

// Temperature variables- global; all temps stored in degF and converted to DegC on the fly
float meatTarget[3] = {203, 203, 203};  // MEAT1..MEAT3 done targets, degF
float pitMin = 225;                     // PIT band in degF; alert after 5 min outside
float pitMax = 250;
// // Traditional low-and-slow cooking target temperature
float currentPitTemp = 60;
float currentMeat1Temp = 40;  // Initial baseline temperature
float currentMeat2Temp = 40;  // Initial baseline temperature
float currentMeat3Temp = 40;  // Initial baseline temperature
bool degCFlag = 0;            // Temperature unit flag: 1 for Centigrade, 0 for Fahrenheit

// Analog telemetry globals
float voltageFactor = 0;
float batteryVoltage_v;
double voltagePit_medianFiltered_V;
double voltageMeat1_medianFiltered_V;
double voltageMeat2_medianFiltered_V;
double voltageMeat3_medianFiltered_V;

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

#ifdef DEBUG_NVS_STATS
void printNvsStats();
#endif

void setup() {
  Serial.begin(SERIAL_MONITOR_SPEED);

  calInit();  // load per-probe calibration from NVS (non-volatile storage)

  cookStoreBegin();
  devConfigBegin();
  bleSerialEnabled = bleSerialLoad();
  bleSerialApply(bleSerialEnabled);

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
  Log.print(F("Reset reason: "));
  Log.println(bootResetReasonStr);

  delay(1000);
  Log.println("\n--- cloudSmoker 2 System Booting (ESP32) ---");

  // initialisations with debugging output to Serial Monitor
  Log.println("Calling lcd.begin()");  // debug
  lcd.begin();
  Log.println("LCD Initialised");  // debug
  lcd.clearScreen();

  Log.println("Before Encoder Init");
  encoder.initialise();
  Log.println("After Encoder Init");  // If this message is not seen, it's hung in initialise()

  // Show in initialisation screen
  // Do this here so that the user sees while the WiFiManager portal is being configured
  lcd.showInitMessage("Connecting Wi-Fi", "Please wait");

  WifiManager_initialise();
  Log.println("WiFi setup finished, entering main loop");
  if (WiFi.status() == WL_CONNECTED) {
    timeSyncWait(5000);  // settle the clock before deciding resume or stale
  }

  // initialise OTA (Over-the-Air) firmware update service
  if (!cookStoreActive()) {
    String otaPw = devOtaPassword();
    otaBegin("cloudsmoker2", otaPw.c_str());
  }

  // Initialise button press context tracking
  button.begin(BUTTON_PIN);

  // Enter initial splash sequence handling
  smokerState = splashScreen;

  if (cookApplyOnBoot()) {
    smokerState = resumePrompt;
  }

  // Configure ADS1115 I2C ADC hardware interface lines
  const uint8_t GAIN_SETTING = 0;  // 2/3x gain, max scale voltage: ±6.144V
  const uint8_t MODE_SETTING = 1;  // Single shot operation
  const uint8_t DATA_RATE_SETTING =
      4;  // Baseline sampling configuration; 4 = 128 SPS for ADS1115 (default)
  ads1115.initialise(GAIN_SETTING, MODE_SETTING, DATA_RATE_SETTING);

  // initialise INA226 current/voltage monitor with calibration values for 5V rail and 100mA shunt
  ina226.initialise(0.79f, 0.1f);

  Log.print(F("INA226 found: "));
  Log.println(ina226.deviceFound() ? F("yes") : F("NO"));
  Log.print(F("INA226 maxCurrent = "));
  Log.print(ina226.getMaxCurrent(), 6);
  Log.print(F(" A, shunt = "));
  Log.print(ina226.getShunt(), 4);
  Log.println(F(" ohm"));
  Log.print(F("INA226 rail = "));
  Log.print(ina226.getRailV(), 4);
  Log.println(F(" V"));

  // Execution hooks for debug routines
#ifdef DEBUG_SERIAL
  SerialTerminal.functionTest();
#endif

#ifdef DEBUG_LCD
  // Optional: Call a primitive test string loop pattern if needed
  lcd.showSplashScreen(degCFlag, meatTarget[0], pitMin, pitMax);
#endif

#ifdef DEBUG_PRESSTYPE
  button.functionTest();
#endif

#ifdef DEBUG_NVS_STATS
  printNvsStats();
#endif
}

void loop() {
  // Keep processor threads stable on underlying FreeRTOS environments
  yield();

  // Handle OTA (Over-the-Air) firmware update service
  otaLoop();

  // Check hardware input triggers
  button.update();

  // Handle primary state machine transitions passing the updated display reference
  processState(lcd);

  // cal / calShow / calReset from the serial monitor
  calPollSerial();

  ina226.updateCharge();  // coulomb counter for power bank SOC, once per second
  bleSerialPump();        // BLE serial: send queued output to the phone

  cookCheckpoint(ina226.getChargeUsed_mAh());

  // ***** debug - ESP32 Native Free Heap Memory Checking *****
#ifdef DEBUG_FREEMEM
  static unsigned long lastMemCheck = 0;
  if (millis() - lastMemCheck > 5000) {
    lastMemCheck = millis();
    // UPGRADED: Replaced old AVR <MemoryFree.h> call with native ESP32 Heap APIs
    Log.printf("[SYSTEM MEMORY] Free Heap: %d Bytes\n", ESP.getFreeHeap());
  }
#endif
}