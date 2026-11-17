/* ***************************************************************
 * lcd.h - display functions for the cloudSmoker2 project
 *   Origin: cloudSmoker1 display code for a 16x2 LCD (C W Greenstreet).
 *   Extended for the 1.28" GC9A01 round TFT display (TFT_eSPI) with AI assistance (Claude),
 *   following the original code structure. Under review by the author.
 *
 *   C W Greenstreet, cloudSmoker2, 1Jul26
 *   Licence (this file): GPLv3
 * ************************************************************* */

#ifndef LCD_H
#define LCD_H

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// Menu enumeration for navigation
enum SettingsMenu {
  MENU_MEAT_TEMP = 1,
  MENU_PIT_TEMP = 2,
  MENU_UNITS = 3,
  MENU_CALIBRATION = 4,
  MENU_RAIL = 5,
  MENU_BANK = 6,
  MENU_BLE = 7,
  MENU_SETUP = 8,
  MENU_UPDATE = 9,
  MENU_BACK = 10,
  MENU_COUNT = 10
};

class CWG_LCD {
 public:
  CWG_LCD();

  // Core initialization and management
  void begin();
  void clearScreen();
  void drawUIFrame(const char* headerText);

  // Drop-in compatible functions for cloudSmoker state machine
  void showSplashScreen(bool degCFlag, float meatTargetTemp, float pitMinTemp, float pitMaxTemp, float bankCapacity_mAh, int bankPct);
  void showValueEdit(const char* header, const char* valueText);
  void showCookHint();
  void printMenuLine(const char* c);

  // New navigation & screen layouts called by smokerStates.cpp
  void showLaunchPad();
  void showSettingsMenu(int16_t prevEncoderValue);
  void showRailStatus(float ina226V, float storedV);
  void showPortalScreen();
  void showCookOverview(bool degCFlag, float pitTemp, float meat1Temp, float meat2Temp, float meat3Temp);
  void showBankLine(int pct);
  void showZoomPage(bool degCFlag, const char* topName, uint16_t topColour, float topTemp, float topFraction, const char* bottomName, uint16_t bottomColour, float bottomTemp, float bottomFraction);
  void showLimitEdit(const char* label, int16_t value, bool degCFlag, bool isOff);
  void showMeatTargetEdit(const char* label, int16_t value, bool degCFlag, bool belowSafe);
  void showListMenu(const char* header, const char* const* labels, const char* const* details,
                    uint8_t count, uint8_t selected);
  void showBleToggle(uint8_t selected, uint32_t passkey);
  void showResumePrompt(bool timeUnverified);
  void showInitMessage(const char* line1, const char* line2);
  void showCalProbeMenu(int16_t prevEncoderValue);
  void showCalCapture(const char* header, const char* message, float value, bool hasValue,
                      const char* hint);
  void showCalResult(const char* probeName, float gain, float offset, bool showValues,
                     const char* note, const char* hint);
  void showSetTempUnitsMenu(int16_t prevEncoderValue, bool degCFlag);

  // Display controls for sleep/blanking modes
  void display();
  void noDisplay();

  // Legacy helper to prevent compilation errors if called elsewhere

 private:
  TFT_eSPI tft;
  uint8_t menuPrintLine;  // Keeps track of line tracking (0 or 1) from legacy cloudSmoker1 code
  void drawProgressDots(int currentIndex, int totalItems);  // currentIndex is 1-based
};

#endif  // LCD_H