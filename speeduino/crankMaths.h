#define CRANKMATH_METHOD_INTERVAL_RPM      0
#define CRANKMATH_METHOD_INTERVAL_TOOTH    1
#define CRANKMATH_METHOD_ALPHA_BETA        2
#define CRANKMATH_METHOD_2ND_DERIVATIVE    3

unsigned long angleToTime(int16_t angle);
uint16_t timeToAngle(unsigned long time);