#ifndef PID_v1_h
#define PID_v1_h
#define LIBRARY_VERSION	1.0.0

class PID
{


  public:

  //Constants used in some of the functions below
  #define AUTOMATIC	1
  #define MANUAL	0
  #define DIRECT  0
  #define REVERSE  1

  //commonly used functions **************************************************************************
    PID(long*, long*, long*,        // * constructor.  links the PID to the Input, Output, and 
        byte, byte, byte, byte);     //   Setpoint.  Initial tuning parameters are also set here
	
    void SetMode(int Mode);               // * sets PID to either Manual (0) or Auto (non-0)

    bool Compute();                       // * performs the PID calculation.  it should be
                                          //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetOutputLimits(long, long); //clamps the output to a specific range. 0-255 by default, but
										  //it's likely the user will want to change this depending on
										  //the application
	


  //available but not commonly used functions ********************************************************
    void SetTunings(byte, byte,       // * While most users will set the tunings once in the 
                    byte);         	  //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control
	void SetControllerDirection(byte);	  // * Sets the Direction, or "Action" of the controller. DIRECT
										  //   means the output will increase when error is positive. REVERSE
										  //   means the opposite.  it's very unlikely that this will be needed
										  //   once it is set in the constructor.
    void SetSampleTime(int);              // * sets the frequency, in Milliseconds, with which 
                                          //   the PID calculation is performed.  default is 100
										  
										  
										  
  //Display functions ****************************************************************
	byte GetKp();						  // These functions query the pid for interal values.
	byte GetKi();						  //  they were created mainly for the pid front-end,
	byte GetKd();						  // where it's important to know what is actually 
	int GetMode();						  //  inside the PID.
	int GetDirection();					  //

  private:
	void Initialize();
	
	byte dispKp;				// * we'll hold on to the tuning parameters in user-entered 
	byte dispKi;				//   format for display purposes
	byte dispKd;				//
    
	byte kp;                  // * (P)roportional Tuning Parameter
    byte ki;                  // * (I)ntegral Tuning Parameter
    byte kd;                  // * (D)erivative Tuning Parameter

	int controllerDirection;

    long *myInput;              // * Pointers to the Input, Output, and Setpoint variables
    long *myOutput;             //   This creates a hard link between the variables and the 
    long *mySetpoint;           //   PID, freeing the user from having to constantly tell us
                                  //   what these values are.  with pointers we'll just know.
			  
	unsigned long lastTime;
	long ITerm, lastInput;

	unsigned long SampleTime;
	long outMin, outMax;
	bool inAuto;
};

class integerPID
{


  public:

  //Constants used in some of the functions below
  #define AUTOMATIC	1
  #define MANUAL	0
  #define DIRECT  0
  #define REVERSE  1

  //commonly used functions **************************************************************************
    integerPID(long*, long*, long*,        // * constructor.  links the PID to the Input, Output, and 
        byte, byte, byte, byte);     //   Setpoint.  Initial tuning parameters are also set here
	
    void SetMode(int Mode);               // * sets PID to either Manual (0) or Auto (non-0)

    bool Compute();                       // * performs the PID calculation.  it should be
                                          //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetOutputLimits(long, long); //clamps the output to a specific range. 0-255 by default, but
										  //it's likely the user will want to change this depending on
										  //the application
	


  //available but not commonly used functions ********************************************************
    void SetTunings(byte, byte,       // * While most users will set the tunings once in the 
                    byte);         	  //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control
	void SetControllerDirection(byte);	  // * Sets the Direction, or "Action" of the controller. DIRECT
										  //   means the output will increase when error is positive. REVERSE
										  //   means the opposite.  it's very unlikely that this will be needed
										  //   once it is set in the constructor.
    void SetSampleTime(int);              // * sets the frequency, in Milliseconds, with which 
                                          //   the PID calculation is performed.  default is 100
										  
										  
										  
  //Display functions ****************************************************************
	byte GetKp();						  // These functions query the pid for interal values.
	byte GetKi();						  //  they were created mainly for the pid front-end,
	byte GetKd();						  // where it's important to know what is actually 
	int GetMode();						  //  inside the PID.
	int GetDirection();					  //

  private:
	void Initialize();
	
	byte dispKp;				// * we'll hold on to the tuning parameters in user-entered 
	byte dispKi;				//   format for display purposes
	byte dispKd;				//
    
	int kp;                  // * (P)roportional Tuning Parameter
    int ki;                  // * (I)ntegral Tuning Parameter
    int kd;                  // * (D)erivative Tuning Parameter

	int controllerDirection;

    long *myInput;              // * Pointers to the Input, Output, and Setpoint variables
    long *myOutput;             //   This creates a hard link between the variables and the 
    long *mySetpoint;           //   PID, freeing the user from having to constantly tell us
                                  //   what these values are.  with pointers we'll just know.
			  
	unsigned long lastTime;
	long ITerm, lastInput;

	unsigned long SampleTime;
	long outMin, outMax;
	bool inAuto;
};
#endif

