#include "statuses.h"
#include "atomic.h"

void statuses::setRpm(uint16_t rpm)
{
  ATOMIC()
  {
    this->RPM = rpm;
    this->RPMdiv100 = div100(rpm);
    this->longRPM = rpm;
  }
}