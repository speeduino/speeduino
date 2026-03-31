#include "statuses.h"
#include "atomic.h"

void setRpm(statuses &status, uint16_t rpm)
{
  ATOMIC()
  {
    status.RPM = rpm;
    status.RPMdiv100 = div100(rpm);
    status.longRPM = rpm;
  }
}