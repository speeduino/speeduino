#include "globals.h"
#if defined(CORE_TEENSY) && defined(__IMXRT1062__)
#include <EEPROM.h>
#include "board_teensy41.h"
#include "auxiliaries.h"
#include "idle.h"
#include "scheduler.h"
#include "timers.h"
#include "comms_secondary.h"
#include <InternalTemperature.h>
#include "storage_api.h"

static byte eeprom_read(uint16_t address) {
  return EEPROM.read(address);
}
static void eeprom_write(uint16_t address, byte val) {
  EEPROM.write(address, val);
}
static uint16_t eeprom_length(void) {
  return EEPROM.length();
}
static void eeprom_clear(void) {
  for (uint16_t address=0; address<EEPROM.length(); ++address) {
    EEPROM.update(address, UINT8_MAX);
  }   
}

void initialiseStorage(void) {
  setStorageAPI(storage_api_t {
    .read = eeprom_read,
    .write = eeprom_write,
    .length = eeprom_length,
    .clear = eeprom_clear,
  });
}

static void PIT_isr();
static void TMR1_isr(void);
static void TMR2_isr(void);
static void TMR3_isr(void);
static void TMR4_isr(void);

void initBoard()
{
    /*
    ***********************************************************************************************************
    * General
    */
   pSecondarySerial = &Serial2;

    /*
    Idle + Boost + VVT use the PIT timer. THIS IS ALSO USED BY THE INTERVAL TIMER THAT CALLS THE 1MS LOW RES TIMER!
    This has 4 channels that don't have compare registers, but will run for a period of time and then fire an interrupt
    The clock for these is set to 24Mhz and a prescale of 48 is used to give a 2uS tick time
    Set Prescaler
      * This is ideally too fast, but appears to be the slowest that the PIT can be set
      * 24Mhz = PER_clock
      * 48 * 1000000uS / PER_clock = 2uS
    */
    CCM_CSCMR1 |= CCM_CSCMR1_PERCLK_CLK_SEL; // 24MHz
    CCM_CSCMR1 |= CCM_CSCMR1_PERCLK_PODF(0b101111); //Prescale to 48

    attachInterruptVector(IRQ_PIT, PIT_isr);
    NVIC_ENABLE_IRQ(IRQ_PIT);
    NVIC_SET_PRIORITY(IRQ_PIT,255);

    /*
    ***********************************************************************************************************
    * Idle
    */
    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OLCL))
    {
      PIT_TCTRL0 = 0;
      PIT_TCTRL0 |= PIT_TCTRL_TIE; // enable Timer 1 interrupts
      PIT_TCTRL0 |= PIT_TCTRL_TEN; // start Timer 1
      PIT_LDVAL0 = 1; //1 * 2uS = 2uS

      idle_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (2U * configPage6.idleFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    }

    /*
    ***********************************************************************************************************
    * Timers
    */
    //Uses the PIT timer channel 4 on Teensy 4.1.
    //lowResTimer.begin(oneMSInterval, 1000);
    PIT_TCTRL3 = 0;
    PIT_TCTRL3 |= PIT_TCTRL_TIE; // enable Timer 2 interrupts
    PIT_TCTRL3 |= PIT_TCTRL_TEN; // start Timer 2
    PIT_LDVAL3 = 500; //500 * 2uS = 1ms

    //TODO: Configure timers here

    /*
    ***********************************************************************************************************
    * Auxiliaries
    */
    if (configPage6.boostEnabled == 1)
    {
      PIT_TCTRL1 = 0;
      PIT_TCTRL1 |= PIT_TCTRL_TIE; // enable Timer 2 interrupts
      PIT_TCTRL1 |= PIT_TCTRL_TEN; // start Timer 2
      PIT_LDVAL1 = 1; //1 * 2uS = 2uS
    }
    if (configPage6.vvtEnabled == 1)
    {
      PIT_TCTRL2 = 0;
      PIT_TCTRL2 |= PIT_TCTRL_TIE; // enable Timer 3 interrupts
      PIT_TCTRL2 |= PIT_TCTRL_TEN; // start Timer 3
      PIT_LDVAL2 = 1; //1 * 2uS = 2uS
    }

    //2uS resolution Min 8Hz, Max 5KHz
    boost_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (2U * configPage6.boostFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. The x2 is there because the frequency is stored at half value (in a byte) to allow frequencies up to 511Hz
    vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (2U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle
    #if defined(PWM_FAN_AVAILABLE)
      fan_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (2U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle
    #endif

    //TODO: Configure timers here

    /*
    ***********************************************************************************************************
    * Schedules
    */
    //Use the Quad timer
    //Uses the BUS clock speed, which is 1/4 of the CPU clock. Maximum prescaler of 128 is used to give a 0.853333uS tick time @ 600Mhz
    //TMR1 - Fuel 1-4
    //0
    TMR1_CTRL0 = 0;
    TMR1_CSCTRL0 = 0;
    TMR1_LOAD0 = 0; /* Reset load register */
    TMR1_CTRL0 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR1_CTRL0 |= TMR_CTRL_CM(1); //Start the timer
    //1
    TMR1_CTRL1 = 0;
    TMR1_CSCTRL1 = 0;
    TMR1_LOAD1 = 0; /* Reset load register */
    TMR1_CTRL1 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR1_CTRL1 |= TMR_CTRL_CM(1); //Start the timer
    //2
    TMR1_CTRL2 = 0;
    TMR1_CSCTRL2 = 0;
    TMR1_LOAD2 = 0; /* Reset load register */
    TMR1_CTRL2 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR1_CTRL2 |= TMR_CTRL_CM(1); //Start the timer
    //3
    TMR1_CTRL3 = 0;
    TMR1_CSCTRL3 = 0;
    TMR1_LOAD3 = 0; /* Reset load register */
    TMR1_CTRL3 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR1_CTRL3 |= TMR_CTRL_CM(1); //Start the timer
    //TMR2 - Ign 1-4
    //0
    TMR2_CTRL0 = 0;
    TMR2_CSCTRL0 = 0;
    TMR2_LOAD0 = 0; /* Reset load register */
    TMR2_CTRL0 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR2_CTRL0 |= TMR_CTRL_CM(1); //Start the timer
    //1
    TMR2_CTRL1 = 0;
    TMR2_CSCTRL1 = 0;
    TMR2_LOAD1 = 0; /* Reset load register */
    TMR2_CTRL1 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR2_CTRL1 |= TMR_CTRL_CM(1); //Start the timer
    //2
    TMR2_CTRL2 = 0;
    TMR2_CSCTRL2 = 0;
    TMR2_LOAD2 = 0; /* Reset load register */
    TMR2_CTRL2 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR2_CTRL2 |= TMR_CTRL_CM(1); //Start the timer
    //3
    TMR2_CTRL3 = 0;
    TMR2_CSCTRL3 = 0;
    TMR2_LOAD3 = 0; /* Reset load register */
    TMR2_CTRL3 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR2_CTRL3 |= TMR_CTRL_CM(1); //Start the timer
    //TMR3 - Fuel 5-8
    //0
    TMR3_CTRL0 = 0;
    TMR3_CSCTRL0 = 0;
    TMR3_LOAD0 = 0; /* Reset load register */
    TMR3_CTRL0 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR3_CTRL0 |= TMR_CTRL_CM(1); //Start the timer
    //1
    TMR3_CTRL1 = 0;
    TMR3_CSCTRL1 = 0;
    TMR3_LOAD1 = 0; /* Reset load register */
    TMR3_CTRL1 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR3_CTRL1 |= TMR_CTRL_CM(1); //Start the timer
    //2
    TMR3_CTRL2 = 0;
    TMR3_CSCTRL2 = 0;
    TMR3_LOAD2 = 0; /* Reset load register */
    TMR3_CTRL2 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR3_CTRL2 |= TMR_CTRL_CM(1); //Start the timer
    //3
    TMR3_CTRL3 = 0;
    TMR3_CSCTRL3 = 0;
    TMR3_LOAD3 = 0; /* Reset load register */
    TMR3_CTRL3 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR3_CTRL3 |= TMR_CTRL_CM(1); //Start the timer
    //TMR4 - IGN 5-8
    //0
    TMR4_CTRL0 = 0;
    TMR4_CSCTRL0 = 0;
    TMR4_LOAD0 = 0; /* Reset load register */
    TMR4_CTRL0 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR4_CTRL0 |= TMR_CTRL_CM(1); //Start the timer
    //1
    TMR4_CTRL1 = 0;
    TMR4_CSCTRL1 = 0;
    TMR4_LOAD1 = 0; /* Reset load register */
    TMR4_CTRL1 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR4_CTRL1 |= TMR_CTRL_CM(1); //Start the timer
    //2
    TMR4_CTRL2 = 0;
    TMR4_CSCTRL2 = 0;
    TMR4_LOAD2 = 0; /* Reset load register */
    TMR4_CTRL2 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR4_CTRL2 |= TMR_CTRL_CM(1); //Start the timer
    //3
    TMR4_CTRL3 = 0;
    TMR4_CSCTRL3 = 0;
    TMR4_LOAD3 = 0; /* Reset load register */
    TMR4_CTRL3 |= TMR_CTRL_PCS(0b1111); //Set the prescaler to 128
    TMR4_CTRL3 |= TMR_CTRL_CM(1); //Start the timer

    attachInterruptVector(IRQ_QTIMER1, TMR1_isr);
    NVIC_ENABLE_IRQ(IRQ_QTIMER1);
    attachInterruptVector(IRQ_QTIMER2, TMR2_isr);
    NVIC_ENABLE_IRQ(IRQ_QTIMER2);
    attachInterruptVector(IRQ_QTIMER3, TMR3_isr);
    NVIC_ENABLE_IRQ(IRQ_QTIMER3);
    attachInterruptVector(IRQ_QTIMER4, TMR4_isr);
    NVIC_ENABLE_IRQ(IRQ_QTIMER4);
}

void PIT_isr()
{
  bool interrupt1 = (PIT_TFLG0 & PIT_TFLG_TIF);
  bool interrupt2 = (PIT_TFLG1 & PIT_TFLG_TIF);
  bool interrupt3 = (PIT_TFLG2 & PIT_TFLG_TIF);
  bool interrupt4 = (PIT_TFLG3 & PIT_TFLG_TIF);

  if(interrupt1)      { PIT_TFLG0 = 1; idleInterrupt();  }
  else if(interrupt2) { PIT_TFLG1 = 1; boostInterrupt(); }
  else if(interrupt3) { PIT_TFLG2 = 1; vvtInterrupt();   }
  else if(interrupt4) { PIT_TFLG3 = 1; oneMSInterval();  }
  asm volatile ("dsb") ;
}

void TMR1_isr(void)
{
  //TMR1 is fuel channels 1-4
  bool interrupt1 = (TMR1_CSCTRL0 & TMR_CSCTRL_TCF1);
  bool interrupt2 = (TMR1_CSCTRL1 & TMR_CSCTRL_TCF1);
  bool interrupt3 = (TMR1_CSCTRL2 & TMR_CSCTRL_TCF1);
  bool interrupt4 = (TMR1_CSCTRL3 & TMR_CSCTRL_TCF1);

  if(interrupt1)      { TMR1_CSCTRL0 &= ~TMR_CSCTRL_TCF1; fuelSchedule1Interrupt(); }
  else if(interrupt2) { TMR1_CSCTRL1 &= ~TMR_CSCTRL_TCF1; fuelSchedule2Interrupt(); }
  else if(interrupt3) { TMR1_CSCTRL2 &= ~TMR_CSCTRL_TCF1; fuelSchedule3Interrupt(); }
  else if(interrupt4) { TMR1_CSCTRL3 &= ~TMR_CSCTRL_TCF1; fuelSchedule4Interrupt(); }
}
void TMR2_isr(void)
{
  //TMR2 is IGN channels 1-4
  bool interrupt1 = (TMR2_CSCTRL0 & TMR_CSCTRL_TCF1);
  bool interrupt2 = (TMR2_CSCTRL1 & TMR_CSCTRL_TCF1);
  bool interrupt3 = (TMR2_CSCTRL2 & TMR_CSCTRL_TCF1);
  bool interrupt4 = (TMR2_CSCTRL3 & TMR_CSCTRL_TCF1);

  if(interrupt1)      { TMR2_CSCTRL0 &= ~TMR_CSCTRL_TCF1; ignitionSchedule1Interrupt(); }
  else if(interrupt2) { TMR2_CSCTRL1 &= ~TMR_CSCTRL_TCF1; ignitionSchedule2Interrupt(); }
  else if(interrupt3) { TMR2_CSCTRL2 &= ~TMR_CSCTRL_TCF1; ignitionSchedule3Interrupt(); }
  else if(interrupt4) { TMR2_CSCTRL3 &= ~TMR_CSCTRL_TCF1; ignitionSchedule4Interrupt(); }
}
void TMR3_isr(void)
{
  //TMR3 is Fuel  channels 5-8
  bool interrupt1 = (TMR3_CSCTRL0 & TMR_CSCTRL_TCF1);
  bool interrupt2 = (TMR3_CSCTRL1 & TMR_CSCTRL_TCF1);
  bool interrupt3 = (TMR3_CSCTRL2 & TMR_CSCTRL_TCF1);
  bool interrupt4 = (TMR3_CSCTRL3 & TMR_CSCTRL_TCF1);

  if(interrupt1)      { TMR3_CSCTRL0 &= ~TMR_CSCTRL_TCF1; fuelSchedule5Interrupt(); }
  else if(interrupt2) { TMR3_CSCTRL1 &= ~TMR_CSCTRL_TCF1; fuelSchedule6Interrupt(); }
  else if(interrupt3) { TMR3_CSCTRL2 &= ~TMR_CSCTRL_TCF1; fuelSchedule7Interrupt(); }
  else if(interrupt4) { TMR3_CSCTRL3 &= ~TMR_CSCTRL_TCF1; fuelSchedule8Interrupt(); }
}
void TMR4_isr(void)
{
  //TMR4 is IGN channels 5-8
  bool interrupt1 = (TMR4_CSCTRL0 & TMR_CSCTRL_TCF1);
  bool interrupt2 = (TMR4_CSCTRL1 & TMR_CSCTRL_TCF1);
  bool interrupt3 = (TMR4_CSCTRL2 & TMR_CSCTRL_TCF1);
  bool interrupt4 = (TMR4_CSCTRL3 & TMR_CSCTRL_TCF1);

  if(interrupt1)      { TMR4_CSCTRL0 &= ~TMR_CSCTRL_TCF1; ignitionSchedule5Interrupt(); }
  else if(interrupt2) { TMR4_CSCTRL1 &= ~TMR_CSCTRL_TCF1; ignitionSchedule6Interrupt(); }
  else if(interrupt3) { TMR4_CSCTRL2 &= ~TMR_CSCTRL_TCF1; ignitionSchedule7Interrupt(); }
  else if(interrupt4) { TMR4_CSCTRL3 &= ~TMR_CSCTRL_TCF1; ignitionSchedule8Interrupt(); }
}

uint16_t freeRam()
{
    uint32_t stackTop;
    uint32_t heapTop;

    // current position of the stack.
    stackTop = (uint32_t) &stackTop;

    // current position of heap.
    void* hTop = malloc(1);
    heapTop = (uint32_t) hTop;
    free(hTop);

    // The difference is the free, available ram.
    return (uint16_t)stackTop - heapTop;
}

//This function is used for attempting to set the RTC time during compile
time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void doSystemReset() { return; }
void jumpToBootloader() { return; }

//Checks if the request pin is being used for rx/tx on secondary serial. Primary (USB) serial does not need to be checked as it is not broken out to an IO on Teensy
//The Secondary serial that it checks against is based on that one set by the pinMapping
bool pinIsSerial(uint8_t pin)
{
  bool isSerial = false;

  if(&secondarySerial == &Serial1)
  {
    if( (pin == 0) || (pin == 1) ) { isSerial = true; }
  }
  else if(&secondarySerial == &Serial2)
  {
    if( (pin == 7) || (pin == 8) ) { isSerial = true; }
  }
  else if(&secondarySerial == &Serial3)
  {
    if( (pin == 14) || (pin == 15) ) { isSerial = true; }
  }
  else if(&secondarySerial == &Serial4)
  {
    if( (pin == 16) || (pin == 17) ) { isSerial = true; }
  }
  else if(&secondarySerial == &Serial5)
  {
    if( (pin == 20) || (pin == 21) ) { isSerial = true; }
  }
  else if(&secondarySerial == &Serial6)
  {
    if( (pin == 24) || (pin == 25) ) { isSerial = true; }
  }
  else if(&secondarySerial == &Serial7)
  {
    if( (pin == 28) || (pin == 29) ) { isSerial = true; }
  }
  else if(&secondarySerial == &Serial8)
  {
    if( (pin == 34) || (pin == 35) ) { isSerial = true; }
  }

  return isSerial;
}

void setPinHysteresis(uint8_t pin)
{
  //Refer to digital.c in the Teensyduino core for the following code
  //Refer also to Pgs 382 and 950 of the iMXRT1060 Reference Manual
  const struct digital_pin_bitband_and_config_table_struct *p;
  const uint32_t padConfig = IOMUXC_PAD_DSE(1) | IOMUXC_PAD_PKE | IOMUXC_PAD_PUE | IOMUXC_PAD_SPEED(0) | IOMUXC_PAD_HYS;

  p = digital_pin_to_info_PGM + pin;
  *(p->reg + 1) &= ~(p->mask); // TODO: atomic
  *(p->pad) = padConfig;
  *(p->mux) = 5 | 0x10;
}

void setTeensy41PinsHysteresis()
{
  //Primary trigger
  setPinHysteresis(pinTrigger);
  //Secondary trigger
  setPinHysteresis(pinTrigger2);
  //Tertiary trigger
  setPinHysteresis(pinTrigger3);

  if(configPage2.flexEnabled > 0) { setPinHysteresis(pinFlex); }
  if(configPage2.vssMode > 1) { setPinHysteresis(pinVSS); }// VSS modes 2 and 3 are interrupt drive (Mode 1 is CAN)
  if(configPage10.knock_mode == KNOCK_MODE_DIGITAL) { setPinHysteresis(configPage10.knock_pin); }

}

/*
* The default Teensy41 serial.begin() has a timeout of 750ms, which is both too long to wait on startup and longer than it needs to be
* This function is a copy of the default serial.begin() but with the timeout lowered to 100ms
*/
void teensy41_customSerialBegin()
{
  uint32_t millis_begin = systick_millis_count;
  while (!Serial) 
  {
    uint32_t elapsed = systick_millis_count - millis_begin;
    //Wait up to 100ms for this. 
    if (elapsed > 100) break;
    yield();
  }
}

uint8_t getSystemTemp()
{
  return trunc(InternalTemperature.readTemperatureC());
}

#endif
