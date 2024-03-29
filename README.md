
<img align="right" src="Images/CloudSmoker-Icon-multicolour-txt-no-bg-resized.png#gh-light-mode-only" width="600"/> 
<img align="right" src="Images/CloudSmoker-Icon-multicolour-txt-with-bg-resized.jpg#gh-dark-mode-only" width="600"/>  
<br>
<br>
<br>
<br>
<br>
<br>
<br>

# cloudSmoker #
**WiFi BBQ Temperature Monitor Project**

[comment]: <> (This project exists as a hands-on opportunity to learn more about electronics, IoT, Arduino / ESP8266 programming, GitHub markdown, etc)

[![GitHub license](https://img.shields.io/github/license/Naereen/StrapDown.js.svg)](https://github.com/Naereen/StrapDown.js/blob/master/LICENSE)
<a href="https://thingspeak.com/channels/1679986">
   <img src="https://img.shields.io/badge/Thingspeak-cloudSmoker%20Dashboard-blue?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACgAAAAjCAYAAADmOUiuAAAABHNCSVQICAgIfAhkiAAABI9JREFUWIXVl1toHFUYx3/nzMnMprubG0Y3jdHWSKlikSqiIpWi4kvBBx98EcWHoj4UDYIoEYsIFq0KfVD0SfooiqBQqIogLdhKMa0mfarSpCFNY8ylyW5uuzvn82FmZze7k+YucWD5c3bO9z//853zXUaZpz4VBFCwFVEjEg62JmoIB1sUdeBP2KoYetCyVdHEuVUpEBEevaed4689ydj0HG6dg4iglGK9j4gEV0zBiXP9HP26h+xcvkYHgAGpiR6FQhCaUx4drSlaG+tJuM66hZUFgm8txtEMjmZxTbh4TBSbaCuVqBVYwa3T+L7Pwdffo//KIC1NDfi+jexXJyrgd7RiKjvDjo7tfPT2q+jcMMXhC0hyF0o7VOsx0ZZi0GjF5OQkZ345zcDAIForrLXr8p7WGmuFkZ23MzH5PIYiMnsd6i0oXaMj3oMEKALGGNx0K/rO3ehEGvJzIdEasq5SQVRqQ117CybZAvZKsC4Vzqn1oA1FVWLFTkwCUhlIJMGGfGstDUqhRFCppuDaScWJxOhYQR4sGdpl5q0NVXRq8e9jozh4V2kUjktEa3DcivljazEsGShl+w0uY9XPEvOWqSTVRhtcKWrE1c7TsVEcYUgQdRdLzVsjsjz//8SDsWI2+Q6ukL8cxVCFy0RxzfwV4qIoXqSwQlz5r5XlwTWrWQnemH+Zjrra5r893qCjXobEWosVu+HiRARrK4Te+A6GBTZCP5zj47ouzQ0pEjNCU2MCGzYRpVSgtUIBufkCM3P5RTyeUTQk6wFBwi5Ta81Ubp6Gehe3rpzlAh1+lQ6LiW0UlAaxzOeLpFIpvvj4LRZ8jed5QehI0FmXMFFnuDQ0zrFvznGqdyCcY3n8vk66n32Ehm0evhVEBK0UCwWfbZ5hZ6aJi+cLQboVC8qhWo9BJNwfEVrrg8DIeJaei38xMT6GSd8Es4XocwCCQu/7FhHhobu289Xhpzn65Rk+P9FDdmaBtpYkezszDIxMcHVsCmOcyPZ6Vrg6OsnZnj7yhTxBe2cX6RAk8GCpoyh3FgqxPuf/HObAoQ/JXz6NTmdQ2kH8YhQzJS9aa3ng3rvp7nqJIwcf4/5dGbo++Z7pmXnm80Xe/eAY3578mUTCq9icRonPwvQ/zNa3Q7MfddSVegwIYsMPpQoEyOeLjM1YrJ+EXL7s/sUJDaUcfjr7B719h+jueplXXnyOm5tTDI1OsZAvMDl6jdm5eWaLFVep9DjNqERLsFlra3QEHgw76EUYLi6pW9CdT4ScSzWhGiXCWO5v3nz/M37v7ePI4TfYt+c2JqayGNdDb98L6TawBYLsVrK3oEz4tSg1OnTp7JdCpTQYF+W44NTFozbBr/FWCu0Pc/zkbxx45gV+PPUrLY1pXM8Dxw024nhV9h6ERxu3vqN27HundPHXgxCOtUGl2xgZz3Lqh+/IZae41D/E5QmL9lKr5lVmf7dUp42NQpsbRa5dQGYncO7YD6nMqnl0FNqbgDrZiup4EN26m+hmr5LHKBEEG+b5jUftppHMnmAs/urtN9ODG4FRHqSU/7YY/gubNGDxQtn8dQAAAABJRU5ErkJggg==&?link=&link=https://thingspeak.com/channels/1679986"> </a> 


<img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white"> 

<br>

### Updates 🆕
1. **Removed pitTemp probe plug-in checks** - no longer displays LCD error or transmits pitTemp values to ThingSpeak if pit temperature probe is unplugged; necessary to allow pitTemp probe to be unplugged after meat is taken off grill so that resting meat temps can continue to be monitored (common to rest briskett in faux cambo for ~1 hour after removing meat from grill).
<br>
<br>

## Project Summary

The cloudSmoker Barbecue Monitor project is a WiFi-enable temperature monitor.  cloudSmoker is designed to read two temperatures (pit and internal meat temperatures) within my Weber BBQ smoker and upload these to an IOT dashboard, allowing remote cloud monitoring of the cook progress showing temperature vs time graphs, gauges, etc, user-definable cooking notification setpoints and exception push notifications to my smartphone, say if the pit temperatures go outside of acceptable ranges and when the meat is done and ready to be removed.

The overall process flow is conceptualised as follows:

<img src="https://raw.githubusercontent.com/cwgstreet/cloudSmoker/main/Images/CloudSmoker%20Process%20Diagram.jpg"> 

Hardware is based around the NodeMCU (V1.0) ESP8266-12E microcontroller module (for both for WiFi access and microcontroller), two Maverick ET-732/733 thermistor probes, an external ADS1015 "12-bit" ADC, plus a 1602 LCD and rotary encoder for user set-up configuration and display.  Minor thermistor and power filtering was also employed for stability.  

Coding was done using VS Code / PlatformIO IDE platform with data uploaded to ThingSpeak for display and further processing.   Push exception notifications are done through Twitter using Thingspeak's React / ThingTweet functionality.  (:warning: Todo:  export cook logs to Google Sheets).


### Project Overview  ###

An extensive project overview and design process description can be found in the project [Wiki](https://github.com/cwgstreet/cloudSmoker/wiki) covering design goals, hardware, software, etc.  


### Hardware ###

<img align="right" width="300" height="400"  src="Images/Cook Images/cloudSmoker_unbuttoned.JPG">

The Hardware folder contains KiCad schematic, PCB board layouts (:warning: TODO) and selected datasheets (including the datasheet for the Semitec 105GT-2 Thermistor, that Maverick customer support kindly provided me.  :clap: :thumbsup: 

Note that I have not found this datasheet available anywhere on the web and many other projects incorrectly characterised the Maverick ET-732/733 probes as being based on a 10k ohm thermistor (when it's actually 1M ohm!).  This incorrect assumption frequebtly led to poor bias resistor selection, resulting in poor measurement acuracy.  As a result, these Maverick thermistors were often not highly regrded.  For example, one project concluded they were [acceptable, but not recommended thermistor choice](https://github.com/CapnBry/HeaterMeter/wiki/HeaterMeter-Probes).   

### cloudSmoker ###

This folder contains the full cloudSmoker software (sketch and libraries); developed under VSCode / Platformio IDE rather than Arduino IDE so folder structure reflect Platformio structure.  Note that the project code base is over 1600 lines total and makes extensive use of user and public libraries.

### Images ###

Self-explanatory

### Project Dashboard ###
A public ThingSpeak [cloudSmoker dashboard](https://thingspeak.com/channels/1679986) is available


<p align="center">
  <img  src="Images/Cook Images/cloudSmoker-dashboard.JPG">
</p>

### Licence ###

MIT Licence

--- END ---


<p align="center">
  <img  src="Images/Cook Images/brisket-final.JPG">
</p>
