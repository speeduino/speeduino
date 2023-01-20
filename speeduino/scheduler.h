/** @file
Injector and Ignition (on/off) scheduling (structs).
This scheduler is designed to maintain 2 schedules for use by the fuel and ignition systems.
It functions by waiting for the overflow vectors from each of the timers in use to overflow, which triggers an interrupt.
## Technical
Currently prescaling the 16-bit timers to 64 for injection and 64 for ignition.
This means that the counter increments every 4us (injection) / 4uS (ignition) and will overflow every 262140uS.
    Max Period = (Prescale)*(1/Frequency)*(2^17)
For more details see https://playground.arduino.cc/Code/Timer1/ (OLD: http://playground.arduino.cc/code/timer1 ).
This means that the precision of the scheduler is:
- 4uS for fuel
- 4uS (+/- 2uS) for ignition
## Features
This differs from most other schedulers in that its calls are non-recurring (ie when you schedule an event at a certain time and once it has occurred,
it will not reoccur unless you explicitly ask/re-register for it).
Each timer can have only 1 callback associated with it at any given time. If you call the setCallback function a 2nd time,
the original schedule will be overwritten and not occur.
## Timer identification
Arduino timers usage for injection and ignition schedules:
- timer3 is used for schedule 1(?) (fuel 1,2,3,4 ign 7,8)
- timer4 is used for schedule 2(?) (fuel 5,6 ign 4,5,6)
- timer5 is used ... (fuel 7,8, ign 1,2,3)
Timers 3,4 and 5 are 16-bit timers (ie count to 65536).
See page 136 of the processors datasheet: http://www.atmel.com/Images/doc2549.pdf .

64 prescale gives tick every 4uS.
64 prescale gives overflow every 262140uS (This means maximum wait time is 0.26214 seconds).

*/
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "globals.h"

#define USE_IGN_REFRESH
#define IGNITION_REFRESH_THRESHOLD  230U //Time in uS that the refresh functions will check to ensure there is enough time before changing the end compare
#define INJECTION_REFRESH_TRESHOLD  230U //Time in us that the refresh functions will check to ensure there is enough time before changing the start or end compare
#define INJECTION_OVERLAP_TRESHOLD  96U //Time in us, basically minimum injector off time that is allowed.

void initialiseSchedulers();
void beginInjectorPriming();

void setIgnitionSchedule(struct Schedule *ignitionSchedule , int16_t crankAngle,int ignitionEndAngle, unsigned long duration);
void setIgnitionSchedule(struct Schedule *ignitionSchedule); //overload function for starting schedule(dwell) immediately, this is used in the fixed cranking ignition

void ignitionScheduleInterrupt(struct Schedule *ignitionSchedule);

void setFuelSchedule (struct Schedule *targetSchedule, unsigned long duration);
void setFuelSchedule (struct Schedule *targetSchedule, int16_t crankAngle, int16_t injectorEndAngle, unsigned long duration);


//The ARM cores use seprate functions for their ISRs
#if defined(ARDUINO_ARCH_STM32) || defined(CORE_TEENSY)
  static inline void fuelSchedule1Interrupt();
  static inline void fuelSchedule2Interrupt();
  static inline void fuelSchedule3Interrupt();
  static inline void fuelSchedule4Interrupt();
#if (INJ_CHANNELS >= 5)
  static inline void fuelSchedule5Interrupt();
#endif
#if (INJ_CHANNELS >= 6)
  static inline void fuelSchedule6Interrupt();
#endif
#if (INJ_CHANNELS >= 7)
  static inline void fuelSchedule7Interrupt();
#endif
#if (INJ_CHANNELS >= 8)
  static inline void fuelSchedule8Interrupt();
#endif
#if (IGN_CHANNELS >= 1)
  static inline void ignitionSchedule1Interrupt();
#endif
#if (IGN_CHANNELS >= 2)
  static inline void ignitionSchedule2Interrupt();
#endif
#if (IGN_CHANNELS >= 3)
  static inline void ignitionSchedule3Interrupt();
#endif
#if (IGN_CHANNELS >= 4)
  static inline void ignitionSchedule4Interrupt();
#endif
#if (IGN_CHANNELS >= 5)
  static inline void ignitionSchedule5Interrupt();
#endif
#if (IGN_CHANNELS >= 6)
  static inline void ignitionSchedule6Interrupt();
#endif
#if (IGN_CHANNELS >= 7)
  static inline void ignitionSchedule7Interrupt();
#endif
#if (IGN_CHANNELS >= 8)
  static inline void ignitionSchedule8Interrupt();
#endif
#endif
/** Schedule statuses.
 * - OFF - Schedule turned off and there is no scheduled plan
 * - PENDING - There's a scheduled plan, but is has not started to run yet
 * - RUNNING - Schedule is currently running
 */
enum ScheduleStatus : uint8_t {OFF, PENDING, RUNNING}; //The statuses that a schedule can have

/** Ignition schedule and Fuel Schedule, both use the same struct now.
 */
struct Schedule {  
  volatile ScheduleStatus Status; ///< Schedule status: OFF, PENDING, STAGED, RUNNING
 
  volatile COMPARE_TYPE endCompare;   ///< The counter value of the timer when this will end

  volatile COMPARE_TYPE nextStartCompare;      ///< Planned start of next schedule (when current schedule is RUNNING)
  volatile COMPARE_TYPE nextEndCompare;        ///< Planned end of next schedule (when current schedule is RUNNING)
  volatile bool hasNextSchedule = false; ///< Enable flag for planned next schedule (when current schedule is RUNNING)  
  int channelDegrees=0; // The number of crank degrees until corresponding cylinder is at TDC (cylinder1 is obviously 0 for virtually ALL engines, but there's some weird ones)

  void (*StartFunction)();        ///< Start Callback function for schedule
  void (*EndFunction)();          ///< End Callback function for schedule

  //pure virtual functions, these are really defined in sub-classes
  virtual COMPARE_TYPE getCounter()=0; //Function for getting counter value
  virtual void setCompare(COMPARE_TYPE compareValue)=0; //Function for setting counter compare value
  virtual void timerDisable()=0; //Function to disable timer for specific channel
  virtual void timerEnable()=0; //Function to enable timer for specific channel  
};

struct Ign1: Schedule //Derived ignitionSchedule structs with  channel specific override functions
{
  COMPARE_TYPE getCounter(){return IGN1_COUNTER;}
  void setCompare(COMPARE_TYPE compareValue){IGN1_COMPARE =compareValue;}
  void timerDisable(){IGN1_TIMER_DISABLE();}
  void timerEnable(){IGN1_TIMER_ENABLE();}
};

struct Ign2: Schedule //Derived ignitionSchedule structs with  channel specific override functions
{
  COMPARE_TYPE getCounter(){return IGN2_COUNTER;}
  void setCompare(COMPARE_TYPE compareValue){IGN2_COMPARE =compareValue;}
  void timerDisable(){IGN2_TIMER_DISABLE();}
  void timerEnable(){IGN2_TIMER_ENABLE();}
};

struct Ign3: Schedule //Derived ignitionSchedule structs with  channel specific override functions
{
  COMPARE_TYPE getCounter(){return IGN3_COUNTER;}
  void setCompare(COMPARE_TYPE compareValue){IGN3_COMPARE =compareValue;}
  void timerDisable(){IGN3_TIMER_DISABLE();}
  void timerEnable(){IGN3_TIMER_ENABLE();}
};

struct Ign4: Schedule //Derived ignitionSchedule structs with  channel specific override functions
{
  COMPARE_TYPE getCounter(){return IGN4_COUNTER;}
  void setCompare(COMPARE_TYPE compareValue){IGN4_COMPARE =compareValue;}
  void timerDisable(){IGN4_TIMER_DISABLE();}
  void timerEnable(){IGN4_TIMER_ENABLE();}
};

struct Ign5: Schedule //Derived ignitionSchedule structs with  channel specific override functions
{
  COMPARE_TYPE getCounter(){return IGN5_COUNTER;}
  void setCompare(COMPARE_TYPE compareValue){IGN5_COMPARE =compareValue;}
  void timerDisable(){IGN5_TIMER_DISABLE();}
  void timerEnable(){IGN5_TIMER_ENABLE();}
};

struct Ign6: Schedule //Derived ignitionSchedule structs with  channel specific override functions
{
  COMPARE_TYPE getCounter(){return IGN6_COUNTER;}
  void setCompare(COMPARE_TYPE compareValue){IGN6_COMPARE =compareValue;}
  void timerDisable(){IGN6_TIMER_DISABLE();}
  void timerEnable(){IGN6_TIMER_ENABLE();}
};

struct Ign7: Schedule //Derived ignitionSchedule structs with  channel specific override functions
{
  COMPARE_TYPE getCounter(){return IGN7_COUNTER;}
  void setCompare(COMPARE_TYPE compareValue){IGN7_COMPARE =compareValue;}
  void timerDisable(){IGN7_TIMER_DISABLE();}
  void timerEnable(){IGN7_TIMER_ENABLE();}
};

struct Ign8: Schedule //Derived ignitionSchedule structs with  channel specific override functions
{
  COMPARE_TYPE getCounter(){return IGN8_COUNTER;}
  void setCompare(COMPARE_TYPE compareValue){IGN8_COMPARE =compareValue;}
  void timerDisable(){IGN8_TIMER_DISABLE();}
  void timerEnable(){IGN8_TIMER_ENABLE();}
};

struct Fuel1: Schedule //derived FuelShedule with channel specific functions
{
  COMPARE_TYPE getCounter() {return FUEL1_COUNTER;};
  void setCompare(COMPARE_TYPE compareValue){FUEL1_COMPARE =compareValue;};
  void timerDisable(){FUEL1_TIMER_DISABLE();};
  void timerEnable(){FUEL1_TIMER_ENABLE();};
};
struct Fuel2: Schedule//derived FuelShedule with channel specific functions
{
  COMPARE_TYPE getCounter() {return FUEL2_COUNTER;};
  void setCompare(COMPARE_TYPE compareValue){FUEL2_COMPARE =compareValue;};
  void timerDisable(){FUEL2_TIMER_DISABLE();};
  void timerEnable(){FUEL2_TIMER_ENABLE();};
};
struct Fuel3: Schedule//derived FuelShedule with channel specific functions
{
  COMPARE_TYPE getCounter() {return FUEL3_COUNTER;};
  void setCompare(COMPARE_TYPE compareValue){FUEL3_COMPARE =compareValue;};
  void timerDisable(){FUEL3_TIMER_DISABLE();};
  void timerEnable(){FUEL3_TIMER_ENABLE();};
};
struct Fuel4: Schedule//derived FuelShedule with channel specific functions
{
  COMPARE_TYPE getCounter() {return FUEL4_COUNTER;};
  void setCompare(COMPARE_TYPE compareValue){FUEL4_COMPARE =compareValue;};
  void timerDisable(){FUEL4_TIMER_DISABLE();};
  void timerEnable(){FUEL4_TIMER_ENABLE();};
};
struct Fuel5: Schedule//derived FuelShedule with channel specific functions
{
  COMPARE_TYPE getCounter() {return FUEL5_COUNTER;};
  void setCompare(COMPARE_TYPE compareValue){FUEL5_COMPARE =compareValue;};
  void timerDisable(){FUEL5_TIMER_DISABLE();};
  void timerEnable(){FUEL5_TIMER_ENABLE();};
};
struct Fuel6: Schedule//derived FuelShedule with channel specific functions
{
  COMPARE_TYPE getCounter() {return FUEL6_COUNTER;};
  void setCompare(COMPARE_TYPE compareValue){FUEL6_COMPARE =compareValue;};
  void timerDisable(){FUEL6_TIMER_DISABLE();};
  void timerEnable(){FUEL6_TIMER_ENABLE();};
};
struct Fuel7: Schedule//derived FuelShedule with channel specific functions
{
  COMPARE_TYPE getCounter() {return FUEL7_COUNTER;};
  void setCompare(COMPARE_TYPE compareValue){FUEL7_COMPARE =compareValue;};
  void timerDisable(){FUEL7_TIMER_DISABLE();};
  void timerEnable(){FUEL7_TIMER_ENABLE();};
};
struct Fuel8: Schedule//derived FuelShedule with channel specific functions
{
  COMPARE_TYPE getCounter() {return FUEL8_COUNTER;};
  void setCompare(COMPARE_TYPE compareValue){FUEL8_COMPARE =compareValue;};
  void timerDisable(){FUEL8_TIMER_DISABLE();};
  void timerEnable(){FUEL8_TIMER_ENABLE();};
};


extern Fuel1 fuelSchedule1;
extern Fuel2 fuelSchedule2;
extern Fuel3 fuelSchedule3;
extern Fuel4 fuelSchedule4;
#if (INJ_CHANNELS >= 5)
extern Fuel5 fuelSchedule5;
#endif
#if (INJ_CHANNELS >= 6)
extern Fuel6 fuelSchedule6;
#endif
#if (INJ_CHANNELS >= 7)
extern Fuel7 fuelSchedule7;
#endif
#if (INJ_CHANNELS >= 8)
extern Fuel8 fuelSchedule8;
#endif

extern Ign1 ignitionSchedule1;
extern Ign2 ignitionSchedule2;
extern Ign3 ignitionSchedule3;
extern Ign4 ignitionSchedule4;
extern Ign5 ignitionSchedule5;
#if IGN_CHANNELS >= 6
extern Ign6 ignitionSchedule6;
#endif
#if IGN_CHANNELS >= 7
extern Ign7 ignitionSchedule7;
#endif
#if IGN_CHANNELS >= 8
extern Ign8 ignitionSchedule8;
#endif


#endif // SCHEDULER_H
