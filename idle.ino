/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
These functions cover the PWM and stepper idle control
*/

/*
Idle Control
Currently limited to on/off control and open loop PWM and stepper drive
*/
void initialiseIdle()
{
  //Initialising comprises of setting the 2D tables with the relevant values from the config pages
  switch(configPage4.iacAlgorithm)
  {
    case 0:
      //Case 0 is no idle control ('None')
      break;
      
    case 1:
      //Case 1 is on/off idle control
      if (currentStatus.coolant < configPage4.iacFastTemp)
      {
        digitalWrite(pinIdle1, HIGH);
      }
      break;
      
    case 2:
      //Case 2 is PWM open loop
      iacPWMTable.xSize = 10;
      iacPWMTable.valueSize = SIZE_BYTE;
      iacPWMTable.values = configPage4.iacOLPWMVal;
      iacPWMTable.axisX = configPage4.iacBins;
      
      iacCrankDutyTable.xSize = 4;
      iacCrankDutyTable.values = configPage4.iacCrankDuty;
      iacCrankDutyTable.axisX = configPage4.iacCrankBins;
      
      idle_pin_port = portOutputRegister(digitalPinToPort(pinIdle1));
      idle_pin_mask = digitalPinToBitMask(pinIdle1);
      idle2_pin_port = portOutputRegister(digitalPinToPort(pinIdle2));
      idle2_pin_mask = digitalPinToBitMask(pinIdle2);
      idle_pwm_max_count = 1000000L / (16 * configPage3.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      TIMSK4 |= (1 << OCIE4C); //Turn on the C compare unit (ie turn on the interrupt)
      break;
    
    case 3:
      //Case 3 is PWM closed loop
      iacClosedLoopTable.xSize = 10;
      iacClosedLoopTable.values = configPage4.iacCLValues;
      iacClosedLoopTable.axisX = configPage4.iacBins;
      
      iacCrankDutyTable.xSize = 4;
      iacCrankDutyTable.values = configPage4.iacCrankDuty;
      iacCrankDutyTable.axisX = configPage4.iacCrankBins;
      break;
      
    case 4:
      //Case 2 is Stepper open loop
      iacStepTable.xSize = 10;
      iacStepTable.valueSize = SIZE_BYTE;
      iacStepTable.values = configPage4.iacOLStepVal;
      iacStepTable.axisX = configPage4.iacBins;
      
      iacCrankStepsTable.xSize = 4;
      iacCrankStepsTable.values = configPage4.iacCrankSteps;
      iacCrankStepsTable.axisX = configPage4.iacCrankBins;
      iacStepTime = configPage4.iacStepTime * 1000;
      
      homeStepper(); //Returns the stepper to the 'home' position
      idleStepper.stepperStatus = SOFF;
      break;
      
    case 5:
      //Case 5 is Stepper closed loop
      iacClosedLoopTable.xSize = 10;
      iacClosedLoopTable.values = configPage4.iacCLValues;
      iacClosedLoopTable.axisX = configPage4.iacBins;
      
      iacCrankStepsTable.xSize = 4;
      iacCrankStepsTable.values = configPage4.iacCrankSteps;
      iacCrankStepsTable.axisX = configPage4.iacCrankBins;
      iacStepTime = configPage4.iacStepTime * 1000;
      
      homeStepper(); //Returns the stepper to the 'home' position
      idleStepper.stepperStatus = SOFF;
      break;
  }
  
}

void idleControl()
{
  switch(configPage4.iacAlgorithm)
  {
    case 0:       //Case 0 is no idle control ('None')
      break;
      
    case 1:      //Case 1 is on/off idle control
      if ( (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) < configPage4.iacFastTemp) //All temps are offset by 40 degrees
      {
        digitalWrite(pinIdle1, HIGH);
        idleOn = true;
      }
      else if (idleOn) { digitalWrite(pinIdle1, LOW); idleOn = false; }
      break;
      
    case 2:      //Case 2 is PWM open loop
      //Check for cranking pulsewidth
      if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
      {
        //Currently cranking. Use the cranking table
        currentStatus.idleDuty = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        idle_pwm_target_value = percentage(currentStatus.idleDuty, idle_pwm_max_count);
        idleOn = true;
      }
      else if( currentStatus.coolant < (iacPWMTable.values[IDLE_TABLE_SIZE-1] + CALIBRATION_TEMPERATURE_OFFSET))
      {
        //Standard running
        currentStatus.idleDuty = table2D_getValue(&iacPWMTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        idle_pwm_target_value = percentage(currentStatus.idleDuty, idle_pwm_max_count);
        idleOn = true;
      }
      else if (idleOn) { digitalWrite(pinIdle1, LOW); idleOn = false; }
      break;
      
    case 3:    //Case 3 is PWM closed loop (Not currently implemented)
      break;
      
    case 4:    //Case 4 is open loop stepper control
      //First thing to check is whether there is currently a step going on and if so, whether it needs to be turned off
      if(idleStepper.stepperStatus == STEPPING || idleStepper.stepperStatus == COOLING)
      {
        if(micros() > (idleStepper.stepStartTime + iacStepTime) )
        {
          if(idleStepper.stepperStatus == STEPPING)
          {
            //Means we're currently in a step, but it needs to be turned off
            digitalWrite(pinStepperStep, LOW); //Turn off the step
            idleStepper.stepStartTime = micros();
            idleStepper.stepperStatus = COOLING; //'Cooling' is the time the stepper needs to sit in LOW state before the next step can be made
            return;
          }
          else
          {
            //Means we're in COOLING status. We need to remain in this state for the step time before the next step can be taken
            idleStepper.stepperStatus = SOFF;
          }
        }
        else
        {
          //Means we're in a step, but it doesn't need to turn off yet. No further action at this time
          return;
        }
      }
      
      //Check for cranking pulsewidth
      if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
      {
        //Currently cranking. Use the cranking table
        idleStepper.targetIdleStep = table2D_getValue(&iacCrankStepsTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
        if ( idleStepper.targetIdleStep > (idleStepper.curIdleStep - configPage4.iacStepHyster) && idleStepper.targetIdleStep < (idleStepper.curIdleStep + configPage4.iacStepHyster) ) { return; } //Hysteris check
        else if(idleStepper.targetIdleStep < idleStepper.curIdleStep) { digitalWrite(pinStepperDir, STEPPER_BACKWARD); idleStepper.curIdleStep--; }//Sets stepper direction to backwards
        else if (idleStepper.targetIdleStep > idleStepper.curIdleStep) { digitalWrite(pinStepperDir, STEPPER_FORWARD); idleStepper.curIdleStep++; }//Sets stepper direction to forwards
        
        digitalWrite(pinStepperStep, HIGH);
        idleStepper.stepStartTime = micros();
        idleStepper.stepperStatus = STEPPING;
        idleOn = true; 
      }
      else if( (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) < iacStepTable.axisX[IDLE_TABLE_SIZE-1])
      {
        //Standard running
        idleStepper.targetIdleStep = table2D_getValue(&iacStepTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
        if ( idleStepper.targetIdleStep > (idleStepper.curIdleStep - configPage4.iacStepHyster) && idleStepper.targetIdleStep < (idleStepper.curIdleStep + configPage4.iacStepHyster) ) { return; } //Hysteris check
        else if(idleStepper.targetIdleStep < idleStepper.curIdleStep) { digitalWrite(pinStepperDir, STEPPER_BACKWARD); idleStepper.curIdleStep--; }//Sets stepper direction to backwards
        else if (idleStepper.targetIdleStep > idleStepper.curIdleStep) { digitalWrite(pinStepperDir, STEPPER_FORWARD); idleStepper.curIdleStep++; }//Sets stepper direction to forwards
        
        digitalWrite(pinStepperStep, HIGH);
        idleStepper.stepStartTime = micros();
        idleStepper.stepperStatus = STEPPING;
        idleOn = true; 
      }
      
      break;
  }
}

/*
A simple function to home the stepper motor (If in use)
*/
void homeStepper()
{
   //Need to 'home' the stepper on startup
   digitalWrite(pinStepperDir, STEPPER_BACKWARD); //Sets stepper direction to backwards
   for(int x=0; x < (configPage4.iacStepHome * 3); x++) //Step counts are divided by 3 in TS. Multiply back out here
   {
     digitalWrite(pinStepperStep, HIGH);
     delayMicroseconds(iacStepTime);
     digitalWrite(pinStepperStep, LOW);
     delayMicroseconds(iacStepTime);
   }
   digitalWrite(pinStepperDir, STEPPER_FORWARD);
   idleStepper.curIdleStep = 0; 
   idleStepper.targetIdleStep = 0;
   idleStepper.stepperStatus = SOFF;
}

//The interrupt to turn off the idle pwm
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
ISR(TIMER4_COMPC_vect)
{
  if (idle_pwm_state)
  {
    *idle_pin_port &= ~(idle_pin_mask);  // Switch pin to low (1 pin mode)
    if(configPage4.iacChannels) { *idle2_pin_port |= (idle2_pin_mask); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
    OCR4C = TCNT4 + (idle_pwm_max_count - idle_pwm_cur_value);
    idle_pwm_state = false;
  }
  else
  {
    *idle_pin_port |= (idle_pin_mask);  // Switch pin high
    if(configPage4.iacChannels) { *idle2_pin_port &= ~(idle2_pin_mask); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
    OCR4C = TCNT4 + idle_pwm_target_value;
    idle_pwm_cur_value = idle_pwm_target_value;
    idle_pwm_state = true;
  }
    
}
#elif defined(PROCESSOR_TEENSY_3_1) || defined(PROCESSOR_TEENSY_3_2)
void idle_off() { }
#endif
