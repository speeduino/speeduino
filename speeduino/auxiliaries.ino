/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
#include "globals.h"
#include "auxiliaries.h"
#include "maths.h"
#include "src/PID_v1/PID_v1.h"

//Old PID method. Retained incase the new one has issues
//integerPID boostPID(&MAPx100, &boost_pwm_target_value, &boostTargetx100, configPage6.boostKP, configPage6.boostKI, configPage6.boostKD, DIRECT);
integerPID_ideal boostPID(&currentStatus.MAP, &currentStatus.boostDuty , &currentStatus.boostTarget, &configPage10.boostSens, &configPage10.boostIntv, configPage6.boostKP, configPage6.boostKI, configPage6.boostKD, DIRECT); //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

/*
Fan control
*/
void initialiseFan()
{
  if( configPage6.fanInv == 1 ) { fanHIGH = LOW; fanLOW = HIGH; }
  else { fanHIGH = HIGH; fanLOW = LOW; }
  digitalWrite(pinFan, fanLOW);         //Initiallise program with the fan in the off state
  currentStatus.fanOn = false;

  fan_pin_port = portOutputRegister(digitalPinToPort(pinFan));
  fan_pin_mask = digitalPinToBitMask(pinFan);
}

void fanControl()
{
  if( configPage6.fanEnable == 1 )
  {
    int onTemp = (int)configPage6.fanSP - CALIBRATION_TEMPERATURE_OFFSET;
    int offTemp = onTemp - configPage6.fanHyster;

    if ( currentStatus.coolant >= onTemp )
    {
      //Fan needs to be turned on. Checked for normal or inverted fan signal
      if( configPage6.fanInv == 0 ) { FAN_PIN_HIGH(); }
      else { FAN_PIN_LOW(); }
      currentStatus.fanOn = true;
    }
    else if ( currentStatus.coolant <= offTemp )
    {
      //Fan needs to be turned off. Checked for normal or inverted fan signal
      if( configPage6.fanInv == 0 ) { FAN_PIN_LOW(); } 
      else { FAN_PIN_HIGH(); }
      currentStatus.fanOn = false;
    }
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
    //FlexTimer 1 is used for boost and VVT. There are 8 channels on this module
    FTM1_MODE |= FTM_MODE_WPDIS; // Write Protection Disable
    FTM1_MODE |= FTM_MODE_FTMEN; //Flex Timer module enable
    FTM1_MODE |= FTM_MODE_INIT;
    FTM1_SC |= FTM_SC_CLKS(0b1); // Set internal clocked
    FTM1_SC |= FTM_SC_PS(0b111); //Set prescaler to 128 (2.1333uS tick time)

    //Enable each compare channel individually
    FTM1_C0SC &= ~FTM_CSC_MSB; //According to Pg 965 of the K64 datasheet, this should not be needed as MSB is reset to 0 upon reset, but the channel interrupt fails to fire without it
    FTM1_C0SC |= FTM_CSC_MSA; //Enable Compare mode
    FTM1_C0SC |= FTM_CSC_CHIE; //Enable channel compare interrupt
    FTM1_C1SC &= ~FTM_CSC_MSB; //According to Pg 965 of the K64 datasheet, this should not be needed as MSB is reset to 0 upon reset, but the channel interrupt fails to fire without it
    FTM1_C1SC |= FTM_CSC_MSA; //Enable Compare mode
    FTM1_C1SC |= FTM_CSC_CHIE; //Enable channel compare interrupt

  #endif

  boost_pin_port = portOutputRegister(digitalPinToPort(pinBoost));
  boost_pin_mask = digitalPinToBitMask(pinBoost);
  vvt_pin_port = portOutputRegister(digitalPinToPort(pinVVT_1));
  vvt_pin_mask = digitalPinToBitMask(pinVVT_1);
  n2o_stage1_pin_port = portOutputRegister(digitalPinToPort(configPage10.n2o_stage1_pin));
  n2o_stage1_pin_mask = digitalPinToBitMask(configPage10.n2o_stage1_pin);
  n2o_stage2_pin_port = portOutputRegister(digitalPinToPort(configPage10.n2o_stage2_pin));
  n2o_stage2_pin_mask = digitalPinToBitMask(configPage10.n2o_stage2_pin);
  n2o_arming_pin_port = portInputRegister(digitalPinToPort(configPage10.n2o_arming_pin));
  n2o_arming_pin_mask = digitalPinToBitMask(configPage10.n2o_arming_pin);

  if(configPage10.n2o_enable > 0)
  {
    //The pin modes are only set if the if n2o is enabled to prevent them conflicting with other outputs. 
    if(configPage10.n2o_pin_polarity == 1) { pinMode(configPage10.n2o_arming_pin, INPUT_PULLUP); }
    else { pinMode(configPage10.n2o_arming_pin, INPUT); }
  }
  

  #if defined(CORE_STM32) || defined(CORE_TEENSY) //2uS resolution Min 8Hz, Max 5KHz
    boost_pwm_max_count = 1000000L / (2 * configPage6.boostFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. The x2 is there because the frequency is stored at half value (in a byte) to allow freqneucies up to 511Hz
    vvt_pwm_max_count = 1000000L / (2 * configPage6.vvtFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle
  #else
    boost_pwm_max_count = 1000000L / (16 * configPage6.boostFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. The x2 is there because the frequency is stored at half value (in a byte) to allow freqneucies up to 511Hz
    vvt_pwm_max_count = 1000000L / (16 * configPage6.vvtFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle
  #endif
  ENABLE_VVT_TIMER(); //Turn on the B compare unit (ie turn on the interrupt)

  boostPID.SetOutputLimits(configPage2.boostMinDuty, configPage2.boostMaxDuty);
  if(configPage6.boostMode == BOOST_MODE_SIMPLE) { boostPID.SetTunings(100, 100, 100); }
  else { boostPID.SetTunings(configPage6.boostKP, configPage6.boostKI, configPage6.boostKD); }

  currentStatus.boostDuty = 0;
  boostCounter = 0;
  #if defined(CORE_STM32) //Need to be initialised last due to instant interrupt
    Timer1.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer1.setMode(3, TIMER_OUTPUT_COMPARE);
    if(boost_pwm_max_count > 0) { Timer1.attachInterrupt(2, boostInterrupt);}
    if(vvt_pwm_max_count > 0) { Timer1.attachInterrupt(3, vvtInterrupt);}
    Timer1.resume();
  #endif

  currentStatus.nitrous_status = NITROUS_OFF;

}

#define BOOST_HYSTER  40
void boostControl()
{
  if( configPage6.boostEnabled==1 )
  {
    if(configPage4.boostType == OPEN_LOOP_BOOST)
    {
      //Open loop
      currentStatus.boostDuty = get3DTableValue(&boostTable, currentStatus.TPS, currentStatus.RPM) * 2 * 100;

      if(currentStatus.boostDuty > 10000) { currentStatus.boostDuty = 10000; } //Safety check
      if(currentStatus.boostDuty == 0) { DISABLE_BOOST_TIMER(); BOOST_PIN_LOW(); } //If boost duty is 0, shut everything down
      else
      {
        boost_pwm_target_value = ((unsigned long)(currentStatus.boostDuty) * boost_pwm_max_count) / 10000; //Convert boost duty (Which is a % multipled by 100) to a pwm count
        ENABLE_BOOST_TIMER(); //Turn on the compare unit (ie turn on the interrupt) if boost duty >0
      }
    }
    else if (configPage4.boostType == CLOSED_LOOP_BOOST)
    {
      if( (boostCounter & 7) == 1) { currentStatus.boostTarget = get3DTableValue(&boostTable, currentStatus.TPS, currentStatus.RPM) * 2; } //Boost target table is in kpa and divided by 2
      if(currentStatus.MAP >= 100 ) //Only engage boost control above 100kpa. 
      {
        //If flex fuel is enabled, there can be an adder to the boost target based on ethanol content
        if( configPage2.flexEnabled == 1 )
        {
          currentStatus.boostTarget += table2D_getValue(&flexBoostTable, currentStatus.ethanolPct);;
        }
        else
        {
          currentStatus.flexBoostCorrection = 0;
        }

        if(currentStatus.boostTarget > 0)
        {
          //This only needs to be run very infrequently, once every 16 calls to boostControl(). This is approx. once per second
          if( (boostCounter & 15) == 1)
          {
            boostPID.SetOutputLimits(configPage2.boostMinDuty, configPage2.boostMaxDuty);

            if(configPage6.boostMode == BOOST_MODE_SIMPLE) { boostPID.SetTunings(100, 100, 100); }
            else { boostPID.SetTunings(configPage6.boostKP, configPage6.boostKI, configPage6.boostKD); }
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
      } //MAP above boost + hyster
    } //Open / Cloosed loop
  }
  else { // Disable timer channel and zero the flex boost correction status
    DISABLE_BOOST_TIMER();
    currentStatus.flexBoostCorrection = 0;
  }

  boostCounter++;
}

void vvtControl()
{
  if( configPage6.vvtEnabled == 1 )
  {
    byte vvtDuty = get3DTableValue(&vvtTable, currentStatus.TPS, currentStatus.RPM);

    //VVT table can be used for controlling on/off switching. If this is turned on, then disregard any interpolation or non-binary values
    if( (configPage6.VVTasOnOff == true) && (vvtDuty < 100) ) { vvtDuty = 0; }

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

void nitrousControl()
{
  bool nitrousOn = false; //This tracks whether the control gets turned on at any point. 
  if(configPage10.n2o_enable > 0)
  {
    bool isArmed = READ_N2O_ARM_PIN();
    if (configPage10.n2o_pin_polarity == 1) { isArmed = !isArmed; } //If nitrous is active when pin is low, flip the reading (n2o_pin_polarity = 0 = active when High)

    //Perform the main checks to see if nitrous is ready
    if( (isArmed == true) && (currentStatus.coolant > (configPage10.n2o_minCLT - CALIBRATION_TEMPERATURE_OFFSET)) && (currentStatus.TPS > configPage10.n2o_minTPS) && (currentStatus.O2 < configPage10.n2o_maxAFR) && (currentStatus.MAP < configPage10.n2o_maxMAP) )
    {
      uint16_t realStage1MinRPM = (uint16_t)configPage10.n2o_stage1_minRPM * 100;
      uint16_t realStage1MaxRPM = (uint16_t)configPage10.n2o_stage1_maxRPM * 100;
      uint16_t realStage2MinRPM = (uint16_t)configPage10.n2o_stage2_minRPM * 100;
      uint16_t realStage2MaxRPM = (uint16_t)configPage10.n2o_stage2_maxRPM * 100;

      if( (currentStatus.RPM > realStage1MinRPM) && (currentStatus.RPM < realStage1MaxRPM) )
      {
        currentStatus.nitrous_status = NITROUS_STAGE1;
        BIT_SET(currentStatus.status3, BIT_STATUS3_NITROUS);
        N2O_STAGE1_PIN_HIGH();
        nitrousOn = true;
      }
      if(configPage10.n2o_enable == NITROUS_STAGE2) //This is really just a sanity check
      {
        if( (currentStatus.RPM > realStage2MinRPM) && (currentStatus.RPM < realStage2MaxRPM) )
        {
          currentStatus.nitrous_status = NITROUS_STAGE2;
          BIT_SET(currentStatus.status3, BIT_STATUS3_NITROUS);
          N2O_STAGE2_PIN_HIGH();
          nitrousOn = true;
        }
      }
    }
  }

  if (nitrousOn == false)
    {
      currentStatus.nitrous_status = NITROUS_OFF;
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_NITROUS);
      N2O_STAGE1_PIN_LOW();
      N2O_STAGE2_PIN_LOW();
    }
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
  if (boost_pwm_state == true)
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
  if (vvt_pwm_state == true)
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

#if defined(CORE_TEENSY)
void ftm1_isr(void)
{
  //FTM1 only has 2 compare channels
  //Use separate variables for each test to ensure conversion to bool
  bool interrupt1 = (FTM1_C0SC & FTM_CSC_CHF);
  bool interrupt2 = (FTM1_C1SC & FTM_CSC_CHF);

  if(interrupt1) { FTM1_C0SC &= ~FTM_CSC_CHF; boostInterrupt(); }
  else if(interrupt2) { FTM1_C1SC &= ~FTM_CSC_CHF; vvtInterrupt(); }

}
#endif
