
#define HARD_REV_FIXED    1
#define HARD_REV_COOLANT  2

bool afrProtectCountEnabled = false;
unsigned long afrProtectCount;

byte checkEngineProtect();
byte checkRevLimit();
byte checkBoostLimit();
byte checkOilPressureLimit();
byte checkAFRLimit();