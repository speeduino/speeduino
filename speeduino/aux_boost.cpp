#include "aux_boost.h"
#include "globals.h"
#include "src/PID_v1/PID_v1.h"
#include "timers.h"

byte boostCounter;

volatile PORT_TYPE *boost_pin_port;
volatile PINMASK_TYPE boost_pin_mask;

long boost_pwm_target_value;
volatile bool boost_pwm_state;
volatile unsigned int boost_pwm_cur_value = 0;

uint16_t boost_pwm_max_count; //Used for variable PWM frequency
static table2D_u8_s16_6 flexBoostTable(&configPage10.flexBoostBins, &configPage10.flexBoostAdj);

integerPID_ideal boostPID(&currentStatus.MAP, &currentStatus.boostDuty , &currentStatus.boostTarget, &configPage10.boostSens, &configPage10.boostIntv, configPage6.boostKP, configPage6.boostKI, configPage6.boostKD, DIRECT); //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

void initialiseBoost(void)
{
  boost_pin_port = portOutputRegister(digitalPinToPort(pinBoost));
  boost_pin_mask = digitalPinToBitMask(pinBoost);

  boostPID.SetOutputLimits(configPage2.boostMinDuty, configPage2.boostMaxDuty);
  if(configPage6.boostMode == BOOST_MODE_SIMPLE) { boostPID.SetTunings(SIMPLE_BOOST_P, SIMPLE_BOOST_I, SIMPLE_BOOST_D); }
  else { boostPID.SetTunings(configPage6.boostKP, configPage6.boostKI, configPage6.boostKD); }

  boostCounter = 0;
  currentStatus.boostDuty = 0;
}

void boostControl(void)
{
  if( configPage6.boostEnabled==1 )
  {
    if(configPage4.boostType == OPEN_LOOP_BOOST)
    {
      //Open loop
      if ( (configPage9.boostByGearEnabled > 0) && (configPage2.vssMode > 1) ){ boostByGear(); }
      else{ currentStatus.boostDuty = get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM) * 2 * 100; }

      if(currentStatus.boostDuty > 10000) { currentStatus.boostDuty = 10000; } //Safety check
      if(currentStatus.boostDuty == 0) { DISABLE_BOOST_TIMER(); BOOST_PIN_LOW(); } //If boost duty is 0, shut everything down
      else
      {
        boost_pwm_target_value = ((unsigned long)(currentStatus.boostDuty) * boost_pwm_max_count) / 10000; //Convert boost duty (Which is a % multiplied by 100) to a pwm count
      }
    }
    else if (configPage4.boostType == CLOSED_LOOP_BOOST)
    {
      if( (boostCounter & 7) == 1) 
      { 
        if ( (configPage9.boostByGearEnabled > 0) && (configPage2.vssMode > 1) ){ boostByGear(); }
        else{ currentStatus.boostTarget = get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM) << 1; } //Boost target table is in kpa and divided by 2

        //If flex fuel is enabled, there can be an adder to the boost target based on ethanol content
        if( configPage2.flexEnabled == 1 )
        {
          currentStatus.flexBoostCorrection = table2D_getValue(&flexBoostTable, currentStatus.ethanolPct);
          currentStatus.boostTarget += currentStatus.flexBoostCorrection;
          currentStatus.boostTarget = min(currentStatus.boostTarget, (uint16_t)511U);
        }
        else
        {
          currentStatus.flexBoostCorrection = 0;
        }
      } 

      if(((configPage15.boostControlEnable == EN_BOOST_CONTROL_BARO) && (currentStatus.MAP >= currentStatus.baro)) || ((configPage15.boostControlEnable == EN_BOOST_CONTROL_FIXED) && (currentStatus.MAP >= configPage15.boostControlEnableThreshold))) //Only enables boost control above baro pressure or above user defined threshold (User defined level is usually set to boost with wastegate actuator only boost level)
      {
        if(currentStatus.boostTarget > 0)
        {
          //This only needs to be run very infrequently, once every 16 calls to boostControl(). This is approx. once per second
          if( (boostCounter & 15) == 1)
          {
            boostPID.SetOutputLimits(configPage2.boostMinDuty, configPage2.boostMaxDuty);

            if(configPage6.boostMode == BOOST_MODE_SIMPLE) { boostPID.SetTunings(SIMPLE_BOOST_P, SIMPLE_BOOST_I, SIMPLE_BOOST_D); }
            else { boostPID.SetTunings(configPage6.boostKP, configPage6.boostKI, configPage6.boostKD); }
          }

          bool PIDcomputed = boostPID.Compute(get3DTableValue(&boostTableLookupDuty, currentStatus.boostTarget, currentStatus.RPM) * 100/2); //Compute() returns false if the required interval has not yet passed.
          if(currentStatus.boostDuty == 0) { DISABLE_BOOST_TIMER(); BOOST_PIN_LOW(); } //If boost duty is 0, shut everything down
          else
          {
            if(PIDcomputed == true)
            {
              boost_pwm_target_value = ((unsigned long)(currentStatus.boostDuty) * boost_pwm_max_count) / 10000; //Convert boost duty (Which is a % multiplied by 100) to a pwm count
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
        boostPID.Initialize(); //This resets the ITerm value to prevent rubber banding
        //Boost control needs to have a high duty cycle if control is below threshold (baro or fixed value). This ensures the waste gate is closed as much as possible, this build boost as fast as possible.
        currentStatus.boostDuty = configPage15.boostDCWhenDisabled*100;
        boost_pwm_target_value = ((unsigned long)(currentStatus.boostDuty) * boost_pwm_max_count) / 10000; //Convert boost duty (Which is a % multiplied by 100) to a pwm count
        ENABLE_BOOST_TIMER(); //Turn on the compare unit (ie turn on the interrupt) if boost duty >0
        if(currentStatus.boostDuty == 0) { boostDisable(); } //If boost control does nothing disable PWM completely
      } //MAP above boost + hyster
    } //Open / Cloosed loop

    //Check for 100% duty cycle
    if(currentStatus.boostDuty >= 10000)
    {
      DISABLE_BOOST_TIMER(); //Turn off the compare unit (ie turn off the interrupt) if boost duty is 100%
      BOOST_PIN_HIGH(); //Turn on boost pin if duty is 100%
    }
    else if(currentStatus.boostDuty > 0)
    {
      ENABLE_BOOST_TIMER(); //Turn on the compare unit (ie turn on the interrupt) if boost duty is > 0
    }
    
  }
  else { // Disable timer channel and zero the flex boost correction status
    DISABLE_BOOST_TIMER();
    currentStatus.flexBoostCorrection = 0;
  }

  boostCounter++;
}

void boostByGear(void)
{
  if(configPage4.boostType == OPEN_LOOP_BOOST)
  {
    if( configPage9.boostByGearEnabled == 1 )
    {
      uint16_t combinedBoost = 0;
      switch (currentStatus.gear)
      {
        case 1:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear1 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 2:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear2 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 3:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear3 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 4:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear4 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 5:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear5 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        case 6:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear6 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM))  ) << 2;
          if( combinedBoost <= 10000 ){ currentStatus.boostDuty = combinedBoost; }
          else{ currentStatus.boostDuty = 10000; }
          break;
        default:
          break;
      }
    }
    else if( configPage9.boostByGearEnabled == 2 ) 
    {
      switch (currentStatus.gear)
      {
        case 1:
          currentStatus.boostDuty = configPage9.boostByGear1 * 2 * 100;
          break;
        case 2:
          currentStatus.boostDuty = configPage9.boostByGear2 * 2 * 100;
          break;
        case 3:
          currentStatus.boostDuty = configPage9.boostByGear3 * 2 * 100;
          break;
        case 4:
          currentStatus.boostDuty = configPage9.boostByGear4 * 2 * 100;
          break;
        case 5:
          currentStatus.boostDuty = configPage9.boostByGear5 * 2 * 100;
          break;
        case 6:
          currentStatus.boostDuty = configPage9.boostByGear6 * 2 * 100;
          break;
        default:
          break;
      }
    }
  }
  else if (configPage4.boostType == CLOSED_LOOP_BOOST)
  {
    if( configPage9.boostByGearEnabled == 1 )
    {
      uint16_t combinedBoost = 0;
      switch (currentStatus.gear)
      {
        case 1:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear1 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 2:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear2 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 3:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear3 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 4:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear4 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 5:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear5 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        case 6:
          combinedBoost = ( ((uint16_t)configPage9.boostByGear6 * (uint16_t)get3DTableValue(&boostTable, (currentStatus.TPS * 2U), currentStatus.RPM)) / 100 ) << 2;
          if( combinedBoost <= 511 ){ currentStatus.boostTarget = combinedBoost; }
          else{ currentStatus.boostTarget = 511; }
          break;
        default:
          break;
      }
    }
    else if( configPage9.boostByGearEnabled == 2 ) 
    {
      switch (currentStatus.gear)
      {
        case 1:
          currentStatus.boostTarget = (configPage9.boostByGear1 << 1);
          break;
        case 2:
          currentStatus.boostTarget = (configPage9.boostByGear2 << 1);
          break;
        case 3:
          currentStatus.boostTarget = (configPage9.boostByGear3 << 1);
          break;
        case 4:
          currentStatus.boostTarget = (configPage9.boostByGear4 << 1);
          break;
        case 5:
          currentStatus.boostTarget = (configPage9.boostByGear5 << 1);
          break;
        case 6:
          currentStatus.boostTarget = (configPage9.boostByGear6 << 1);
          break;
        default:
          break;
      }
    }
  }
}

void boostDisable(void)
{
  boostPID.Initialize(); //This resets the ITerm value to prevent rubber banding
  currentStatus.boostDuty = 0;
  DISABLE_BOOST_TIMER(); //Turn off timer
  BOOST_PIN_LOW(); //Make sure solenoid is off (0% duty)
}

//The interrupt to control the Boost PWM
#if defined(CORE_AVR)
  ISR(TIMER1_COMPA_vect) //cppcheck-suppress misra-c2012-8.2
#else
  void boostInterrupt(void) //Most ARM chips can simply call a function
#endif
{
  if (boost_pwm_state == true)
  {
    #if defined(CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
    BOOST_PIN_HIGH();
    #else
    BOOST_PIN_LOW();  // Switch pin to low
    #endif
    SET_COMPARE(BOOST_TIMER_COMPARE, BOOST_TIMER_COUNTER + (boost_pwm_max_count - boost_pwm_cur_value) );
    boost_pwm_state = false;
  }
  else
  {
    #if defined(CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
    BOOST_PIN_LOW();
    #else
    BOOST_PIN_HIGH();  // Switch pin high
    #endif
    SET_COMPARE(BOOST_TIMER_COMPARE, BOOST_TIMER_COUNTER + boost_pwm_target_value);
    boost_pwm_cur_value = boost_pwm_target_value;
    boost_pwm_state = true;
  }
}