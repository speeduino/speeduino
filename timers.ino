


void initialiseTimers() 
{  
   //Configure Timer2 for our low-freq interrupt code.
   TCCR2B = 0x00;          //Disbale Timer2 while we set it up
   TCNT2  = 99;           //Preload timer2 with 100 cycles, leaving 156 till overflow.
   TIFR2  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
   TIMSK2 = 0x01;          //Timer2 Set Overflow Interrupt enabled.
   TCCR2A = 0x00;          //Timer2 Control Reg A: Wave Gen Mode normal
   TCCR2B = ((1 << CS10) | (1 << CS11) | (1 << CS12));   //Timer2 Set Prescaler to 5 (101), 1024 mode. 
}


//Timer2 Overflow Interrupt Vector, called when the timer overflows.
//SHOULD execute every ~10ms.
ISR(TIMER2_OVF_vect) 
{
  
  //Increment Loop Counters
  loopGen++;
  loopSec++;
  
  
  //Loop executed every 250ms loop (10ms x 25 = 250ms)
  if (loopGen == 25) 
  {
    loopGen = 0; //Reset Counter.
    //INSERT 250ms Code here. 
  }
  
  //Loop executed every 1 second (10ms x 100 = 1000ms)
  if (loopSec == 100) 
  {
    loopSec = 0; //Reset counter.

    //If the engine is running or cranking, we need ot update the run time counter.
    if (((currentStatus.engine & ENGINE_RUN) || (currentStatus.engine & ENGINE_CRANK)))
    { //NOTE - There is a potential for a ~1sec gap between engine crank starting and ths runSec number being incremented. This may delay ASE!         
      if (currentStatus.runSecs <= 254) //Ensure we cap out at 255 and don't overflow. (which would reset ASE)
        { currentStatus.runSecs++; } //Increment our run counter by 1 second.
    }
    
    //Reset Timer2 to trigger in another ~10ms
    TCNT2  = 99;           //Preload timer2 with 100 cycles, leaving 156 till overflow.
    TIFR2  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
  }
  
}
