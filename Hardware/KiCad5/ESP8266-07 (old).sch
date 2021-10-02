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
Sheet 3 5
Title "CloudSmoker - Wifi enabled BBQ Temperature Monitor"
Date "2017-09-14"
Rev "1"
Comp "Carl Greenstreet"
Comment1 ""
Comment2 "ESP8266-07 configuration"
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ESP-07v2 U?
U 1 1 59BACA1D
P 5650 3650
F 0 "U?" H 5650 3550 50  0000 C CNN
F 1 "ESP-07v2" H 5650 3750 50  0000 C CNN
F 2 "" H 5650 3650 50  0001 C CNN
F 3 "" H 5650 3650 50  0001 C CNN
	1    5650 3650
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR010
U 1 1 59BACAED
P 5650 4750
F 0 "#PWR010" H 5650 4500 50  0001 C CNN
F 1 "GND" H 5650 4600 50  0000 C CNN
F 2 "" H 5650 4750 50  0001 C CNN
F 3 "" H 5650 4750 50  0001 C CNN
	1    5650 4750
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR011
U 1 1 59BACB07
P 5650 2550
F 0 "#PWR011" H 5650 2400 50  0001 C CNN
F 1 "+3.3V" H 5650 2690 50  0000 C CNN
F 2 "" H 5650 2550 50  0001 C CNN
F 3 "" H 5650 2550 50  0001 C CNN
	1    5650 2550
	1    0    0    -1  
$EndComp
$Comp
L Conn_01x06 J?
U 1 1 59BACD0E
P 9550 3550
F 0 "J?" H 9550 3850 50  0000 C CNN
F 1 "Conn_01x06" H 9550 3150 50  0000 C CNN
F 2 "" H 9550 3550 50  0001 C CNN
F 3 "" H 9550 3550 50  0001 C CNN
	1    9550 3550
	1    0    0    1   
$EndComp
$Comp
L SW_Push SW?
U 1 1 59BACFC1
P 3400 3900
F 0 "SW?" H 3450 4000 50  0000 L CNN
F 1 "SW_Push" H 3400 3840 50  0000 C CNN
F 2 "" H 3400 4100 50  0001 C CNN
F 3 "" H 3400 4100 50  0001 C CNN
	1    3400 3900
	0    -1   -1   0   
$EndComp
$Comp
L SW_Push SW?
U 1 1 59BAD014
P 8550 4000
F 0 "SW?" H 8600 4100 50  0000 L CNN
F 1 "SW_Push" H 8550 3940 50  0000 C CNN
F 2 "" H 8550 4200 50  0001 C CNN
F 3 "" H 8550 4200 50  0001 C CNN
	1    8550 4000
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 59BADA4D
P 7350 3000
F 0 "R?" V 7430 3000 50  0000 C CNN
F 1 "4.7k" V 7350 3000 50  0000 C CNN
F 2 "" V 7280 3000 50  0001 C CNN
F 3 "" H 7350 3000 50  0001 C CNN
	1    7350 3000
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 59BADA94
P 3400 3000
F 0 "R?" V 3480 3000 50  0000 C CNN
F 1 "4.7k" V 3400 3000 50  0000 C CNN
F 2 "" V 3330 3000 50  0001 C CNN
F 3 "" H 3400 3000 50  0001 C CNN
	1    3400 3000
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 59BADABD
P 3150 3000
F 0 "R?" V 3230 3000 50  0000 C CNN
F 1 "4.7k" V 3150 3000 50  0000 C CNN
F 2 "" V 3080 3000 50  0001 C CNN
F 3 "" H 3150 3000 50  0001 C CNN
	1    3150 3000
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 59BADAEB
P 7150 3000
F 0 "R?" V 7230 3000 50  0000 C CNN
F 1 "4.7k" V 7150 3000 50  0000 C CNN
F 2 "" V 7080 3000 50  0001 C CNN
F 3 "" H 7150 3000 50  0001 C CNN
	1    7150 3000
	1    0    0    -1  
$EndComp
$Comp
L CP1 C?
U 1 1 59BADB14
P 2450 3600
F 0 "C?" H 2475 3700 50  0000 L CNN
F 1 "470uF" H 2475 3500 50  0000 L CNN
F 2 "" H 2450 3600 50  0001 C CNN
F 3 "" H 2450 3600 50  0001 C CNN
	1    2450 3600
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 59BADBA3
P 2900 4150
F 0 "C?" H 2925 4250 50  0000 L CNN
F 1 "0.1uF" H 2925 4050 50  0000 L CNN
F 2 "" H 2938 4000 50  0001 C CNN
F 3 "" H 2900 4150 50  0001 C CNN
	1    2900 4150
	1    0    0    -1  
$EndComp
Text Notes 9750 3750 2    39   ~ 0
GND
Text Notes 9750 3650 2    39   ~ 0
CTS
Text Notes 9750 3550 2    39   ~ 0
VCC
Text Notes 9750 3450 2    39   ~ 0
TX 
Text Notes 9750 3350 2    39   ~ 0
RX 
Text Notes 9750 3250 2    39   ~ 0
DTR
Text Notes 8700 3800 2    51   ~ 10
PRG+
NoConn ~ 9350 3250
NoConn ~ 9350 3550
NoConn ~ 9350 3650
Text Notes 10000 3850 1    51   ~ 10
FTDI Module Header\n(flash new firmware)
Text Notes 2800 3850 2    39   ~ 0
Electrolitic
Text Notes 2650 4400 0    39   ~ 0
Ceramic\nWithin 1/2" of pins
Text Notes 3250 3800 3    51   ~ 10
RST+
Text HLabel 7700 2450 1    60   BiDi ~ 0
SDA
Text HLabel 7550 2450 1    60   BiDi ~ 0
SCL
Text HLabel 4650 2450 1    60   Input ~ 0
MUX
$Comp
L D_Zener D?
U 1 1 59BB0CC7
P 9050 4300
F 0 "D?" H 9050 4400 50  0000 C CNN
F 1 "3.3V" H 9050 4200 50  0000 C CNN
F 2 "" H 9050 4300 50  0001 C CNN
F 3 "" H 9050 4300 50  0001 C CNN
	1    9050 4300
	0    1    1    0   
$EndComp
Text Notes 4850 5800 0    60   ~ 0
okay to leave unused GPIO pins floating; careful with others
$Comp
L C C?
U 1 1 59BB116B
P 3650 3900
F 0 "C?" H 3675 4000 50  0000 L CNN
F 1 "470pF" H 3675 3800 50  0000 L CNN
F 2 "" H 3688 3750 50  0001 C CNN
F 3 "" H 3650 3900 50  0001 C CNN
	1    3650 3900
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 59BB233B
P 6550 4300
F 0 "R?" V 6630 4300 50  0000 C CNN
F 1 "10k" V 6550 4300 50  0000 C CNN
F 2 "" V 6480 4300 50  0001 C CNN
F 3 "" H 6550 4300 50  0001 C CNN
	1    6550 4300
	1    0    0    -1  
$EndComp
Text Notes 4850 6050 0    60   ~ 0
GPIO4/GPIO5, GPIO12, 13, 14 => General purpose pins\nGPIO16 needed for deep sleep
$Comp
L R R?
U 1 1 59BB2B78
P 6950 3000
F 0 "R?" V 7030 3000 50  0000 C CNN
F 1 "4.7k" V 6950 3000 50  0000 C CNN
F 2 "" V 6880 3000 50  0001 C CNN
F 3 "" H 6950 3000 50  0001 C CNN
	1    6950 3000
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 59BB2BB6
P 6750 3000
F 0 "R?" V 6830 3000 50  0000 C CNN
F 1 "4.7k" V 6750 3000 50  0000 C CNN
F 2 "" V 6680 3000 50  0001 C CNN
F 3 "" H 6750 3000 50  0001 C CNN
	1    6750 3000
	1    0    0    -1  
$EndComp
$Comp
L D_Schottky D?
U 1 1 59BB5CEA
P 4450 3500
F 0 "D?" H 4450 3600 50  0000 C CNN
F 1 "D_Schottky" H 4450 3400 50  0000 C CNN
F 2 "" H 4450 3500 50  0001 C CNN
F 3 "" H 4450 3500 50  0001 C CNN
	1    4450 3500
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4450 3650 4750 3650
Connection ~ 9050 3450
Wire Wire Line
	9050 4150 9050 3450
Wire Wire Line
	6550 3450 9350 3450
Connection ~ 6750 2700
Wire Wire Line
	6750 2850 6750 2700
Connection ~ 6750 3550
Wire Wire Line
	6750 3150 6750 3550
Connection ~ 6950 2700
Wire Wire Line
	6950 2700 6950 2850
Connection ~ 6950 3650
Wire Wire Line
	6950 3150 6950 3650
Connection ~ 6550 4600
Wire Wire Line
	6550 4600 6550 4450
Wire Wire Line
	6550 3950 6550 4150
Connection ~ 3400 4150
Wire Wire Line
	3650 4150 3400 4150
Wire Wire Line
	3650 4050 3650 4150
Connection ~ 3400 3700
Wire Wire Line
	3400 3700 3650 3700
Wire Wire Line
	3650 3700 3650 3750
Connection ~ 9050 4600
Wire Wire Line
	9050 4450 9050 4600
Connection ~ 7350 3750
Connection ~ 7150 2700
Wire Wire Line
	7350 2700 7350 2850
Wire Wire Line
	4650 3450 4650 2450
Wire Wire Line
	4750 3450 4650 3450
Wire Wire Line
	7350 3750 7350 3150
Wire Wire Line
	7550 3550 6550 3550
Wire Wire Line
	6550 3650 7700 3650
Connection ~ 3400 4700
Wire Wire Line
	3400 4700 3400 4100
Connection ~ 3400 3350
Connection ~ 3400 2600
Wire Wire Line
	3400 2600 3400 2850
Wire Wire Line
	3400 3150 3400 3700
Wire Wire Line
	3400 3350 4750 3350
Connection ~ 3150 2600
Wire Wire Line
	3150 2600 3150 2850
Wire Wire Line
	3150 3550 3150 3150
Wire Wire Line
	3150 3550 4750 3550
Connection ~ 5650 4700
Wire Wire Line
	7150 3850 7150 3150
Wire Wire Line
	6550 3850 7150 3850
Wire Wire Line
	7150 2700 7150 2850
Wire Wire Line
	5650 2700 7350 2700
Connection ~ 2900 4700
Wire Wire Line
	2900 4700 2900 4300
Connection ~ 2900 2600
Wire Wire Line
	2900 2600 2900 4000
Wire Wire Line
	5650 4600 9250 4600
Connection ~ 2450 4700
Wire Wire Line
	2450 3750 2450 4700
Connection ~ 2450 2600
Wire Wire Line
	2450 2600 2450 3450
Wire Wire Line
	2450 4700 5650 4700
Wire Wire Line
	9250 4600 9250 3750
Wire Wire Line
	9250 3750 9350 3750
Wire Wire Line
	6550 3350 9350 3350
Wire Wire Line
	5650 4550 5650 4750
Text Notes 800  7500 0    60   ~ 0
Schottky signal diode or 330 to 1K ohm resistor required between GPIO16 and REST pins\nGPIO16 is ony pin belonging to RTC (real time clock) module.  During deep sleep, GPIO16 pin is high.  \nWhen RTC triggers wake-up, it drives GPIO16 pin low, which also drives REST pin low, tirggering reset\n
Wire Wire Line
	4750 3750 4200 3750
Wire Wire Line
	4200 3750 4200 2450
Wire Wire Line
	4750 3850 4000 3850
Wire Wire Line
	4000 3850 4000 2450
Text HLabel 4200 2450 1    60   Output ~ 0
CTRL-A
Text HLabel 4000 2450 1    60   Output ~ 0
CTL-B
Wire Wire Line
	2450 2600 5650 2600
Text Notes 800  7200 0    60   ~ 12
Deep Sleep
$Comp
L Jumper_NC_Dual JP?
U 1 1 59D35A5B
P 7900 3750
F 0 "JP?" H 7950 3650 50  0000 L CNN
F 1 "Jumper_NC_Dual" H 7950 3850 50  0000 C BNN
F 2 "" H 7900 3750 50  0001 C CNN
F 3 "" H 7900 3750 50  0001 C CNN
	1    7900 3750
	0    1    1    0   
$EndComp
Wire Wire Line
	7550 2450 7550 3550
Wire Wire Line
	6550 3750 7800 3750
Wire Wire Line
	7700 3650 7700 2450
Wire Wire Line
	8750 4000 9250 4000
Connection ~ 9250 4000
Wire Wire Line
	8350 4000 7900 4000
Connection ~ 5650 4600
Wire Wire Line
	3800 3950 4750 3950
Wire Wire Line
	3800 3950 3800 2450
Wire Wire Line
	5650 2550 5650 2750
Connection ~ 5650 2700
Connection ~ 5650 2600
Text HLabel 3800 2450 1    60   Input ~ 0
Button-Lft
Text HLabel 7900 2450 1    60   Input ~ 0
Button-Rt
Wire Wire Line
	7900 3500 7900 2450
Text Notes 7400 4050 0    39   ~ 8
Close 2-3 \n  Jumper  \n to Flash 
Text Notes 8100 3700 0    39   ~ 8
Close 1-2 \nJumper for \nnormal use
Text Notes 7650 4350 0    39   ~ 0
Jumper used to \nconfigure GPIO0\nfor flash or input
Wire Notes Line
	7600 4150 7600 4400
Wire Notes Line
	7600 4400 8200 4400
Wire Notes Line
	8200 4400 8200 4150
Wire Notes Line
	8200 4150 7600 4150
$EndSCHEMATC
