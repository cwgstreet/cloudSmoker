/* ***************************************************************
 * lcd.h - library containing display functions for cloudSmoker2 project
 * - Ported to TFT_eSPI for 1.28" GC9A01 Round Display
 *
 * * C W Greenstreet, cloudSmoker2, 1Jul26
 * Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 * distribution must also share the full source code under this same license.
 *
 * * ************************************************************* */

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
  MENU_BACK = 6,
  MENU_COUNT = 6
};

class CWG_LCD {
 public:
  CWG_LCD();

  // Core initialization and management
  void begin();
  void clearScreen();
  void drawUIFrame(const char* headerText);

  // Drop-in compatible functions for cloudSmoker state machine
  void showSplashScreen(bool degCFlag, float meatDoneTemp, float pitTempTarget);
  void printMenuLine(const char* c);
  void printMenuLine_noArrow(const char* c);
  void showTemperatureTargetAdjustment(float temporaryTemperatureTarget, bool meatTargetFlag);

  // New navigation & screen layouts called by smokerStates.cpp
  void showLaunchPad();
  void showSettingsMenu(int16_t prevEncoderValue);
  void showRailStatus(float ina226V, float storedV);
  void showEndCookConfirm();
  void showMeatTargetEdit(const char* label, int16_t value, bool degCFlag);
  void showListMenu(const char* header, const char* const* labels, const char* const* details,
                    uint8_t count, uint8_t selected);
  void showResumePrompt(bool timeUnverified);
  void showInitMessage(const char* line1, const char* line2);
  void showCalProbeMenu(int16_t prevEncoderValue);
  void showCalCapture(const char* header, const char* message, float value, bool hasValue,
                      const char* hint);
  void showCalResult(const char* probeName, float gain, float offset, bool showValues,
                     const char* note, const char* hint);
  void showSetMeatDoneTempMenu(int16_t prevEncoderValue, float meatDoneTemp, bool degCFlag);
  void showSetPitTempTargetMenu(int16_t prevEncoderValue, float pitTempTarget, bool degCFlag);
  void showSetTempUnitsMenu(int16_t prevEncoderValue, bool degCFlag);
  void showBBQStatusScreen(bool degCFlag, float currentMeatTemp, const char* meatLabel,
                           float meatDoneTemp, float currentPitTemp, float pitTempTarget);

  // Display controls for sleep/blanking modes
  void display();
  void noDisplay();

  // Legacy helper to prevent compilation errors if called elsewhere
  void setCursor(uint8_t col, uint8_t row) { /* Handled automatically by UI layers */ }

 private:
  TFT_eSPI tft;
  uint8_t menuPrintLine;  // Keeps track of line tracking (0 or 1) from legacy cloudSmoker1 code
  void drawProgressDots(int currentIndex, int totalItems);  // currentIndex is 1-basedprivate:
};

#endif  // LCD_H