EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:ESP8266
LIBS:74xgxx
LIBS:ac-dc
LIBS:actel
LIBS:allegro
LIBS:Altera
LIBS:analog_devices
LIBS:battery_management
LIBS:bbd
LIBS:bosch
LIBS:brooktre
LIBS:cmos_ieee
LIBS:dc-dc
LIBS:diode
LIBS:elec-unifil
LIBS:ESD_Protection
LIBS:ftdi
LIBS:gennum
LIBS:graphic_symbols
LIBS:hc11
LIBS:infineon
LIBS:intersil
LIBS:ir
LIBS:Lattice
LIBS:leds
LIBS:LEM
LIBS:logic_programmable
LIBS:maxim
LIBS:mechanical
LIBS:microchip_dspic33dsc
LIBS:microchip_pic10mcu
LIBS:microchip_pic12mcu
LIBS:microchip_pic16mcu
LIBS:microchip_pic18mcu
LIBS:microchip_pic24mcu
LIBS:microchip_pic32mcu
LIBS:modules
LIBS:motor_drivers
LIBS:motors
LIBS:msp430
LIBS:nordicsemi
LIBS:nxp_armmcu
LIBS:nxp
LIBS:onsemi
LIBS:Oscillators
LIBS:Power_Management
LIBS:powerint
LIBS:pspice
LIBS:references
LIBS:relays
LIBS:rfcom
LIBS:RFSolutions
LIBS:sensors
LIBS:silabs
LIBS:stm8
LIBS:stm32
LIBS:supertex
LIBS:switches
LIBS:transf
LIBS:triac_thyristor
LIBS:ttl_ieee
LIBS:video
LIBS:wiznet
LIBS:Worldsemi
LIBS:Xicor
LIBS:zetex
LIBS:Zilog
LIBS:CloudSmoker-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 4 6
Title "CloudSmoker - Wifi enabled BBQ Temperature Monitor"
Date "2017-09-14"
Rev "1"
Comp "Carl Greenstreet"
Comment1 ""
Comment2 "Manages multiple sensors through multiplex (MUX) using 4066 Quad bilateral switch"
Comment3 "Includes voltage scaling for ESP8266 0.2-1.25V ADC"
Comment4 "Thermister Temp Probes"
$EndDescr
$Comp
L 4066 U?
U 1 1 59BB773F
P 1900 2400
F 0 "U?" H 2100 2251 50  0000 C CNN
F 1 "4066" H 2100 2550 50  0000 C CNN
F 2 "" H 1900 2400 60  0001 C CNN
F 3 "" H 1900 2400 60  0001 C CNN
	1    1900 2400
	1    0    0    -1  
$EndComp
$Comp
L 4066 U?
U 2 1 59BB778B
P 3100 2400
F 0 "U?" H 3300 2251 50  0000 C CNN
F 1 "4066" H 3300 2550 50  0000 C CNN
F 2 "" H 3100 2400 60  0001 C CNN
F 3 "" H 3100 2400 60  0001 C CNN
	2    3100 2400
	1    0    0    -1  
$EndComp
$Comp
L 4066 U?
U 3 1 59BB77C2
P 4200 2400
F 0 "U?" H 4400 2251 50  0000 C CNN
F 1 "4066" H 4400 2550 50  0000 C CNN
F 2 "" H 4200 2400 60  0001 C CNN
F 3 "" H 4200 2400 60  0001 C CNN
	3    4200 2400
	1    0    0    -1  
$EndComp
$Comp
L 4066 U?
U 4 1 59BB77FC
P 5300 2400
F 0 "U?" H 5500 2251 50  0000 C CNN
F 1 "4066" H 5500 2550 50  0000 C CNN
F 2 "" H 5300 2400 60  0001 C CNN
F 3 "" H 5300 2400 60  0001 C CNN
	4    5300 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	1900 2150 5300 2150
Wire Wire Line
	1900 2650 5300 2650
Connection ~ 3100 2650
$Comp
L GND #PWR012
U 1 1 59BB79EF
P 4800 2750
F 0 "#PWR012" H 4800 2500 50  0001 C CNN
F 1 "GND" H 4800 2600 50  0000 C CNN
F 2 "" H 4800 2750 50  0001 C CNN
F 3 "" H 4800 2750 50  0001 C CNN
	1    4800 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	4800 2650 4800 2750
Connection ~ 4800 2650
$Comp
L +3.3V #PWR013
U 1 1 59BB7C0A
P 4800 2050
F 0 "#PWR013" H 4800 1900 50  0001 C CNN
F 1 "+3.3V" H 4800 2190 50  0000 C CNN
F 2 "" H 4800 2050 50  0001 C CNN
F 3 "" H 4800 2050 50  0001 C CNN
	1    4800 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3900 2400 3900 2150
Connection ~ 3900 2150
Wire Wire Line
	5000 2400 5000 2150
Connection ~ 5000 2150
NoConn ~ 4500 2400
NoConn ~ 5600 2400
Text HLabel 1450 2550 0    60   Input ~ 0
CTL-A
NoConn ~ 3900 2550
NoConn ~ 5000 2550
Wire Wire Line
	4800 2050 4800 2150
Connection ~ 4800 2150
Text HLabel 3600 1900 2    60   Output ~ 0
MUX
Wire Wire Line
	3600 1900 3550 1900
Wire Wire Line
	3550 1900 3550 2400
Wire Wire Line
	3550 2400 3400 2400
Wire Wire Line
	2250 2050 3550 2050
Connection ~ 3550 2050
Text HLabel 2800 2550 0    60   Input ~ 0
CTL-B
Wire Wire Line
	1600 2400 1450 2400
Wire Wire Line
	1600 2550 1450 2550
Text HLabel 1450 2400 0    60   Input ~ 0
MeatProbe
Text HLabel 2750 2400 0    60   Input ~ 0
PitProbe
Wire Wire Line
	2800 2400 2750 2400
Wire Wire Line
	2200 2400 2250 2400
Wire Wire Line
	2250 2400 2250 2050
$EndSCHEMATC
