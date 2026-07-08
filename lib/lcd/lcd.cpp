/* ***************************************************************
 * lcd.cpp - library containing display functions for cloudSmoker2 project
 *   -> Ported to TFT_eSPI for 1.28" GC9A01 Round Display
 *
 *  C W Greenstreet, cloudSmoker2, 1Jul26
 *   Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *    distribution must also share the full source code under this same license.
 *
 * * **************************************************************/

#include "lcd.h"

#include "helper_functions.h"

extern char bootResetReasonStr[24];  //! DEBUG CODE - temporary

// Constructor
CWG_LCD::CWG_LCD() : tft(TFT_eSPI()), menuPrintLine(0) {}

// Initialize display and manage backlight pin from platformio.ini
void CWG_LCD::begin() {
#ifdef PIN_TFT_BLK
  pinMode(PIN_TFT_BLK, OUTPUT);
  digitalWrite(PIN_TFT_BLK, HIGH);  // Turn backlight on
#endif

  tft.init();
  tft.setRotation(0);  // Adjust 0-3 if your physical screen orientation is upside down
  clearScreen();
}

void CWG_LCD::clearScreen() { tft.fillScreen(TFT_BLACK); }

// Draws a premium dashboard ring suited for the round form factor
void CWG_LCD::drawUIFrame(const char* headerText) {
  tft.drawCircle(120, 120, 119, TFT_SILVER);
  tft.drawCircle(120, 120, 116, TFT_NAVY);

  // Render a clean top header
  tft.setTextDatum(TC_DATUM);  // Top-Center anchoring
  tft.setTextColor(TFT_GOLD, TFT_BLACK);
  tft.drawString(headerText, 120, 25, 2);  // Font 2

  // Draw a subtle divider line below the header
  tft.drawFastHLine(40, 48, 160, TFT_DARKGREY);
}

// Ported splash screen displaying target metrics inside UI framework
void CWG_LCD::showSplashScreen(bool degCFlag, float meatDoneTemp, float pitTempTarget) {
  clearScreen();
  drawUIFrame("cloudSmoker2");

  char meatStr[12];
  char pitStr[12];

  // Convert values matching your legacy conversions
  if (degCFlag) {
    snprintf(meatStr, sizeof(meatStr), "Meat: %.0fC", convertDegFtoDegC(meatDoneTemp));
    snprintf(pitStr, sizeof(pitStr), "Pit: %.0fC", convertDegFtoDegC(pitTempTarget));
  } else {
    snprintf(meatStr, sizeof(meatStr), "Meat: %.0fF", meatDoneTemp);
    snprintf(pitStr, sizeof(pitStr), "Pit: %.0fF", pitTempTarget);
  }

  tft.setTextDatum(MC_DATUM);  // Middle-Center anchoring
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.drawString("BBQ TARGETS", 120, 95, 2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(meatStr, 120, 130, 4);  // Larger Font 4 for readability
  tft.drawString(pitStr, 120, 165, 4);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Turn Encoder", 120, 205, 2);

  //! DEBUG CODE BLOCK - Temporary
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString(bootResetReasonStr, 120, 228, 1);  // small font, clear of "Turn Encoder" above it
                                                    //! end DEBUG block
}

// Handles alternating text lines dynamically inside the display circle center
void CWG_LCD::printMenuLine(const char* c) {
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Map line 0 to upper center, line 1 to lower center
  uint16_t yPos = (menuPrintLine == 0) ? 110 : 150;

  // Clear the line horizontal track locally before redrawing text string
  tft.fillRect(20, yPos - 15, 200, 30, TFT_BLACK);

  // Add visual indicator accent color for standard selection menus
  tft.drawString(c, 120, yPos, 4);

  // Toggle back and forth between line indexes mimicking character LCD behavior
  menuPrintLine = (menuPrintLine == 0) ? 1 : 0;
}

// Identical track logic without custom markers
void CWG_LCD::printMenuLine_noArrow(const char* c) { printMenuLine(c); }

// Dedicated menu rendering for tweaking parameters on the fly
void CWG_LCD::showTemperatureTargetAdjustment(float temporaryTemperatureTarget,
                                              bool meatTargetFlag) {
  clearScreen();
  if (meatTargetFlag) {
    drawUIFrame("SET MEAT TARGET");
  } else {
    drawUIFrame("SET PIT TARGET");
  }

  char tempBuffer[16];
  snprintf(tempBuffer, sizeof(tempBuffer), "%.0f Value", temporaryTemperatureTarget);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString(tempBuffer, 120, 120, 4);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Click to Save", 120, 180, 2);
}

// ====================================================================
// Implementation of New UI Screen Stubs
// ====================================================================

void CWG_LCD::showLaunchPad() {
  clearScreen();
  drawUIFrame("LAUNCH PAD");

  tft.setTextDatum(MC_DATUM);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Ready to Cook", 120, 95, 2);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Press: Start Cook", 120, 140, 2);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Dbl-Press: Setup", 120, 165, 2);
}

// Draws small dots along the bottom arc of the round display, one per menu item,
// with the current selection lit up gold. currentIndex is 1-based.
void CWG_LCD::drawProgressDots(int currentIndex, int totalItems) {
  const int centerX = 120, centerY = 120;
  const int ringRadius = 95;
  const int dotRadius = 5;

  const float arcCenterDeg = 90.0f;
  const float arcSpanDeg = 100.0f;

  for (int i = 1; i <= totalItems; i++) {
    // Reversed vs. before: item 1 now starts at the left end of the arc,
    // so turning the encoder clockwise (increasing index) sweeps the
    // highlighted dot left -> right, matching the direction of the turn.
    float angleDeg = (totalItems == 1) ? arcCenterDeg
                                       : (arcCenterDeg + arcSpanDeg / 2.0f) -
                                             (arcSpanDeg * (i - 1) / (totalItems - 1));
    float angleRad = angleDeg * (PI / 180.0f);
    int dx = centerX + (int)(ringRadius * cos(angleRad));
    int dy = centerY + (int)(ringRadius * sin(angleRad));
    uint16_t color = (i == currentIndex) ? TFT_GOLD : TFT_DARKGREY;
    tft.fillCircle(dx, dy, dotRadius, color);
  }
}

void CWG_LCD::showSettingsMenu(int16_t prevEncoderValue) {
  clearScreen();
  drawUIFrame("SETTINGS");

  static const char* menuLabels[MENU_COUNT + 1] = {"",  // index 0 unused - menu values are 1-based
                                                   "Meat Temp", "Pit Temp", "Units", "Back"};

  int16_t index = constrain(prevEncoderValue, 1, (int16_t)MENU_COUNT);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(menuLabels[index], 120, 120, 4);

  drawProgressDots(index, MENU_COUNT);
}

void CWG_LCD::showSetMeatDoneTempMenu(int16_t prevEncoderValue, float meatDoneTemp, bool degCFlag) {
  clearScreen();
  drawUIFrame("MEAT TARGET");

  char currentValueStr[20];
  if (degCFlag) {
    snprintf(currentValueStr, sizeof(currentValueStr), "Current: %.0fC",
             convertDegFtoDegC(meatDoneTemp));
  } else {
    snprintf(currentValueStr, sizeof(currentValueStr), "Current: %.0fF", meatDoneTemp);
  }

  int16_t index = constrain(prevEncoderValue, 1, 2);
  const char* label = (index == 1) ? "Edit" : "Back";

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(currentValueStr, 120, 90, 2);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(label, 120, 130, 4);

  drawProgressDots(index, 2);
}

void CWG_LCD::showSetPitTempTargetMenu(int16_t prevEncoderValue, float pitTempTarget,
                                       bool degCFlag) {
  clearScreen();
  drawUIFrame("PIT TARGET");

  char currentValueStr[20];
  if (degCFlag) {
    snprintf(currentValueStr, sizeof(currentValueStr), "Current: %.0fC",
             convertDegFtoDegC(pitTempTarget));
  } else {
    snprintf(currentValueStr, sizeof(currentValueStr), "Current: %.0fF", pitTempTarget);
  }

  int16_t index = constrain(prevEncoderValue, 1, 2);
  const char* label = (index == 1) ? "Edit" : "Back";

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(currentValueStr, 120, 90, 2);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(label, 120, 130, 4);

  drawProgressDots(index, 2);
}

void CWG_LCD::showSetTempUnitsMenu(int16_t prevEncoderValue, bool degCFlag) {
  clearScreen();
  drawUIFrame("UNITS");

  char currentUnitStr[20];
  snprintf(currentUnitStr, sizeof(currentUnitStr), "Current: deg%s", degCFlag ? "C" : "F");

  int16_t index = constrain(prevEncoderValue, 1, 2);
  const char* label = (index == 1) ? "Toggle" : "Back";

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(currentUnitStr, 120, 90, 2);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(label, 120, 130, 4);

  drawProgressDots(index, 2);
}

void CWG_LCD::showBBQStatusScreen(bool degCFlag, float currentMeatTemp, float meatDoneTemp,
                                  float currentPitTemp, float pitTempTarget) {
  clearScreen();

  const int cx = 120, cy = 120;
  const int pitRadius = 118, pitThickness = 14;
  const int meatRadius = 96, meatThickness = 14;

  // Clamp to [0,1] so an over-target probe doesn't overflow the ring past full
  float pitFraction = constrain(currentPitTemp / pitTempTarget, 0.0f, 1.0f);
  float meatFraction = constrain(currentMeatTemp / meatDoneTemp, 0.0f, 1.0f);

  uint16_t pitAngle = (uint16_t)(360.0f * pitFraction);
  uint16_t meatAngle = (uint16_t)(360.0f * meatFraction);

  // Dim background "track" rings (full circle) drawn first
  tft.drawSmoothArc(cx, cy, pitRadius, pitRadius - pitThickness, 0, 360, TFT_DARKGREY, TFT_BLACK,
                    true);
  tft.drawSmoothArc(cx, cy, meatRadius, meatRadius - meatThickness, 0, 360, TFT_DARKGREY, TFT_BLACK,
                    true);

  // Progress arcs drawn on top, clockwise from 12 o'clock
  if (pitAngle > 0) {
    tft.drawSmoothArc(cx, cy, pitRadius, pitRadius - pitThickness, 0, pitAngle, TFT_ORANGE,
                      TFT_BLACK, true);
  }
  if (meatAngle > 0) {
    tft.drawSmoothArc(cx, cy, meatRadius, meatRadius - meatThickness, 0, meatAngle, TFT_CYAN,
                      TFT_BLACK, true);
  }

  // Center digits
  const char* unit = degCFlag ? "C" : "F";
  float pitDisplay = degCFlag ? convertDegFtoDegC(currentPitTemp) : currentPitTemp;
  float meatDisplay = degCFlag ? convertDegFtoDegC(currentMeatTemp) : currentMeatTemp;

  char pitStr[16], meatStr[16];
  snprintf(pitStr, sizeof(pitStr), "%.0f%s", pitDisplay, unit);
  snprintf(meatStr, sizeof(meatStr), "%.0f%s", meatDisplay, unit);

  tft.setTextDatum(MC_DATUM);

  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawString("PIT", 120, 85, 2);
  tft.drawString(pitStr, 120, 108, 4);

  tft.drawFastHLine(70, 120, 100, TFT_DARKGREY);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("MEAT", 120, 133, 2);
  tft.drawString(meatStr, 120, 156, 4);
}

void CWG_LCD::display() {
#ifdef PIN_TFT_BLK
  digitalWrite(PIN_TFT_BLK, HIGH);
#endif
}

void CWG_LCD::noDisplay() {
#ifdef PIN_TFT_BLK
  digitalWrite(PIN_TFT_BLK, LOW);
#endif
}