#pragma once

void initDirectIgn(uint8_t (&pins)[IGN_CHANNELS]);

void coil1Low_DIRECT(void);
void coil1High_DIRECT(void);
void coil2Low_DIRECT(void);
void coil2High_DIRECT(void);
void coil3Low_DIRECT(void);
void coil3High_DIRECT(void);
void coil4Low_DIRECT(void);
void coil4High_DIRECT(void);
void coil5Low_DIRECT(void);
void coil5High_DIRECT(void);
void coil6Low_DIRECT(void);
void coil6High_DIRECT(void);
void coil7Low_DIRECT(void);
void coil7High_DIRECT(void);
void coil8Low_DIRECT(void);
void coil8High_DIRECT(void);

//Set the value of the coil pins to the coilHIGH or coilLOW state
#define coil1Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil1Low_DIRECT() : coil1High_DIRECT())
#define coil1StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil1High_DIRECT() : coil1Low_DIRECT())
#define coil2Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil2Low_DIRECT() : coil2High_DIRECT())
#define coil2StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil2High_DIRECT() : coil2Low_DIRECT())
#define coil3Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil3Low_DIRECT() : coil3High_DIRECT())
#define coil3StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil3High_DIRECT() : coil3Low_DIRECT())
#define coil4Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil4Low_DIRECT() : coil4High_DIRECT())
#define coil4StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil4High_DIRECT() : coil4Low_DIRECT())
#define coil5Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil5Low_DIRECT() : coil5High_DIRECT())
#define coil5StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil5High_DIRECT() : coil5Low_DIRECT())
#define coil6Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil6Low_DIRECT() : coil6High_DIRECT())
#define coil6StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil6High_DIRECT() : coil6Low_DIRECT())
#define coil7Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil7Low_DIRECT() : coil7High_DIRECT())
#define coil7StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil7High_DIRECT() : coil7Low_DIRECT())
#define coil8Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil8Low_DIRECT() : coil8High_DIRECT())
#define coil8StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil8High_DIRECT() : coil8Low_DIRECT())
