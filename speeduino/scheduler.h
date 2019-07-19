/*
This scheduler is designed to maintain 2 schedules for use by the fuel and ignition systems.
It functions by waiting for the overflow vectors from each of the timers in use to overflow, which triggers an interrupt

//Technical
Currently I am prescaling the 16-bit timers to 256 for injection and 64 for ignition. This means that the counter increments every 16us (injection) / 4uS (ignition) and will overflow every 1048576uS
Max Period = (Prescale)*(1/Frequency)*(2^17)
(See http://playground.arduino.cc/code/timer1)
This means that the precision of the scheduler is 16uS (+/- 8uS of target) for fuel and 4uS (+/- 2uS) for ignition

/Features
This differs from most other schedulers in that its calls are non-recurring (IE You schedule an event at a certain time and once it has occurred, it will not reoccur unless you explicitely ask for it)
Each timer can have only 1 callback associated with it at any given time. If you call the setCallback function a 2nd time, the original schedule will be overwritten and not occur

Timer identification
The Arduino timer3 is used for schedule 1
The Arduino timer4 is used for schedule 2
Both of these are 16-bit timers (ie count to 65536)
See page 136 of the processors datasheet: http://www.atmel.com/Images/doc2549.pdf

256 prescale gives tick every 16uS
256 prescale gives overflow every 1048576uS (This means maximum wait time is 1.0485 seconds)

*/
#ifndef SCHEDULER_H
#define SCHEDULER_H

#define USE_IGN_REFRESH
#define IGNITION_REFRESH_THRESHOLD  30 //Time in uS that the refresh functions will check to ensure there is enough time before changing the end compare

void initialiseSchedulers();

static inline void refreshIgnitionSchedule1(unsigned long timeToEnd) __attribute__((always_inline));

//The ARM cores use seprate functions for their ISRs
#if defined(CORE_STM32_OFFICIAL) || defined(CORE_STM32_GENERIC) || defined(CORE_TEENSY)
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

class Schedule {

#if defined(CORE_AVR)
  typedef uint16_t DataRegister;
  typedef uint8_t EnableRegister;
#else
  typedef uint32_t DataRegister;
  typedef uint32_t EnableRegister;
#endif

public:
  enum ScheduleStatus {OFF, PENDING, STAGED, RUNNING}; //The 3 statuses that a schedule can have

  volatile unsigned long duration;
  volatile ScheduleStatus Status = OFF;
  volatile byte schedulesSet = 0; //A counter of how many times the schedule has been set
  void (*StartCallback)() = nullptr;//Start Callback function for schedule
  void (*EndCallback)() = nullptr; //Start Callback function for schedule
  volatile unsigned long startTime; /**< The system time (in uS) that the schedule started, used by the overdwell protection in timers.ino */
  volatile uint16_t startCompare; //The counter value of the timer when this will start
  volatile uint16_t endCompare;

  unsigned int nextStartCompare;
  unsigned int nextDuration;
  volatile bool hasNextSchedule = false;
  volatile bool endScheduleSetByDecoder = false;

  struct {
    volatile DataRegister& counter;
    volatile DataRegister& compare;
    volatile EnableRegister& enableRegister;
    const EnableRegister enableBitMask;
  } timer;

public:
  Schedule(volatile DataRegister& _counter, volatile DataRegister& _compare, volatile EnableRegister& _enable, const EnableRegister _bitMask)
      : timer({_counter, _compare, _enable, _bitMask})
  {
    // Nothing else to do here
  }
  void enable() { timer.enableRegister |= timer.enableBitMask; }
  void disable() { timer.enableRegister &= ~timer.enableBitMask; }
  void setSchedule(void (*_startCallback)(), uint32_t _timeout, uint32_t _duration, void(*_endCallBack)());
  void interrupt();
};

//volatile Schedule *timer3Aqueue[4];
//Schedule *timer3Bqueue[4];
//Schedule *timer3Cqueue[4];

Schedule fuelSchedule1(FUEL1_COUNTER, FUEL1_COMPARE, FUEL1_TIMER_ENABLE, FUEL1_ENABLE_BITMASK);
Schedule fuelSchedule2(FUEL2_COUNTER, FUEL2_COMPARE, FUEL2_TIMER_ENABLE, FUEL2_ENABLE_BITMASK);
Schedule fuelSchedule3(FUEL3_COUNTER, FUEL3_COMPARE, FUEL3_TIMER_ENABLE, FUEL3_ENABLE_BITMASK);
Schedule fuelSchedule4(FUEL4_COUNTER, FUEL4_COMPARE, FUEL4_TIMER_ENABLE, FUEL4_ENABLE_BITMASK);
Schedule fuelSchedule5(FUEL5_COUNTER, FUEL5_COMPARE, FUEL5_TIMER_ENABLE, FUEL5_ENABLE_BITMASK);
Schedule fuelSchedule6(FUEL6_COUNTER, FUEL6_COMPARE, FUEL6_TIMER_ENABLE, FUEL6_ENABLE_BITMASK);
Schedule fuelSchedule7(FUEL7_COUNTER, FUEL7_COMPARE, FUEL7_TIMER_ENABLE, FUEL7_ENABLE_BITMASK);
Schedule fuelSchedule8(FUEL8_COUNTER, FUEL8_COMPARE, FUEL8_TIMER_ENABLE, FUEL8_ENABLE_BITMASK);

Schedule ignitionSchedule1(IGN1_COUNTER, IGN1_COMPARE, IGN1_TIMER_ENABLE, IGN1_ENABLE_BITMASK);
Schedule ignitionSchedule2(IGN2_COUNTER, IGN2_COMPARE, IGN2_TIMER_ENABLE, IGN2_ENABLE_BITMASK);
Schedule ignitionSchedule3(IGN3_COUNTER, IGN3_COMPARE, IGN3_TIMER_ENABLE, IGN3_ENABLE_BITMASK);
Schedule ignitionSchedule4(IGN4_COUNTER, IGN4_COMPARE, IGN4_TIMER_ENABLE, IGN4_ENABLE_BITMASK);
Schedule ignitionSchedule5(IGN5_COUNTER, IGN5_COMPARE, IGN5_TIMER_ENABLE, IGN5_ENABLE_BITMASK);
Schedule ignitionSchedule6(IGN6_COUNTER, IGN6_COMPARE, IGN6_TIMER_ENABLE, IGN6_ENABLE_BITMASK);
Schedule ignitionSchedule7(IGN7_COUNTER, IGN7_COMPARE, IGN7_TIMER_ENABLE, IGN7_ENABLE_BITMASK);
Schedule ignitionSchedule8(IGN8_COUNTER, IGN8_COMPARE, IGN8_TIMER_ENABLE, IGN8_ENABLE_BITMASK);

//Schedule nullSchedule; //This is placed at the end of the queue. It's status will always be set to OFF and hence will never perform any action within an ISR

static inline unsigned int setQueue(volatile Schedule *queue[], Schedule *schedule1, Schedule *schedule2, unsigned int CNT)
{
  //Create an array of all the upcoming targets, relative to the current count on the timer
  unsigned int tmpQueue[4];

  //Set the initial queue state. This order matches the tmpQueue order
  if(schedule1->Status == Schedule::OFF)
  {
    queue[0] = schedule2;
    queue[1] = schedule2;
    tmpQueue[0] = schedule2->startCompare - CNT;
    tmpQueue[1] = schedule2->endCompare - CNT;
  }
  else
  {
    queue[0] = schedule1;
    queue[1] = schedule1;
    tmpQueue[0] = schedule1->startCompare - CNT;
    tmpQueue[1] = schedule1->endCompare - CNT;
  }

  if(schedule2->Status == Schedule::OFF)
  {
    queue[2] = schedule1;
    queue[3] = schedule1;
    tmpQueue[2] = schedule1->startCompare - CNT;
    tmpQueue[3] = schedule1->endCompare - CNT;
  }
  else
  {
    queue[2] = schedule2;
    queue[3] = schedule2;
    tmpQueue[2] = schedule2->startCompare - CNT;
    tmpQueue[3] = schedule2->endCompare - CNT;
  }


  //Sort the queues. Both queues are kept in sync.
  //This implementes a sorting networking based on the Bose-Nelson sorting network
  //See: http://pages.ripco.net/~jgamble/nw.html
  #define SWAP(x,y) if(tmpQueue[y] < tmpQueue[x]) { unsigned int tmp = tmpQueue[x]; tmpQueue[x] = tmpQueue[y]; tmpQueue[y] = tmp; volatile Schedule *tmpS = queue[x]; queue[x] = queue[y]; queue[y] = tmpS; }
  //SWAP(0, 1); //Likely not needed
  //SWAP(2, 3); //Likely not needed
  SWAP(0, 2);
  SWAP(1, 3);
  SWAP(1, 2);

  //Return the next compare time in the queue
  return tmpQueue[0] + CNT; //Return the
}

/*
 * Moves all the Schedules in a queue forward one position.
 * The current item (0) is discarded
 * The final queue slot is set to nullSchedule to indicate that no action should be taken
 */
static inline unsigned int popQueue(volatile Schedule *queue[])
{
  queue[0] = queue[1];
  queue[1] = queue[2];
  queue[2] = queue[3];
  queue[3] = nullptr;

  unsigned int returnCompare;
  if( queue[0]->Status == Schedule::PENDING ) { returnCompare = queue[0]->startCompare; }
  else { returnCompare = queue[0]->endCompare; }

  return returnCompare;
}


#endif // SCHEDULER_H
