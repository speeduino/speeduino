/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
Fan control
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

void initialiseAuxPWM()
{
  TCCR1B = 0x00;          //Disbale Timer1 while we set it up
  TCNT1  = 0;             //Reset Timer Count
  TIFR1  = 0x00;          //Timer3 INT Flag Reg: Clear Timer Overflow Flag
  TCCR1A = 0x00;          //Timer3 Control Reg A: Wave Gen Mode normal
  TCCR1B = (1 << CS12);   //Timer3 Control Reg B: Timer Prescaler set to 256. 1 tick = 16uS. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg 
  
  boost_pin_port = portOutputRegister(digitalPinToPort(pinBoost));
  boost_pin_mask = digitalPinToBitMask(pinBoost);
  boost_pin_port = portOutputRegister(digitalPinToPort(pinBoost));
  boost_pin_mask = digitalPinToBitMask(pinBoost);
  
  boost_pwm_max_count = 1000000L / (16 * configPage3.boostFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. The x2 is there because the frequency is stored at half value (in a byte)
  vvt_pwm_max_count = 1000000L / (16 * configPage3.vvtFreq * 2);; //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle
  TIMSK1 |= (1 << OCIE1A); //Turn on the A compare unit (ie turn on the interrupt)
  TIMSK1 |= (1 << OCIE1B); //Turn on the B compare unit (ie turn on the interrupt)
}

void boostControl()
{
  
}


