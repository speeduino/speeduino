
/** \file
 * Header file for the TunerStudio command handler
 * The command handler manages all the inputs FROM TS which are issued when a command button is clicked by the user
 */

#include "TS_CommandButtonHandler.h"
#include "globals.h"
#include "utils.h"
#include "scheduledIO.h"
#include "sensors.h"
#ifdef USE_MC33810
  #include "acc_mc33810.h"
#endif

/**
 * @brief 
 * 
 * @param buttonCommand The command number of the button that was clicked. See TS_CommendButtonHandler.h for a list of button IDs
 * @return uint16_t If the button command remains incomplete (IE When it must wait for a certain action to complete) the return value is eqaul to the button ID. Otherwise this function returns 0
 */
uint16_t TS_CommandButtonsHandler(int buttonCommand)
{
  uint16_t returnValue = 0;
  switch (buttonCommand)
  {
    case TS_CMD_TEST_DSBL: // cmd is stop
      BIT_CLEAR(currentStatus.testOutputs, 1);
      endCoil1Charge();
      endCoil2Charge();
      endCoil3Charge();
      endCoil4Charge();
      closeInjector1();
      closeInjector2();
      closeInjector3();
      closeInjector4();
      #if INJ_CHANNELS >= 5
      closeInjector5();
      #endif
      #if INJ_CHANNELS >= 6
      closeInjector6();
      #endif
      #if INJ_CHANNELS >= 7
      closeInjector7();
      #endif
      #if INJ_CHANNELS >= 8
      closeInjector8();
      #endif

      HWTest_INJ_50pc = 0;
      HWTest_IGN_50pc = 0;
      break;

    case TS_CMD_TEST_ENBL: // cmd is enable
      // currentStatus.testactive = 1;
      BIT_SET(currentStatus.testOutputs, 1);
      break;

    case TS_CMD_INJ1_ON: // cmd group is for injector1 on actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ){ openInjector1(); }
      break;

    case TS_CMD_INJ1_OFF: // cmd group is for injector1 off actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ){ closeInjector1(); BIT_CLEAR(HWTest_INJ_50pc, 1); }
      break;

    case TS_CMD_INJ1_50PC: // cmd group is for injector1 50% dc actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_INJ_50pc, 1); }
      if(!BIT_CHECK(HWTest_INJ_50pc, 1)) { closeInjector1(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_INJ2_ON: // cmd group is for injector2 on actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ){ openInjector2(); }
      break;

    case TS_CMD_INJ2_OFF: // cmd group is for injector2 off actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ){ closeInjector2(); BIT_CLEAR(HWTest_INJ_50pc, 2); }
      break;

    case TS_CMD_INJ2_50PC: // cmd group is for injector2 50%dc actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_INJ_50pc, 2); }
      if(!BIT_CHECK(HWTest_INJ_50pc, 2)) { closeInjector2(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_INJ3_ON: // cmd group is for injector3 on actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ){ openInjector3(); }
      break;

    case TS_CMD_INJ3_OFF: // cmd group is for injector3 off actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ){ closeInjector3(); BIT_CLEAR(HWTest_INJ_50pc, 3); }
      break;

    case TS_CMD_INJ3_50PC: // cmd group is for injector3 50%dc actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_INJ_50pc, 3); }
      if(!BIT_CHECK(HWTest_INJ_50pc, 3)) { closeInjector3(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_INJ4_ON: // cmd group is for injector4 on actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ){ openInjector4(); }
      break;

    case TS_CMD_INJ4_OFF: // cmd group is for injector4 off actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ){ closeInjector4(); BIT_CLEAR(HWTest_INJ_50pc, 4); }
      break;

    case TS_CMD_INJ4_50PC: // cmd group is for injector4 50% dc actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_INJ_50pc, 4); }
      if(!BIT_CHECK(HWTest_INJ_50pc, 4)) { closeInjector4(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_INJ5_ON: // cmd group is for injector5 on actions
      #if INJ_CHANNELS >= 5
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ openInjector5(); }
      #endif
      break;

    case TS_CMD_INJ5_OFF: // cmd group is for injector5 off actions
        #if INJ_CHANNELS >= 5
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ closeInjector5(); BIT_CLEAR(HWTest_INJ_50pc, 5); }
        #endif
      break;

    case TS_CMD_INJ5_50PC: // cmd group is for injector5 50%dc actions
      #if INJ_CHANNELS >= 5
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_INJ_50pc, 5); }
        if(!BIT_CHECK(HWTest_INJ_50pc, 5)) { closeInjector5(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      #endif
      break;

    case TS_CMD_INJ6_ON: // cmd group is for injector6 on actions
        #if INJ_CHANNELS >= 6
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ openInjector6(); }
        #endif
      break;

    case TS_CMD_INJ6_OFF: // cmd group is for injector6 off actions
        #if INJ_CHANNELS >= 6
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ closeInjector6(); BIT_CLEAR(HWTest_INJ_50pc, 6); }
        #endif
      break;

    case TS_CMD_INJ6_50PC: // cmd group is for injector6 50% dc actions
      #if INJ_CHANNELS >= 6
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_INJ_50pc, 6); }
        if(!BIT_CHECK(HWTest_INJ_50pc, 6)) { closeInjector6(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      #endif
      break;

    case TS_CMD_INJ7_ON: // cmd group is for injector7 on actions
        #if INJ_CHANNELS >= 7
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ openInjector7(); }
        #endif
      break;

    case TS_CMD_INJ7_OFF: // cmd group is for injector7 off actions
        #if INJ_CHANNELS >= 7
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ closeInjector7(); BIT_CLEAR(HWTest_INJ_50pc, 7); }
        #endif
      break;

    case TS_CMD_INJ7_50PC: // cmd group is for injector7 50%dc actions
      #if INJ_CHANNELS >= 7
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_INJ_50pc, 7); }
        if(!BIT_CHECK(HWTest_INJ_50pc, 7)) { closeInjector7(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      #endif
      break;

    case TS_CMD_INJ8_ON: // cmd group is for injector8 on actions
        #if INJ_CHANNELS >= 8
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ openInjector8(); }
        #endif
      break;

    case TS_CMD_INJ8_OFF: // cmd group is for injector8 off actions
      #if INJ_CHANNELS >= 8
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ closeInjector8(); BIT_CLEAR(HWTest_INJ_50pc, 8); }
      #endif
      break;

    case TS_CMD_INJ8_50PC: // cmd group is for injector8 50% dc actions
      #if INJ_CHANNELS >= 8
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_INJ_50pc, 8); }
        if(!BIT_CHECK(HWTest_INJ_50pc, 8)) { closeInjector8(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      #endif
      break;

    case TS_CMD_IGN1_ON: // cmd group is for spark1 on actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { beginCoil1Charge(); }
      break;

    case TS_CMD_IGN1_OFF: // cmd group is for spark1 off actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { endCoil1Charge(); BIT_CLEAR(HWTest_IGN_50pc, 1); }
      break;

    case TS_CMD_IGN1_50PC: // cmd group is for spark1 50%dc actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_IGN_50pc, 1); }
      if(!BIT_CHECK(HWTest_IGN_50pc, 1)) { coil1Low(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_IGN2_ON: // cmd group is for spark2 on actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { beginCoil2Charge(); }
      break;

    case TS_CMD_IGN2_OFF: // cmd group is for spark2 off actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { endCoil2Charge(); BIT_CLEAR(HWTest_IGN_50pc, 2); }
      break;

    case TS_CMD_IGN2_50PC: // cmd group is for spark2 50%dc actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_IGN_50pc, 2); }
      if(!BIT_CHECK(HWTest_IGN_50pc, 2)) { coil2Low(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_IGN3_ON: // cmd group is for spark3 on actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { beginCoil3Charge(); }
      break;

    case TS_CMD_IGN3_OFF: // cmd group is for spark3 off actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { endCoil3Charge(); BIT_CLEAR(HWTest_IGN_50pc, 3); }
      break;

    case TS_CMD_IGN3_50PC: // cmd group is for spark3 50%dc actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_IGN_50pc, 3); }
      if(!BIT_CHECK(HWTest_IGN_50pc, 3)) { coil3Low(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_IGN4_ON: // cmd group is for spark4 on actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { beginCoil4Charge(); }
      break;

    case TS_CMD_IGN4_OFF: // cmd group is for spark4 off actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { endCoil4Charge(); BIT_CLEAR(HWTest_IGN_50pc, 4); }
      break;

    case TS_CMD_IGN4_50PC: // cmd group is for spark4 50%dc actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_IGN_50pc, 4); }
      if(!BIT_CHECK(HWTest_IGN_50pc, 4)) { coil4Low(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_IGN5_ON: // cmd group is for spark5 on actions
      #if IGN_CHANNELS >= 5
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { beginCoil5Charge(); }
      #endif
      break;

    case TS_CMD_IGN5_OFF: // cmd group is for spark5 off actions
      #if IGN_CHANNELS >= 5
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { endCoil5Charge(); BIT_CLEAR(HWTest_IGN_50pc, 5); }
      #endif
      break;

    case TS_CMD_IGN5_50PC: // cmd group is for spark4 50%dc actions
      #if IGN_CHANNELS >= 5
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_IGN_50pc, 5); }
        if(!BIT_CHECK(HWTest_IGN_50pc, 5)) { coil5Low(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      #endif
      break;

    case TS_CMD_IGN6_ON: // cmd group is for spark6 on actions
      #if IGN_CHANNELS >= 6
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { beginCoil6Charge(); }
      #endif
      break;

    case TS_CMD_IGN6_OFF: // cmd group is for spark6 off actions
      #if IGN_CHANNELS >= 6
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { endCoil6Charge(); BIT_CLEAR(HWTest_IGN_50pc, 6); }
      #endif
      break;

    case TS_CMD_IGN6_50PC: // cmd group is for spark6 50%dc actions
      #if IGN_CHANNELS >= 6
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_IGN_50pc, 6); }
        if(!BIT_CHECK(HWTest_IGN_50pc, 6)) { coil6Low(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      #endif
      break;

    case TS_CMD_IGN7_ON: // cmd group is for spark7 on actions
      #if IGN_CHANNELS >= 7
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { beginCoil7Charge(); }
      #endif
      break;

    case TS_CMD_IGN7_OFF: // cmd group is for spark7 off actions
      #if IGN_CHANNELS >= 7
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { endCoil7Charge(); BIT_CLEAR(HWTest_IGN_50pc, 7); }
      #endif
      break;

    case TS_CMD_IGN7_50PC: // cmd group is for spark7 50%dc actions
      #if IGN_CHANNELS >= 7
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_IGN_50pc, 7); }
        if(!BIT_CHECK(HWTest_IGN_50pc, 7)) { coil7Low(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      #endif
      break;

    case TS_CMD_IGN8_ON: // cmd group is for spark8 on actions
      #if IGN_CHANNELS >= 8
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { beginCoil8Charge(); }
      #endif
      break;

    case TS_CMD_IGN8_OFF: // cmd group is for spark8 off actions
      #if IGN_CHANNELS >= 8
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { endCoil8Charge(); BIT_CLEAR(HWTest_IGN_50pc, 8); }
      #endif
      break;

    case TS_CMD_IGN8_50PC: // cmd group is for spark8 50%dc actions
      #if IGN_CHANNELS >= 8
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { BIT_TOGGLE(HWTest_IGN_50pc, 8); }
        if(!BIT_CHECK(HWTest_IGN_50pc, 8)) { coil8Low(); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      #endif
      break;

    //VSS Calibration routines
    case TS_CMD_VSS_60KMH:
      //Calibrate the actual pulses per distance
      if( (vssLastPulseTime > 0) && (vssLastMinusOnePulseTime > 0) )
      {
        if(vssLastPulseTime > vssLastMinusOnePulseTime)
        {
          configPage2.vssPulsesPerKm = 60000000UL / (vssLastPulseTime - vssLastMinusOnePulseTime);
        }
      }
      break;

    //Calculate the RPM to speed ratio for each gear
    case TS_CMD_VSS_RATIO1:
      if(currentStatus.vss > 0)
      {
        configPage2.vssRatio1 = (currentStatus.vss * 10000) / currentStatus.RPM;
      }
      break;

    case TS_CMD_VSS_RATIO2:
      if(currentStatus.vss > 0)
      {
        configPage2.vssRatio2 = (currentStatus.vss * 10000) / currentStatus.RPM;
      }
      break;

    case TS_CMD_VSS_RATIO3:
      if(currentStatus.vss > 0)
      {
        configPage2.vssRatio3 = (currentStatus.vss * 10000) / currentStatus.RPM;
      }
      break;

    case TS_CMD_VSS_RATIO4: 
      if(currentStatus.vss > 0)
      {
        configPage2.vssRatio4 = (currentStatus.vss * 10000) / currentStatus.RPM;
      }
      break;

    case TS_CMD_VSS_RATIO5:
      if(currentStatus.vss > 0)
      {
        configPage2.vssRatio5 = (currentStatus.vss * 10000) / currentStatus.RPM;
      }
      break;

    case TS_CMD_VSS_RATIO6:
      if(currentStatus.vss > 0)
      {
        configPage2.vssRatio6 = (currentStatus.vss * 10000) / currentStatus.RPM;
      }
      break;

    default:
      break;
  }

  return returnValue;
}