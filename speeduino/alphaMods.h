byte carSelect = 1; //0 - generic car; 1 - Corolla XRS; 2 - Hyundai Tiburon; 3 - Subaru WRX; 4 - Audi A4 1.8T; 255 - mods disabled
//**
bool ACOn; //whether AC is on
bool AcReq; // AC request
bool highIdleReq; //raises idle in open loop to evade stalling
byte highIdleCount = 0;// counts to wait for normal idle
bool DFCOwait; // waits to enable DFCO
byte DFCOcounter = 0;// counts cycles till dfco
bool vvlOn = false;

//**

void ACControl();
void CELcontrol();
void vvlControl();
void fanControl2();
void highIdleFunc();
void DFCOwaitFunc();
void XRSgaugeCLT();


// Custom pins
 byte pinAC; // pin for AC clutch
 byte pinAcReq;
 byte pinFan2;
 byte pinCEL;
 byte pinVVL;
 byte pinACpress;
 byte pinACtemp;
 byte pinOilPress;
 byte pinCLTgauge;



