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
LIBS:cg_device
LIBS:cg_display
LIBS:cg_modules
LIBS:cg_off-board
LIBS:CloudSmoker-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 4
Title "CloudSmoker - Wifi enabled BBQ Temperature Monitor"
Date "2017-09-14"
Rev "1"
Comp "Carl Greenstreet"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Barrel_Jack J1
U 1 1 59B9C571
P 1200 1250
F 0 "J1" H 1200 1460 50  0000 C CNN
F 1 "Barrel_Jack" H 1200 1075 50  0000 C CNN
F 2 "" H 1250 1210 50  0001 C CNN
F 3 "" H 1250 1210 50  0001 C CNN
	1    1200 1250
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR02
U 1 1 59B9C9B0
P 2200 7450
F 0 "#PWR02" H 2200 7300 50  0001 C CNN
F 1 "+5V" H 2200 7590 50  0000 C CNN
F 2 "" H 2200 7450 50  0001 C CNN
F 3 "" H 2200 7450 50  0001 C CNN
	1    2200 7450
	1    0    0    -1  
$EndComp
$Comp
L C C1
U 1 1 59B9C9F5
P 2000 1400
F 0 "C1" H 2025 1500 50  0000 L CNN
F 1 "C" H 2025 1300 50  0000 L CNN
F 2 "" H 2038 1250 50  0001 C CNN
F 3 "" H 2000 1400 50  0001 C CNN
	1    2000 1400
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR03
U 1 1 59B9CA3D
P 2000 1600
F 0 "#PWR03" H 2000 1350 50  0001 C CNN
F 1 "GND" H 2000 1450 50  0000 C CNN
F 2 "" H 2000 1600 50  0001 C CNN
F 3 "" H 2000 1600 50  0001 C CNN
	1    2000 1600
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR04
U 1 1 59B9CD04
P 1150 6750
F 0 "#PWR04" H 1150 6500 50  0001 C CNN
F 1 "GND" H 1150 6600 50  0000 C CNN
F 2 "" H 1150 6750 50  0001 C CNN
F 3 "" H 1150 6750 50  0001 C CNN
	1    1150 6750
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG05
U 1 1 59B9CD34
P 1150 6500
F 0 "#FLG05" H 1150 6575 50  0001 C CNN
F 1 "PWR_FLAG" H 1150 6650 50  0000 C CNN
F 2 "" H 1150 6500 50  0001 C CNN
F 3 "" H 1150 6500 50  0001 C CNN
	1    1150 6500
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG06
U 1 1 59B9CD50
P 1600 6750
F 0 "#FLG06" H 1600 6825 50  0001 C CNN
F 1 "PWR_FLAG" H 1600 6900 50  0000 C CNN
F 2 "" H 1600 6750 50  0001 C CNN
F 3 "" H 1600 6750 50  0001 C CNN
	1    1600 6750
	-1   0    0    1   
$EndComp
$Sheet
S 3200 3900 1400 1150
U 59BAB80D
F0 "ESP8266-07" 60
F1 "ESP8266-07.sch" 60
F2 "SDA" B R 4600 4850 60 
F3 "SCL" B R 4600 5000 60 
F4 "MUX" I R 4600 4400 60 
F5 "CTRL-A" O R 4600 4250 60 
F6 "CTL-B" O R 4600 4100 60 
F7 "SW" I R 4600 3950 60 
F8 "CLK" I R 4600 4700 60 
F9 "DT" I R 4600 4550 60 
$EndSheet
$Sheet
S 7150 3000 1450 1100
U 59BAB9BF
F0 "Thermistors" 60
F1 "Thermstors.sch" 60
F2 "CTL-A" I L 7150 3950 60 
F3 "MUX" O R 8600 3350 60 
F4 "CTL-B" I L 7150 3750 60 
$EndSheet
$Sheet
S 7200 4850 1450 1100
U 59BABB2A
F0 "LCD" 60
F1 "LCD.sch" 60
F2 "SCL" I R 8650 5700 60 
F3 "SDA" I R 8650 5500 60 
F4 "CLK" I R 8650 4950 60 
F5 "DT" I R 8650 5100 60 
F6 "SW" I R 8650 5250 60 
$EndSheet
$Comp
L +5V #PWR07
U 1 1 59B9CD1C
P 1600 6500
F 0 "#PWR07" H 1600 6350 50  0001 C CNN
F 1 "+5V" H 1600 6640 50  0000 C CNN
F 2 "" H 1600 6500 50  0001 C CNN
F 3 "" H 1600 6500 50  0001 C CNN
	1    1600 6500
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG08
U 1 1 59BABC00
P 2050 6750
F 0 "#FLG08" H 2050 6825 50  0001 C CNN
F 1 "PWR_FLAG" H 2050 6900 50  0000 C CNN
F 2 "" H 2050 6750 50  0001 C CNN
F 3 "" H 2050 6750 50  0001 C CNN
	1    2050 6750
	-1   0    0    1   
$EndComp
$Comp
L +3.3V #PWR09
U 1 1 59BABBE5
P 2050 6500
F 0 "#PWR09" H 2050 6350 50  0001 C CNN
F 1 "+3.3V" H 2050 6640 50  0000 C CNN
F 2 "" H 2050 6500 50  0001 C CNN
F 3 "" H 2050 6500 50  0001 C CNN
	1    2050 6500
	1    0    0    -1  
$EndComp
Text Notes 1350 950  0    47   ~ 0
Centre positive polarity
Text GLabel 3200 4300 2    60   Input ~ 0
GND
Text GLabel 3200 4100 2    60   Input ~ 0
+3.3V
Text GLabel 7200 5100 2    60   Input ~ 0
+3.3V
Text GLabel 7200 5300 2    60   Input ~ 0
+5V
Text GLabel 7200 5500 2    60   Input ~ 0
GND
Text Label 9300 3350 2    60   ~ 0
MUX
Text Label 6500 3750 0    60   ~ 0
CTL-B
Text Label 6500 3950 0    60   ~ 0
CTL-A
Text Label 6500 3500 0    60   ~ 0
GND
Text Label 6500 3250 0    60   ~ 0
+3.3V
Text Label 6550 5500 0    60   ~ 0
GND
Text Label 6550 5100 0    60   ~ 0
+3.3V
Text Label 6550 5300 0    60   ~ 0
+5V
Text Label 9350 4950 2    60   ~ 0
CLK
Text Label 9350 5100 2    60   ~ 0
DT
Text Label 9350 5250 2    60   ~ 0
SW
Text Label 9350 5500 2    60   ~ 0
SDA
Text Label 9350 5700 2    60   ~ 0
SCL
Text Label 5300 4850 2    60   ~ 0
SDA
Text Label 5300 5000 2    60   ~ 0
SCL
Text Label 5300 4550 2    60   ~ 0
DT
Text Label 5300 4700 2    60   ~ 0
CLK
Text Label 5300 4250 2    60   ~ 0
CTL-A
Text Label 5300 4400 2    60   ~ 0
MUX
Text Label 5300 3950 2    60   ~ 0
SW
Text Label 5300 4100 2    60   ~ 0
CTL-B
Text Label 2550 4100 0    60   ~ 0
+3.3V
Text Label 2550 4300 0    60   ~ 0
GND
$Comp
L Conn_01x03 J2
U 1 1 59DEBC06
P 5600 1550
F 0 "J2" H 5600 1750 50  0000 C CNN
F 1 "Conn_01x03" H 5600 1350 50  0000 C CNN
F 2 "" H 5600 1550 50  0001 C CNN
F 3 "" H 5600 1550 50  0001 C CNN
	1    5600 1550
	-1   0    0    -1  
$EndComp
$Comp
L 5V-3V3_StepDown_module_off-board U1
U 1 1 59DEBC0D
P 4200 1550
F 0 "U1" H 4200 1850 50  0000 C CNN
F 1 "5V-3V3_StepDown_module_off-board" H 4200 1290 50  0000 C CNN
F 2 "" H 4100 1710 50  0001 C CNN
F 3 "" H 4200 1810 50  0001 C CNN
	1    4200 1550
	1    0    0    -1  
$EndComp
Text GLabel 6150 1450 2    60   Input ~ 0
+5V
Text GLabel 8500 1300 2    60   Output ~ 0
+3.3V
Text GLabel 6150 1650 2    60   Input ~ 0
GND
Text Notes 3700 1050 0    60   ~ 12
Off-Board Module
$Comp
L L L1
U 1 1 59DEBC22
P 6900 1300
F 0 "L1" V 6850 1300 50  0000 C CNN
F 1 "10 uH" V 6975 1300 50  0000 C CNN
F 2 "" H 6900 1300 50  0001 C CNN
F 3 "" H 6900 1300 50  0001 C CNN
	1    6900 1300
	0    1    1    0   
$EndComp
$Comp
L R R1
U 1 1 59DEBC29
P 7450 1300
F 0 "R1" V 7530 1300 50  0000 C CNN
F 1 "10" V 7450 1300 50  0000 C CNN
F 2 "" V 7380 1300 50  0001 C CNN
F 3 "" H 7450 1300 50  0001 C CNN
	1    7450 1300
	0    1    1    0   
$EndComp
$Comp
L C C2
U 1 1 59DEBC30
P 7800 1700
F 0 "C2" H 7825 1800 50  0000 L CNN
F 1 "0.1uF" H 7825 1600 50  0000 L CNN
F 2 "" H 7838 1550 50  0001 C CNN
F 3 "" H 7800 1700 50  0001 C CNN
	1    7800 1700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR010
U 1 1 59DEBC3C
P 8500 2150
F 0 "#PWR010" H 8500 1900 50  0001 C CNN
F 1 "GND" H 8500 2000 50  0000 C CNN
F 2 "" H 8500 2150 50  0001 C CNN
F 3 "" H 8500 2150 50  0001 C CNN
	1    8500 2150
	1    0    0    -1  
$EndComp
Text Notes 6750 1050 0    60   ~ 0
RLC Lowpass Power Filtering 
$Comp
L CP C3
U 1 1 59DEBC50
P 8200 1700
F 0 "C3" H 8225 1800 50  0000 L CNN
F 1 "10uF" H 8225 1600 50  0000 L CNN
F 2 "" H 8238 1550 50  0001 C CNN
F 3 "" H 8200 1700 50  0001 C CNN
	1    8200 1700
	1    0    0    -1  
$EndComp
Text Notes 4700 2450 0    60   ~ 12
5V->3.3V stepdown and power regulation / stabilisation 
$Comp
L +3V3 #PWR011
U 1 1 59DEC909
P 6550 1550
F 0 "#PWR011" H 6550 1400 50  0001 C CNN
F 1 "+3V3" H 6550 1690 50  0000 C CNN
F 2 "" H 6550 1550 50  0001 C CNN
F 3 "" H 6550 1550 50  0001 C CNN
	1    6550 1550
	1    0    0    -1  
$EndComp
Text GLabel 7150 3250 2    60   Input ~ 0
+3.3V
Text GLabel 7150 3500 2    60   Input ~ 0
GND
Text Notes 1950 6300 0    39   Italic 0
Filtered
Text Notes 6450 1650 0    39   Italic 0
Unfiltered (in)
Text Notes 4700 1400 0    51   ~ 10
Wired connections
Text GLabel 2200 1150 2    60   Output ~ 0
+5V
Wire Wire Line
	1500 1350 1700 1350
Wire Wire Line
	1500 1250 1600 1250
Wire Wire Line
	1600 1250 1600 1350
Connection ~ 1600 1350
Wire Wire Line
	1700 1350 1700 1600
Wire Wire Line
	1500 1150 2200 1150
Wire Wire Line
	2000 1550 2000 1600
Wire Wire Line
	2000 1250 2000 1150
Connection ~ 2000 1150
Wire Wire Line
	1150 6500 1150 6750
Wire Wire Line
	1600 6500 1600 6750
Wire Wire Line
	2050 6750 2050 6500
Wire Wire Line
	8600 3350 9300 3350
Wire Wire Line
	7150 3750 6500 3750
Wire Wire Line
	7150 3950 6500 3950
Wire Wire Line
	7150 3500 6500 3500
Wire Wire Line
	7150 3250 6500 3250
Wire Wire Line
	7200 5500 6550 5500
Wire Wire Line
	7200 5100 6550 5100
Wire Wire Line
	7200 5300 6550 5300
Wire Wire Line
	8650 4950 9350 4950
Wire Wire Line
	8650 5100 9350 5100
Wire Wire Line
	8650 5250 9350 5250
Wire Wire Line
	8650 5500 9350 5500
Wire Wire Line
	8650 5700 9350 5700
Wire Wire Line
	4600 4850 5300 4850
Wire Wire Line
	4600 5000 5300 5000
Wire Wire Line
	4600 4550 5300 4550
Wire Wire Line
	4600 4700 5300 4700
Wire Wire Line
	4600 4250 5300 4250
Wire Wire Line
	4600 4400 5300 4400
Wire Wire Line
	4600 3950 5300 3950
Wire Wire Line
	4600 4100 5300 4100
Wire Wire Line
	3200 4100 2550 4100
Wire Wire Line
	3200 4300 2550 4300
Wire Wire Line
	5800 1450 6150 1450
Wire Wire Line
	5800 1550 6550 1550
Wire Wire Line
	5800 1650 6150 1650
Wire Notes Line
	5500 1450 4600 1450
Wire Notes Line
	5500 1550 4600 1550
Wire Notes Line
	5500 1650 4600 1650
Wire Notes Line
	3300 900  3300 2200
Wire Notes Line
	3300 2200 5000 2200
Wire Notes Line
	5000 2200 5000 900 
Wire Notes Line
	5000 900  3300 900 
Wire Wire Line
	6100 1650 6100 1650
Wire Wire Line
	6100 1650 6100 2050
Wire Wire Line
	6100 2050 8500 2050
Wire Wire Line
	8500 2050 8500 2150
Connection ~ 6100 1650
Wire Wire Line
	5800 1550 5800 1300
Wire Wire Line
	5800 1300 6750 1300
Connection ~ 5800 1550
Wire Wire Line
	7800 1850 7800 2050
Connection ~ 7800 2050
Wire Wire Line
	8200 1850 8200 2050
Connection ~ 8200 2050
Wire Wire Line
	7600 1300 8500 1300
Wire Wire Line
	8200 1300 8200 1550
Connection ~ 8200 1300
Wire Wire Line
	7800 1550 7800 1300
Connection ~ 7800 1300
Wire Wire Line
	7300 1300 7050 1300
$Comp
L GND #PWR01
U 1 1 59B9C960
P 1700 1600
F 0 "#PWR01" H 1700 1350 50  0001 C CNN
F 1 "GND" H 1700 1450 50  0000 C CNN
F 2 "" H 1700 1600 50  0001 C CNN
F 3 "" H 1700 1600 50  0001 C CNN
	1    1700 1600
	1    0    0    -1  
$EndComp
$EndSCHEMATC
