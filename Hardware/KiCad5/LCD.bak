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
Sheet 4 4
Title "CloudSmoker - Wifi enabled BBQ Temperature Monitor"
Date "2017-09-14"
Rev "1"
Comp "Carl Greenstreet"
Comment1 ""
Comment2 "Includes 5V <-> 3.3V level shifting"
Comment3 "LCD Displacy - I2C"
Comment4 ""
$EndDescr
$Comp
L Conn_01x04 J6
U 1 1 59D724D0
P 5200 3250
F 0 "J6" H 5200 3450 50  0000 C CNN
F 1 "Conn_01x04" H 5200 2950 50  0000 C CNN
F 2 "" H 5200 3250 50  0001 C CNN
F 3 "" H 5200 3250 50  0001 C CNN
	1    5200 3250
	1    0    0    -1  
$EndComp
$Comp
L 2CH_I2C_Logic_Level_Convertor_Bidirectional U5
U 1 1 59D72818
P 3350 3200
F 0 "U5" H 3550 3700 50  0000 C CNN
F 1 "2CH_I2C_Logic_Level_Convertor_Bidirectional" H 3350 2400 50  0000 C CNN
F 2 "" H 4300 2600 50  0001 C CNN
F 3 "" H 3350 3170 50  0001 C CNN
	1    3350 3200
	1    0    0    -1  
$EndComp
Wire Notes Line
	5400 3150 6750 3150
Wire Notes Line
	5400 3350 6750 3350
Wire Notes Line
	5400 3250 6750 3250
Wire Notes Line
	5400 3450 6750 3450
Text Notes 5450 3000 0    51   ~ 10
Dupont wires to off-Board LCD\n(Mounted to enclosure window)
$Comp
L +3.3V #PWR028
U 1 1 59D72B2B
P 3250 2700
F 0 "#PWR028" H 3250 2550 50  0001 C CNN
F 1 "+3.3V" H 3250 2840 50  0000 C CNN
F 2 "" H 3250 2700 50  0001 C CNN
F 3 "" H 3250 2700 50  0001 C CNN
	1    3250 2700
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR029
U 1 1 59D72B4E
P 3450 2700
F 0 "#PWR029" H 3450 2550 50  0001 C CNN
F 1 "+5V" H 3450 2840 50  0000 C CNN
F 2 "" H 3450 2700 50  0001 C CNN
F 3 "" H 3450 2700 50  0001 C CNN
	1    3450 2700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR030
U 1 1 59D72B81
P 3250 3700
F 0 "#PWR030" H 3250 3450 50  0001 C CNN
F 1 "GND" H 3250 3550 50  0000 C CNN
F 2 "" H 3250 3700 50  0001 C CNN
F 3 "" H 3250 3700 50  0001 C CNN
	1    3250 3700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR031
U 1 1 59D72B9C
P 3450 3700
F 0 "#PWR031" H 3450 3450 50  0001 C CNN
F 1 "GND" H 3450 3550 50  0000 C CNN
F 2 "" H 3450 3700 50  0001 C CNN
F 3 "" H 3450 3700 50  0001 C CNN
	1    3450 3700
	1    0    0    -1  
$EndComp
Text HLabel 2950 3100 0    51   Input ~ 10
SCL
Text HLabel 2950 3300 0    51   Input ~ 10
SDA
$Comp
L LCD_16x2_I2C_OffBoard U6
U 1 1 59D80FE1
P 7600 3250
F 0 "U6" H 7250 3700 50  0000 C CNN
F 1 "LCD_16x2_I2C_OffBoard" H 7740 3710 50  0000 L CNN
F 2 "" H 7600 2650 50  0001 C CNN
F 3 "" H 7600 2750 50  0001 C CNN
	1    7600 3250
	1    0    0    -1  
$EndComp
Wire Notes Line
	6350 2300 6350 4650
Wire Notes Line
	6350 4650 8950 4650
Wire Notes Line
	8950 4650 8950 2300
Wire Notes Line
	8950 2300 6350 2300
Text Notes 6950 2550 0    60   ~ 12
Off-Board (no PCB footprints)\n Mounted onto Enclosure
Wire Notes Line
	5400 4100 6750 4100
Wire Notes Line
	5400 4300 6750 4300
Wire Notes Line
	5400 4200 6750 4200
Wire Notes Line
	5400 4400 6750 4400
$Comp
L Rotary_Encoder_Switch_off-board SW3
U 1 1 59DC4E10
P 7600 4250
F 0 "SW3" H 7600 4510 50  0000 C CNN
F 1 "Rotary_Encoder_Switch_off-board" H 7600 3990 50  0000 C CNN
F 2 "" H 7500 4410 50  0001 C CNN
F 3 "" H 7600 4510 50  0001 C CNN
	1    7600 4250
	1    0    0    -1  
$EndComp
$Comp
L Conn_01x05 J7
U 1 1 59DC4E88
P 5200 4300
F 0 "J7" H 5200 4600 50  0000 C CNN
F 1 "Conn_01x05" H 5200 4000 50  0000 C CNN
F 2 "" H 5200 4300 50  0001 C CNN
F 3 "" H 5200 4300 50  0001 C CNN
	1    5200 4300
	1    0    0    -1  
$EndComp
Wire Notes Line
	6350 2400 6350 4750
Wire Notes Line
	5400 4500 6750 4500
Text Label 3750 3100 0    60   ~ 0
SCL-5V
Text Label 3750 3300 0    60   ~ 0
SDA-5V
Text Label 5000 3450 2    60   ~ 0
SCL-5V
Text Label 5000 3350 2    60   ~ 0
SDA-5V
Text Label 5000 3250 2    60   ~ 0
+5V
Text Label 5000 3150 2    60   ~ 0
GND
Text Notes 5250 3900 0    51   ~ 10
Dupont wires to off-Board rotary encoder\n           (Mounted to enclosure)
Text GLabel 5000 4400 0    60   Input ~ 0
+5V
Text GLabel 5000 4500 0    60   Input ~ 0
GND
Text HLabel 5000 4100 0    60   Input ~ 0
CLK
Text HLabel 5000 4200 0    60   Input ~ 0
DT
Text HLabel 5000 4300 0    60   Input ~ 0
SW
Text Notes 8000 4200 0    39   ~ 0
Solder two 0.1uF caps directly \nbetween encoder pins A-C \nand B-C (where C = GND).  \nAlso, 4-5 where 5=GND
$EndSCHEMATC
