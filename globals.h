#include <Arduino.h>

const byte ms_version = 20;
const byte signature = 20;
const byte data_structure_version = 2; //This identifies the data structure when reading / writing. 
const byte page_size = 128;

//Handy bitsetting macros
#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define BIT_CHECK(var,pos) ((var) & (1<<(pos)))

//Define bit positions within engine virable
#define BIT_ENGINE_RUN      0     // Engine running
#define BIT_ENGINE_CRANK    1   // Engine cranking
#define BIT_ENGINE_ASE      2    // after start enrichment (ASE)
#define BIT_ENGINE_WARMUP   3  // Engine in warmup
#define BIT_ENGINE_TPS      4    // in TPS acceleration mode
#define BIT_ENGINE_ACC      5    // in deceleration mode
#define BIT_ENGINE_MAP      6    // in MAP acceleration mode
#define BIT_ENGINE_IDLE     7  // idle on

//Define masks for Squirt
#define BIT_SQUIRT_INJ1           0  //inj1 Squirt
#define BIT_SQUIRT_INJ2           1  //inj2 Squirt
#define BIT_SQUIRT_SCHSQRT        2  //Scheduled to squirt
#define BIT_SQUIRT_SQRTING        3  //Squirting
#define BIT_SQUIRT_INJ2SCHED      4
#define BIT_SQUIRT_INJ2SQRT       5  //Injector2 (Schedule2)
#define BIT_SQUIRT_BOOSTCTRLOFF   6  //Squirting Injector 2

#define SIZE_BYTE   8
#define SIZE_INT    16

//The status struct contains the current values for all 'live' variables
//In current version this is 64 bytes
struct statuses {
  volatile boolean hasSync;
  unsigned int RPM;
  byte MAP;
  byte TPS; //The current TPS reading (0% - 100%)
  byte TPSlast; //The previous TPS reading
  byte tpsADC; //0-255 byte representation of the TPS
  byte tpsDOT;
  byte VE;
  byte O2;
  byte coolant;
  byte cltADC;
  byte IAT;
  byte iatADC;
  byte batADC;
  byte advance;
  byte corrections;
  volatile byte squirt;
  byte engine;
  unsigned long PW; //In uS
  volatile byte runSecs; //Counter of seconds since cranking commenced (overflows at 255 obviously)
  volatile byte secl; //Continous 
  volatile int loopsPerSecond;
  
  //Helpful bitwise operations:
  //Useful reference: http://playground.arduino.cc/Code/BitMath
  // y = (x >> n) & 1;    // n=0..15.  stores nth bit of x in y.  y becomes 0 or 1.
  // x &= ~(1 << n);      // forces nth bit of x to be 0.  all other bits left alone.
  // x |= (1 << n);       // forces nth bit of x to be 1.  all other bits left alone.
  
};


//Page 1 of the config - See the ini file for further reference
//This mostly covers off variables that are required for fuel
struct config1 {
  /*
  byte engineCylinders; 1 // May support more than 1 cyl later. Always will assume 1 injector per cylinder. 
  byte engineInjectorSize; 80 // In cc/min
  byte engineStoich; 14.7 // Stoichiometric ratio of fuel used
  byte engineStrokes; 4 //Can be 2 stroke or 4 stroke, any other value will cause problems
  byte engineDwell; 3000 //The spark dwell time in uS
  */
  
  byte crankCold; //Cold cranking pulsewidth modifier. This is added to the fuel pulsewidth when cranking under a certain temp threshold (ms)
  byte crankHot; //Warm cranking pulsewidth modifier. This is added to the fuel pulsewidth when cranking (ms)
  byte asePct; //Afterstart enrichment (%)
  byte aseCount; //Afterstart enrichment cycles. This is the number of ignition cycles that the afterstart enrichment % lasts for
  byte wueValues[10]; //Warm up enrichment array (10 bytes)
  byte taeBins[4]; //TPS based acceleration enrichment bin 1 of 4 (ms)
  byte taeColdA;
  byte tpsThresh;
  byte taeTime;
  byte tdePct;
  byte egoTemp; //The temperature at which the EGO / O2 sensor values start being used (Degrees)
  byte egoCount;
  byte egoDelta;
  byte egoLimit;
  byte reqFuel;
  byte divider;
  byte alternate;
  byte injOpen;
  byte injOCfuel;
  byte injPwmP;
  byte injPwmT;
  byte battFac; //Whether to compensate pulsewidth for battery voltage (ms/v)
  int rpmk; //2 bytes
//36
  //config1 in ini
  byte mapType : 2;
  byte strokes : 1;
  byte injType : 1;
  byte nCylinders : 4; //Number of cylinders

  //config2 in ini  
  byte cltType : 2;
  byte matType : 2;
  byte nInjectors : 4; //Number of injectors
  

  //config3 in ini
  byte engineType : 1;
  byte egoType : 1;
  byte algorithm : 1; //"Speed Density", "Alpha-N"
  byte baroCorr : 1;
  
  byte primePulse;
  byte egoRPM;
  byte fastIdleT; //Fast idle temperature
  byte egoSwitch;
  byte taeColdM;
  byte tpsMin;
  byte tpsMax;
  byte unused1;

  //48
  
};

//Page 2 of the config - See the ini file for further reference
//This mostly covers off variables that are required for ignition
struct config2 {
  
  byte triggerAngle;
  byte FixAng;
  byte Trim;
  byte CrankAng;
  byte IgHold;
  
  byte Trig_plus : 2;
  byte TrigCrank : 1;
  byte IgInv : 1;
  byte oddfire : 4;
  
  byte IdleAdv;
  byte IdleAdvTPS;
  byte IdleAdvRPM;
  byte IdleAdvCLT; //The temperature below which the idle is advanced
  byte IdleDelayTime;
  byte StgCycles; //The number of initial cycles before the ignition should fire when first cranking
  byte dwellCont; //Fixed duty dwell control
  byte dwellCrank; //Dwell time whilst cranking
  byte dwellRun; //Dwell time whilst running 
  byte triggerTeeth; //The full count of teeth on the trigger wheel if there were no gaps
  byte triggerMissingTeeth; //The size of the tooth gap (ie number of missing teeth)
  byte crankRPM; //RPM below which the engine is considered to be cranking
  byte floodClear; //TPS value that triggers flood clear mode (No fuel whilst cranking)
  byte SoftRevLim; //Soft rev limit (RPM/100)
  byte SoftLimRetard; //Amount soft limit retards (degrees)
  byte SoftLimMax; //Time the soft limit can run
  byte HardRevLim; //Hard rev limit (RPM/100)
  byte taeBins[4]; //TPS based acceleration enrichment bins (%/s)
  byte taeValues[4]; //TPS based acceleration enrichment rates (% to add)
  byte wueBins[10]; //Warmup Enrichment bins (Values are in configTable1)
  byte unused115;
  byte unused116;
  byte unused117;
  byte unused118;
  byte unused119;
  byte unused120;
  byte unused121;
  byte unused122;
  byte unused123;
  byte unused124;
  byte unused125;
  byte unused126;
  byte unused127;

  
};

//Pin mappings as per the v0.2 shield
#define pinInjector1 8 //Output pin injector 1 is on
#define pinInjector2 9 //Output pin injector 2 is on
#define pinInjector3 10 //Output pin injector 3 is on
#define pinInjector4 11 //Output pin injector 4 is on
#define pinCoil1 24 //Pin for coil 1
#define pinCoil2 28 //Pin for coil 2
#define pinCoil3 36 //Pin for coil 3
#define pinCoil4 40 //Pin for coil 4
#define pinTrigger 20 //The CAS pin
#define pinTrigger2 21 //The Cam Sensor pin
#define pinTPS A2 //TPS input pin
#define pinMAP A3 //MAP sensor pin
#define pinIAT A0 //IAT sensor pin
#define pinCLT A1 //CLS sensor pin
#define pinO2 A8 //O2 Sensor pin
#define pinBat A4 //O2 Sensor pin

//Pin mappings as per the v0.1 shield
/*
#define pinInjector1 8 //Output pin injector 1 is on
#define pinInjector2 9 //Output pin injector 2 is on
#define pinInjector3 11 //Output pin injector 3 is on
#define pinInjector4 10 //Output pin injector 4 is on
#define pinCoil1 6 //Pin for coil 1
#define pinCoil2 7 //Pin for coil 2
#define pinCoil3 12 //Pin for coil 3
#define pinCoil4 13 //Pin for coil 4
#define pinTrigger 2 //The CAS pin
#define pinTPS A0 //TPS input pin
#define pinMAP A1 //MAP sensor pin
#define pinIAT A2 //IAT sensor pin
#define pinCLT A3 //CLS sensor pin
#define pinO2 A4 //O2 Sensor pin
*/
