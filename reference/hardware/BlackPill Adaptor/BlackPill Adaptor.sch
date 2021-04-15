EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 2
Title "STM32F4 BlackPill"
Date "2020-01-22"
Rev "a"
Comp ""
Comment1 "Designed by Vitor_Boss for Speeduino"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Device:R R2
U 1 1 5E12587F
P 1900 1350
F 0 "R2" H 1970 1396 50  0000 L CNN
F 1 "9.1K" H 1970 1305 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 1830 1350 50  0001 C CNN
F 3 "~" H 1900 1350 50  0001 C CNN
	1    1900 1350
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5E126656
P 1700 1150
F 0 "R1" H 1770 1196 50  0000 L CNN
F 1 "4.7K" H 1770 1105 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 1630 1150 50  0001 C CNN
F 3 "~" H 1700 1150 50  0001 C CNN
	1    1700 1150
	0    -1   -1   0   
$EndComp
$Comp
L Amplifier_Operational:LM324A U1
U 1 1 5E1278BD
P 1150 1150
F 0 "U1" H 1150 1517 50  0000 C CNN
F 1 "LM324A" H 1150 1426 50  0000 C CNN
F 2 "Package_SO:SOIC-14_3.9x8.7mm_P1.27mm" H 1100 1250 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 1200 1350 50  0001 C CNN
	1    1150 1150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR01
U 1 1 5E16237C
P 1900 1500
F 0 "#PWR01" H 1900 1250 50  0001 C CNN
F 1 "GND" H 1905 1327 50  0000 C CNN
F 2 "" H 1900 1500 50  0001 C CNN
F 3 "" H 1900 1500 50  0001 C CNN
	1    1900 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	850  1250 850  1400
Wire Wire Line
	1450 1150 1500 1150
Wire Wire Line
	850  1400 1500 1400
Wire Wire Line
	1500 1400 1500 1150
Wire Wire Line
	1550 1150 1500 1150
Connection ~ 1500 1150
Wire Wire Line
	1850 1150 1900 1150
$Comp
L Amplifier_Operational:LM324A U1
U 5 1 5E8422DF
P 6550 3250
F 0 "U1" V 6225 3250 50  0000 C CNN
F 1 "LM324A" V 6316 3250 50  0000 C CNN
F 2 "" H 6500 3350 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 6600 3450 50  0001 C CNN
	5    6550 3250
	0    1    1    0   
$EndComp
Wire Wire Line
	6850 3150 7050 3150
Wire Wire Line
	6250 3150 6050 3150
Text Label 7050 3150 0    50   ~ 0
5VB
Text Label 6050 3150 0    50   ~ 0
GND
$Comp
L Amplifier_Operational:LM324A U2
U 5 1 5E8A83BA
P 6550 3650
F 0 "U2" V 6225 3650 50  0000 C CNN
F 1 "LM324A" V 6316 3650 50  0000 C CNN
F 2 "" H 6500 3750 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 6600 3850 50  0001 C CNN
	5    6550 3650
	0    1    1    0   
$EndComp
Wire Wire Line
	6850 3550 7050 3550
Wire Wire Line
	6250 3550 6050 3550
Text Label 7050 3550 0    50   ~ 0
5VB
Text Label 6050 3550 0    50   ~ 0
GND
$Comp
L Device:C C1
U 1 1 5E94CFEF
P 900 4850
F 0 "C1" H 1015 4896 50  0000 L CNN
F 1 ".1uF" H 1015 4805 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 938 4700 50  0001 C CNN
F 3 "~" H 900 4850 50  0001 C CNN
	1    900  4850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5E94D6BE
P 900 5100
F 0 "#PWR0101" H 900 4850 50  0001 C CNN
F 1 "GND" H 905 4927 50  0000 C CNN
F 2 "" H 900 5100 50  0001 C CNN
F 3 "" H 900 5100 50  0001 C CNN
	1    900  5100
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0102
U 1 1 5E94DD5D
P 900 4600
F 0 "#PWR0102" H 900 4450 50  0001 C CNN
F 1 "+3.3V" H 915 4773 50  0000 C CNN
F 2 "" H 900 4600 50  0001 C CNN
F 3 "" H 900 4600 50  0001 C CNN
	1    900  4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	900  4700 900  4600
Wire Wire Line
	900  5000 900  5100
$Comp
L power:+3.3V #PWR0103
U 1 1 5E9B77F9
P 1000 5700
F 0 "#PWR0103" H 1000 5550 50  0001 C CNN
F 1 "+3.3V" V 1015 5828 50  0000 L CNN
F 2 "" H 1000 5700 50  0001 C CNN
F 3 "" H 1000 5700 50  0001 C CNN
	1    1000 5700
	0    -1   -1   0   
$EndComp
Wire Wire Line
	1000 5700 1700 5700
$Comp
L Device:C C2
U 1 1 5EA5BEAC
P 1350 4850
F 0 "C2" H 1465 4896 50  0000 L CNN
F 1 ".1uF" H 1465 4805 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 1388 4700 50  0001 C CNN
F 3 "~" H 1350 4850 50  0001 C CNN
	1    1350 4850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5EA5BEB2
P 1350 5100
F 0 "#PWR0104" H 1350 4850 50  0001 C CNN
F 1 "GND" H 1355 4927 50  0000 C CNN
F 2 "" H 1350 5100 50  0001 C CNN
F 3 "" H 1350 5100 50  0001 C CNN
	1    1350 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1350 4700 1350 4600
Wire Wire Line
	1350 5000 1350 5100
$Comp
L Device:C C3
U 1 1 5CBBE7C3
P 1800 4850
F 0 "C3" H 1915 4896 50  0000 L CNN
F 1 "1uF" H 1915 4805 50  0000 L CNN
F 2 "Capacitor_SMD:C_1210_3225Metric" H 1838 4700 50  0001 C CNN
F 3 "~" H 1800 4850 50  0001 C CNN
	1    1800 4850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0106
U 1 1 5CBBE7C9
P 1800 5100
F 0 "#PWR0106" H 1800 4850 50  0001 C CNN
F 1 "GND" H 1805 4927 50  0000 C CNN
F 2 "" H 1800 5100 50  0001 C CNN
F 3 "" H 1800 5100 50  0001 C CNN
	1    1800 5100
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0107
U 1 1 5CBBE7CF
P 1800 4600
F 0 "#PWR0107" H 1800 4450 50  0001 C CNN
F 1 "+3.3V" H 1815 4773 50  0000 C CNN
F 2 "" H 1800 4600 50  0001 C CNN
F 3 "" H 1800 4600 50  0001 C CNN
	1    1800 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	1800 4700 1800 4600
Wire Wire Line
	1800 5000 1800 5100
$Comp
L Device:C C4
U 1 1 5CBFD4E3
P 2200 4850
F 0 "C4" H 2315 4896 50  0000 L CNN
F 1 "4.7uF" H 2315 4805 50  0000 L CNN
F 2 "Capacitor_SMD:C_1210_3225Metric" H 2238 4700 50  0001 C CNN
F 3 "~" H 2200 4850 50  0001 C CNN
	1    2200 4850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0108
U 1 1 5CBFD4E9
P 2200 5100
F 0 "#PWR0108" H 2200 4850 50  0001 C CNN
F 1 "GND" H 2205 4927 50  0000 C CNN
F 2 "" H 2200 5100 50  0001 C CNN
F 3 "" H 2200 5100 50  0001 C CNN
	1    2200 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2200 4700 2200 4600
Wire Wire Line
	2200 5000 2200 5100
$Comp
L power:+5V #PWR0109
U 1 1 5CC3D578
P 2200 4600
F 0 "#PWR0109" H 2200 4450 50  0001 C CNN
F 1 "+5V" H 2215 4773 50  0000 C CNN
F 2 "" H 2200 4600 50  0001 C CNN
F 3 "" H 2200 4600 50  0001 C CNN
	1    2200 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	1000 5900 1700 5900
$Comp
L power:+5V #PWR0110
U 1 1 5CC8041B
P 1000 5900
F 0 "#PWR0110" H 1000 5750 50  0001 C CNN
F 1 "+5V" V 1015 6028 50  0000 L CNN
F 2 "" H 1000 5900 50  0001 C CNN
F 3 "" H 1000 5900 50  0001 C CNN
	1    1000 5900
	0    -1   -1   0   
$EndComp
Text Label 1850 4050 0    50   ~ 0
GND
Text Label 1850 3450 0    50   ~ 0
3V3
Wire Wire Line
	1450 3850 1350 3850
Wire Wire Line
	1350 3850 1350 4050
Wire Wire Line
	1350 4050 950  4050
Connection ~ 1350 3850
Wire Wire Line
	1350 3850 1250 3850
$Comp
L Device:R R17
U 1 1 5DCAC8EB
P 1100 3850
F 0 "R17" V 893 3850 50  0000 C CNN
F 1 "10K" V 984 3850 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 1030 3850 50  0001 C CNN
F 3 "~" H 1100 3850 50  0001 C CNN
	1    1100 3850
	0    1    1    0   
$EndComp
$Comp
L Device:R R18
U 1 1 5DE44FE2
P 3750 5600
F 0 "R18" V 3543 5600 50  0000 C CNN
F 1 "10K" V 3634 5600 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3680 5600 50  0001 C CNN
F 3 "~" H 3750 5600 50  0001 C CNN
	1    3750 5600
	0    1    1    0   
$EndComp
$Comp
L Device:R R21
U 1 1 5DE5B89F
P 4250 6150
F 0 "R21" V 4043 6150 50  0000 C CNN
F 1 "15K" V 4134 6150 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 4180 6150 50  0001 C CNN
F 3 "~" H 4250 6150 50  0001 C CNN
	1    4250 6150
	1    0    0    -1  
$EndComp
$Comp
L Device:R R19
U 1 1 5DE5C183
P 4550 6150
F 0 "R19" V 4343 6150 50  0000 C CNN
F 1 "15K" V 4434 6150 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 4480 6150 50  0001 C CNN
F 3 "~" H 4550 6150 50  0001 C CNN
	1    4550 6150
	1    0    0    -1  
$EndComp
$Comp
L Device:R R23
U 1 1 5DE6F91A
P 3950 6150
F 0 "R23" V 3743 6150 50  0000 C CNN
F 1 "15K" V 3834 6150 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3880 6150 50  0001 C CNN
F 3 "~" H 3950 6150 50  0001 C CNN
	1    3950 6150
	1    0    0    -1  
$EndComp
$Comp
L Device:C C5
U 1 1 5DEB15CA
P 2650 4850
F 0 "C5" H 2765 4896 50  0000 L CNN
F 1 ".1uF" H 2765 4805 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 2688 4700 50  0001 C CNN
F 3 "~" H 2650 4850 50  0001 C CNN
	1    2650 4850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0111
U 1 1 5DEB15D0
P 2650 5100
F 0 "#PWR0111" H 2650 4850 50  0001 C CNN
F 1 "GND" H 2655 4927 50  0000 C CNN
F 2 "" H 2650 5100 50  0001 C CNN
F 3 "" H 2650 5100 50  0001 C CNN
	1    2650 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 4700 2650 4600
Wire Wire Line
	2650 5000 2650 5100
$Comp
L power:+5V #PWR0112
U 1 1 5DEB5F74
P 2650 4600
F 0 "#PWR0112" H 2650 4450 50  0001 C CNN
F 1 "+5V" H 2665 4773 50  0000 C CNN
F 2 "" H 2650 4600 50  0001 C CNN
F 3 "" H 2650 4600 50  0001 C CNN
	1    2650 4600
	1    0    0    -1  
$EndComp
Text GLabel 850  1050 0    50   Input ~ 0
AD0
Text GLabel 1950 1150 2    50   Input ~ 0
PA0
Text GLabel 2250 3650 2    50   Input ~ 0
SCK
Text GLabel 2250 3750 2    50   Input ~ 0
MOSI
Text GLabel 2250 3850 2    50   Input ~ 0
MISO
Text GLabel 900  3450 0    50   Input ~ 0
3V3
Wire Wire Line
	1850 3450 1350 3450
Wire Wire Line
	950  3850 950  3450
Connection ~ 950  3450
Wire Wire Line
	950  3450 900  3450
Wire Wire Line
	1450 3650 1350 3650
Wire Wire Line
	1350 3650 1350 3450
Connection ~ 1350 3450
Wire Wire Line
	1350 3450 950  3450
Wire Wire Line
	1450 3750 1350 3750
Wire Wire Line
	1350 3750 1350 3650
Connection ~ 1350 3650
Text GLabel 950  4050 0    50   Input ~ 0
CS
Text GLabel 6400 6400 2    50   Input ~ 0
pinIGN3
Text GLabel 1700 5700 2    50   Input ~ 0
3V3
Text GLabel 1700 5900 2    50   Input ~ 0
5V
$Comp
L BlackPill-Adaptor-rescue:BlackPill-bluepill U3
U 1 1 5E4BF5B8
P 5650 6400
F 0 "U3" H 5650 7587 60  0000 C CNN
F 1 "BlackPill" H 5650 7481 60  0000 C CNN
F 2 "Module:Maple_Mini" H 5550 7150 60  0001 C CNN
F 3 "" H 5550 7150 60  0001 C CNN
	1    5650 6400
	1    0    0    -1  
$EndComp
Text GLabel 4900 6900 0    50   Input ~ 0
pinTachOut
Text GLabel 4900 7100 0    50   Input ~ 0
pinStepperDir
Text GLabel 4900 6000 0    50   Input ~ 0
PA0
Text GLabel 4900 6100 0    50   Input ~ 0
PA1
Text GLabel 4900 6200 0    50   Input ~ 0
PA2
Text GLabel 4900 6300 0    50   Input ~ 0
PA3
Text GLabel 4900 6400 0    50   Input ~ 0
PA4
Text GLabel 4900 6500 0    50   Input ~ 0
PA5
Text GLabel 4900 6600 0    50   Input ~ 0
PA6
Text GLabel 4900 6700 0    50   Input ~ 0
PA7
Text GLabel 6400 7400 2    50   Input ~ 0
CS
Text GLabel 6400 7000 2    50   Input ~ 0
pinFuelPump
Text GLabel 4900 7200 0    50   Input ~ 0
3V3
Text GLabel 4900 7300 0    50   Input ~ 0
GND
Text GLabel 4900 7400 0    50   Input ~ 0
5V
Text GLabel 6400 5500 2    50   Input ~ 0
3V3
Text GLabel 6400 5600 2    50   Input ~ 0
GND
Text GLabel 6400 7300 2    50   Input ~ 0
SCK
Text GLabel 6400 7200 2    50   Input ~ 0
MISO
Text GLabel 6400 7100 2    50   Input ~ 0
MOSI
Text GLabel 6400 6500 2    50   Input ~ 0
pinIGN4
Text GLabel 6400 6900 2    50   Input ~ 0
pinTX
Text GLabel 6400 6800 2    50   Input ~ 0
pinRX
Text GLabel 6400 6700 2    50   Input ~ 0
PA11
Text GLabel 6400 6600 2    50   Input ~ 0
PA12
Text GLabel 6400 5900 2    50   Input ~ 0
pinIGN2
Text GLabel 6400 5800 2    50   Input ~ 0
pinIGN1
Text GLabel 6400 6300 2    50   Input ~ 0
pinINJ4
Text GLabel 6400 6200 2    50   Input ~ 0
pinINJ3
Text GLabel 6400 6100 2    50   Input ~ 0
pinINJ2
Text GLabel 6400 6000 2    50   Input ~ 0
pinINJ1
$Comp
L power:GND #PWR0114
U 1 1 5E51F631
P 4250 6500
F 0 "#PWR0114" H 4250 6250 50  0001 C CNN
F 1 "GND" H 4255 6327 50  0000 C CNN
F 2 "" H 4250 6500 50  0001 C CNN
F 3 "" H 4250 6500 50  0001 C CNN
	1    4250 6500
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 5800 4900 5800
Wire Wire Line
	3950 6450 4250 6450
Wire Wire Line
	4250 6450 4250 6500
Connection ~ 4250 6450
Wire Wire Line
	4550 6450 4250 6450
Wire Wire Line
	3950 6300 3950 6450
Wire Wire Line
	4250 6300 4250 6450
Wire Wire Line
	4550 6300 4550 6450
Connection ~ 4550 5600
Wire Wire Line
	4550 5600 4900 5600
$Comp
L Device:R R22
U 1 1 5DE4BECA
P 3750 5800
F 0 "R22" V 3543 5800 50  0000 C CNN
F 1 "10K" V 3634 5800 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3680 5800 50  0001 C CNN
F 3 "~" H 3750 5800 50  0001 C CNN
	1    3750 5800
	0    1    -1   0   
$EndComp
$Comp
L Device:R R20
U 1 1 5DE4C55C
P 4000 5700
F 0 "R20" V 3793 5700 50  0000 C CNN
F 1 "10K" V 3884 5700 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3930 5700 50  0001 C CNN
F 3 "~" H 4000 5700 50  0001 C CNN
	1    4000 5700
	0    1    1    0   
$EndComp
Text GLabel 3600 5600 0    50   Input ~ 0
pinTrigger
Text GLabel 3600 5800 0    50   Input ~ 0
pinTrigger2
Text GLabel 3600 5700 0    50   Input ~ 0
pinFLEX
Text GLabel 4900 5500 0    50   Input ~ 0
VBAT
Text GLabel 4900 7000 0    50   Input ~ 0
pinStepperStep
Wire Wire Line
	3900 5600 4550 5600
Wire Wire Line
	3950 5800 3950 6000
Wire Wire Line
	4250 5700 4900 5700
Wire Wire Line
	4550 5600 4550 6000
Wire Wire Line
	4250 6000 4250 5700
Wire Wire Line
	3950 5800 3900 5800
Connection ~ 3950 5800
Wire Wire Line
	4250 5700 4150 5700
Connection ~ 4250 5700
Wire Wire Line
	3600 5700 3850 5700
$Comp
L Connector:Conn_01x06_Female J5
U 1 1 5E2FC672
P 2900 3800
F 0 "J5" H 2792 3275 50  0000 C CNN
F 1 "Conn_01x06_Female" H 2792 3366 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" H 2900 3800 50  0001 C CNN
F 3 "~" H 2900 3800 50  0001 C CNN
	1    2900 3800
	-1   0    0    1   
$EndComp
Text GLabel 3100 3600 2    50   Input ~ 0
MISO
Text GLabel 3100 3700 2    50   Input ~ 0
MOSI
Text GLabel 3100 3500 2    50   Input ~ 0
SCK
Text GLabel 3100 3800 2    50   Input ~ 0
3V3
Text GLabel 3100 3900 2    50   Input ~ 0
GND
Text GLabel 3100 4000 2    50   Input ~ 0
PA7
$Comp
L Device:C C6
U 1 1 5E351960
P 3050 4850
F 0 "C6" H 3165 4896 50  0000 L CNN
F 1 ".1uF" H 3165 4805 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 3088 4700 50  0001 C CNN
F 3 "~" H 3050 4850 50  0001 C CNN
	1    3050 4850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0115
U 1 1 5E351966
P 3050 5100
F 0 "#PWR0115" H 3050 4850 50  0001 C CNN
F 1 "GND" H 3055 4927 50  0000 C CNN
F 2 "" H 3050 5100 50  0001 C CNN
F 3 "" H 3050 5100 50  0001 C CNN
	1    3050 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	3050 4700 3050 4600
Wire Wire Line
	3050 5000 3050 5100
$Comp
L Graphic:Logo_Open_Hardware_Small #LOGO1
U 1 1 5E3586CD
P 1000 7400
F 0 "#LOGO1" H 1000 7675 50  0001 C CNN
F 1 "Logo_Open_Hardware_Small" H 1000 7175 50  0001 C CNN
F 2 "Symbol:OSHW-Logo2_9.8x8mm_SilkScreen" H 1000 7400 50  0001 C CNN
F 3 "~" H 1000 7400 50  0001 C CNN
	1    1000 7400
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0116
U 1 1 5E361B80
P 3050 4600
F 0 "#PWR0116" H 3050 4450 50  0001 C CNN
F 1 "+3.3V" H 3065 4773 50  0000 C CNN
F 2 "" H 3050 4600 50  0001 C CNN
F 3 "" H 3050 4600 50  0001 C CNN
	1    3050 4600
	1    0    0    -1  
$EndComp
Text GLabel 4150 3800 0    50   Input ~ 0
pinIDLE1
Text GLabel 4350 3800 2    50   Input ~ 0
PB2
Wire Wire Line
	4150 3800 4350 3800
Text GLabel 4900 6800 0    50   Input ~ 0
PB0
Wire Wire Line
	4150 3700 4350 3700
Text GLabel 4350 3700 2    50   Input ~ 0
PA6
Text GLabel 4150 3700 0    50   Input ~ 0
pinBoost
Text GLabel 6450 3850 0    50   Input ~ 0
5VB
Text GLabel 6750 3850 2    50   Input ~ 0
5V
Wire Wire Line
	1900 1200 1900 1150
Connection ~ 1900 1150
Wire Wire Line
	1900 1150 1950 1150
$Comp
L Device:R R4
U 1 1 5EA99D92
P 3650 1350
F 0 "R4" H 3720 1396 50  0000 L CNN
F 1 "9.1K" H 3720 1305 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3580 1350 50  0001 C CNN
F 3 "~" H 3650 1350 50  0001 C CNN
	1    3650 1350
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 5EA99D98
P 3450 1150
F 0 "R3" H 3520 1196 50  0000 L CNN
F 1 "4.7K" H 3520 1105 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3380 1150 50  0001 C CNN
F 3 "~" H 3450 1150 50  0001 C CNN
	1    3450 1150
	0    -1   -1   0   
$EndComp
$Comp
L Amplifier_Operational:LM324A U1
U 2 1 5EA99D9E
P 2900 1150
F 0 "U1" H 2900 1517 50  0000 C CNN
F 1 "LM324A" H 2900 1426 50  0000 C CNN
F 2 "Package_SO:SOIC-14_3.9x8.7mm_P1.27mm" H 2850 1250 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 2950 1350 50  0001 C CNN
	2    2900 1150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0117
U 1 1 5EA99DA4
P 3650 1500
F 0 "#PWR0117" H 3650 1250 50  0001 C CNN
F 1 "GND" H 3655 1327 50  0000 C CNN
F 2 "" H 3650 1500 50  0001 C CNN
F 3 "" H 3650 1500 50  0001 C CNN
	1    3650 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	2600 1250 2600 1400
Wire Wire Line
	3200 1150 3250 1150
Wire Wire Line
	2600 1400 3250 1400
Wire Wire Line
	3250 1400 3250 1150
Wire Wire Line
	3300 1150 3250 1150
Connection ~ 3250 1150
Wire Wire Line
	3600 1150 3650 1150
Text GLabel 2600 1050 0    50   Input ~ 0
AD1
Text GLabel 3700 1150 2    50   Input ~ 0
PA1
Wire Wire Line
	3650 1200 3650 1150
Connection ~ 3650 1150
Wire Wire Line
	3650 1150 3700 1150
$Comp
L Device:R R6
U 1 1 5EAAC1D3
P 5400 1350
F 0 "R6" H 5470 1396 50  0000 L CNN
F 1 "9.1K" H 5470 1305 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 5330 1350 50  0001 C CNN
F 3 "~" H 5400 1350 50  0001 C CNN
	1    5400 1350
	1    0    0    -1  
$EndComp
$Comp
L Device:R R5
U 1 1 5EAAC1D9
P 5200 1150
F 0 "R5" H 5270 1196 50  0000 L CNN
F 1 "4.7K" H 5270 1105 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 5130 1150 50  0001 C CNN
F 3 "~" H 5200 1150 50  0001 C CNN
	1    5200 1150
	0    -1   -1   0   
$EndComp
$Comp
L Amplifier_Operational:LM324A U1
U 4 1 5EAAC1DF
P 6400 1150
F 0 "U1" H 6400 1517 50  0000 C CNN
F 1 "LM324A" H 6400 1426 50  0000 C CNN
F 2 "Package_SO:SOIC-14_3.9x8.7mm_P1.27mm" H 6350 1250 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 6450 1350 50  0001 C CNN
	4    6400 1150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0118
U 1 1 5EAAC1E5
P 5400 1500
F 0 "#PWR0118" H 5400 1250 50  0001 C CNN
F 1 "GND" H 5405 1327 50  0000 C CNN
F 2 "" H 5400 1500 50  0001 C CNN
F 3 "" H 5400 1500 50  0001 C CNN
	1    5400 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4350 1250 4350 1400
Wire Wire Line
	4950 1150 5000 1150
Wire Wire Line
	4350 1400 5000 1400
Wire Wire Line
	5000 1400 5000 1150
Wire Wire Line
	5050 1150 5000 1150
Connection ~ 5000 1150
Wire Wire Line
	5350 1150 5400 1150
Text GLabel 4350 1050 0    50   Input ~ 0
AD2
Text GLabel 5450 1150 2    50   Input ~ 0
PA2
Wire Wire Line
	5400 1200 5400 1150
Connection ~ 5400 1150
Wire Wire Line
	5400 1150 5450 1150
$Comp
L Device:R R8
U 1 1 5EAB6677
P 7150 1350
F 0 "R8" H 7220 1396 50  0000 L CNN
F 1 "9.1K" H 7220 1305 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 7080 1350 50  0001 C CNN
F 3 "~" H 7150 1350 50  0001 C CNN
	1    7150 1350
	1    0    0    -1  
$EndComp
$Comp
L Device:R R7
U 1 1 5EAB667D
P 6950 1150
F 0 "R7" H 7020 1196 50  0000 L CNN
F 1 "4.7K" H 7020 1105 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 6880 1150 50  0001 C CNN
F 3 "~" H 6950 1150 50  0001 C CNN
	1    6950 1150
	0    -1   -1   0   
$EndComp
$Comp
L Amplifier_Operational:LM324A U1
U 3 1 5EAB6683
P 4650 1150
F 0 "U1" H 4650 1517 50  0000 C CNN
F 1 "LM324A" H 4650 1426 50  0000 C CNN
F 2 "Package_SO:SOIC-14_3.9x8.7mm_P1.27mm" H 4600 1250 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 4700 1350 50  0001 C CNN
	3    4650 1150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0119
U 1 1 5EAB6689
P 7150 1500
F 0 "#PWR0119" H 7150 1250 50  0001 C CNN
F 1 "GND" H 7155 1327 50  0000 C CNN
F 2 "" H 7150 1500 50  0001 C CNN
F 3 "" H 7150 1500 50  0001 C CNN
	1    7150 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 1250 6100 1400
Wire Wire Line
	6700 1150 6750 1150
Wire Wire Line
	6100 1400 6750 1400
Wire Wire Line
	6750 1400 6750 1150
Wire Wire Line
	6800 1150 6750 1150
Connection ~ 6750 1150
Wire Wire Line
	7100 1150 7150 1150
Text GLabel 6100 1050 0    50   Input ~ 0
AD3
Text GLabel 7200 1150 2    50   Input ~ 0
PA3
Wire Wire Line
	7150 1200 7150 1150
Connection ~ 7150 1150
Wire Wire Line
	7150 1150 7200 1150
$Comp
L Device:R R10
U 1 1 5EAC4C98
P 1900 2400
F 0 "R10" H 1970 2446 50  0000 L CNN
F 1 "9.1K" H 1970 2355 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 1830 2400 50  0001 C CNN
F 3 "~" H 1900 2400 50  0001 C CNN
	1    1900 2400
	1    0    0    -1  
$EndComp
$Comp
L Device:R R9
U 1 1 5EAC4C9E
P 1700 2200
F 0 "R9" H 1770 2246 50  0000 L CNN
F 1 "4.7K" H 1770 2155 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 1630 2200 50  0001 C CNN
F 3 "~" H 1700 2200 50  0001 C CNN
	1    1700 2200
	0    -1   -1   0   
$EndComp
$Comp
L Amplifier_Operational:LM324A U2
U 1 1 5EAC4CA4
P 1150 2200
F 0 "U2" H 1150 2567 50  0000 C CNN
F 1 "LM324A" H 1150 2476 50  0000 C CNN
F 2 "Package_SO:SOIC-14_3.9x8.7mm_P1.27mm" H 1100 2300 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 1200 2400 50  0001 C CNN
	1    1150 2200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0120
U 1 1 5EAC4CAA
P 1900 2550
F 0 "#PWR0120" H 1900 2300 50  0001 C CNN
F 1 "GND" H 1905 2377 50  0000 C CNN
F 2 "" H 1900 2550 50  0001 C CNN
F 3 "" H 1900 2550 50  0001 C CNN
	1    1900 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	850  2300 850  2450
Wire Wire Line
	1450 2200 1500 2200
Wire Wire Line
	850  2450 1500 2450
Wire Wire Line
	1500 2450 1500 2200
Wire Wire Line
	1550 2200 1500 2200
Connection ~ 1500 2200
Wire Wire Line
	1850 2200 1900 2200
Text GLabel 850  2100 0    50   Input ~ 0
AD4
Text GLabel 1950 2200 2    50   Input ~ 0
PA4
Wire Wire Line
	1900 2250 1900 2200
Connection ~ 1900 2200
Wire Wire Line
	1900 2200 1950 2200
$Comp
L Device:R R12
U 1 1 5EAD102A
P 3650 2350
F 0 "R12" H 3720 2396 50  0000 L CNN
F 1 "9.1K" H 3720 2305 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3580 2350 50  0001 C CNN
F 3 "~" H 3650 2350 50  0001 C CNN
	1    3650 2350
	1    0    0    -1  
$EndComp
$Comp
L Device:R R11
U 1 1 5EAD1030
P 3450 2150
F 0 "R11" H 3520 2196 50  0000 L CNN
F 1 "4.7K" H 3520 2105 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3380 2150 50  0001 C CNN
F 3 "~" H 3450 2150 50  0001 C CNN
	1    3450 2150
	0    -1   -1   0   
$EndComp
$Comp
L Amplifier_Operational:LM324A U2
U 2 1 5EAD1036
P 2900 2150
F 0 "U2" H 2900 2517 50  0000 C CNN
F 1 "LM324A" H 2900 2426 50  0000 C CNN
F 2 "Package_SO:SOIC-14_3.9x8.7mm_P1.27mm" H 2850 2250 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 2950 2350 50  0001 C CNN
	2    2900 2150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0121
U 1 1 5EAD103C
P 3650 2500
F 0 "#PWR0121" H 3650 2250 50  0001 C CNN
F 1 "GND" H 3655 2327 50  0000 C CNN
F 2 "" H 3650 2500 50  0001 C CNN
F 3 "" H 3650 2500 50  0001 C CNN
	1    3650 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	2600 2250 2600 2400
Wire Wire Line
	3200 2150 3250 2150
Wire Wire Line
	2600 2400 3250 2400
Wire Wire Line
	3250 2400 3250 2150
Wire Wire Line
	3300 2150 3250 2150
Connection ~ 3250 2150
Wire Wire Line
	3600 2150 3650 2150
Text GLabel 2600 2050 0    50   Input ~ 0
AD5
Text GLabel 3700 2150 2    50   Input ~ 0
PA5
Wire Wire Line
	3650 2200 3650 2150
Connection ~ 3650 2150
Wire Wire Line
	3650 2150 3700 2150
$Comp
L Device:R R14
U 1 1 5EADEEBD
P 5400 2350
F 0 "R14" H 5470 2396 50  0000 L CNN
F 1 "9.1K" H 5470 2305 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 5330 2350 50  0001 C CNN
F 3 "~" H 5400 2350 50  0001 C CNN
	1    5400 2350
	1    0    0    -1  
$EndComp
$Comp
L Device:R R13
U 1 1 5EADEEC3
P 5200 2150
F 0 "R13" H 5270 2196 50  0000 L CNN
F 1 "4.7K" H 5270 2105 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 5130 2150 50  0001 C CNN
F 3 "~" H 5200 2150 50  0001 C CNN
	1    5200 2150
	0    -1   -1   0   
$EndComp
$Comp
L Amplifier_Operational:LM324A U2
U 4 1 5EADEEC9
P 6400 2150
F 0 "U2" H 6400 2517 50  0000 C CNN
F 1 "LM324A" H 6400 2426 50  0000 C CNN
F 2 "Package_SO:SOIC-14_3.9x8.7mm_P1.27mm" H 6350 2250 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 6450 2350 50  0001 C CNN
	4    6400 2150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0122
U 1 1 5EADEECF
P 5400 2500
F 0 "#PWR0122" H 5400 2250 50  0001 C CNN
F 1 "GND" H 5405 2327 50  0000 C CNN
F 2 "" H 5400 2500 50  0001 C CNN
F 3 "" H 5400 2500 50  0001 C CNN
	1    5400 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4350 2250 4350 2400
Wire Wire Line
	4950 2150 5000 2150
Wire Wire Line
	4350 2400 5000 2400
Wire Wire Line
	5000 2400 5000 2150
Wire Wire Line
	5050 2150 5000 2150
Connection ~ 5000 2150
Wire Wire Line
	5350 2150 5400 2150
Text GLabel 4350 2050 0    50   Input ~ 0
AD6
Text GLabel 5450 2150 2    50   Input ~ 0
PA6
Wire Wire Line
	5400 2200 5400 2150
Connection ~ 5400 2150
Wire Wire Line
	5400 2150 5450 2150
$Comp
L Device:R R16
U 1 1 5EAEC763
P 7150 2350
F 0 "R16" H 7220 2396 50  0000 L CNN
F 1 "9.1K" H 7220 2305 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 7080 2350 50  0001 C CNN
F 3 "~" H 7150 2350 50  0001 C CNN
	1    7150 2350
	1    0    0    -1  
$EndComp
$Comp
L Device:R R15
U 1 1 5EAEC769
P 6950 2150
F 0 "R15" H 7020 2196 50  0000 L CNN
F 1 "4.7K" H 7020 2105 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 6880 2150 50  0001 C CNN
F 3 "~" H 6950 2150 50  0001 C CNN
	1    6950 2150
	0    -1   -1   0   
$EndComp
$Comp
L Amplifier_Operational:LM324A U2
U 3 1 5EAEC76F
P 4650 2150
F 0 "U2" H 4650 2517 50  0000 C CNN
F 1 "LM324A" H 4650 2426 50  0000 C CNN
F 2 "Package_SO:SOIC-14_3.9x8.7mm_P1.27mm" H 4600 2250 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 4700 2350 50  0001 C CNN
	3    4650 2150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0123
U 1 1 5EAEC775
P 7150 2500
F 0 "#PWR0123" H 7150 2250 50  0001 C CNN
F 1 "GND" H 7155 2327 50  0000 C CNN
F 2 "" H 7150 2500 50  0001 C CNN
F 3 "" H 7150 2500 50  0001 C CNN
	1    7150 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 2250 6100 2400
Wire Wire Line
	6700 2150 6750 2150
Wire Wire Line
	6100 2400 6750 2400
Wire Wire Line
	6750 2400 6750 2150
Wire Wire Line
	6800 2150 6750 2150
Connection ~ 6750 2150
Wire Wire Line
	7100 2150 7150 2150
Text GLabel 6100 2050 0    50   Input ~ 0
AD8
Text GLabel 7200 2150 2    50   Input ~ 0
PB0
Wire Wire Line
	7150 2200 7150 2150
Connection ~ 7150 2150
Wire Wire Line
	7150 2150 7200 2150
Text GLabel 1350 4600 1    50   Input ~ 0
5VB
Text GLabel 4150 3900 0    50   Input ~ 0
pinIDLE2
Text GLabel 4350 3900 2    50   Input ~ 0
PB10
Wire Wire Line
	4150 3900 4350 3900
$Sheet
S 7750 700  3200 5600
U 5DBA6F8D
F0 "Shield" 50
F1 "Arduino Mega Shield.sch" 50
$EndSheet
Text GLabel 8300 5900 0    50   Output ~ 0
pinIGN2
Text GLabel 8300 5800 0    50   Input ~ 0
Pin39
Text GLabel 8300 5700 0    50   Output ~ 0
pinIGN1
Text GLabel 8300 5600 0    50   Input ~ 0
Pin41
Text GLabel 8300 5450 0    50   Input ~ 0
Pin42
Text GLabel 8300 5350 0    50   Input ~ 0
Pin43
Text GLabel 8300 5250 0    50   Input ~ 0
Pin44
Text GLabel 8300 5150 0    50   Output ~ 0
pinFuelPump
Text GLabel 8300 5050 0    50   Input ~ 0
Pin46
Text GLabel 8300 4950 0    50   Output ~ 0
pinFan
Text GLabel 8300 4850 0    50   Input ~ 0
Pin48
Text GLabel 8300 4750 0    50   Output ~ 0
pinTachOut
Text GLabel 8300 4600 0    50   Output ~ 0
pinIGN4
Text GLabel 8300 4500 0    50   Input ~ 0
pinLaunch
Text GLabel 8300 4400 0    50   Output ~ 0
pinIGN3
Text GLabel 8300 4300 0    50   Input ~ 0
pin53
Text GLabel 8300 4150 0    50   Input ~ 0
GND
Text GLabel 8300 4050 0    50   Input ~ 0
GND
Text GLabel 8300 3800 0    50   Input ~ 0
AD15
Text GLabel 8300 3700 0    50   Input ~ 0
AD14
Text GLabel 8300 3600 0    50   Input ~ 0
AD13
Text GLabel 8300 3500 0    50   Input ~ 0
AD12
Text GLabel 8300 3400 0    50   Input ~ 0
AD11
Text GLabel 8300 3300 0    50   Input ~ 0
AD10
Text GLabel 8300 3200 0    50   Input ~ 0
AD9
Text GLabel 8300 3100 0    50   Input ~ 0
AD8
Text GLabel 8300 2900 0    50   Input ~ 0
AD7
Text GLabel 8300 2800 0    50   Input ~ 0
AD6
Text GLabel 8300 2700 0    50   Input ~ 0
AD5
Text GLabel 8300 2600 0    50   Input ~ 0
AD4
Text GLabel 8300 2500 0    50   Input ~ 0
AD3
Text GLabel 8300 2400 0    50   Input ~ 0
AD2
Text GLabel 8300 2300 0    50   Input ~ 0
AD1
Text GLabel 8300 2200 0    50   Input ~ 0
AD0
Text GLabel 8300 2000 0    50   UnSpc ~ 0
V_IN
Text GLabel 8300 1900 0    50   Input ~ 0
GND
Text GLabel 8300 1800 0    50   Input ~ 0
GND
Text GLabel 8300 1600 0    50   UnSpc ~ 0
3V3
Text GLabel 10200 5650 2    50   BiDi ~ 0
pin37
Text GLabel 10200 5550 2    50   BiDi ~ 0
pin36
Text GLabel 10200 5450 2    50   BiDi ~ 0
pin35
Text GLabel 10200 5350 2    50   BiDi ~ 0
pinIGN5
Text GLabel 10200 5250 2    50   BiDi ~ 0
pin33
Text GLabel 10200 5150 2    50   BiDi ~ 0
pin32
Text GLabel 10200 5050 2    50   BiDi ~ 0
pin31
Text GLabel 10200 4950 2    50   BiDi ~ 0
pin30
Text GLabel 10200 4800 2    50   BiDi ~ 0
pin29
Text GLabel 10200 4700 2    50   BiDi ~ 0
pin28
Text GLabel 10200 4600 2    50   BiDi ~ 0
pin27
Text GLabel 10200 4500 2    50   BiDi ~ 0
pin26
Text GLabel 10200 4400 2    50   BiDi ~ 0
pin25
Text GLabel 10200 4300 2    50   Output ~ 0
pinStepperEnable
Text GLabel 10200 4200 2    50   3State ~ 0
pin23
Text GLabel 10200 4100 2    50   BiDi ~ 0
pin22
Text GLabel 10200 3950 2    50   Input ~ 0
5VB
Text GLabel 10200 3850 2    50   Input ~ 0
5VB
Text GLabel 10200 3600 2    50   BiDi ~ 0
pin21
Text GLabel 10200 3500 2    50   BiDi ~ 0
pin20
Text GLabel 10200 3400 2    50   Input ~ 0
pinTrigger
Text GLabel 10200 3300 2    50   Input ~ 0
pinTrigger2
Text GLabel 10200 3200 2    50   Output ~ 0
pinStepperStep
Text GLabel 10200 3100 2    50   Output ~ 0
pinStepperDir
Text GLabel 10200 3000 2    50   3State ~ 0
pin15
Text GLabel 10200 2900 2    50   3State ~ 0
pin14
Text GLabel 10200 2700 2    50   Input ~ 0
pinRX
Text GLabel 10200 2600 2    50   Output ~ 0
pinTX
Text GLabel 10200 2500 2    50   Input ~ 0
pinFLEX
Text GLabel 10200 2400 2    50   BiDi ~ 0
pin3
Text GLabel 10200 2300 2    50   Output ~ 0
pinVVT_1
Text GLabel 10200 2200 2    50   Output ~ 0
pinIDLE1
Text GLabel 10200 2100 2    50   Output ~ 0
pinIDLE2
Text GLabel 10200 2000 2    50   Output ~ 0
pinBoost
Text GLabel 10200 1800 2    50   Output ~ 0
pinINJ1
Text GLabel 10200 1700 2    50   Output ~ 0
pinINJ2
Text GLabel 10200 1600 2    50   Output ~ 0
pinINJ3
Text GLabel 10200 1500 2    50   Output ~ 0
pinINJ4
Text GLabel 10200 1400 2    50   Output ~ 0
pinINJ5
Text GLabel 10200 1300 2    50   Output ~ 0
STATUS
Text GLabel 10200 1200 2    50   UnSpc ~ 0
GND
Text GLabel 10200 1100 2    50   UnSpc ~ 0
AREF
$Comp
L BlackPill-Adaptor-rescue:ARDUINO_MEGA_SHIELD-arduino_shieldsNCL SHIELD1
U 1 1 5CB97754
P 9300 3450
F 0 "SHIELD1" H 9250 6087 60  0000 C CNN
F 1 "ARDUINO_MEGA_SHIELD" H 9250 5981 60  0000 C CNN
F 2 "arduino_shields:ARDUINO MEGA SHIELD" H 9300 3450 50  0001 C CNN
F 3 "" H 9300 3450 50  0001 C CNN
	1    9300 3450
	1    0    0    -1  
$EndComp
$Comp
L Memory_NVRAM:MB85RS64 U4
U 1 1 5DCA5323
P 1850 3750
F 0 "U4" H 1850 4231 50  0000 C CNN
F 1 "MB85RS64" H 1850 4140 50  0000 C CNN
F 2 "Package_SO:SOIC-8_5.23x5.23mm_P1.27mm" H 1500 3700 50  0001 C CNN
F 3 "http://www.fujitsu.com/downloads/MICRO/fme/fram/datasheet-MB85RS64.pdf" H 1500 3700 50  0001 C CNN
	1    1850 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 3600 4350 3600
Text GLabel 4350 3600 2    50   Input ~ 0
PA5
Text GLabel 4150 3600 0    50   Input ~ 0
pinFan
Text GLabel 8300 1700 0    50   UnSpc ~ 0
5VB
$Comp
L Diode:1N5819 D1
U 1 1 5E43C317
P 6600 3850
F 0 "D1" V 6554 3929 50  0000 L CNN
F 1 "1N5819" V 6645 3929 50  0000 L CNN
F 2 "Diode_THT:D_DO-41_SOD81_P10.16mm_Horizontal" H 6600 3675 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88525/1n5817.pdf" H 6600 3850 50  0001 C CNN
	1    6600 3850
	-1   0    0    1   
$EndComp
$EndSCHEMATC
