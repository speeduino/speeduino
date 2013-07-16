#include <Arduino.h>

byte ms_version = 20;
byte data_structure_version = 1; //This identifies the data structure when reading / writing. 

//The status struct contains the current values for all 'live' variables
//In current version this is 64 bytes
struct statuses {
  volatile boolean hasSync;
  unsigned int RPM;
  byte MAP;
  byte TPS;
  byte VE;
  unsigned long PW; //In uS
  
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
  
  byte crankCold;
  byte crankHot;
  byte asePct;
  byte aseCount;
  byte wueBins[10];
  byte taeBins1;
  byte taeBins2;
  byte taeBins3;
  byte taeBins4;
  byte taeColdA;
  byte tpsThresh;
  byte taeTime;
  byte tdePct;
  byte egoTemp;
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
  int rpmk;

  byte mapType;
  byte twoStroke;
  byte injType;
  byte nCylinders;
  byte cltType;
  byte matType;
  byte nInjectors;
  byte engineType;
  byte egoType;
  byte algorithm; //"Speed Density", "Alpha-N"
  byte baroCorr;
  byte primePulse;
  byte egoRPM;
  byte fastIdleT; //Fast idle temperature
  byte egoSwitch;
  byte taeColdM;
  
};

//Page 2 of the config - See the ini file for further reference
//This mostly covers off variables that are required for ignition
struct config2 {
  
  byte trigerAngle;
  byte FixAng;
  byte Trim;
  byte CrankAng;
  byte IgHold;
  byte Trig_plus;
  byte TrigCrank;
  byte IgInv;
  byte oddfire;
  byte IdleAdv;
  byte IdleAdvTPS;
  byte IdleAdvRPM;
  byte IdleAdvCLT;
  byte IdleDelayTime;
  byte StgCycles;

  byte dwellcont; //Fixed duty dwell control
  byte dwellcrank;
  byte dwellrun;
  
  byte triggerTeeth; //The full count of teeth on the trigger wheel if there were no gaps
  byte triggerMissingTeeth; //The size of the tooth gap (ie number of missing teeth)
  
};
