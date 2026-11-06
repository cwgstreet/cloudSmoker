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
CWG_LCD::CWG_LCD() : tft(), menuPrintLine(0) {}
// CWG_LCD::CWG_LCD() : tft(TFT_eSPI()), menuPrintLine(0) {}

// Initialize display and manage backlight pin from platformio.ini
void CWG_LCD::begin() {
#ifdef PIN_DISP_BL
  pinMode(PIN_DISP_BL, OUTPUT);
  digitalWrite(PIN_DISP_BL, HIGH);  // Turn backlight on
#endif

  tft.init();
  tft.setRotation(2);  // Adjust 0-3 if your physical screen orientation is upside down
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


void CWG_LCD::showCalProbeMenu(int16_t prevEncoderValue) {
  clearScreen();
  drawUIFrame("CALIBRATE");

  static const char* calLabels[6] = {"",  // index 0 unused - menu values are 1-based
                                     "PIT", "MEAT1", "MEAT2", "MEAT3", "Back"};
  int16_t index = constrain(prevEncoderValue, 1, 5);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(calLabels[index], 120, 120, 4);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  if (index == 5) {
    tft.drawString("Press: back", 120, 160, 2);
  } else {
    tft.drawString("Press: calibrate", 120, 150, 2);
    tft.drawString("Dbl-press: reset", 120, 170, 2);
  }

  drawProgressDots(index, 5);
}

void CWG_LCD::showCalCapture(const char* header, const char* message, float value, bool hasValue,
                             const char* hint) {
  clearScreen();
  drawUIFrame(header);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(message, 120, 90, 2);

  char valueStr[16];
  if (hasValue) {
    snprintf(valueStr, sizeof(valueStr), "%.1fF", value);
  } else {
    snprintf(valueStr, sizeof(valueStr), "--");
  }
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(valueStr, 120, 130, 4);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(hint, 120, 170, 2);
}

void CWG_LCD::showCalResult(const char* probeName, float gain, float offset, bool showValues,
                            const char* note, const char* hint) {
  clearScreen();
  drawUIFrame("RESULT");

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(probeName, 120, 80, 2);

  char buf[24];
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (showValues) {
    snprintf(buf, sizeof(buf), "gain %.4f", gain);
    tft.drawString(buf, 120, 110, 2);
    snprintf(buf, sizeof(buf), "offset %+.2fF", offset);
    tft.drawString(buf, 120, 135, 2);
  }
  if (note[0] != '\0') {
    tft.drawString(note, 120, 158, 2);
  }

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(hint, 120, 175, 2);
}

void CWG_LCD::showRailStatus(float ina226V, float storedV) {
  clearScreen();
  drawUIFrame("RAIL VOLTAGE");

  char buf[32];
  tft.setTextDatum(MC_DATUM);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (isnan(ina226V)) {
    snprintf(buf, sizeof(buf), "INA226: no device");
  } else {
    snprintf(buf, sizeof(buf), "INA226 %.3f V", ina226V);
  }
  tft.drawString(buf, 120, 100, 2);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  snprintf(buf, sizeof(buf), "used %.3f V", storedV);
  tft.drawString(buf, 120, 130, 2);

  tft.drawString("Hold: return", 120, 175, 2);
}

void CWG_LCD::showEndCookConfirm() {
  clearScreen();
  drawUIFrame("END COOK?");

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Tap: end cook", 120, 110, 2);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Hold: keep cooking", 120, 140, 2);
}

void CWG_LCD::showListMenu(const char* header, const char* const* labels,
                           const char* const* details, uint8_t count, uint8_t selected) {
  clearScreen();
  drawUIFrame(header);
  if (count == 0) {
    return;
  }
  uint8_t index = constrain(selected, (uint8_t)1, count);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(labels[index - 1], 120, 110, 4);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(details[index - 1], 120, 150, 4);

  drawProgressDots(index, count);
}

void CWG_LCD::showResumePrompt(bool timeUnverified) {
  clearScreen();
  drawUIFrame("RESUME COOK?");

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Tap: resume", 120, 105, 2);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Hold: restart", 120, 135, 2);

  if (timeUnverified) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("time unverified", 120, 170, 2);
  }
}

// Shown while setup() is blocked (Wi-Fi connect and portal timeout). loop() is not running
// yet, so the state machine cannot draw anything; the splash screen replaces this message
// once loop() starts.
void CWG_LCD::showInitMessage(const char* line1, const char* line2) {
  clearScreen();
  drawUIFrame("STARTING");

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(line1, 120, 100, 2);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(line2, 120, 135, 2);
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
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(currentUnitStr, 120, 90, 4);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(label, 120, 130, 4);

  drawProgressDots(index, 2);
}

// Progress fraction for an arc. Missing probe (NaN) or a bad target gives an empty arc.
static float progressFraction(float temp, float target) {
  if (isnan(temp) || target <= 0.0f) {
    return 0.0f;
  }
  return constrain(temp / target, 0.0f, 1.0f);
}


void CWG_LCD::display() {
#ifdef PIN_DISP_BL
  digitalWrite(PIN_DISP_BL, HIGH);
#endif
}

void CWG_LCD::noDisplay() {
#ifdef PIN_DISP_BL
  digitalWrite(PIN_DISP_BL, LOW);
#endif
}


void CWG_LCD::showMeatTargetEdit(const char* label, int16_t value, bool degCFlag, bool belowSafe) {
  clearScreen();

  char header[20];
  snprintf(header, sizeof(header), "%s TARGET", label);
  drawUIFrame(header);

  char valueStr[12];
  snprintf(valueStr, sizeof(valueStr), "%d%c", value, degCFlag ? 'C' : 'F');

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(valueStr, 120, 100, 4);

  if (belowSafe) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("BELOW SAFE MINIMUM", 120, 135, 2);
    tft.drawString("USDA whole cuts: 145F", 120, 152, 2);
    tft.drawString("(63C), 3 min rest", 120, 169, 2);
  }

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Tap: save  Hold: back", 120, 200, 2);
}

void CWG_LCD::showBBQStatusScreen(bool degCFlag, float currentMeatTemp, const char* meatLabel,
                                  float meatDoneTemp, float currentPitTemp, float pitMinTemp,
                                  float pitMaxTemp) {
  clearScreen();

  const int cx = 120, cy = 120;
  const int pitRadius = 118, pitThickness = 14;
  const int meatRadius = 96, meatThickness = 14;

  // PIT ring fills across the band: from Min up to Max. With Min Off, it starts at 50F.
  float pitLow = isnan(pitMinTemp) ? 50.0f : pitMinTemp;
  float pitSpan = pitMaxTemp - pitLow;
  float pitFraction = 0.0f;
  if (pitSpan > 0 && !isnan(currentPitTemp)) {
    pitFraction = constrain((currentPitTemp - pitLow) / pitSpan, 0.0f, 1.0f);
  }
  float meatFraction = progressFraction(currentMeatTemp, meatDoneTemp);

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

  // Centre digits
  const char* unit = degCFlag ? "C" : "F";
  float pitDisplay = degCFlag ? convertDegFtoDegC(currentPitTemp) : currentPitTemp;
  float meatDisplay = degCFlag ? convertDegFtoDegC(currentMeatTemp) : currentMeatTemp;

  char pitStr[16], meatStr[16];
  if (isnan(currentPitTemp)) {
    snprintf(pitStr, sizeof(pitStr), "--%s", unit);  // no probe
  } else {
    snprintf(pitStr, sizeof(pitStr), "%.0f%s", pitDisplay, unit);
  }
  if (isnan(currentMeatTemp)) {
    snprintf(meatStr, sizeof(meatStr), "--%s", unit);  // no probe
  } else {
    snprintf(meatStr, sizeof(meatStr), "%.0f%s", meatDisplay, unit);
  }

  tft.setTextDatum(MC_DATUM);

  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawString("PIT", 120, 85, 2);
  tft.drawString(pitStr, 120, 108, 4);

  tft.drawFastHLine(70, 120, 100, TFT_DARKGREY);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(meatLabel, 120, 133, 2);  // MEAT1, MEAT2 or MEAT3, or "MEAT" if none
  tft.drawString(meatStr, 120, 156, 4);
}

void CWG_LCD::showLimitEdit(const char* label, int16_t value, bool degCFlag, bool isOff) {
  clearScreen();

  char header[20];
  snprintf(header, sizeof(header), "%s", label);
  drawUIFrame(header);

  char valueStr[12];
  if (isOff) {
    snprintf(valueStr, sizeof(valueStr), "Off");
  } else {
    snprintf(valueStr, sizeof(valueStr), "%d%c", value, degCFlag ? 'C' : 'F');
  }

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(valueStr, 120, 110, 4);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Tap: save  Hold: back", 120, 160, 2);
}




// Name above its value, both centred on cx. Used by the overview and the zoom pages.
static void drawNameValue(TFT_eSPI& t, int cx, int nameY, int valueY, const char* name,
                          uint16_t colour, float temp, bool degCFlag) {
  char unit = degCFlag ? 'C' : 'F';
  char valStr[12];

  t.setTextDatum(MC_DATUM);
  t.setTextColor(colour, TFT_BLACK);
  t.drawString(name, cx, nameY, 4);

  if (isnan(temp)) {
    snprintf(valStr, sizeof(valStr), "--%c", unit);
  } else {
    float shown = degCFlag ? convertDegFtoDegC(temp) : temp;
    snprintf(valStr, sizeof(valStr), "%.0f%c", shown, unit);
  }
  t.setTextColor(TFT_WHITE, TFT_BLACK);
  t.drawString(valStr, cx, valueY, 4);
}

// Four probes in a 2x2 grid under a COOK heading
void CWG_LCD::showCookOverview(bool degCFlag, float pitTemp, float meat1Temp, float meat2Temp,
                               float meat3Temp) {
  clearScreen();
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("COOK", 120, 22, 2);
  tft.drawFastHLine(40, 40, 160, TFT_DARKGREY);
  tft.drawFastHLine(40, 124, 160, TFT_DARKGREY);
  tft.drawFastVLine(120, 48, 152, TFT_DARKGREY);

  drawNameValue(tft, 60, 72, 100, "PIT", TFT_ORANGE, pitTemp, degCFlag);
  drawNameValue(tft, 180, 72, 100, "MEAT1", TFT_CYAN, meat1Temp, degCFlag);
  drawNameValue(tft, 60, 150, 178, "MEAT2", TFT_GREEN, meat2Temp, degCFlag);
  drawNameValue(tft, 180, 150, 178, "MEAT3", TFT_MAGENTA, meat3Temp, degCFlag);
}


// Dims an RGB565 colour to a percentage of its brightness, for the empty part of a ring
static uint16_t dimColour(uint16_t c, int percent) {
  int r = ((c >> 11) & 0x1F) * percent / 100;
  int g = ((c >> 5) & 0x3F) * percent / 100;
  int b = (c & 0x1F) * percent / 100;
  return (uint16_t)((r << 11) | (g << 5) | b);
}

// One concentric ring: dim track in the probe's own colour, then the progress arc on top
static void drawZoomRing(TFT_eSPI& t, int outer, int thick, uint16_t colour, float fraction) {
  t.drawSmoothArc(120, 120, outer, outer - thick, 0, 360, dimColour(colour, 25), TFT_BLACK, true);
  uint16_t angle = (uint16_t)(360.0f * constrain(fraction, 0.0f, 1.0f));
  if (angle > 0) {
    t.drawSmoothArc(120, 120, outer, outer - thick, 0, angle, colour, TFT_BLACK, true);
  }
}

// Two probes stacked: top probe on the outer ring, bottom probe on the inner ring
void CWG_LCD::showZoomPage(bool degCFlag, const char* topName, uint16_t topColour, float topTemp,
                           float topFraction, const char* bottomName, uint16_t bottomColour,
                           float bottomTemp, float bottomFraction) {
  clearScreen();
  drawZoomRing(tft, 118, 14, topColour, topFraction);       // outer ring: top probe
  drawZoomRing(tft, 96, 14, bottomColour, bottomFraction);  // inner ring: bottom probe

  drawNameValue(tft, 120, 66, 98, topName, topColour, topTemp, degCFlag);
  tft.drawFastHLine(78, 120, 84, TFT_DARKGREY);
  drawNameValue(tft, 120, 142, 170, bottomName, bottomColour, bottomTemp, degCFlag);
}

void CWG_LCD::showSplashScreen(bool degCFlag, float meatTargetTemp, float pitMinTemp,
                               float pitMaxTemp, float bankCapacity_mAh, int bankPct) {
  clearScreen();
  drawUIFrame("cloudSmoker2");

  char meatStr[12];
  char pitStr[24];
  char bankStr[32];
  char unit = degCFlag ? 'C' : 'F';

  float meatShown = degCFlag ? convertDegFtoDegC(meatTargetTemp) : meatTargetTemp;
  float maxShown = degCFlag ? convertDegFtoDegC(pitMaxTemp) : pitMaxTemp;

  snprintf(meatStr, sizeof(meatStr), "Meat: %.0f%c", meatShown, unit);
  if (isnan(pitMinTemp)) {
    snprintf(pitStr, sizeof(pitStr), "Pit: OFF / %.0f%c", maxShown, unit);
  } else {
    float minShown = degCFlag ? convertDegFtoDegC(pitMinTemp) : pitMinTemp;
    snprintf(pitStr, sizeof(pitStr), "Pit: %.0f / %.0f%c", minShown, maxShown, unit);
  }
  snprintf(bankStr, sizeof(bankStr), "Bank %.0f mAh, %d%%", bankCapacity_mAh, bankPct);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("BBQ TARGETS", 120, 95, 2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(meatStr, 120, 130, 4);
  tft.drawString(pitStr, 120, 165, 4);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(bankStr, 120, 190, 2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Turn Encoder", 120, 210, 2);

  //! DEBUG CODE BLOCK - Temporary
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString(bootResetReasonStr, 120, 228, 1);
  //! end DEBUG block
}

void CWG_LCD::showValueEdit(const char* header, const char* valueText) {
  clearScreen();
  drawUIFrame(header);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(valueText, 120, 110, 4);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Tap: save  Hold: back", 120, 160, 2);
}

void CWG_LCD::showSettingsMenu(int16_t prevEncoderValue) {
  clearScreen();
  drawUIFrame("SETTINGS");

  // Index 0 unused: menu values are 1-based. One label per MENU_* entry, in order.
  static const char* menuLabels[MENU_COUNT + 1] = {"", "Meat Temp", "Pit Temp", "Units",
                                                   "Calibrate", "Rail", "Power Bank", "Back"};

  int16_t index = constrain(prevEncoderValue, 1, (int16_t)MENU_COUNT);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(menuLabels[index], 120, 120, 4);

  drawProgressDots(index, MENU_COUNT);
}
