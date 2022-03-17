/* ***************************************************************
 * cwg_ads1x15.h - library to extend ads1x15 library functionality
 *   includes
 *    1) RobTillaart/ADS1X15 library, licenced under MIT Licence
 *         reference:  https://github.com/RobTillaart/ADS1X15
 *    2) Luis Llamas/Arduino-MedianFilter, licenced under Apache Licence 2.0
 *         reference: https://github-com.translate.goog/luisllamasbinaburo/Arduino-MedianFilter?_x_tr_sl=auto&_x_tr_tl=en&_x_tr_hl=en
 *
 *  C W Greenstreet, Ver1, 17Feb2022
 *    MIT Licence - Released into the public domain
 *
 ** ************************************************************* */

#ifndef CWG_ADS1x15_H
#define CWG_ADS1x15_H

#if ARDUINO >= 100  // this if-else block manages depreciated versions of Arduino IDE
#include <Arduino.h>
#else
#include <WConstants.h>
#include <WProgram.h>
#include <pins_arduino.h>
#endif  // end if-block

/******************************************************
//   Required library
*******************************************************/
#include <ADS1X15.h>  // ADS1x15 I2C ADC device functionality

/******************************************************
// Class definition:
//   CWG_ADS1015 is child (derrived) class of public library ADS1x15::ADS1015
******************************************************/

class CWG_ADS1015 : public ADS1015 {
   public:
    // constructors of derrived class
    CWG_ADS1015() : ADS1015() {
    }
    CWG_ADS1015(uint8_t Address = ADS1015_ADDRESS, TwoWire *wire = &Wire) : ADS1015(Address = ADS1015_ADDRESS) {
    }
    virtual ~CWG_ADS1015() {
    }

    // methods (function):
    void initialise(uint8_t gainSetting, uint8_t modeSetting, uint8_t dataRateSetting);
    float getVoltageFactor_V();
    float getSensorValue_MedianFiltered_V(uint8_t pin, int windowSize);
    // float getMedFiltered();

   protected:
    //
};

// ensure objects are visable everywhere (global)
extern CWG_ADS1015 ads1015;

//==============================================================================================
// For reference:
//  ADS1X15 ADCs do not take an external reference voltage but rely on an internal full scale
//   voltage (fsv) that is a function of the PGA gain setting - see table below
//
//  Also note that AD1015 is notionally 12 bit but only for full scale differential measurements
//     with the one bit used for sign (neg / positive)
//   When single shot, the ADS1015 ADC is effectively 11 bit (2^11 = 2048 measurements)
//  For example, using default 2/3x gain, 6.144V fsv / 2048 measurements = 0.003 or 3mV LSB
// -------------------------
// PGA gain settings, max Voltage and LSB (least significant bit)
//   taken from Adafruit ADS1x15 library, where bit = LSB
//                                                                ADS1015  ADS1115
//                                                                -------  -------
// ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
// ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
// ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
// ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
// ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
// ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
//==============================================================================================

#endif  // end header guard