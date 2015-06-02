/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
These functions control the auxillary outputs
*/

void initialiseFan()
{
if(configPage4.FanInv == 1) {FanHIGH = LOW, FanLOW = HIGH; }
else {FanHIGH = HIGH, FanLOW = LOW;}
digitalWrite(pinFan, FanLOW);         //Initiallise program with the fan in the off state
}

void FanControl()
{
   if (currentStatus.coolant >= (configPage4.FanSP - CALIBRATION_TEMPERATURE_OFFSET)) {digitalWrite(pinFan,FanHIGH);}
    else if (currentStatus.coolant <= (configPage4.FanSP - configPage4.FanHyster)){ digitalWrite(pinFan, FanLOW);}
}
