<img style="float: right;" src="https://github.com/cwgstreet/cloudSmoker/blob/8ebc0d06341cf46aa576d8159f0cbd5d78bda683/Images/CloudSmoker-Icon-multicolour-txt-no-bg-resized.png" width="150"/> 


# Cloud Smoker BBQ Monitor Project
[![GitHub license](https://img.shields.io/github/license/Naereen/StrapDown.js.svg)](https://github.com/Naereen/StrapDown.js/blob/master/LICENSE)

**Note**:  *This project exists as a hands-on opportunity to learn more about electronics, IoT, Arduino / ESP8266 programming, GitHub markdown, etc*



<br>

⚠️<b>Work in Progress</b><br>
<br>
 </p>
 


## Project Summary

The Cloud Smoker Barbecue Monitor project is an IoT device / workflow to read two temperatures (pit and internal meat temperatures) within my Weber BBQ smoker and upload these to a cloud based dashboard allowing me to set cooking notification setpoints, view temp vs time graphs and dials within a dashboard and to received text notifications to my smartphone if pit temperatures go outside acceptable ranges and when the meat is done and ready to be removed.

The overall process flow is as follows:


<img src="https://raw.githubusercontent.com/cwgstreet/cloudSmoker/main/Images/CloudSmoker%20Process%20Diagram.jpg"> 


Hardware is based around the ESP8266 ESP-07 module (both for WiFi access and microcontroller), two Maverick ET-732 thermistor probes, and a 4066B switch to allow multiplexing due to limitation of having only the single ADC pin on the ESP-07.

Coding was done using the Arduino IDE with data uploaded to ThingSpeak for display and further processing.  IFTT used for text notifications and Google Sheets for creating barbeque logs.


### Project Overview  ###

A thorough project overview and description can be found in the project [Wiki](https://github.com/cwgstreet/cloudSmoker/wiki) covering design goals, hardware, software, etc.


### Hardware ###

The Hardware folder contains 1) KiCad schematics and PCB board layouts and 2) selected datasheets.

### cloudSmoker_uno ###

This folder contains the full CloudSmoker software (sketch and libraries); developed under VSCode / Platformio IDE rather than Arduino IDE so folders reflect Platformio structure

### Images ###

Self-explanatory


### Licence ###

MIT Licence

--- END ---
