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
if(configPage4.fanInv == 1) {fanHIGH = LOW, fanLOW = HIGH; }
else {fanHIGH = HIGH, fanLOW = LOW;}
digitalWrite(pinFan, fanLOW);         //Initiallise program with the fan in the off state
}

void fanControl()
{
   if (currentStatus.coolant >= (configPage4.fanSP - CALIBRATION_TEMPERATURE_OFFSET)) { digitalWrite(pinFan,fanHIGH); }
   else if (currentStatus.coolant <= (configPage4.fanSP - configPage4.fanHyster)) { digitalWrite(pinFan, fanLOW); }
}
