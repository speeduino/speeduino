EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr User 8268 5846
encoding utf-8
Sheet 1 1
Title "Dual Variable Reluctance MAX9926 mode A2"
Date "2021-02-23"
Rev "0.4"
Comp "(c)2021 Phill Rogers @ TechColab.co.je"
Comment1 "Optional onboard regulator or 5V direct"
Comment2 "Optional power resistors"
Comment3 "Optional edge header for breadboard etc."
Comment4 "M2 mounting holes"
$EndDescr
Text Notes 6450 1000 0    60   ~ 0
Furniture
$Comp
L Mechanical:Fiducial FID1
U 1 1 60331E91
P 6600 600
F 0 "FID1" H 6668 642 50  0000 L CNN
F 1 "Fiducial" H 6668 567 50  0000 L CNN
F 2 "Fiducial:Fiducial_0.5mm_Mask1mm" H 6600 600 50  0001 C CNN
F 3 "~" H 6600 600 50  0001 C CNN
	1    6600 600 
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:Fiducial FID2
U 1 1 6034DBD2
P 6600 750
F 0 "FID2" H 6668 792 50  0000 L CNN
F 1 "Fiducial" H 6668 717 50  0000 L CNN
F 2 "Fiducial:Fiducial_0.5mm_Mask1mm" H 6600 750 50  0001 C CNN
F 3 "~" H 6600 750 50  0001 C CNN
	1    6600 750 
	1    0    0    -1  
$EndComp
$Comp
L 2021-02-17_17-37-40:MAX9926UAEE+ U1
U 1 1 602DAA0C
P 2400 2750
F 0 "U1" H 2950 3000 60  0000 C CNN
F 1 "MAX9926UAEE+" H 4550 3000 60  0000 C CNN
F 2 "MAX9926UAEE+:MAX9926UAEE&plus_" H 3900 2990 60  0001 C CNN
F 3 "https://datasheet.octopart.com/MAX9926UAEE%2B-Maxim-Integrated-datasheet-11046290.pdf" H 2400 2750 60  0001 C CNN
F 4 "Maxim Integrated" H 2400 2750 50  0001 C CNN "MFG Name"
F 5 "MAX9926UAEE+" H 2400 2750 50  0001 C CNN "MFG Part Num"
F 6 "MAX9926 Series 10 mA Differential Input Logic Output Sensor Interface -SSOP-16" H 2400 2750 50  0001 C CNN "Description"
F 7 "SSOP-16" H 2400 2750 50  0001 C CNN "Package JEDEC ID"
F 8 "Yes" H 2400 2750 50  0001 C CNN "Critical"
	1    2400 2750
	1    0    0    -1  
$EndComp
$Comp
L Device:C C1
U 1 1 602DCA31
P 5650 3100
F 0 "C1" H 5650 3200 50  0000 L CNN
F 1 "10n" H 5650 3000 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 5688 2950 50  0001 C CNN
F 3 "~" H 5650 3100 50  0001 C CNN
F 4 "50 V ±10 % Tolerance X7R Surface Mount Multilayer Ceramic" H 5650 3100 50  0001 C CNN "Description"
F 5 "0603_1608Metric" H 5650 3100 50  0001 C CNN "Package JEDEC ID"
F 6 "Kemet" H 5650 3100 50  0001 C CNN "MFG Name"
F 7 "C0603X103G5JACTU" H 5650 3100 50  0001 C CNN "MFG Part Num"
	1    5650 3100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 602DDA11
P 5500 3750
F 0 "#PWR0101" H 5500 3500 50  0001 C CNN
F 1 "GND" H 5300 3700 50  0000 C CNN
F 2 "" H 5500 3750 50  0001 C CNN
F 3 "" H 5500 3750 50  0001 C CNN
	1    5500 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	2400 2750 2300 2750
Wire Wire Line
	2300 2750 2300 2950
Wire Wire Line
	2300 2950 2400 2950
Wire Wire Line
	2400 3450 2300 3450
Wire Wire Line
	2300 3450 2300 3250
Wire Wire Line
	2300 3250 2400 3250
Connection ~ 2300 3250
Wire Wire Line
	2300 3450 2300 3750
Connection ~ 2300 3450
Wire Wire Line
	5500 3050 5400 3050
$Comp
L Device:C C2
U 1 1 602EB266
P 5850 3100
F 0 "C2" H 5850 3200 50  0000 L CNN
F 1 "100n" H 5850 3000 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 5888 2950 50  0001 C CNN
F 3 "~" H 5850 3100 50  0001 C CNN
F 4 "50 V ±10 % Tolerance X7R Surface Mount Multilayer Ceramic" H 5850 3100 50  0001 C CNN "Description"
F 5 "0603_1608Metric" H 5850 3100 50  0001 C CNN "Package JEDEC ID"
F 6 "Multicomp" H 5850 3100 50  0001 C CNN "MFG Name"
F 7 "MC0603F104Z250CT" H 5850 3100 50  0001 C CNN "MFG Part Num"
	1    5850 3100
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 602EBADD
P 6050 3100
F 0 "C3" H 6050 3200 50  0000 L CNN
F 1 "1u" H 6050 3000 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 6088 2950 50  0001 C CNN
F 3 "~" H 6050 3100 50  0001 C CNN
F 4 "50 V ±10 % Tolerance X7R Surface Mount Multilayer Ceramic" H 6050 3100 50  0001 C CNN "Description"
F 5 "0603_1608Metric" H 6050 3100 50  0001 C CNN "Package JEDEC ID"
F 6 "Kemet" H 6050 3100 50  0001 C CNN "MFG Name"
F 7 "C0603C105Z3VACTU" H 6050 3100 50  0001 C CNN "MFG Part Num"
	1    6050 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5650 2950 5850 2950
Connection ~ 5650 2950
Wire Wire Line
	5850 2950 6050 2950
Connection ~ 5850 2950
Wire Wire Line
	5650 3250 5850 3250
Wire Wire Line
	5850 3250 6050 3250
Connection ~ 5850 3250
Wire Wire Line
	5500 3050 5500 3250
Wire Wire Line
	5650 3250 5500 3250
Connection ~ 5650 3250
Connection ~ 5500 3250
Wire Wire Line
	5500 3250 5500 3750
NoConn ~ 5400 3150
NoConn ~ 2400 2850
NoConn ~ 2400 3350
Wire Wire Line
	5500 2450 5500 2950
$Comp
L power:VCC #PWR0102
U 1 1 602F5E11
P 5500 2450
F 0 "#PWR0102" H 5500 2300 50  0001 C CNN
F 1 "VCC" H 5350 2450 50  0000 C CNN
F 2 "" H 5500 2450 50  0001 C CNN
F 3 "" H 5500 2450 50  0001 C CNN
	1    5500 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2300 2950 2300 3250
Connection ~ 2300 2950
Wire Wire Line
	5500 2950 5650 2950
Wire Wire Line
	5400 3250 5500 3250
Wire Wire Line
	5400 2950 5500 2950
Connection ~ 5500 2950
Connection ~ 5500 3750
Connection ~ 5500 2450
$Comp
L Device:C C20
U 1 1 60314324
P 5650 3500
F 0 "C20" H 5500 3600 50  0000 L CNN
F 1 "1n" H 5550 3400 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 5688 3350 50  0001 C CNN
F 3 "~" H 5650 3500 50  0001 C CNN
F 4 "100V ±10 % Tolerance X7R Surface Mount Multilayer Ceramic" H 5650 3500 50  0001 C CNN "Description"
F 5 "0603_1608Metric" H 5650 3500 50  0001 C CNN "Package JEDEC ID"
F 6 "Kemet" H 5650 3500 50  0001 C CNN "MFG Name"
F 7 "C0603C102G5GACTU" H 5650 3500 50  0001 C CNN "MFG Part Num"
	1    5650 3500
	1    0    0    -1  
$EndComp
$Comp
L Device:C C10
U 1 1 603156D1
P 5650 2700
F 0 "C10" H 5500 2800 50  0000 L CNN
F 1 "1n" H 5550 2600 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 5688 2550 50  0001 C CNN
F 3 "~" H 5650 2700 50  0001 C CNN
F 4 "100V ±10 % Tolerance X7R Surface Mount Multilayer Ceramic" H 5650 2700 50  0001 C CNN "Description"
F 5 "0603_1608Metric" H 5650 2700 50  0001 C CNN "Package JEDEC ID"
F 6 "Kemet" H 5650 2700 50  0001 C CNN "MFG Name"
F 7 "C0603C102G5GACTU" H 5650 2700 50  0001 C CNN "MFG Part Num"
	1    5650 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5400 3350 5650 3350
Wire Wire Line
	5400 2850 5650 2850
Wire Wire Line
	5400 3450 5400 3650
Wire Wire Line
	5400 3650 5650 3650
Wire Wire Line
	5400 2750 5400 2550
Wire Wire Line
	5400 2550 5650 2550
$Comp
L Device:R R22
U 1 1 603198B2
P 5900 3650
F 0 "R22" V 6000 3650 50  0000 L CNN
F 1 "10k" V 6000 3450 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 5830 3650 50  0001 C CNN
F 3 "~" H 5900 3650 50  0001 C CNN
F 4 "Thick Film; +/-1% 0.2W" H 5900 3650 50  0001 C CNN "Description"
F 5 "0603_1608Metric" H 5900 3650 50  0001 C CNN "Package JEDEC ID"
F 6 "Vishay" H 5900 3650 50  0001 C CNN "MFG Name"
F 7 "CRCW060310K0FKEAHP" H 5900 3650 50  0001 C CNN "MFG Part Num"
	1    5900 3650
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R21
U 1 1 6031FC34
P 5900 3350
F 0 "R21" V 6000 3200 50  0000 L CNN
F 1 "10k" V 6000 3400 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 5830 3350 50  0001 C CNN
F 3 "~" H 5900 3350 50  0001 C CNN
F 4 "0603_1608Metric" H 5900 3350 50  0001 C CNN "Package JEDEC ID"
F 5 "Thick Film; +/-1% 0.2W" H 5900 3350 50  0001 C CNN "Description"
F 6 "Vishay" H 5900 3350 50  0001 C CNN "MFG Name"
F 7 "CRCW060310K0FKEAHP" H 5900 3350 50  0001 C CNN "MFG Part Num"
	1    5900 3350
	0    1    1    0   
$EndComp
$Comp
L Device:R R11
U 1 1 60320C9F
P 5900 2850
F 0 "R11" V 6000 2850 50  0000 L CNN
F 1 "10k" V 6000 2650 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 5830 2850 50  0001 C CNN
F 3 "~" H 5900 2850 50  0001 C CNN
F 4 "Thick Film; +/-1% 0.2W" H 5900 2850 50  0001 C CNN "Description"
F 5 "0603_1608Metric" H 5900 2850 50  0001 C CNN "Package JEDEC ID"
F 6 "Vishay" H 5900 2850 50  0001 C CNN "MFG Name"
F 7 "CRCW060310K0FKEAHP" H 5900 2850 50  0001 C CNN "MFG Part Num"
	1    5900 2850
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R12
U 1 1 6032192F
P 5900 2550
F 0 "R12" V 5800 2550 50  0000 L CNN
F 1 "10k" V 5800 2350 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 5830 2550 50  0001 C CNN
F 3 "~" H 5900 2550 50  0001 C CNN
F 4 "Thick Film; +/-1% 0.2W" H 5900 2550 50  0001 C CNN "Description"
F 5 "0603_1608Metric" H 5900 2550 50  0001 C CNN "Package JEDEC ID"
F 6 "Vishay" H 5900 2550 50  0001 C CNN "MFG Name"
F 7 "CRCW060310K0FKEAHP" H 5900 2550 50  0001 C CNN "MFG Part Num"
	1    5900 2550
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5650 3650 5750 3650
Connection ~ 5650 3650
Wire Wire Line
	5650 3350 5750 3350
Connection ~ 5650 3350
Wire Wire Line
	5650 2550 5750 2550
Connection ~ 5650 2550
Wire Wire Line
	5650 2850 5750 2850
Connection ~ 5650 2850
$Comp
L Device:R R13
U 1 1 6032AC19
P 6150 2700
F 0 "R13" H 6200 2650 50  0000 L CNN
F 1 "4.7k" H 6200 2750 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 6080 2700 50  0001 C CNN
F 3 "~" H 6150 2700 50  0001 C CNN
F 4 "No. Option for user install" H 6150 2700 50  0001 C CNN "Critical"
F 5 "3W, 5%" H 6150 2700 50  0001 C CNN "Description"
	1    6150 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	6050 2550 6150 2550
Wire Wire Line
	6050 2850 6150 2850
Connection ~ 6150 2550
Wire Wire Line
	6150 2850 6350 2850
Connection ~ 6150 2850
$Comp
L Connector_Generic:Conn_02x04_Counter_Clockwise J1
U 1 1 6033C0E4
P 6700 3050
F 0 "J1" H 6750 3250 50  0000 C CNN
F 1 "8-pin header" H 6750 2700 50  0000 C CNN
F 2 "Package_DIP:DIP-8_W7.62mm" H 6700 3050 50  0001 C CNN
F 3 "~" H 6700 3050 50  0001 C CNN
	1    6700 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 3350 6350 3250
Wire Wire Line
	6350 2850 6350 3050
Wire Wire Line
	6400 2950 6400 2550
Wire Wire Line
	6150 2550 6400 2550
$Comp
L Device:R R20
U 1 1 60351FB1
P 7100 2600
F 0 "R20" H 7150 2650 50  0000 L CNN
F 1 "1k" H 7150 2550 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 7030 2600 50  0001 C CNN
F 3 "~" H 7100 2600 50  0001 C CNN
F 4 "Thick Film; +/-1% 0.2W" H 7100 2600 50  0001 C CNN "Description"
F 5 "0603_1608Metric" H 7100 2600 50  0001 C CNN "Package JEDEC ID"
F 6 "Vishay" H 7100 2600 50  0001 C CNN "MFG Name"
F 7 "CRCW06031K00FKEAHP" H 7100 2600 50  0001 C CNN "MFG Part Num"
	1    7100 2600
	1    0    0    -1  
$EndComp
$Comp
L Device:R R10
U 1 1 60351FB7
P 7000 2600
F 0 "R10" H 6800 2650 50  0000 L CNN
F 1 "1k" H 6850 2550 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 6930 2600 50  0001 C CNN
F 3 "~" H 7000 2600 50  0001 C CNN
F 4 "Thick Film; +/-1% 0.2W" H 7000 2600 50  0001 C CNN "Description"
F 5 "0603_1608Metric" H 7000 2600 50  0001 C CNN "Package JEDEC ID"
F 6 "Vishay" H 7000 2600 50  0001 C CNN "MFG Name"
F 7 "CRCW06031K00FKEAHP" H 7000 2600 50  0001 C CNN "MFG Part Num"
	1    7000 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	7000 2950 7000 2750
Wire Wire Line
	2400 3150 2350 3150
Wire Wire Line
	2350 3150 2350 3900
Wire Wire Line
	2400 3050 2350 3050
Wire Wire Line
	2350 3050 2350 2300
Wire Wire Line
	7000 3250 7000 3350
Wire Wire Line
	7000 3350 6550 3350
Wire Wire Line
	6550 3350 6550 2450
Wire Wire Line
	6350 3050 6500 3050
Wire Wire Line
	6400 2950 6500 2950
Wire Wire Line
	5500 2450 6550 2450
Wire Wire Line
	2300 3750 5500 3750
Wire Wire Line
	2350 3900 7100 3900
Wire Wire Line
	6050 3350 6150 3350
Wire Wire Line
	6050 3650 6150 3650
Connection ~ 6150 3350
Wire Wire Line
	6150 3350 6350 3350
Connection ~ 6150 3650
Wire Wire Line
	6150 3650 6400 3650
Wire Wire Line
	6400 3150 6500 3150
Wire Wire Line
	6400 3150 6400 3650
Wire Wire Line
	6350 3250 6500 3250
$Comp
L Mechanical:MountingHole H1
U 1 1 6037799D
P 7100 600
F 0 "H1" H 7200 646 50  0000 L CNN
F 1 "MountingHole" H 7200 555 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.2mm_M2" H 7100 600 50  0001 C CNN
F 3 "~" H 7100 600 50  0001 C CNN
	1    7100 600 
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H2
U 1 1 603785C5
P 7100 800
F 0 "H2" H 7200 846 50  0000 L CNN
F 1 "MountingHole" H 7200 755 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.2mm_M2" H 7100 800 50  0001 C CNN
F 3 "~" H 7100 800 50  0001 C CNN
	1    7100 800 
	1    0    0    -1  
$EndComp
Text Notes 5550 2400 0    50   ~ 0
VCC = 5V
Wire Notes Line
	6450 1000 6450 500 
Wire Notes Line
	6450 1000 7750 1000
Wire Wire Line
	7150 3050 7100 3050
Wire Wire Line
	7000 2950 7150 2950
Connection ~ 7000 2950
Wire Wire Line
	7000 2450 7100 2450
Wire Wire Line
	7100 2750 7100 3050
Connection ~ 7100 3050
Wire Wire Line
	7100 3050 7000 3050
Wire Wire Line
	6550 2450 7000 2450
Connection ~ 6550 2450
Connection ~ 7000 2450
Wire Wire Line
	7150 2950 7150 2300
Wire Wire Line
	2350 2300 7150 2300
$Comp
L Device:CP C4
U 1 1 6037FBE4
P 6550 1650
F 0 "C4" H 6550 1750 50  0000 L CNN
F 1 "10u" H 6550 1550 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 6588 1500 50  0001 C CNN
F 3 "~" H 6550 1650 50  0001 C CNN
F 4 "Multilayer Ceramic Capacitors (High dielectric type), 25V, 10 uF, ± 10 %, X5R," H 6550 1650 50  0001 C CNN "Description"
F 5 "0805_2012Metric" H 6550 1650 50  0001 C CNN "Package JEDEC ID"
F 6 "Kemet" H 6550 1650 50  0001 C CNN "MFG Name"
F 7 "C0805C106K9PACTU" H 6550 1650 50  0001 C CNN "MFG Part Num"
	1    6550 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	7000 3150 7050 3150
Wire Wire Line
	7050 3150 7050 3750
Wire Wire Line
	7050 3750 5500 3750
Wire Wire Line
	7100 3050 7100 3900
Text Label 7150 2950 0    50   ~ 0
COUT1
Text Label 7150 3050 0    50   ~ 0
COUT2
Text Label 7150 3150 0    50   ~ 0
GND
Text Label 7150 3250 0    50   ~ 0
VCC
Text Label 7200 1500 0    50   ~ 0
VCC
Text Label 6150 1800 0    50   ~ 0
COUT1
Text Label 6150 1700 0    50   ~ 0
COUT2
Text Label 6150 1600 0    50   ~ 0
GND
Text Label 6150 1500 0    50   ~ 0
Vin
Wire Notes Line
	7450 1250 5750 1250
Text Notes 5800 2050 0    60   ~ 0
Optional, but SMT capacitor should\n always be fitted for convenience
Wire Wire Line
	6150 1600 6400 1600
Wire Wire Line
	6400 1600 6400 1800
Wire Wire Line
	6400 1800 6550 1800
Wire Wire Line
	6150 1500 6400 1500
Connection ~ 6550 1800
Wire Wire Line
	6550 1800 6900 1800
Wire Notes Line
	5750 2100 7450 2100
$Comp
L Device:R R23
U 1 1 6032659E
P 6150 3500
F 0 "R23" H 5950 3550 50  0000 L CNN
F 1 "4.7k" H 5950 3450 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 6080 3500 50  0001 C CNN
F 3 "~" H 6150 3500 50  0001 C CNN
F 4 "No. Option for user install" H 6150 3500 50  0001 C CNN "Critical"
F 5 "" H 6150 3500 50  0001 C CNN "Characteristics"
F 6 "3W, 5%" H 6150 3500 50  0001 C CNN "Description"
	1    6150 3500
	-1   0    0    1   
$EndComp
$Comp
L Regulator_Linear:AMS1117CD-5.0 U2
U 1 1 60325A8A
P 6900 1500
F 0 "U2" H 7000 1250 50  0000 C CNN
F 1 "AMS1117CD-5.0" H 6900 1651 50  0000 C CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline_Wide" H 6900 1700 50  0001 C CNN
F 3 "http://www.advanced-monolithic.com/pdf/ds1117.pdf" H 7000 1250 50  0001 C CNN
F 4 "Advanced Monolithic Systems" H 6900 1500 50  0001 C CNN "MFG Name"
F 5 "AMS1117-5.0" H 6900 1500 50  0001 C CNN "MFG Part Num"
F 6 "1A Low Dropout Voltage Regulator" H 6900 1500 50  0001 C CNN "Description"
F 7 "TO92 inline wide" H 6900 1500 50  0001 C CNN "Package JEDEC ID"
F 8 "No. Option for user install" H 6900 1500 50  0001 C CNN "Critical"
	1    6900 1500
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J2
U 1 1 603276E3
P 5950 1600
F 0 "J2" H 5900 1800 50  0000 L CNN
F 1 "Edge" V 6050 1500 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Horizontal" H 5950 1600 50  0001 C CNN
F 3 "~" H 5950 1600 50  0001 C CNN
F 4 "No. Option for user install" H 5950 1600 50  0001 C CNN "Critical"
	1    5950 1600
	-1   0    0    -1  
$EndComp
$Comp
L power:+12V #PWR0103
U 1 1 60369B79
P 6400 1500
F 0 "#PWR0103" H 6400 1350 50  0001 C CNN
F 1 "+12V" H 6415 1673 50  0000 C CNN
F 2 "" H 6400 1500 50  0001 C CNN
F 3 "" H 6400 1500 50  0001 C CNN
	1    6400 1500
	1    0    0    -1  
$EndComp
Connection ~ 6400 1500
Wire Wire Line
	6400 1500 6550 1500
Wire Notes Line
	5750 1250 5750 2100
Wire Notes Line
	7450 1250 7450 2100
Wire Wire Line
	6550 1500 6600 1500
Connection ~ 6550 1500
Wire Wire Line
	7050 3150 7150 3150
Connection ~ 7050 3150
Wire Wire Line
	7150 3250 7000 3250
Connection ~ 7000 3250
$EndSCHEMATC
