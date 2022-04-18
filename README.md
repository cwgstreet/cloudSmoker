<img style="float: right;" src="https://github.com/cwgstreet/cloudSmoker/blob/8ebc0d06341cf46aa576d8159f0cbd5d78bda683/Images/CloudSmoker-Icon-multicolour-txt-no-bg-resized.png" width="600"/> 




# Cloud Smoker BBQ Monitor Project

*This project exists as a hands-on opportunity to learn more about electronics, IoT, Arduino / ESP8266 programming, GitHub markdown, etc*

[![GitHub license](https://img.shields.io/github/license/Naereen/StrapDown.js.svg)](https://github.com/Naereen/StrapDown.js/blob/master/LICENSE)
<br>
<img src="https://img.shields.io/badge/Thingspeak-cloudSmoker%20Dashboard-blue?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACgAAAAjCAYAAADmOUiuAAAABHNCSVQICAgIfAhkiAAABI9JREFUWIXVl1toHFUYx3/nzMnMprubG0Y3jdHWSKlikSqiIpWi4kvBBx98EcWHoj4UDYIoEYsIFq0KfVD0SfooiqBQqIogLdhKMa0mfarSpCFNY8ylyW5uuzvn82FmZze7k+YucWD5c3bO9z//853zXUaZpz4VBFCwFVEjEg62JmoIB1sUdeBP2KoYetCyVdHEuVUpEBEevaed4689ydj0HG6dg4iglGK9j4gEV0zBiXP9HP26h+xcvkYHgAGpiR6FQhCaUx4drSlaG+tJuM66hZUFgm8txtEMjmZxTbh4TBSbaCuVqBVYwa3T+L7Pwdffo//KIC1NDfi+jexXJyrgd7RiKjvDjo7tfPT2q+jcMMXhC0hyF0o7VOsx0ZZi0GjF5OQkZ345zcDAIForrLXr8p7WGmuFkZ23MzH5PIYiMnsd6i0oXaMj3oMEKALGGNx0K/rO3ehEGvJzIdEasq5SQVRqQ117CybZAvZKsC4Vzqn1oA1FVWLFTkwCUhlIJMGGfGstDUqhRFCppuDaScWJxOhYQR4sGdpl5q0NVXRq8e9jozh4V2kUjktEa3DcivljazEsGShl+w0uY9XPEvOWqSTVRhtcKWrE1c7TsVEcYUgQdRdLzVsjsjz//8SDsWI2+Q6ukL8cxVCFy0RxzfwV4qIoXqSwQlz5r5XlwTWrWQnemH+Zjrra5r893qCjXobEWosVu+HiRARrK4Te+A6GBTZCP5zj47ouzQ0pEjNCU2MCGzYRpVSgtUIBufkCM3P5RTyeUTQk6wFBwi5Ta81Ubp6Gehe3rpzlAh1+lQ6LiW0UlAaxzOeLpFIpvvj4LRZ8jed5QehI0FmXMFFnuDQ0zrFvznGqdyCcY3n8vk66n32Ehm0evhVEBK0UCwWfbZ5hZ6aJi+cLQboVC8qhWo9BJNwfEVrrg8DIeJaei38xMT6GSd8Es4XocwCCQu/7FhHhobu289Xhpzn65Rk+P9FDdmaBtpYkezszDIxMcHVsCmOcyPZ6Vrg6OsnZnj7yhTxBe2cX6RAk8GCpoyh3FgqxPuf/HObAoQ/JXz6NTmdQ2kH8YhQzJS9aa3ng3rvp7nqJIwcf4/5dGbo++Z7pmXnm80Xe/eAY3578mUTCq9icRonPwvQ/zNa3Q7MfddSVegwIYsMPpQoEyOeLjM1YrJ+EXL7s/sUJDaUcfjr7B719h+jueplXXnyOm5tTDI1OsZAvMDl6jdm5eWaLFVep9DjNqERLsFlra3QEHgw76EUYLi6pW9CdT4ScSzWhGiXCWO5v3nz/M37v7ePI4TfYt+c2JqayGNdDb98L6TawBYLsVrK3oEz4tSg1OnTp7JdCpTQYF+W44NTFozbBr/FWCu0Pc/zkbxx45gV+PPUrLY1pXM8Dxw024nhV9h6ERxu3vqN27HundPHXgxCOtUGl2xgZz3Lqh+/IZae41D/E5QmL9lKr5lVmf7dUp42NQpsbRa5dQGYncO7YD6nMqnl0FNqbgDrZiup4EN26m+hmr5LHKBEEG+b5jUftppHMnmAs/urtN9ODG4FRHqSU/7YY/gubNGDxQtn8dQAAAABJRU5ErkJggg==&?link=&link=https://thingspeak.com/channels/1679986"> 
<-Link to project public dashboard



## Project Summary

The Cloud Smoker Barbecue Monitor project is an IoT device / workflow to read two temperatures (pit and internal meat temperatures) within my Weber BBQ smoker and upload these to a cloud based dashboard allowing me to remotely monitor cook progress, view temp vs time graphs, gaugest etc within a dashboard, define cooking notification setpoints and to received push notifications to my smartphone if pit temperatures go outside acceptable ranges and when the meat is done and ready to be removed.

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
