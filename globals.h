#include <Arduino.h>

byte ms_version = 20;

//The status struct contains the current values for all 'live' variables
struct statuses {
  volatile boolean hasSync;
  unsigned int RPM;
  byte MAP;
  byte TPS;
  byte VE;
  unsigned long PW; //In uS
  
};
