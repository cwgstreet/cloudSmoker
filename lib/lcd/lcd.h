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
  MENU_BACK = 4,
  MENU_COUNT = 4
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
  void showSetMeatDoneTempMenu(int16_t prevEncoderValue);
  void showSetPitTempTargetMenu(int16_t prevEncoderValue);
  void showSetTempUnitsMenu(int16_t prevEncoderValue);
  void showBBQStatusScreen(bool degCFlag, float currentMeatTemp, float meatDoneTemp,
                           float currentPitTemp, float pitTempTarget);

  // Display controls for sleep/blanking modes
  void display();
  void noDisplay();

  // Legacy helper to prevent compilation errors if called elsewhere
  void setCursor(uint8_t col, uint8_t row) { /* Handled automatically by UI layers */ }

 private:
  TFT_eSPI tft;
  uint8_t menuPrintLine;  // Keeps track of line tracking (0 or 1) from legacy cloudSmoker1 code
};

#endif  // LCD_H