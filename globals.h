#include <Arduino.h>

const byte ms_version = 20;
const byte signature = 20;
const byte data_structure_version = 2; //This identifies the data structure when reading / writing. 
const byte page_size = 128;

//Handy bitsetting macros
#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))

//Define masks for engine
#define ENGINE_RUN 1     // Engine running
#define ENGINE_CRANK 2   // Engine cranking
#define ENGINE_ASE  4    // after start enrichment (ASE)
#define ENGINE_WARMUP 8  // Engine in warmup
#define ENGINE_TPS 16    // in TPS acceleration mode
#define ENGINE_ACC 32    // in deceleration mode
#define ENGINE_MAP 64    // in MAP acceleration mode
#define ENGINE_IDLE 128  // idle on

//Define masks for Squirt
#define SQUIRT_INJ1           1  //inj1 Squirt
#define SQUIRT_INJ2           2  //inj2 Squirt
#define SQUIRT_SCHSQRT        4  //Scheduled to squirt
#define SQUIRT_SQRTING        8  //Squirting
#define SQUIRT_INJ2SCHED      16
#define SQUIRT_INJ2SQRT      32  //Injector2 (Schedule2)
#define SQUIRT_BOOSTCTRLOFF  64  //Squirting Injector 2


//The status struct contains the current values for all 'live' variables
//In current version this is 64 bytes
struct statuses {
  volatile boolean hasSync;
  unsigned int RPM;
  byte MAP;
  byte TPS; //The current TPS reading (0% - 100%)
  byte TPSlast; //The previous TPS reading
  byte tpsADC; //0-255 byte representation of the TPS
  byte VE;
  byte O2;
  byte advance;
  volatile byte squirt;
  byte engine;
  unsigned long PW; //In uS
  byte runSecs;
  volatile unsigned int loopsPerSecond;
  
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
  byte wueBins[10]; //Warm up enrichment array (10 bytes)
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
  byte IdleAdvCLT;
  byte IdleDelayTime;
  byte StgCycles;
  byte dwellCont; //Fixed duty dwell control
  byte dwellCrank;
  byte dwellRun;
  byte triggerTeeth; //The full count of teeth on the trigger wheel if there were no gaps
  byte triggerMissingTeeth; //The size of the tooth gap (ie number of missing teeth)
  byte crankRPM; //RPM below which the engine is considered to be cranking
  byte floodClear; //TPS value that triggers flood clear mode (No fuel whilst cranking)
  byte SoftRevLim; //Soft rev limit (RPM/100)
  byte SoftLimRetard; //Amount soft limit retards (degrees)
  byte SoftLimMax; //Time the soft limit can run
  byte HardRevLim; //Hard rev limit (RPM/100)
  byte taeBins[4]; //TPS based acceleration enrichment bins (%/s)
  int taeRates[4]; //TPS based acceleration enrichment rates (% to add)
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
