#ifndef GLOBALS_H
#define GLOBALS_H
#include <Arduino.h>

#define DIGITALIO_NO_MIX_ANALOGWRITE

const byte ms_version = 20;
const byte signature = 20;
const byte data_structure_version = 2; //This identifies the data structure when reading / writing. 
const byte page_size = 128;

//Handy bitsetting macros
#define BIT_SET(a,b) ((a) |= (b))
#define BIT_CLEAR(a,b) ((a) &= ~(b))
#define BIT_CHECK(var,pos) ((var) & (pos))

//Define bit positions within engine variable
#define BIT_ENGINE_RUN      1          // Engine running
#define BIT_ENGINE_CRANK    (1 << 1)   // Engine cranking
#define BIT_ENGINE_ASE      (1 << 2)   // after start enrichment (ASE)
#define BIT_ENGINE_WARMUP   (1 << 3)   // Engine in warmup
#define BIT_ENGINE_ACC      (1 << 4)   // in TPS acceleration mode
#define BIT_ENGINE_DCC      (1 << 5)   // in deceleration mode
#define BIT_ENGINE_MAP      (1 << 6)   // in MAP acceleration mode
#define BIT_ENGINE_IDLE     (1 << 7)   // idle on

//Define masks for Squirt
#define BIT_SQUIRT_INJ1           1         //inj1 Squirt
#define BIT_SQUIRT_INJ2           (1 << 1)  //inj2 Squirt
#define BIT_SQUIRT_SCHSQRT        (1 << 2)  //Scheduled to squirt
#define BIT_SQUIRT_SQRTING        (1 << 3)  //Squirting
#define BIT_SQUIRT_INJ2SCHED      (1 << 4)
#define BIT_SQUIRT_INJ2SQRT       (1 << 5)  //Injector2 (Schedule2)
#define BIT_SQUIRT_BOOSTCTRLOFF   (1 << 6)  //Squirting Injector 2

#define SIZE_BYTE   8
#define SIZE_INT    16

//Table sizes
#define CALIBRATION_TABLE_SIZE 512
#define CALIBRATION_TEMPERATURE_OFFSET 40 // All temperature measurements are stored offset by 40 degrees. This is so we can use an unsigned byte (0-255) to represent temperature ranges from -40 to 215

#define SERIAL_BUFFER_THRESHOLD 32 // When the serial buffer is filled to greater than this threshold value, the serial processing operations will be performed more urgently in order to avoid it overflowing. Serial buffer is 64 bytes long, so the threshold is set at half this as a reasonable figure

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
  int coolant;
  int cltADC;
  int IAT;
  int iatADC;
  int batADC;
  int O2ADC;
  int dwell;
  byte battery10; //The current BRV in volts (multiplied by 10. Eg 12.5V = 125)
  byte advance;
  byte corrections;
  byte TAEamount; //The amount of accleration enrichment currently being applied
  byte egoCorrection; //The amount of closed loop AFR enrichment currently being applied
  byte wueCorrection; //The amount of closed loop AFR enrichment currently being applied
  byte afrTarget;
  unsigned long TAEEndTime; //The target end time used whenever TAE is turned on
  volatile byte squirt;
  volatile byte spark;
  byte engine;
  unsigned int PW; //In uS
  volatile byte runSecs; //Counter of seconds since cranking commenced (overflows at 255 obviously)
  volatile byte secl; //Continous 
  volatile int loopsPerSecond;
  

  bool isSequential = false; // when true engine squirts and fires sequentially
  bool onSecondRev = false;  // true when engine is on second revolution (cyl1 in intake-compression stroke)

  //Helpful bitwise operations:
  //Useful reference: http://playground.arduino.cc/Code/BitMath
  // y = (x >> n) & 1;    // n=0..15.  stores nth bit of x in y.  y becomes 0 or 1.
  // x &= ~(1 << n);      // forces nth bit of x to be 0.  all other bits left alone.
  // x |= (1 << n);       // forces nth bit of x to be 1.  all other bits left alone.
  
};


//Page 1 of the config - See the ini file for further reference
//This mostly covers off variables that are required for fuel
struct config1 {
  
  byte crankCold; //Cold cranking pulsewidth modifier. This is added to the fuel pulsewidth when cranking under a certain temp threshold (ms)
  byte crankHot; //Warm cranking pulsewidth modifier. This is added to the fuel pulsewidth when cranking (ms)
  byte asePct; //Afterstart enrichment (%)
  byte aseCount; //Afterstart enrichment cycles. This is the number of ignition cycles that the afterstart enrichment % lasts for
  byte wueValues[10]; //Warm up enrichment array (10 bytes)
  byte crankingPct; //Cranking enrichment
  byte unused95;
  byte unused96;
  byte unused97;
  byte taeColdA;
  byte tpsThresh;
  byte taeTime;
  byte tdePct;
  byte unused102;
  byte unused103;
  byte unused104;
  byte unused105;
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
  byte dwellLimit;
  byte unused122;
  byte unused123;
  byte unused124;
  byte unused125;
  byte unused126;
  byte unused127;

  
};

//Page 3 of the config - See the ini file for further reference
//This mostly covers off variables that are required for AFR targets and closed loop
struct config3 {
  
  byte egoAlgorithm : 2;
  byte egoType : 2;
  byte unused : 4;
  
  byte egoKP;
  byte egoKI;
  byte egoKD;
  byte egoTemp; //The temperature above which closed loop functions
  byte egoCount; //The number of ignition cylces per step
  byte egoDelta; //The step size (In %) when using simple algorithm
  byte egoLimit; //Maximum amount the closed loop will vary the fueling
  byte ego_min; //AFR must be above this for closed loop to function
  byte ego_max; //AFR must be below this for closed loop to function
  byte ego_sdelay; //Time in seconds after engine starts that closed loop becomes available
  byte egoRPM; //RPM must be above this for closed loop to function
  byte egoTPSMax; //TPS must be below this for closed loop to function
  byte floodClear; //TPS value that triggers flood clear mode (No fuel whilst cranking)
  byte egoLoadMax; //Load (TPS or MAP) must be below this for closed loop to function
  byte egoLoadMin; //Load (TPS or MAP) must be above this for closed loop to function
  byte unused95; 
  byte unused96;
  byte unused97; 
  byte unused98; 
  byte unused99; 
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
#define pinCoil1 28 //Pin for coil 1
#define pinCoil2 24 //Pin for coil 2
#define pinCoil3 40 //Pin for coil 3
#define pinCoil4 36 //Pin for coil 4
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

// global variables // from speeduino.ino
extern struct statuses currentStatus; // from speeduino.ino
extern struct table3D fuelTable; //8x8 fuel map
extern struct table3D ignitionTable; //8x8 ignition map
extern struct table3D afrTable; //8x8 afr target map
extern struct table2D taeTable; //4 bin TPS Acceleration Enrichment map
extern struct table2D WUETable; //10 bin Warm Up Enrichment map (2D)
extern struct config1 configPage1;
extern struct config2 configPage2;
extern struct config3 configPage3;
extern unsigned long currentLoopTime; //The time the current loop started (uS)
extern unsigned long previousLoopTime; //The time the previous loop started (uS)
extern byte ignitionCount;
extern byte cltCalibrationTable[CALIBRATION_TABLE_SIZE];
extern byte iatCalibrationTable[CALIBRATION_TABLE_SIZE];
extern byte o2CalibrationTable[CALIBRATION_TABLE_SIZE];
extern volatile int toothHistory[512];
extern volatile int toothHistoryIndex;


#endif // GLOBALS_H
