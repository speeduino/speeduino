/** @file
Injector and Ignition (on/off) scheduling (structs).

This scheduler is designed to maintain 2 schedules for use by the fuel and ignition systems.
It functions by waiting for the overflow vectors from each of the timers in use to overflow, which triggers an interrupt.

## Technical

Currently I am prescaling the 16-bit timers to 256 for injection and 64 for ignition.
This means that the counter increments every 16us (injection) / 4uS (ignition) and will overflow every 1048576uS.

    Max Period = (Prescale)*(1/Frequency)*(2^17)

For more details see https://playground.arduino.cc/Code/Timer1/ (OLD: http://playground.arduino.cc/code/timer1 ).
This means that the precision of the scheduler is:

- 16uS (+/- 8uS of target) for fuel
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

256 prescale gives tick every 16uS.
256 prescale gives overflow every 1048576uS (This means maximum wait time is 1.0485 seconds).

*/
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "globals.h"

#define USE_IGN_REFRESH
#define IGNITION_REFRESH_THRESHOLD  30 //Time in uS that the refresh functions will check to ensure there is enough time before changing the end compare

#define DWELL_AVERAGE_ALPHA 30
#define DWELL_AVERAGE(input) (((long)input * (256 - DWELL_AVERAGE_ALPHA) + ((long)currentStatus.actualDwell * DWELL_AVERAGE_ALPHA))) >> 8
//#define DWELL_AVERAGE(input) (currentStatus.dwell) //Can be use to disable the above for testing

/** @name IgnitionCallbacks
 * These are the (global) function pointers that get called to begin and end the ignition coil charging.
 * They are required for the various spark output modes.
 * @{
*/
extern void (*ign1StartFunction)(void);
extern void (*ign1EndFunction)(void);
extern void (*ign2StartFunction)(void);
extern void (*ign2EndFunction)(void);
extern void (*ign3StartFunction)(void);
extern void (*ign3EndFunction)(void);
extern void (*ign4StartFunction)(void);
extern void (*ign4EndFunction)(void);
extern void (*ign5StartFunction)(void);
extern void (*ign5EndFunction)(void);
#if IGN_CHANNELS >= 6
extern void (*ign6StartFunction)(void);
extern void (*ign6EndFunction)(void);
#endif
#if IGN_CHANNELS >= 7
extern void (*ign7StartFunction)(void);
extern void (*ign7EndFunction)(void);
#endif
#if IGN_CHANNELS >= 8
extern void (*ign8StartFunction)(void);
extern void (*ign8EndFunction)(void);
#endif
/** @} */

void initialiseSchedulers(void);
void beginInjectorPriming(void);
void setFuelSchedule1(unsigned long timeout, unsigned long duration);
void setFuelSchedule2(unsigned long timeout, unsigned long duration);
void setFuelSchedule3(unsigned long timeout, unsigned long duration);
void setFuelSchedule4(unsigned long timeout, unsigned long duration);
#if INJ_CHANNELS >= 5
void setFuelSchedule5(unsigned long timeout, unsigned long duration);
#endif
#if INJ_CHANNELS >= 6
void setFuelSchedule6(unsigned long timeout, unsigned long duration);
#endif
#if INJ_CHANNELS >= 7
void setFuelSchedule7(unsigned long timeout, unsigned long duration);
#endif
#if INJ_CHANNELS >= 8
void setFuelSchedule8(unsigned long timeout, unsigned long duration);
#endif

void setIgnitionSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setIgnitionSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setIgnitionSchedule3(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setIgnitionSchedule4(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setIgnitionSchedule5(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
#if IGN_CHANNELS >= 6
void setIgnitionSchedule6(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
#endif
#if IGN_CHANNELS >= 7
void setIgnitionSchedule7(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
#endif
#if IGN_CHANNELS >= 8
void setIgnitionSchedule8(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
#endif

void disablePendingFuelSchedule(byte channel);
void disablePendingIgnSchedule(byte channel);

inline void refreshIgnitionSchedule1(unsigned long timeToEnd) __attribute__((always_inline));

//The ARM cores use separate functions for their ISRs
#if defined(ARDUINO_ARCH_STM32) || defined(CORE_TEENSY)
  inline void fuelSchedule1Interrupt();
  inline void fuelSchedule2Interrupt();
  inline void fuelSchedule3Interrupt();
  inline void fuelSchedule4Interrupt();
#if INJ_CHANNELS >= 5
  inline void fuelSchedule5Interrupt();
#endif
#if INJ_CHANNELS >= 6
  inline void fuelSchedule6Interrupt();
#endif
#if INJ_CHANNELS >= 7
  inline void fuelSchedule7Interrupt();
#endif
#if INJ_CHANNELS >= 8
  inline void fuelSchedule8Interrupt();
#endif

  inline void ignitionSchedule1Interrupt();
  inline void ignitionSchedule2Interrupt();
  inline void ignitionSchedule3Interrupt();
  inline void ignitionSchedule4Interrupt();
  inline void ignitionSchedule5Interrupt();
#if IGN_CHANNELS >= 6
  inline void ignitionSchedule6Interrupt();
#endif
#if IGN_CHANNELS >= 7
  inline void ignitionSchedule7Interrupt();
#endif
#if IGN_CHANNELS >= 8
  inline void ignitionSchedule8Interrupt();
#endif
#endif
/** Schedule statuses.
 * - OFF - Schedule turned off and there is no scheduled plan
 * - PENDING - There's a scheduled plan, but is has not started to run yet
 * - STAGED - (???, Not used)
 * - RUNNING - Schedule is currently running
 */
enum ScheduleStatus {OFF, PENDING, STAGED, RUNNING}; //The statuses that a schedule can have

/** Ignition schedule.
 */
struct IgnitionSchedule {
  volatile unsigned long duration;///< Scheduled duration (uS ?)
  volatile ScheduleStatus Status; ///< Schedule status: OFF, PENDING, STAGED, RUNNING
  void (*StartCallback)(void);        ///< Start Callback function for schedule
  void (*EndCallback)(void);          ///< End Callback function for schedule
  volatile unsigned long startTime; /**< The system time (in uS) that the schedule started, used by the overdwell protection in timers.ino */
  volatile COMPARE_TYPE startCompare; ///< The counter value of the timer when this will start
  volatile COMPARE_TYPE endCompare;   ///< The counter value of the timer when this will end

  COMPARE_TYPE nextStartCompare;      ///< Planned start of next schedule (when current schedule is RUNNING)
  COMPARE_TYPE nextEndCompare;        ///< Planned end of next schedule (when current schedule is RUNNING)
  volatile bool hasNextSchedule = false; ///< Enable flag for planned next schedule (when current schedule is RUNNING)
  volatile bool endScheduleSetByDecoder = false;
};


/** Fuel injection schedule.
* Fuel schedules don't use the callback pointers, or the startTime/endScheduleSetByDecoder variables.
* They are removed in this struct to save RAM.
*/
struct FuelSchedule {
  volatile unsigned long duration;///< Scheduled duration (uS ?)
  volatile ScheduleStatus Status; ///< Schedule status: OFF, PENDING, STAGED, RUNNING
  volatile COMPARE_TYPE startCompare; ///< The counter value of the timer when this will start
  volatile COMPARE_TYPE endCompare;   ///< The counter value of the timer when this will end
  void (*pStartFunction)(void);
  void (*pEndFunction)(void);  
  COMPARE_TYPE nextStartCompare;
  COMPARE_TYPE nextEndCompare;
  volatile bool hasNextSchedule = false;
};

extern FuelSchedule fuelSchedule1;
extern FuelSchedule fuelSchedule2;
extern FuelSchedule fuelSchedule3;
extern FuelSchedule fuelSchedule4;
#if INJ_CHANNELS >= 5
extern FuelSchedule fuelSchedule5;
#endif
#if INJ_CHANNELS >= 6
extern FuelSchedule fuelSchedule6;
#endif
#if INJ_CHANNELS >= 7
extern FuelSchedule fuelSchedule7;
#endif
#if INJ_CHANNELS >= 8
extern FuelSchedule fuelSchedule8;
#endif

extern IgnitionSchedule ignitionSchedule1;
extern IgnitionSchedule ignitionSchedule2;
extern IgnitionSchedule ignitionSchedule3;
extern IgnitionSchedule ignitionSchedule4;
extern IgnitionSchedule ignitionSchedule5;
#if IGN_CHANNELS >= 6
extern IgnitionSchedule ignitionSchedule6;
#endif
#if IGN_CHANNELS >= 7
extern IgnitionSchedule ignitionSchedule7;
#endif
#if IGN_CHANNELS >= 8
extern IgnitionSchedule ignitionSchedule8;
#endif

#endif // SCHEDULER_H
