/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
//Old PID method. Retained incase the new one has issues
//integerPID boostPID(&MAPx100, &boost_pwm_target_value, &boostTargetx100, configPage3.boostKP, configPage3.boostKI, configPage3.boostKD, DIRECT);
integerPID_ideal boostPID(&currentStatus.MAP, &currentStatus.boostDuty , &currentStatus.boostTarget, &configPage11.boostSens, &configPage11.boostIntv, configPage3.boostKP, configPage3.boostKI, configPage3.boostKD, DIRECT); //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

/*
Fan control
*/
void initialiseFan()
{
  if( configPage3.fanInv == 1 ) { fanHIGH = LOW; fanLOW = HIGH; }
  else { fanHIGH = HIGH; fanLOW = LOW; }
  digitalWrite(pinFan, fanLOW);         //Initiallise program with the fan in the off state
  currentStatus.fanOn = false;
}

void fanControl()
{
  if( configPage3.fanEnable == 1 )
  {
    int onTemp = (int)configPage3.fanSP - CALIBRATION_TEMPERATURE_OFFSET;
    int offTemp = onTemp - configPage3.fanHyster;

    if ( (!currentStatus.fanOn) && (currentStatus.coolant >= onTemp) ) { digitalWrite(pinFan,fanHIGH); currentStatus.fanOn = true; }
    if ( (currentStatus.fanOn) && (currentStatus.coolant <= offTemp) ) { digitalWrite(pinFan, fanLOW); currentStatus.fanOn = false; }
  }
}

void initialiseAuxPWM()
{
  #if defined(CORE_AVR)
    TCCR1B = 0x00;          //Disbale Timer1 while we set it up
    TCNT1  = 0;             //Reset Timer Count
    TIFR1  = 0x00;          //Timer1 INT Flag Reg: Clear Timer Overflow Flag
    TCCR1A = 0x00;          //Timer1 Control Reg A: Wave Gen Mode normal (Simply counts up from 0 to 65535 (16-bit int)
    TCCR1B = (1 << CS12);   //Timer1 Control Reg B: Timer Prescaler set to 256. 1 tick = 16uS. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
  #elif defined(CORE_TEENSY)
    //REALLY NEED TO DO THIS!
  #elif defined(CORE_STM32)
    Timer1.attachInterrupt(2, boostInterrupt);
    Timer1.attachInterrupt(3, vvtInterrupt);
    Timer1.resume();
  #endif

  boost_pin_port = portOutputRegister(digitalPinToPort(pinBoost));
  boost_pin_mask = digitalPinToBitMask(pinBoost);
  vvt_pin_port = portOutputRegister(digitalPinToPort(pinVVT_1));
  vvt_pin_mask = digitalPinToBitMask(pinVVT_1);

  #if defined(CORE_STM32) //2uS resolution Min 8Hz, Max 5KHz
    boost_pwm_max_count = 1000000L / (configPage3.boostFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. The x2 is there because the frequency is stored at half value (in a byte) to allow freqneucies up to 511Hz
    vvt_pwm_max_count = 1000000L / (configPage3.vvtFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle
  #else
    boost_pwm_max_count = 1000000L / (16 * configPage3.boostFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. The x2 is there because the frequency is stored at half value (in a byte) to allow freqneucies up to 511Hz
    vvt_pwm_max_count = 1000000L / (16 * configPage3.vvtFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle
  #endif
  //TIMSK1 |= (1 << OCIE1A); <---- Not required as compare A is turned on when needed by boost control
  ENABLE_VVT_TIMER(); //Turn on the B compare unit (ie turn on the interrupt)

  boostPID.SetOutputLimits(configPage1.boostMinDuty, configPage1.boostMaxDuty);
  if(configPage3.boostMode == BOOST_MODE_SIMPLE) { boostPID.SetTunings(100, 100, 100); }
  else { boostPID.SetTunings(configPage3.boostKP, configPage3.boostKI, configPage3.boostKD); }

  currentStatus.boostDuty = 0;
  boostCounter = 0;
}

#define BOOST_HYSTER  40
void boostControl()
{
  if( configPage3.boostEnabled==1 )
  {
    if( (boostCounter & 7) == 1) { currentStatus.boostTarget = get3DTableValue(&boostTable, currentStatus.TPS, currentStatus.RPM) * 2; } //Boost target table is in kpa and divided by 2
    if(currentStatus.MAP >= (currentStatus.boostTarget - BOOST_HYSTER) )
    {
      //If flex fuel is enabled, there can be an adder to the boost target based on ethanol content
      if( configPage1.flexEnabled == 1 )
      {
        int16_t boostAdder = (((int16_t)configPage1.flexBoostHigh - (int16_t)configPage1.flexBoostLow) * currentStatus.ethanolPct) / 100;
        boostAdder = boostAdder + configPage1.flexBoostLow; //Required in case flexBoostLow is less than 0
        currentStatus.boostTarget += boostAdder;
      }

      if(currentStatus.boostTarget > 0)
      {
        //This only needs to be run very infrequently, once every 16 calls to boostControl(). This is approx. once per second
        if( (boostCounter & 15) == 1)
        {
          boostPID.SetOutputLimits(configPage1.boostMinDuty, configPage1.boostMaxDuty);

          if(configPage3.boostMode == BOOST_MODE_SIMPLE) { boostPID.SetTunings(100, 100, 100); }
          else { boostPID.SetTunings(configPage3.boostKP, configPage3.boostKI, configPage3.boostKD); }
        }

        bool PIDcomputed = boostPID.Compute(); //Compute() returns false if the required interval has not yet passed.
        if(currentStatus.boostDuty == 0) { DISABLE_BOOST_TIMER(); BOOST_PIN_LOW(); } //If boost duty is 0, shut everything down
        else
        {
          if(PIDcomputed == true)
          {
            boost_pwm_target_value = ((unsigned long)(currentStatus.boostDuty) * boost_pwm_max_count) / 10000; //Convert boost duty (Which is a % multipled by 100) to a pwm count
            ENABLE_BOOST_TIMER(); //Turn on the compare unit (ie turn on the interrupt) if boost duty >0
          }
        }

      }
      else
      {
        //If boost target is 0, turn everything off
        boostDisable();
      }
    }
    else
    {
      //Boost control does nothing if kPa below the hyster point
      boostDisable();
    }
  }
  else { DISABLE_BOOST_TIMER(); } // Disable timer channel

  boostCounter++;
}

void vvtControl()
{
  if( configPage3.vvtEnabled == 1 )
  {
    byte vvtDuty = get3DTableValue(&vvtTable, currentStatus.TPS, currentStatus.RPM);

    //VVT table can be used for controlling on/off switching. If this is turned on, then disregard any interpolation or non-binary values
    if( (configPage3.VVTasOnOff == true) && (vvtDuty < 100) ) { vvtDuty = 0; }

    if(vvtDuty == 0)
    {
      //Make sure solenoid is off (0% duty)
      VVT_PIN_LOW();
      DISABLE_VVT_TIMER();
    }
    else if (vvtDuty >= 100)
    {
      //Make sure solenoid is on (100% duty)
      VVT_PIN_HIGH();
      DISABLE_VVT_TIMER();
    }
    else
    {
      vvt_pwm_target_value = percentage(vvtDuty, vvt_pwm_max_count);
      ENABLE_VVT_TIMER();
    }
  }
  else { DISABLE_VVT_TIMER(); } // Disable timer channel
}

void boostDisable()
{
  boostPID.Initialize(); //This resets the ITerm value to prevent rubber banding
  currentStatus.boostDuty = 0;
  DISABLE_BOOST_TIMER(); //Turn off timer
  BOOST_PIN_LOW(); //Make sure solenoid is off (0% duty)
}

//The interrupt to control the Boost PWM
#if defined(CORE_AVR)
  ISR(TIMER1_COMPA_vect)
#elif defined (CORE_TEENSY) || defined(CORE_STM32)
  static inline void boostInterrupt() //Most ARM chips can simply call a function
#endif
{
  if (boost_pwm_state)
  {
    BOOST_PIN_LOW();  // Switch pin to low
    BOOST_TIMER_COMPARE = BOOST_TIMER_COUNTER + (boost_pwm_max_count - boost_pwm_cur_value);
    boost_pwm_state = false;
  }
  else
  {
    BOOST_PIN_HIGH();  // Switch pin high
    BOOST_TIMER_COMPARE = BOOST_TIMER_COUNTER + boost_pwm_target_value;
    boost_pwm_cur_value = boost_pwm_target_value;
    boost_pwm_state = true;
  }
}

//The interrupt to control the VVT PWM
#if defined(CORE_AVR)
  ISR(TIMER1_COMPB_vect)
#elif defined (CORE_TEENSY) || defined(CORE_STM32)
  static inline void vvtInterrupt() //Most ARM chips can simply call a function
#endif
{
  if (vvt_pwm_state)
  {
    VVT_PIN_LOW();  // Switch pin to low
    VVT_TIMER_COMPARE = VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt_pwm_cur_value);
    vvt_pwm_state = false;
  }
  else
  {
    VVT_PIN_HIGH();  // Switch pin high
    VVT_TIMER_COMPARE = VVT_TIMER_COUNTER + vvt_pwm_target_value;
    vvt_pwm_cur_value = vvt_pwm_target_value;
    vvt_pwm_state = true;
  }
}
