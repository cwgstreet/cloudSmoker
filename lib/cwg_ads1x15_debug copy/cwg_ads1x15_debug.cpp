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

// set up debug scaffold; comment out following line if you want to "turn off" serial monitor debugging
//#define DEBUG 1

// include 3rd party libraries
#include <ADS1X15.h>  // ADS1x15 I2C ADC device functionality
#include <Arduino.h>  // Arduino framework; not explicitly included in PlatformIO
//#include <MedianFilterLib.h>  // fast, efficient median filter
#include <Wire.h>  // i2C device communication

// incliude local libraries
#include <cwg_ads1x15.h>
#include <myConstants.h>  // all constants together in one file
#include <periphials.h>

// CWG_ADS1015 ads1015;  // instantiate ADS1015 ADC object at default address (ADDR conneced to GND)
CWG_ADS1015 ads1015(0x48);  // instantiate ADS1015 ADC object at default address (ADDR conneced to GND)

/******************************************************
//
*******************************************************/

/* ----------------------------------------
 *  initialise() method: set up ADC hardware and configure programmable settings
 *    gainSetting: (gainSetting/±Max Volage) 0/6.144V default, 1/4.096V, 2/2.048V, 4/1.024V, 8/0.512V, 16/0.256V,
 *    modeSetting: 0 = Continuous, 1 = Single Shot (default)
 *    dataRateSetting: (rateSetting/samples per second) 0/128, 1/250, 2/490, 3/920, 4/1660 default, 5/2400, 6/3300, 7/3300
 ** --------------------------------------*/
void CWG_ADS1015::initialise(uint8_t gainSetting, uint8_t modeSetting, uint8_t dataRateSetting) {
    Wire.begin(I2C_SDA, I2C_SCL);
    ads1015.begin();
    if (!ads1015.isConnected()) {
        Serial.print(F(" **ERROR** ADS1x15 not connected! "));
    }
    ads1015.setGain(gainSetting);          // PGA max voltage ±6.144V -> for 5V setup
    ads1015.setMode(modeSetting);          // SINGLE SHOT MODE
    ads1015.setDataRate(dataRateSetting);  // 4 == default ADS1015 data rate of 1600 sps; available range 128-3300 sps
    ads1015.requestADC(A0);                // parameters not set in the device until an explicit read/request of the ADC (any read call will do)
}

/* ----------------------------------------
 *  getVoltageFactor_V() method: retuns voltage factor in volts; voltage factor dependent on ADS1015 PGA settings
 ** --------------------------------------*/
float CWG_ADS1015::getVoltageFactor_V() {
    float voltageFactor_V = 0;
    voltageFactor_V = ads1015.toVoltage();
    return voltageFactor_V;
}

/* ----------------------------------------
 *  getSensorValue_MedianFiltered_V() method: acquires requested number (windowSize) of sensor readings from designated ADC pin and returns median filtered voltage in Volts
 ** --------------------------------------*/
float CWG_ADS1015::getSensorValue_MedianFiltered_V(uint8_t pin, int windowSize) {
    //  first adjust windowSize, if necessary, for optimum median filter
    if (windowSize < 3) {
        windowSize = 3;  // minimum window size of 3
    }
    if ((windowSize % 2) == 0) {
        windowSize += 1;  // make window size odd to avoid having to average between samples
    }

    //! temp. code comment next line
    // MedianFilter<float> medianFilter(windowSize);  //instantiate MedianFilter<T=float>
    int16_t ADCPinReading[windowSize];
    float valueADC_V[windowSize];
    float voltageFactor = getVoltageFactor_V();

    // debug code follows - delete after testing
    float sum = 0.0;

#if DEBUG
    Serial.print(F("voltageFactor = "));
    Serial.println(voltageFactor, 10);
#endif

    uint8_t i;
    for (i = 0; i < windowSize;) {
        yield();
        if (ads1015.isReady()) {
            ADCPinReading[i] = ads1015.getValue();  // get value from conversion
            ads1015.requestADC(pin);                // request a new conversion
            valueADC_V[i] = ADCPinReading[i] * voltageFactor;
            //! medianFilter.AddValue(valueADC_V[i]);

#if DEBUG
            Serial.print(F("\tIteration #"));
            Serial.print(i);
            Serial.print(F("\t"));
            Serial.print(F("\tAnalog"));
            Serial.print(pin);
            Serial.print(F(": "));
            Serial.print(ADCPinReading[i]);
            Serial.print(F("\t"));
            Serial.println(valueADC_V[i], 7);
#endif  // end DEBUG

            //! debug code follows - delete after testing
            sum += valueADC_V[i];
            // end debug code

            i += 1;  //  only increment loop when ADC.isReady == true or you'll skip values in loops when ADC is busy
        }
    }

    //! temp. code comment next line and delete line after
    // float valueADC_medianFiltered_V = medianFilter.GetFiltered();
    float valueADC_medianFiltered_V = sum / windowSize;

#if DEBUG
    // Serial.println();
    Serial.print(F("\tMedian Filtered Analog0 = "));
    Serial.println(valueADC_medianFiltered_V, 4);
#endif  // end DEBUG

    return valueADC_medianFiltered_V;
}