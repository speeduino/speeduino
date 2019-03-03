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
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 8250 2300 1    60   Input ~ 0
INJ1
Wire Wire Line
	8000 2300 8500 2300
Text GLabel 7850 2700 0    60   Input ~ 0
INJ2
Text GLabel 8650 2700 2    60   Input ~ 0
INJ2
Text GLabel 7850 2850 0    60   Input ~ 0
INJ3
Wire Wire Line
	7850 2800 7850 2900
Text GLabel 7850 3050 0    60   Input ~ 0
INJ4
Wire Wire Line
	7850 3000 7850 3100
Text GLabel 7850 3200 0    60   Input ~ 0
IGN1
Text GLabel 7850 3300 0    60   Input ~ 0
IGN4
Text GLabel 8650 3300 2    60   Input ~ 0
IGN3
$Comp
L GND #PWR01
U 1 1 5A5847A9
P 7250 3400
F 0 "#PWR01" H 7250 3150 50  0001 C CNN
F 1 "GND" H 7250 3250 50  0000 C CNN
F 2 "" H 7250 3400 50  0000 C CNN
F 3 "" H 7250 3400 50  0000 C CNN
	1    7250 3400
	0    1    1    0   
$EndComp
Wire Wire Line
	7900 3400 7900 3500
Connection ~ 7900 3400
Text GLabel 7850 3600 0    60   Input ~ 0
MAP
Wire Wire Line
	7500 3700 7500 3400
Connection ~ 7500 3400
$Comp
L +5V #PWR02
U 1 1 5A5848BD
P 7250 3800
F 0 "#PWR02" H 7250 3650 50  0001 C CNN
F 1 "+5V" H 7250 3940 50  0000 C CNN
F 2 "" H 7250 3800 50  0000 C CNN
F 3 "" H 7250 3800 50  0000 C CNN
	1    7250 3800
	0    -1   -1   0   
$EndComp
Text GLabel 7850 3900 0    60   Input ~ 0
PROTO1
Text GLabel 7850 4000 0    60   Input ~ 0
PROTO2
Text GLabel 7850 4100 0    60   Input ~ 0
PROTO3
Text GLabel 7850 4200 0    60   Input ~ 0
PROTO4
Text GLabel 7850 4300 0    60   Input ~ 0
PROTO5
Text GLabel 7850 4400 0    60   Input ~ 0
CLT
Text GLabel 7850 4500 0    60   Input ~ 0
IAT
Text GLabel 8650 4500 2    60   Input ~ 0
O2
Text GLabel 8650 4400 2    60   Input ~ 0
TPS
$Comp
L GND #PWR03
U 1 1 5A584BE2
P 9100 4300
F 0 "#PWR03" H 9100 4050 50  0001 C CNN
F 1 "GND" H 9100 4150 50  0000 C CNN
F 2 "" H 9100 4300 50  0000 C CNN
F 3 "" H 9100 4300 50  0000 C CNN
	1    9100 4300
	0    -1   -1   0   
$EndComp
Text GLabel 8650 4200 2    60   Input ~ 0
VR2+
Text GLabel 8650 4100 2    60   Input ~ 0
VR1+
Text GLabel 8650 4000 2    60   Input ~ 0
VR2-
Text GLabel 8650 3900 2    60   Input ~ 0
VR1-
$Comp
L +5V #PWR04
U 1 1 5A584D66
P 9100 3800
F 0 "#PWR04" H 9100 3650 50  0001 C CNN
F 1 "+5V" H 9100 3940 50  0000 C CNN
F 2 "" H 9100 3800 50  0000 C CNN
F 3 "" H 9100 3800 50  0000 C CNN
	1    9100 3800
	0    1    1    0   
$EndComp
Text GLabel 8650 3700 2    60   Input ~ 0
STEP-2B
Text GLabel 8650 3600 2    60   Input ~ 0
STEP-2A
Text GLabel 8650 3500 2    60   Input ~ 0
STEP-1A
Text GLabel 8650 3400 2    60   Input ~ 0
STEP-1B
Text GLabel 8650 3200 2    60   Input ~ 0
IGN2
Text GLabel 8650 3100 2    60   Input ~ 0
BOOST
Text GLabel 8650 3000 2    60   Input ~ 0
IDLE2
Text GLabel 8650 2900 2    60   Input ~ 0
IDLE1
Text GLabel 8650 2800 2    60   Input ~ 0
VVT
Wire Wire Line
	7850 4500 8000 4500
Wire Wire Line
	7850 4400 8000 4400
Wire Wire Line
	7850 4300 8000 4300
Wire Wire Line
	7850 4200 8000 4200
Wire Wire Line
	7850 4100 8000 4100
Wire Wire Line
	7850 4000 8000 4000
Wire Wire Line
	7850 3900 8000 3900
Wire Wire Line
	7250 3800 8000 3800
Wire Wire Line
	8000 3700 7500 3700
Wire Wire Line
	7850 3600 8000 3600
Wire Wire Line
	7850 3000 8000 3000
Wire Wire Line
	7900 3500 8000 3500
Wire Wire Line
	7250 3400 8000 3400
Wire Wire Line
	7850 3300 8000 3300
Wire Wire Line
	7850 3200 8000 3200
Wire Wire Line
	7850 3100 8000 3100
Wire Wire Line
	7850 2900 8000 2900
Wire Wire Line
	8000 2800 7850 2800
Wire Wire Line
	8000 2700 7850 2700
Wire Wire Line
	8000 2600 8000 2300
Wire Wire Line
	8650 3000 8500 3000
Wire Wire Line
	8500 2900 8650 2900
Wire Wire Line
	8650 2800 8500 2800
Wire Wire Line
	8650 3100 8500 3100
Wire Wire Line
	8650 3200 8500 3200
Wire Wire Line
	8650 3300 8500 3300
Wire Wire Line
	8500 3400 8650 3400
Wire Wire Line
	8650 3500 8500 3500
Wire Wire Line
	8500 3600 8650 3600
Wire Wire Line
	8650 3700 8500 3700
Wire Wire Line
	9100 3800 8500 3800
Wire Wire Line
	8650 3900 8500 3900
Wire Wire Line
	8500 4000 8650 4000
Wire Wire Line
	8650 4100 8500 4100
Wire Wire Line
	8650 4200 8500 4200
Wire Wire Line
	9100 4300 8500 4300
Wire Wire Line
	8650 4400 8500 4400
Wire Wire Line
	8650 4500 8500 4500
Wire Wire Line
	8650 2700 8500 2700
Wire Wire Line
	8500 2300 8500 2600
$Comp
L CONN_02X20 P1
U 1 1 5A5844A5
P 8250 3550
F 0 "P1" H 8250 4600 50  0000 C CNN
F 1 "CONN_02X20" V 8250 3550 50  0000 C CNN
F 2 "Connect:IDC_Header_Straight_40pins" H 8250 2600 50  0001 C CNN
F 3 "" H 8250 2600 50  0000 C CNN
	1    8250 3550
	1    0    0    -1  
$EndComp
Text GLabel 1450 1850 2    60   Input ~ 0
INJ1
Text GLabel 1450 2050 2    60   Input ~ 0
INJ2
Text GLabel 1450 2250 2    60   Input ~ 0
INJ3
Text GLabel 1450 2450 2    60   Input ~ 0
INJ4
Text GLabel 4050 3200 0    60   Input ~ 0
IGN1
Text GLabel 4050 3300 0    60   Input ~ 0
IGN4
Text GLabel 4850 3300 2    60   Input ~ 0
IGN3
$Comp
L GND #PWR05
U 1 1 5A58530F
P 4850 2250
F 0 "#PWR05" H 4850 2000 50  0001 C CNN
F 1 "GND" H 4850 2100 50  0000 C CNN
F 2 "" H 4850 2250 50  0000 C CNN
F 3 "" H 4850 2250 50  0000 C CNN
	1    4850 2250
	0    1    1    0   
$EndComp
Text GLabel 4050 3600 0    60   Input ~ 0
MAP
$Comp
L +5V #PWR06
U 1 1 5A58531A
P 3450 3800
F 0 "#PWR06" H 3450 3650 50  0001 C CNN
F 1 "+5V" H 3450 3940 50  0000 C CNN
F 2 "" H 3450 3800 50  0000 C CNN
F 3 "" H 3450 3800 50  0000 C CNN
	1    3450 3800
	0    -1   -1   0   
$EndComp
Text GLabel 4050 3900 0    60   Input ~ 0
PROTO1
Text GLabel 4050 4000 0    60   Input ~ 0
PROTO2
Text GLabel 4050 4100 0    60   Input ~ 0
PROTO3
Text GLabel 4050 4200 0    60   Input ~ 0
PROTO4
Text GLabel 4050 4300 0    60   Input ~ 0
PROTO5
Text GLabel 4050 4400 0    60   Input ~ 0
CLT
Text GLabel 4050 4500 0    60   Input ~ 0
IAT
Text GLabel 4850 4500 2    60   Input ~ 0
O2
Text GLabel 4850 4400 2    60   Input ~ 0
TPS
$Comp
L GND #PWR07
U 1 1 5A585329
P 5300 4300
F 0 "#PWR07" H 5300 4050 50  0001 C CNN
F 1 "GND" H 5300 4150 50  0000 C CNN
F 2 "" H 5300 4300 50  0000 C CNN
F 3 "" H 5300 4300 50  0000 C CNN
	1    5300 4300
	0    -1   -1   0   
$EndComp
Text GLabel 4850 4200 2    60   Input ~ 0
VR2+
Text GLabel 4850 4100 2    60   Input ~ 0
VR1+
Text GLabel 4850 4000 2    60   Input ~ 0
VR2-
Text GLabel 4850 3900 2    60   Input ~ 0
VR1-
$Comp
L +5V #PWR08
U 1 1 5A585333
P 5300 3800
F 0 "#PWR08" H 5300 3650 50  0001 C CNN
F 1 "+5V" H 5300 3940 50  0000 C CNN
F 2 "" H 5300 3800 50  0000 C CNN
F 3 "" H 5300 3800 50  0000 C CNN
	1    5300 3800
	0    1    1    0   
$EndComp
Text GLabel 1900 2250 2    60   Input ~ 0
STEP-2B
Text GLabel 1900 2050 2    60   Input ~ 0
STEP-2A
Text GLabel 2000 1850 2    60   Input ~ 0
STEP-1A
Text GLabel 2050 1650 2    60   Input ~ 0
STEP-1B
Text GLabel 4850 3200 2    60   Input ~ 0
IGN2
Text GLabel 4850 3100 2    60   Input ~ 0
BOOST
Text GLabel 4850 3000 2    60   Input ~ 0
IDLE2
Text GLabel 4850 2900 2    60   Input ~ 0
IDLE1
Text GLabel 4850 2800 2    60   Input ~ 0
VVT
$Comp
L Screw_Terminal_1x05 J1
U 1 1 5A5956BA
P 1050 2250
F 0 "J1" H 1050 2800 50  0000 C TNN
F 1 "Screw_Terminal_1x05" V 900 2250 50  0000 C TNN
F 2 "TerminalBlock_Phoenix:TerminalBlock_Phoenix_PT-1,5-5-5.0-H_1x05_P5.00mm_Horizontal" H 1050 1725 50  0001 C CNN
F 3 "" H 1025 2550 50  0001 C CNN
	1    1050 2250
	1    0    0    -1  
$EndComp
$Comp
L Screw_Terminal_1x05 J2
U 1 1 5A5956EF
P 1050 3200
F 0 "J2" H 1050 3750 50  0000 C TNN
F 1 "Screw_Terminal_1x05" V 900 3200 50  0000 C TNN
F 2 "TerminalBlock_Phoenix:TerminalBlock_Phoenix_PT-1,5-5-5.0-H_1x05_P5.00mm_Horizontal" H 1050 2675 50  0001 C CNN
F 3 "" H 1025 3500 50  0001 C CNN
	1    1050 3200
	1    0    0    -1  
$EndComp
$Comp
L Screw_Terminal_1x05 J3
U 1 1 5A595747
P 1050 4150
F 0 "J3" H 1050 4700 50  0000 C TNN
F 1 "Screw_Terminal_1x05" V 900 4150 50  0000 C TNN
F 2 "TerminalBlock_Phoenix:TerminalBlock_Phoenix_PT-1,5-5-5.0-H_1x05_P5.00mm_Horizontal" H 1050 3625 50  0001 C CNN
F 3 "" H 1025 4450 50  0001 C CNN
	1    1050 4150
	1    0    0    -1  
$EndComp
$Comp
L Screw_Terminal_1x05 J4
U 1 1 5A595AD4
P 2700 2250
F 0 "J4" H 2700 2800 50  0000 C TNN
F 1 "Screw_Terminal_1x05" V 2550 2250 50  0000 C TNN
F 2 "TerminalBlock_Phoenix:TerminalBlock_Phoenix_PT-1,5-5-5.0-H_1x05_P5.00mm_Horizontal" H 2700 1725 50  0001 C CNN
F 3 "" H 2675 2550 50  0001 C CNN
	1    2700 2250
	1    0    0    -1  
$EndComp
$Comp
L Screw_Terminal_1x05 J5
U 1 1 5A595ADA
P 2700 3200
F 0 "J5" H 2700 3750 50  0000 C TNN
F 1 "Screw_Terminal_1x05" V 2550 3200 50  0000 C TNN
F 2 "TerminalBlock_Phoenix:TerminalBlock_Phoenix_PT-1,5-5-5.0-H_1x05_P5.00mm_Horizontal" H 2700 2675 50  0001 C CNN
F 3 "" H 2675 3500 50  0001 C CNN
	1    2700 3200
	1    0    0    -1  
$EndComp
$Comp
L Screw_Terminal_1x05 J6
U 1 1 5A595AE0
P 2700 4150
F 0 "J6" H 2700 4700 50  0000 C TNN
F 1 "Screw_Terminal_1x05" V 2550 4150 50  0000 C TNN
F 2 "TerminalBlock_Phoenix:TerminalBlock_Phoenix_PT-1,5-5-5.0-H_1x05_P5.00mm_Horizontal" H 2700 3625 50  0001 C CNN
F 3 "" H 2675 4450 50  0001 C CNN
	1    2700 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	1450 1850 1250 1850
Wire Wire Line
	1250 2050 1450 2050
Wire Wire Line
	1450 2250 1250 2250
Wire Wire Line
	1250 2450 1450 2450
$EndSCHEMATC
