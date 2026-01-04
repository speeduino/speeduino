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
#include "crankMaths.h"
#include "scheduledIO.h"
#include "decoders.h"

#define USE_IGN_REFRESH
#define IGNITION_REFRESH_THRESHOLD  30 //Time in uS that the refresh functions will check to ensure there is enough time before changing the end compare

/** @brief Initialize all schedulers to the OFF state */
void initialiseIgnitionSchedulers(void);

/** @brief Start the timers that drive schedulers  */
void startIgnitionSchedulers(void);

/** @brief Stop the timers that drive schedulers  */
void stopIgnitionSchedulers(void);

/** @brief Disable an ignition scheduler */
void disableIgnSchedule(uint8_t channel);

/** @brief Disable all ignition schedulers */
void disableAllIgnSchedules(void);

/** @brief Initialize all schedulers to the OFF state */
void initialiseFuelSchedulers(void);

/** @brief Start the timers that drive schedulers  */
void startFuelSchedulers(void);

/** @brief Stop the timers that drive schedulers  */
void stopFuelSchedulers(void);

/** @brief Start fuel system priming the fuel */
void beginInjectorPriming(void);

/** @brief Disable a fuel scheduler  */
void disableFuelSchedule(uint8_t channel);

/** @brief Disable all ignition schedulers */
void disableAllFuelSchedules(void);

/** \enum ScheduleStatus
 * @brief The current state of a schedule
 * */
enum ScheduleStatus {
  // We use powers of 2 so we can check multiple states with a single bitwise AND

  /** Not running */
  OFF              = 0b00000000U, 
  /** The delay phase of the schedule is active */
  PENDING          = 0b00000001U,
  /** The schedule action is running */
  RUNNING          = 0b00000010U,
  /** The schedule is running, with a next schedule queued up */
  RUNNING_WITHNEXT = 0b00000100U,
}; 

/**
 * @brief A schedule for a single output channel. 
 * 
 * @details
 * @par A schedule consists of 3 independent parts:
 * - an action that can be started and stopped. E.g. charge ignition coil or injection pulse
 * - a delay until the action is started
 * - a duration until the action is stopped
 * 
 * I.e.\n 
 * \code 
 *   <--------------- Delay ---------------><---- Duration ---->
 *                                          ^                  ^
 *                              Action starts                  Action ends
 * \endcode
 * 
 * @par Timers are modelled as registers
 * Once set, Schedule instances are usually driven externally by a timer
 * ISR calling moveToNextState() periodically to update the schedule states.
 */
struct Schedule {
  // Deduce the real types of the counter and compare registers.
  // COMPARE_TYPE is NOT the same - it's just an integer type wide enough to
  // store 16-bit counter/compare calculation results.
  /** @brief The type of a timer counter register (this varies between platforms) */
  using counter_t = decltype(FUEL1_COUNTER /* <-- Arbitrary choice of macro, assumes all have the same type */);
  /** @brief The type of a timer compare register (this varies between platforms) */
  using compare_t = decltype(FUEL1_COMPARE /* <-- Arbitrary choice of macro, assumes all have the same type */);

  /**
   * @brief Construct a new Schedule object
   * 
   * @param counter A <b>reference</b> to the timer counter
   * @param compare A <b>reference</b> to the timer comparator
   */
  constexpr Schedule(counter_t &counter, compare_t &compare)
    : _counter(counter)
    , _compare(compare) 
  {
  }  

  /**
   * @brief Scheduled duration (timer ticks) 
   *
   * This captures the duration of the *next* interval to be scheduled. I.e.
   *  * Status==PENDING: this is the duration that will be used when the schedule moves to the RUNNING state 
   *  * Status==RUNNING_WITHNEXT: this is the duration that will be used after the current schedule finishes and the queued up scheduled starts 
   */
  volatile COMPARE_TYPE duration = 0U;
  volatile ScheduleStatus Status = OFF;  ///< Schedule status: OFF, PENDING, STAGED, RUNNING
  voidVoidCallback pStartCallback = &nullCallback; ///< Start Callback function for schedule
  voidVoidCallback pEndCallback = &nullCallback;   ///< End Callback function for schedule
  COMPARE_TYPE nextStartCompare = 0U;   ///< Planned start of next schedule (when current schedule is RUNNING)
  
  counter_t &_counter;       ///< **Reference** to the counter register. E.g. TCNT3
  compare_t &_compare;       ///< **Reference**to the compare register. E.g. OCR3A
};

/**
 * @brief Is the schedule running?
 * I.e. the action has started, but not finished. E.g. injector is open
 */
static inline bool isRunning(const Schedule &schedule) {
  // Using flags and bitwise AND (&) to check multiple states is much quicker
  // than a logical or (||) (one less branch & 30% less instructions)
  static constexpr uint8_t flags = RUNNING | RUNNING_WITHNEXT;
  return ((uint8_t)schedule.Status & flags)!=0U;
}

/**
 * @brief Set the schedule callbacks. I.e the functions called when the action
 * needs to start & stop
 * 
 * @param schedule Schedule to modify
 * @param pStartCallback The new start callback - called when the schedule switches to RUNNING status
 * @param pEndCallback The new end callback - called when the schedule switches from RUNNING to OFF status
 */
void setCallbacks(Schedule &schedule, voidVoidCallback pStartCallback, voidVoidCallback pEndCallback);

/**
 * @brief Set the schedule action to run for a certain duration in the future
 * 
 * @param schedule Schedule to modify
 * @param delay Delay until the action starts (µS)
 * @param duration Action duration (µS)
 * @param allowQueuedSchedule true to allow a schedule to be queued up if one is currently running; false otherwise
 */
void setSchedule(Schedule &schedule, uint32_t delay, uint16_t duration, bool allowQueuedSchedule);

/** @brief Disable the schedule */
void disableSchedule(Schedule &schedule);

/** @brief An ignition schedule.
 *
 * Goal is to fire the spark as close to the requested angle as possible.
 * 
 * \code 
 *   <--------------- Delay ---------------><---- Charge Coil ---->
 *                                                                ^
 *                                                              Spark
 * \endcode
 * 
 * Terminology: dwell is the period when the ignition system applies an electric
 * current to the ignition coil's primary winding in order to charge up the coil
 * so it can generate a spark. 
 * 
 * Note that dwell times use uint16_t & therefore maximum dwell is 65.535ms. 
 * This limit is imposed elsewhere in Speeduino also.
 */
struct IgnitionSchedule : public Schedule {

  using Schedule::Schedule;

  volatile uint32_t _startTime;///< The system time (in uS) that the schedule started, used by the overdwell protection in timers.ino
  int16_t chargeAngle;        ///< Angle the coil should begin charging.
  int16_t dischargeAngle;     ///< Angle the coil should discharge at. I.e. spark.
  int16_t channelDegrees;     ///< The number of crank degrees until cylinder is at TDC  
};

/// @cond 
// Private functions - not for use external to the scheduler code

/**
 * @brief Set the ignition schedule action (charge & fire a coil) to run for a certain duration in the future
 * 
 * @param schedule Schedule to modify
 * @param delay Delay until the coil begins charging (µS)
 * @param duration Dwell time (µS)
 */
static inline void _setIgnitionScheduleDuration(IgnitionSchedule &schedule, uint32_t delay, uint16_t duration) 
{
  // Only queue up the next schedule if the maximum time between sparks (Based on CRANK_ANGLE_MAX_IGN) is less than the max timer period
  setSchedule(schedule, delay, duration, angleToTimeMicroSecPerDegree((uint16_t)CRANK_ANGLE_MAX_IGN) < MAX_TIMER_PERIOD);
}
/// @endcond

/**
 * @brief Check that no ignition channel has been charging the coil for too long
 * 
 * The over dwell protection system runs independently of the standard ignition 
 * schedules and monitors the time that each ignition output has been active. If the 
 * active time exceeds this amount, the output will be ended to prevent damage to coils.
 * 
 * @note Must be called once per millisecond by an **external** timer.
 */
void applyOverDwellProtection(void);

/**
 * @brief Shared ignition schedule timer ISR *implementation*. Should be called by the actual ignition timer ISRs
 * (as timed interrupts) when either the start time or the duration time are reached. See @ref schedule-state-machine
 * 
 * @param schedule The ignition schedule to move to the next state
 */
void moveToNextState(IgnitionSchedule &schedule);

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


/** Fuel injection schedule.
* Fuel schedules don't use the callback pointers, or the _startTime/endScheduleSetByDecoder variables.
* They are removed in this struct to save RAM.
*/
struct FuelSchedule : public Schedule {

  using Schedule::Schedule;

};

/**
 * @brief Set the fuel schedule action (open & close an injector) to run for a certain duration in the future
 * 
 * @param schedule Schedule to modify
 * @param delay Delay until the injector opens (µS)
 * @param duration Injector open time (µS)
 */
static inline void setFuelSchedule(FuelSchedule &schedule, uint32_t delay, uint16_t duration) 
{
  // Only queue up the next schedule if the maximum time between squirts (Based on CRANK_ANGLE_MAX_INJ) is less than the max timer period
  setSchedule(schedule, delay, duration, angleToTimeMicroSecPerDegree((uint16_t)CRANK_ANGLE_MAX_INJ) < MAX_TIMER_PERIOD);
}

/**
 * @brief Shared fuel schedule timer ISR implementation. Should be called by the actual timer ISRs
 * (as timed interrupts) when either the start time or the duration time are reached. See @ref schedule-state-machine
 * 
 * @param schedule The fuel schedule to move to the next state
 */
void moveToNextState(FuelSchedule &schedule);

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

void changeHalfToFullSync(const config2 &page2, statuses &current);
void changeFullToHalfSync(const config2 &page2, const config4 &page4, statuses &current);

void calculateIgnitionAngles(const config2 &page2, const config4 &page4, const decoder_status_t &decoderStatus, statuses &current);

#endif // SCHEDULER_H
