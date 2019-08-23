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
void setFuelSchedule1(unsigned long timeout, unsigned long duration);
void setFuelSchedule2(unsigned long timeout, unsigned long duration);
void setFuelSchedule3(unsigned long timeout, unsigned long duration);
void setFuelSchedule4(unsigned long timeout, unsigned long duration);
void setFuelSchedule5(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)()); //Schedule 5 remains a special case for now due to the way it's implemented 
//void setFuelSchedule5(unsigned long timeout, unsigned long duration);
void setFuelSchedule6(unsigned long timeout, unsigned long duration);
void setFuelSchedule7(unsigned long timeout, unsigned long duration);
void setFuelSchedule8(unsigned long timeout, unsigned long duration);
void setIgnitionSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setIgnitionSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setIgnitionSchedule3(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setIgnitionSchedule4(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setIgnitionSchedule5(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setIgnitionSchedule6(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setIgnitionSchedule7(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());
void setIgnitionSchedule8(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)());

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

enum ScheduleStatus {OFF, PENDING, STAGED, RUNNING}; //The 3 statuses that a schedule can have

struct Schedule {
  volatile unsigned long duration;
  volatile ScheduleStatus Status;
  volatile byte schedulesSet; //A counter of how many times the schedule has been set
  void (*StartCallback)(); //Start Callback function for schedule
  void (*EndCallback)(); //Start Callback function for schedule
  volatile unsigned long startTime; /**< The system time (in uS) that the schedule started, used by the overdwell protection in timers.ino */
  volatile COMPARE_TYPE startCompare; //The counter value of the timer when this will start
  volatile COMPARE_TYPE endCompare;

  unsigned int nextStartCompare;
  unsigned int nextEndCompare;
  volatile bool hasNextSchedule = false;
  volatile bool endScheduleSetByDecoder = false;
#if defined(CORE_AVR) || defined(CORE_TEENSY40)
  volatile uint16_t * counter;
  volatile uint16_t * compare;
#else
  volatile uint32_t * counter;
  volatile uint32_t * compare;
#endif
};

//volatile Schedule *timer3Aqueue[4];
//Schedule *timer3Bqueue[4];
//Schedule *timer3Cqueue[4];

Schedule fuelSchedule1;
Schedule fuelSchedule2;
Schedule fuelSchedule3;
Schedule fuelSchedule4;
Schedule fuelSchedule5;
Schedule fuelSchedule6;
Schedule fuelSchedule7;
Schedule fuelSchedule8;

Schedule ignitionSchedule1;
Schedule ignitionSchedule2;
Schedule ignitionSchedule3;
Schedule ignitionSchedule4;
Schedule ignitionSchedule5;
Schedule ignitionSchedule6;
Schedule ignitionSchedule7;
Schedule ignitionSchedule8;

Schedule nullSchedule; //This is placed at the end of the queue. It's status will always be set to OFF and hence will never perform any action within an ISR

static inline unsigned int setQueue(volatile Schedule *queue[], Schedule *schedule1, Schedule *schedule2, unsigned int CNT)
{
  //Create an array of all the upcoming targets, relative to the current count on the timer
  unsigned int tmpQueue[4];

  //Set the initial queue state. This order matches the tmpQueue order
  if(schedule1->Status == OFF)
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

  if(schedule2->Status == OFF)
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
  queue[3] = &nullSchedule;

  unsigned int returnCompare;
  if( queue[0]->Status == PENDING ) { returnCompare = queue[0]->startCompare; }
  else { returnCompare = queue[0]->endCompare; }

  return returnCompare;
}


#endif // SCHEDULER_H
