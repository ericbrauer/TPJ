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
LIBS:tpj-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Project Skylight"
Date "2017-03-15"
Rev "1.1"
Comp "TPJ655 Final Project"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ESP-12E U2
U 1 1 58BC5888
P 6600 4100
F 0 "U2" H 6600 4000 50  0000 C CNN
F 1 "ESP-12E" H 6600 4200 50  0000 C CNN
F 2 "ESP8266:ESP-12E" H 6600 4100 50  0001 C CNN
F 3 "" H 6600 4100 50  0001 C CNN
	1    6600 4100
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR3
U 1 1 58BC58E2
P 7800 4750
F 0 "#PWR3" H 7800 4500 50  0001 C CNN
F 1 "GND" H 7800 4600 50  0000 C CNN
F 2 "" H 7800 4750 50  0000 C CNN
F 3 "" H 7800 4750 50  0000 C CNN
	1    7800 4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	7500 4500 7800 4500
Wire Wire Line
	7800 3150 7800 4750
$Comp
L LD1117S33TR U1
U 1 1 58BC5F84
P 3100 3700
F 0 "U1" H 3100 3950 50  0000 C CNN
F 1 "LD1117S33TR" H 3100 3900 50  0000 C CNN
F 2 "TO_SOT_Packages_THT:TO-220_Neutral123_Horizontal" H 3100 3800 50  0000 C CNN
F 3 "" H 3100 3700 50  0000 C CNN
	1    3100 3700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR2
U 1 1 58BC607A
P 3100 4300
F 0 "#PWR2" H 3100 4050 50  0001 C CNN
F 1 "GND" H 3100 4150 50  0000 C CNN
F 2 "" H 3100 4300 50  0000 C CNN
F 3 "" H 3100 4300 50  0000 C CNN
	1    3100 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 3950 3100 4300
$Comp
L POT RV1
U 1 1 58BC60B2
P 4500 3900
F 0 "RV1" H 4500 3820 50  0000 C CNN
F 1 "10kOhm" H 4500 3900 50  0000 C CNN
F 2 "Potentiometers:Potentiometer_WirePads_largePads" H 4500 3900 50  0001 C CNN
F 3 "" H 4500 3900 50  0000 C CNN
	1    4500 3900
	0    1    1    0   
$EndComp
Wire Wire Line
	5700 3900 4650 3900
Wire Wire Line
	2700 3650 2350 3650
Wire Wire Line
	2350 2350 2350 3750
Wire Wire Line
	3500 3650 5000 3650
Wire Wire Line
	4500 3650 4500 3750
Connection ~ 3650 3650
$Comp
L CONN_01X03 P1
U 1 1 58BC631B
P 8150 3050
F 0 "P1" H 8150 3250 50  0000 C CNN
F 1 "CONN_01X03" V 8250 3050 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Angled_1x03" H 8150 3050 50  0001 C CNN
F 3 "" H 8150 3050 50  0000 C CNN
	1    8150 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	7800 3150 7950 3150
Connection ~ 7800 4500
Wire Wire Line
	5700 4200 5550 4200
$Comp
L R R1
U 1 1 58BC6467
P 5550 3450
F 0 "R1" V 5630 3450 50  0000 C CNN
F 1 "1kOhm" V 5550 3450 50  0000 C CNN
F 2 "Resistors_ThroughHole:Resistor_Horizontal_RM7mm" V 5480 3450 50  0001 C CNN
F 3 "" H 5550 3450 50  0000 C CNN
	1    5550 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 4200 5550 3600
Wire Wire Line
	5550 2950 5550 3300
$Comp
L CP C3
U 1 1 58BC6555
P 7350 3250
F 0 "C3" H 7375 3350 50  0000 L CNN
F 1 "100uF" H 7375 3150 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Axial_D5_L11_P18" H 7388 3100 50  0001 C CNN
F 3 "" H 7350 3250 50  0000 C CNN
	1    7350 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	7550 2950 7550 2950
Wire Wire Line
	7950 3050 2350 3050
Connection ~ 2350 3050
Wire Wire Line
	5550 2950 7950 2950
Wire Wire Line
	7350 3100 7350 3050
Connection ~ 7350 3050
Wire Wire Line
	7350 3400 7350 3450
Wire Wire Line
	7350 3450 7800 3450
Connection ~ 7800 3450
$Comp
L CP C1
U 1 1 58BC6897
P 2350 3900
F 0 "C1" H 2375 4000 50  0000 L CNN
F 1 "100nF" H 2375 3800 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Axial_D5_L11_P18" H 2388 3750 50  0001 C CNN
F 3 "" H 2350 3900 50  0000 C CNN
	1    2350 3900
	1    0    0    -1  
$EndComp
$Comp
L CP C2
U 1 1 58BC693B
P 3650 3900
F 0 "C2" H 3675 4000 50  0000 L CNN
F 1 "10uF" H 3675 3800 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Axial_D5_L11_P18" H 3688 3750 50  0001 C CNN
F 3 "" H 3650 3900 50  0000 C CNN
	1    3650 3900
	1    0    0    -1  
$EndComp
Connection ~ 2350 3650
Wire Wire Line
	2350 4050 2350 4250
Wire Wire Line
	2350 4250 4500 4250
Connection ~ 3100 4250
Wire Wire Line
	3650 3750 3650 3650
Wire Wire Line
	3650 4250 3650 4050
Wire Wire Line
	4500 4250 4500 4050
Connection ~ 3650 4250
Wire Wire Line
	5700 4500 5000 4500
Wire Wire Line
	5000 4500 5000 3650
Connection ~ 4500 3650
$Comp
L USB_A P2
U 1 1 58BC6E76
P 2550 2050
F 0 "P2" H 2750 1850 50  0000 C CNN
F 1 "USB_A" H 2500 2250 50  0000 C CNN
F 2 "Connect:USB_Micro-B" V 2500 1950 50  0001 C CNN
F 3 "" V 2500 1950 50  0000 C CNN
	1    2550 2050
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR1
U 1 1 58BC6FBA
P 2650 2450
F 0 "#PWR1" H 2650 2200 50  0001 C CNN
F 1 "GND" H 2650 2300 50  0000 C CNN
F 2 "" H 2650 2450 50  0000 C CNN
F 3 "" H 2650 2450 50  0000 C CNN
	1    2650 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 2450 2650 2350
Wire Wire Line
	2850 1950 2900 1950
Wire Wire Line
	2900 1950 2900 2400
Wire Wire Line
	2900 2400 2650 2400
Connection ~ 2650 2400
$Comp
L ESP-12E U?
U 1 1 58C94F68
P 6600 4100
F 0 "U?" H 6600 4000 50  0000 C CNN
F 1 "ESP-12E" H 6600 4200 50  0000 C CNN
F 2 "ESP8266:ESP-12E" H 6600 4100 50  0001 C CNN
F 3 "" H 6600 4100 50  0001 C CNN
	1    6600 4100
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 58C94F69
P 7800 4750
F 0 "#PWR?" H 7800 4500 50  0001 C CNN
F 1 "GND" H 7800 4600 50  0000 C CNN
F 2 "" H 7800 4750 50  0000 C CNN
F 3 "" H 7800 4750 50  0000 C CNN
	1    7800 4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	7500 4500 7800 4500
Wire Wire Line
	7800 3150 7800 4750
$Comp
L LD1117S33TR U?
U 1 1 58C94F6A
P 3100 3700
F 0 "U?" H 3100 3950 50  0000 C CNN
F 1 "LD1117S33TR" H 3100 3900 50  0000 C CNN
F 2 "TO_SOT_Packages_THT:TO-220_Neutral123_Horizontal" H 3100 3800 50  0000 C CNN
F 3 "" H 3100 3700 50  0000 C CNN
	1    3100 3700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 58C94F6B
P 3100 4300
F 0 "#PWR?" H 3100 4050 50  0001 C CNN
F 1 "GND" H 3100 4150 50  0000 C CNN
F 2 "" H 3100 4300 50  0000 C CNN
F 3 "" H 3100 4300 50  0000 C CNN
	1    3100 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 3950 3100 4300
$Comp
L POT RV?
U 1 1 58C94F6C
P 4500 3900
F 0 "RV?" H 4500 3820 50  0000 C CNN
F 1 "10kOhm" H 4500 3900 50  0000 C CNN
F 2 "Potentiometers:Potentiometer_WirePads_largePads" H 4500 3900 50  0001 C CNN
F 3 "" H 4500 3900 50  0000 C CNN
	1    4500 3900
	0    1    1    0   
$EndComp
Wire Wire Line
	5700 3900 4650 3900
Wire Wire Line
	2700 3650 2350 3650
Wire Wire Line
	2350 2350 2350 3750
Wire Wire Line
	3500 3650 5000 3650
Wire Wire Line
	4500 3650 4500 3750
Connection ~ 3650 3650
$Comp
L CONN_01X03 P?
U 1 1 58C94F6D
P 8150 3050
F 0 "P?" H 8150 3250 50  0000 C CNN
F 1 "CONN_01X03" V 8250 3050 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Angled_1x03" H 8150 3050 50  0001 C CNN
F 3 "" H 8150 3050 50  0000 C CNN
	1    8150 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	7800 3150 7950 3150
Connection ~ 7800 4500
Wire Wire Line
	5700 4200 5550 4200
$Comp
L R R?
U 1 1 58C94F6E
P 5550 3450
F 0 "R?" V 5630 3450 50  0000 C CNN
F 1 "1kOhm" V 5550 3450 50  0000 C CNN
F 2 "Resistors_ThroughHole:Resistor_Horizontal_RM7mm" V 5480 3450 50  0001 C CNN
F 3 "" H 5550 3450 50  0000 C CNN
	1    5550 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 4200 5550 3600
Wire Wire Line
	5550 2950 5550 3300
$Comp
L CP C?
U 1 1 58C94F6F
P 7350 3250
F 0 "C?" H 7375 3350 50  0000 L CNN
F 1 "100uF" H 7375 3150 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Axial_D5_L11_P18" H 7388 3100 50  0001 C CNN
F 3 "" H 7350 3250 50  0000 C CNN
	1    7350 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	7550 2950 7550 2950
Wire Wire Line
	7950 3050 2350 3050
Connection ~ 2350 3050
Wire Wire Line
	5550 2950 7950 2950
Wire Wire Line
	7350 3100 7350 3050
Connection ~ 7350 3050
Wire Wire Line
	7350 3400 7350 3450
Wire Wire Line
	7350 3450 7800 3450
Connection ~ 7800 3450
$Comp
L CP C?
U 1 1 58C94F70
P 2350 3900
F 0 "C?" H 2375 4000 50  0000 L CNN
F 1 "100nF" H 2375 3800 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Axial_D5_L11_P18" H 2388 3750 50  0001 C CNN
F 3 "" H 2350 3900 50  0000 C CNN
	1    2350 3900
	1    0    0    -1  
$EndComp
$Comp
L CP C?
U 1 1 58C94F71
P 3650 3900
F 0 "C?" H 3675 4000 50  0000 L CNN
F 1 "10uF" H 3675 3800 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Axial_D5_L11_P18" H 3688 3750 50  0001 C CNN
F 3 "" H 3650 3900 50  0000 C CNN
	1    3650 3900
	1    0    0    -1  
$EndComp
Connection ~ 2350 3650
Wire Wire Line
	2350 4050 2350 4250
Wire Wire Line
	2350 4250 4500 4250
Connection ~ 3100 4250
Wire Wire Line
	3650 3750 3650 3650
Wire Wire Line
	3650 4250 3650 4050
Wire Wire Line
	4500 4250 4500 4050
Connection ~ 3650 4250
Wire Wire Line
	5700 4500 5000 4500
Wire Wire Line
	5000 4500 5000 3650
Connection ~ 4500 3650
$Comp
L USB_A P?
U 1 1 58C94F72
P 2550 2050
F 0 "P?" H 2750 1850 50  0000 C CNN
F 1 "USB_A" H 2500 2250 50  0000 C CNN
F 2 "Connect:USB_Micro-B" V 2500 1950 50  0001 C CNN
F 3 "" V 2500 1950 50  0000 C CNN
	1    2550 2050
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 58C94F73
P 2650 2450
F 0 "#PWR?" H 2650 2200 50  0001 C CNN
F 1 "GND" H 2650 2300 50  0000 C CNN
F 2 "" H 2650 2450 50  0000 C CNN
F 3 "" H 2650 2450 50  0000 C CNN
	1    2650 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 2450 2650 2350
Wire Wire Line
	2850 1950 2900 1950
Wire Wire Line
	2900 1950 2900 2400
Wire Wire Line
	2900 2400 2650 2400
Connection ~ 2650 2400
$EndSCHEMATC
