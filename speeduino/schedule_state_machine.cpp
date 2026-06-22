#include "schedule_state_machine.h"
#include "timers.h"
#include "preprocessor.h"

BEGIN_LTO_ALWAYS_INLINE(void) defaultPendingToRunning(Schedule *schedule) {
  schedule->_pStartCallback();
  schedule->_status = RUNNING; //Set the status to be in progress (ie The start callback has been called, but not the end callback)
  SET_COMPARE(schedule->_compare, schedule->_counter + schedule->_duration);
}
END_LTO_INLINE()

BEGIN_LTO_ALWAYS_INLINE(void) defaultRunningToOff(Schedule *schedule) {
  schedule->_pEndCallback();
  schedule->_status = OFF;
}
END_LTO_INLINE()

BEGIN_LTO_ALWAYS_INLINE(void) defaultRunningToPending(Schedule *schedule) {
  schedule->_pEndCallback();
  SET_COMPARE(schedule->_compare, schedule->_nextStartCompare);
  schedule->_status = PENDING;
}
END_LTO_INLINE()

static inline bool hasNextSchedule(const Schedule &schedule) {
  return schedule._status==RUNNING_WITHNEXT;
}

BEGIN_LTO_ALWAYS_INLINE(void) movetoNextState(Schedule &schedule, 
                    scheduleStateTranstionFunc pendingToRunning, 
                    scheduleStateTranstionFunc runningToOff,
                    scheduleStateTranstionFunc runningToPending)
{
  if (schedule._status == PENDING) //Check to see if this schedule is turn on
  {
    pendingToRunning(&schedule);
  }
  else if (isRunning(schedule))
  {
    //If there is a next schedule queued up, activate it
    if(hasNextSchedule(schedule)) {
      runningToPending(&schedule);
    } else {
      runningToOff(&schedule);
    }
  } else {
    // Nothing to do but keep MISRA checker happy
  }
}
END_LTO_INLINE()