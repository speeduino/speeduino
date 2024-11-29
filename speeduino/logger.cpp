#include "globals.h"
#include "logger.h"
#include "decoders.h"
#include "init.h"
#include "maths.h"
#include "utilities.h"
#include BOARD_H 

/** 
 * Returns a numbered byte-field (partial field in case of multi-byte fields) from "current status" structure in the format expected by TunerStudio
 * Notes on fields:
 * - Numbered field will be fields from @ref currentStatus, but not at all in the internal order of strct (e.g. field RPM value, number 14 will be
 *   2nd field in struct)
 * - The fields stored in multi-byte types will be accessed lowbyte and highbyte separately (e.g. PW1 will be broken into numbered byte-fields 75,76)
 * - Values have the value offsets and shifts expected by TunerStudio. They will not all be a 'human readable value'
 * @param byteNum - byte-Field number. This is not the entry number (As some entries have multiple byets), but the byte number that is needed
 * @return Field value in 1 byte size struct fields or 1 byte partial value (chunk) on multibyte fields.
 */
byte getTSLogEntry(uint16_t byteNum)
{
  byte statusValue = 0;

  switch(byteNum)
  {
    case 0: statusValue = currentStatus.secl; break; //secl is simply a counter that increments each second. Used to track unexpected resets (Which will reset this count to 0)
    case 1: statusValue = currentStatus.status1; break; //status1 Bitfield
    case 2: statusValue = currentStatus.engine; break; //Engine Status Bitfield
    case 3: statusValue = currentStatus.syncLossCounter; break;
    case 4: statusValue = lowByte(currentStatus.MAP); break; //2 bytes for MAP
    case 5: statusValue = highByte(currentStatus.MAP); break;
    case 6: statusValue = lowByte(currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET); break; //mat
    case 7: statusValue = lowByte(currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); break; //Coolant ADC
    case 8: statusValue = currentStatus.batCorrection; break; //Battery voltage correction (%)
    case 9: statusValue = currentStatus.battery10; break; //battery voltage
    case 10: statusValue = currentStatus.O2; break; //O2
    case 11: statusValue = currentStatus.egoCorrection; break; //Exhaust gas correction (%)
    case 12: statusValue = currentStatus.iatCorrection; break; //Air temperature Correction (%)
    case 13: statusValue = currentStatus.wueCorrection; break; //Warmup enrichment (%)
    case 14: statusValue = lowByte(currentStatus.RPM); break; //rpm HB
    case 15: statusValue = highByte(currentStatus.RPM); break; //rpm LB
    case 16: statusValue = lowByte(currentStatus.AEamount >> 1U); break; //TPS acceleration enrichment (%) divided by 2 (Can exceed 255)
    case 17: statusValue = lowByte(currentStatus.corrections); break; //Total GammaE (%)
    case 18: statusValue = highByte(currentStatus.corrections); break; //Total GammaE (%)
    case 19: statusValue = currentStatus.VE1; break; //VE 1 (%)
    case 20: statusValue = currentStatus.VE2; break; //VE 2 (%)
    case 21: statusValue = currentStatus.afrTarget; break;
    case 22: statusValue = lowByte(currentStatus.tpsDOT); break; //TPS DOT
    case 23: statusValue = highByte(currentStatus.tpsDOT); break; //TPS DOT
    case 24: statusValue = currentStatus.advance; break;
    case 25: statusValue = currentStatus.TPS; break; // TPS (0% to 100%)
    
    case 26: 
      statusValue = lowByte(currentStatus.loopsPerSecond); 
      break;
    case 27: 
      statusValue = highByte(currentStatus.loopsPerSecond); 
      break;
    
    case 28: 
      currentStatus.freeRAM = freeRam();
      statusValue = lowByte(currentStatus.freeRAM); //(byte)((currentStatus.loopsPerSecond >> 8) & 0xFF);
      break; 
    case 29: 
      currentStatus.freeRAM = freeRam();
      statusValue = highByte(currentStatus.freeRAM); 
      break;

    case 30: statusValue = lowByte(currentStatus.boostTarget >> 1U); break; //Divide boost target by 2 to fit in a byte
    case 31: statusValue = lowByte(div100(currentStatus.boostDuty)); break;
    case 32: statusValue = currentStatus.status2; break; //Spark related bitfield

    //rpmDOT must be sent as a signed integer
    case 33: statusValue = lowByte(currentStatus.rpmDOT); break;
    case 34: statusValue = highByte(currentStatus.rpmDOT); break;

    case 35: statusValue = currentStatus.ethanolPct; break; //Flex sensor value (or 0 if not used)
    case 36: statusValue = currentStatus.flexCorrection; break; //Flex fuel correction (% above or below 100)
    case 37: statusValue = currentStatus.flexIgnCorrection; break; //Ignition correction (Increased degrees of advance) for flex fuel

    case 38: statusValue = currentStatus.idleLoad; break;
    case 39: statusValue = currentStatus.testOutputs; break;

    case 40: statusValue = currentStatus.O2_2; break; //O2
    case 41: statusValue = currentStatus.baro; break; //Barometer value

    case 42: statusValue = lowByte(currentStatus.canin[0]); break;
    case 43: statusValue = highByte(currentStatus.canin[0]); break;
    case 44: statusValue = lowByte(currentStatus.canin[1]); break;
    case 45: statusValue = highByte(currentStatus.canin[1]); break;
    case 46: statusValue = lowByte(currentStatus.canin[2]); break;
    case 47: statusValue = highByte(currentStatus.canin[2]); break;
    case 48: statusValue = lowByte(currentStatus.canin[3]); break;
    case 49: statusValue = highByte(currentStatus.canin[3]); break;
    case 50: statusValue = lowByte(currentStatus.canin[4]); break;
    case 51: statusValue = highByte(currentStatus.canin[4]); break;
    case 52: statusValue = lowByte(currentStatus.canin[5]); break;
    case 53: statusValue = highByte(currentStatus.canin[5]); break;
    case 54: statusValue = lowByte(currentStatus.canin[6]); break;
    case 55: statusValue = highByte(currentStatus.canin[6]); break;
    case 56: statusValue = lowByte(currentStatus.canin[7]); break;
    case 57: statusValue = highByte(currentStatus.canin[7]); break;
    case 58: statusValue = lowByte(currentStatus.canin[8]); break;
    case 59: statusValue = highByte(currentStatus.canin[8]); break;
    case 60: statusValue = lowByte(currentStatus.canin[9]); break;
    case 61: statusValue = highByte(currentStatus.canin[9]); break;
    case 62: statusValue = lowByte(currentStatus.canin[10]); break;
    case 63: statusValue = highByte(currentStatus.canin[10]); break;
    case 64: statusValue = lowByte(currentStatus.canin[11]); break;
    case 65: statusValue = highByte(currentStatus.canin[11]); break;
    case 66: statusValue = lowByte(currentStatus.canin[12]); break;
    case 67: statusValue = highByte(currentStatus.canin[12]); break;
    case 68: statusValue = lowByte(currentStatus.canin[13]); break;
    case 69: statusValue = highByte(currentStatus.canin[13]); break;
    case 70: statusValue = lowByte(currentStatus.canin[14]); break;
    case 71: statusValue = highByte(currentStatus.canin[14]); break;
    case 72: statusValue = lowByte(currentStatus.canin[15]); break;
    case 73: statusValue = highByte(currentStatus.canin[15]); break;

    case 74: statusValue = currentStatus.tpsADC; break;
    case 75: statusValue = 0U /*getNextError()*/; break;

    case 76: statusValue = lowByte(currentStatus.PW1); break; //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
    case 77: statusValue = highByte(currentStatus.PW1); break; //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
    case 78: statusValue = lowByte(currentStatus.PW2); break; //Pulsewidth 2 multiplied by 10 in ms. Have to convert from uS to mS.
    case 79: statusValue = highByte(currentStatus.PW2); break; //Pulsewidth 2 multiplied by 10 in ms. Have to convert from uS to mS.
    case 80: statusValue = lowByte(currentStatus.PW3); break; //Pulsewidth 3 multiplied by 10 in ms. Have to convert from uS to mS.
    case 81: statusValue = highByte(currentStatus.PW3); break; //Pulsewidth 3 multiplied by 10 in ms. Have to convert from uS to mS.
    case 82: statusValue = lowByte(currentStatus.PW4); break; //Pulsewidth 4 multiplied by 10 in ms. Have to convert from uS to mS.
    case 83: statusValue = highByte(currentStatus.PW4); break; //Pulsewidth 4 multiplied by 10 in ms. Have to convert from uS to mS.

    case 84: statusValue = currentStatus.status3; break;
    case 85: statusValue = currentStatus.engineProtectStatus; break;
    case 86: statusValue = lowByte(currentStatus.fuelLoad); break;
    case 87: statusValue = highByte(currentStatus.fuelLoad); break;
    case 88: statusValue = lowByte(currentStatus.ignLoad); break;
    case 89: statusValue = highByte(currentStatus.ignLoad); break;
    case 90: statusValue = lowByte(currentStatus.dwell); break;
    case 91: statusValue = highByte(currentStatus.dwell); break;
    case 92: statusValue = currentStatus.CLIdleTarget; break;
    case 93: statusValue = lowByte(currentStatus.mapDOT); break;
    case 94: statusValue = highByte(currentStatus.mapDOT); break;
    case 95: statusValue = lowByte(currentStatus.vvt1Angle); break; //2 bytes for vvt1Angle
    case 96: statusValue = highByte(currentStatus.vvt1Angle); break;
    case 97: statusValue = currentStatus.vvt1TargetAngle; break;
    case 98: statusValue = lowByte(currentStatus.vvt1Duty); break;
    case 99: statusValue = lowByte(currentStatus.flexBoostCorrection); break;
    case 100: statusValue = highByte(currentStatus.flexBoostCorrection); break;
    case 101: statusValue = currentStatus.baroCorrection; break;
    case 102: statusValue = currentStatus.VE; break; //Current VE (%). Can be equal to VE1 or VE2 or a calculated value from both of them
    case 103: statusValue = currentStatus.ASEValue; break; //Current ASE (%)
    case 104: statusValue = lowByte(currentStatus.vss); break;
    case 105: statusValue = highByte(currentStatus.vss); break;
    case 106: statusValue = currentStatus.gear; break;
    case 107: statusValue = currentStatus.fuelPressure; break;
    case 108: statusValue = currentStatus.oilPressure; break;
    case 109: statusValue = currentStatus.wmiPW; break;
    case 110: statusValue = currentStatus.status4; break;
    case 111: statusValue = lowByte(currentStatus.vvt2Angle); break; //2 bytes for vvt2Angle
    case 112: statusValue = highByte(currentStatus.vvt2Angle); break;
    case 113: statusValue = currentStatus.vvt2TargetAngle; break;
    case 114: statusValue = lowByte(currentStatus.vvt2Duty); break;
    case 115: statusValue = currentStatus.outputsStatus; break;
    case 116: statusValue = lowByte(currentStatus.fuelTemp + CALIBRATION_TEMPERATURE_OFFSET); break; //Fuel temperature from flex sensor
    case 117: statusValue = currentStatus.fuelTempCorrection; break; //Fuel temperature Correction (%)
    case 118: statusValue = currentStatus.advance1; break; //advance 1 (%)
    case 119: statusValue = currentStatus.advance2; break; //advance 2 (%)
    case 120: statusValue = currentStatus.TS_SD_Status; break; //SD card status
    case 121: statusValue = lowByte(currentStatus.EMAP); break; //2 bytes for EMAP
    case 122: statusValue = highByte(currentStatus.EMAP); break;
    case 123: statusValue = currentStatus.fanDuty; break;
    case 124: statusValue = currentStatus.airConStatus; break;
    case 125: statusValue = lowByte(currentStatus.actualDwell); break;
    case 126: statusValue = highByte(currentStatus.actualDwell); break;
    case 127: statusValue = currentStatus.status5; break;
    case 128: statusValue = currentStatus.knockCount; break;
    case 129: statusValue = currentStatus.knockRetard; break;
    default: statusValue = 0; // MISRA check
  }

  return statusValue;
}

/** 
 * Similar to the @ref getTSLogEntry function, however this returns a full, unadjusted (ie human readable) log entry value.
 * See logger.h for the field names and order
 * @param logIndex - The log index required. Note that this is NOT the byte number, but the index in the log
 * @return Raw, unadjusted value of the log entry. No offset or multiply is applied like it is with the TS log
 */
int16_t getReadableLogEntry(uint16_t logIndex)
{
  int16_t statusValue = 0;

  switch(logIndex)
  {
    case 0: statusValue = currentStatus.secl; break; //secl is simply a counter that increments each second. Used to track unexpected resets (Which will reset this count to 0)
    case 1: statusValue = currentStatus.status1; break; //status1 Bitfield
    case 2: statusValue = currentStatus.engine; break; //Engine Status Bitfield
    case 3: statusValue = currentStatus.syncLossCounter; break;
    case 4: statusValue = currentStatus.MAP; break; //2 bytes for MAP
    case 5: statusValue = currentStatus.IAT; break; //mat
    case 6: statusValue = currentStatus.coolant; break; //Coolant ADC
    case 7: statusValue = currentStatus.batCorrection; break; //Battery voltage correction (%)
    case 8: statusValue = currentStatus.battery10; break; //battery voltage
    case 9: statusValue = currentStatus.O2; break; //O2
    case 10: statusValue = currentStatus.egoCorrection; break; //Exhaust gas correction (%)
    case 11: statusValue = currentStatus.iatCorrection; break; //Air temperature Correction (%)
    case 12: statusValue = currentStatus.wueCorrection; break; //Warmup enrichment (%)
    case 13: statusValue = currentStatus.RPM; break; //rpm HB
    case 14: statusValue = currentStatus.AEamount; break; //TPS acceleration enrichment (%)
    case 15: statusValue = currentStatus.corrections; break; //Total GammaE (%)
    case 16: statusValue = currentStatus.VE1; break; //VE 1 (%)
    case 17: statusValue = currentStatus.VE2; break; //VE 2 (%)
    case 18: statusValue = currentStatus.afrTarget; break;
    case 19: statusValue = currentStatus.tpsDOT; break; //TPS DOT
    case 20: statusValue = currentStatus.advance; break;
    case 21: statusValue = currentStatus.TPS; break; // TPS (0% to 100%)
    
    case 22: 
      if(currentStatus.loopsPerSecond > 60000U) { currentStatus.loopsPerSecond = 60000U;}
      statusValue = currentStatus.loopsPerSecond; 
      break;
    
    case 23: 
      currentStatus.freeRAM = freeRam();
      statusValue = currentStatus.freeRAM;
      break; 

    case 24: statusValue = currentStatus.boostTarget; break;
    case 25: statusValue = currentStatus.boostDuty; break;
    case 26: statusValue = currentStatus.status2; break; //Spark related bitfield
    case 27: statusValue = currentStatus.rpmDOT; break;
    case 28: statusValue = currentStatus.ethanolPct; break; //Flex sensor value (or 0 if not used)
    case 29: statusValue = currentStatus.flexCorrection; break; //Flex fuel correction (% above or below 100)
    case 30: statusValue = currentStatus.flexIgnCorrection; break; //Ignition correction (Increased degrees of advance) for flex fuel
    case 31: statusValue = currentStatus.idleLoad; break;
    case 32: statusValue = currentStatus.testOutputs; break;
    case 33: statusValue = currentStatus.O2_2; break; //O2
    case 34: statusValue = currentStatus.baro; break; //Barometer value

    case 35: statusValue = currentStatus.canin[0]; break;
    case 36: statusValue = currentStatus.canin[1]; break;
    case 37: statusValue = currentStatus.canin[2]; break;
    case 38: statusValue = currentStatus.canin[3]; break;
    case 39: statusValue = currentStatus.canin[4]; break;
    case 40: statusValue = currentStatus.canin[5]; break;
    case 41: statusValue = currentStatus.canin[6]; break;
    case 42: statusValue = currentStatus.canin[7]; break;
    case 43: statusValue = currentStatus.canin[8]; break;
    case 44: statusValue = currentStatus.canin[9]; break;
    case 45: statusValue = currentStatus.canin[10]; break;
    case 46: statusValue = currentStatus.canin[11]; break;
    case 47: statusValue = currentStatus.canin[12]; break;
    case 48: statusValue = currentStatus.canin[13]; break;
    case 49: statusValue = currentStatus.canin[14]; break;
    case 50: statusValue = currentStatus.canin[15]; break;
    
    case 51: statusValue = currentStatus.tpsADC; break;
    case 52: statusValue = 0U /*getNextError()*/; break;

    case 53: statusValue = currentStatus.PW1; break; //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
    case 54: statusValue = currentStatus.PW2; break; //Pulsewidth 2 multiplied by 10 in ms. Have to convert from uS to mS.
    case 55: statusValue = currentStatus.PW3; break; //Pulsewidth 3 multiplied by 10 in ms. Have to convert from uS to mS.
    case 56: statusValue = currentStatus.PW4; break; //Pulsewidth 4 multiplied by 10 in ms. Have to convert from uS to mS.
  
    case 57: statusValue = currentStatus.status3; break;
    case 58: statusValue = currentStatus.engineProtectStatus; break;

    case 59: break; //UNUSED!!

    case 60: statusValue = currentStatus.fuelLoad; break;
    case 61: statusValue = currentStatus.ignLoad; break;
    case 62: statusValue = (int16_t)currentStatus.dwell; break;
    case 63: statusValue = currentStatus.CLIdleTarget; break;
    case 64: statusValue = currentStatus.mapDOT; break;
    case 65: statusValue = currentStatus.vvt1Angle; break;
    case 66: statusValue = currentStatus.vvt1TargetAngle; break;
    case 67: statusValue = currentStatus.vvt1Duty; break;
    case 68: statusValue = currentStatus.flexBoostCorrection; break;
    case 69: statusValue = currentStatus.baroCorrection; break;
    case 70: statusValue = currentStatus.VE; break; //Current VE (%). Can be equal to VE1 or VE2 or a calculated value from both of them
    case 71: statusValue = currentStatus.ASEValue; break; //Current ASE (%)
    case 72: statusValue = currentStatus.vss; break;
    case 73: statusValue = currentStatus.gear; break;
    case 74: statusValue = currentStatus.fuelPressure; break;
    case 75: statusValue = currentStatus.oilPressure; break;
    case 76: statusValue = currentStatus.wmiPW; break;
    case 77: statusValue = currentStatus.status4; break;
    case 78: statusValue = currentStatus.vvt2Angle; break; //2 bytes for vvt2Angle
    case 79: statusValue = currentStatus.vvt2TargetAngle; break;
    case 80: statusValue = currentStatus.vvt2Duty; break;
    case 81: statusValue = currentStatus.outputsStatus; break;
    case 82: statusValue = currentStatus.fuelTemp; break; //Fuel temperature from flex sensor
    case 83: statusValue = currentStatus.fuelTempCorrection; break; //Fuel temperature Correction (%)
    case 84: statusValue = currentStatus.advance1; break; //advance 1 (%)
    case 85: statusValue = currentStatus.advance2; break; //advance 2 (%)
    case 86: statusValue = currentStatus.TS_SD_Status; break; //SD card status
    case 87: statusValue = currentStatus.EMAP; break;
    case 88: statusValue = currentStatus.fanDuty; break;
    case 89: statusValue = currentStatus.airConStatus; break;
    case 90: statusValue = currentStatus.actualDwell; break;
    case 91: statusValue = currentStatus.status5; break;
    case 92: statusValue = currentStatus.knockCount; break;
    case 93: statusValue = currentStatus.knockRetard; break;
    default: statusValue = 0; // MISRA check
  }

  return statusValue;
}

/** 
 * An expansion to the @ref getReadableLogEntry function for systems that have an FPU. It will provide a floating point value for any parameter that this is appropriate for, otherwise will return the result of @ref getReadableLogEntry.
 * See logger.h for the field names and order
 * @param logIndex - The log index required. Note that this is NOT the byte number, but the index in the log
 * @return float value of the requested log entry. 
 */
#if defined(FPU_MAX_SIZE) && FPU_MAX_SIZE >= 32 //cppcheck-suppress misra-c2012-20.9
float getReadableFloatLogEntry(uint16_t logIndex)
{
  float statusValue = 0.0;

  switch(logIndex)
  {
    case 8: statusValue = currentStatus.battery10 / 10.0; break; //battery voltage
    case 9: statusValue = currentStatus.O2 / 10.0; break;
    case 18: statusValue = currentStatus.afrTarget / 10.0; break;
    case 21: statusValue = currentStatus.TPS / 2.0; break; // TPS (0% to 100% = 0 to 200)
    case 33: statusValue = currentStatus.O2_2 / 10.0; break; //O2

    case 53: statusValue = currentStatus.PW1 / 1000.0; break; //Pulsewidth 1 Have to convert from uS to mS.
    case 54: statusValue = currentStatus.PW2 / 1000.0; break; //Pulsewidth 2 Have to convert from uS to mS.
    case 55: statusValue = currentStatus.PW3 / 1000.0; break; //Pulsewidth 3 Have to convert from uS to mS.
    case 56: statusValue = currentStatus.PW4 / 1000.0; break; //Pulsewidth 4 Have to convert from uS to mS.

    default: statusValue = getReadableLogEntry(logIndex); break; //If logIndex value is NOT a float based one, use the regular function
  }

  return statusValue;
}
#endif

uint8_t getLegacySecondarySerialLogEntry(uint16_t byteNum)
{
  uint8_t statusValue = 0;
  currentStatus.status2 ^= (-currentStatus.hasSync ^ currentStatus.status2) & (1U << BIT_STATUS2_SYNC); //Set the sync bit of the Spark variable to match the hasSync variable

  switch(byteNum)
  {
    case 0: statusValue = currentStatus.secl; break; //secl is simply a counter that increments each second. Used to track unexpected resets (Which will reset this count to 0)
    case 1: statusValue = currentStatus.status1; break; //status1 Bitfield, inj1Status(0), inj2Status(1), inj3Status(2), inj4Status(3), DFCOOn(4), boostCutFuel(5), toothLog1Ready(6), toothLog2Ready(7)
    case 2: statusValue = currentStatus.engine; break; //Engine Status Bitfield, running(0), crank(1), ase(2), warmup(3), tpsaccaen(4), tpsacden(5), mapaccaen(6), mapaccden(7)
    case 3: statusValue = (byte)div100(currentStatus.dwell); break; //Dwell in ms * 10
    case 4: statusValue = lowByte(currentStatus.MAP); break; //2 bytes for MAP
    case 5: statusValue = highByte(currentStatus.MAP); break;
    case 6: statusValue = (byte)(currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET); break; //mat
    case 7: statusValue = (byte)(currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); break; //Coolant ADC
    case 8: statusValue = currentStatus.batCorrection; break; //Battery voltage correction (%)
    case 9: statusValue = currentStatus.battery10; break; //battery voltage
    case 10: statusValue = currentStatus.O2; break; //O2
    case 11: statusValue = currentStatus.egoCorrection; break; //Exhaust gas correction (%)
    case 12: statusValue = currentStatus.iatCorrection; break; //Air temperature Correction (%)
    case 13: statusValue = currentStatus.wueCorrection; break; //Warmup enrichment (%)
    case 14: statusValue = lowByte(currentStatus.RPM); break; //rpm HB
    case 15: statusValue = highByte(currentStatus.RPM); break; //rpm LB
    case 16: statusValue = currentStatus.AEamount; break; //acceleration enrichment (%)
    case 17: statusValue = currentStatus.corrections; break; //Total GammaE (%)
    case 18: statusValue = currentStatus.VE; break; //Current VE 1 (%)
    case 19: statusValue = currentStatus.afrTarget; break;
    case 20: statusValue = lowByte(currentStatus.PW1); break; //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
    case 21: statusValue = highByte(currentStatus.PW1); break; //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
    case 22: statusValue = (uint8_t)(currentStatus.tpsDOT / 10); break; //TPS DOT
    case 23: statusValue = currentStatus.advance; break;
    case 24: statusValue = currentStatus.TPS; break; // TPS (0% to 100%)
    case 25: statusValue = lowByte(currentStatus.loopsPerSecond); break;
    case 26: statusValue = highByte(currentStatus.loopsPerSecond); break;

    case 27: currentStatus.freeRAM = freeRam(); statusValue = lowByte(currentStatus.freeRAM); break; //(byte)((currentStatus.loopsPerSecond >> 8) & 0xFF); break;
    case 28: currentStatus.freeRAM = freeRam(); statusValue = highByte(currentStatus.freeRAM); break;

    case 29: statusValue = (byte)(currentStatus.boostTarget >> 1); break; //Divide boost target by 2 to fit in a byte
    case 30: statusValue = (byte)(currentStatus.boostDuty / 100); break;
    case 31: statusValue = currentStatus.status2; break; //Spark related bitfield, launchHard(0), launchSoft(1), hardLimitOn(2), softLimitOn(3), boostCutSpark(4), error(5), idleControlOn(6), sync(7)
    case 32: statusValue = lowByte(currentStatus.rpmDOT); break;
    case 33: statusValue = highByte(currentStatus.rpmDOT); break;
    case 34: statusValue = currentStatus.ethanolPct; break; //Flex sensor value (or 0 if not used)
    case 35: statusValue = currentStatus.flexCorrection; break; //Flex fuel correction (% above or below 100)
    case 36: statusValue = currentStatus.flexIgnCorrection; break; //Ignition correction (Increased degrees of advance) for flex fuel
    case 37: statusValue = currentStatus.idleLoad; break;
    case 38: statusValue = currentStatus.testOutputs; break; // testEnabled(0), testActive(1)
    case 39: statusValue = currentStatus.O2_2; break; //O2
    case 40: statusValue = currentStatus.baro; break; //Barometer value
    case 41: statusValue = lowByte(currentStatus.canin[0]); break;
    case 42: statusValue = highByte(currentStatus.canin[0]); break;
    case 43: statusValue = lowByte(currentStatus.canin[1]); break;
    case 44: statusValue = highByte(currentStatus.canin[1]); break;
    case 45: statusValue = lowByte(currentStatus.canin[2]); break;
    case 46: statusValue = highByte(currentStatus.canin[2]); break;
    case 47: statusValue = lowByte(currentStatus.canin[3]); break;
    case 48: statusValue = highByte(currentStatus.canin[3]); break;
    case 49: statusValue = lowByte(currentStatus.canin[4]); break;
    case 50: statusValue = highByte(currentStatus.canin[4]); break;
    case 51: statusValue = lowByte(currentStatus.canin[5]); break;
    case 52: statusValue = highByte(currentStatus.canin[5]); break;
    case 53: statusValue = lowByte(currentStatus.canin[6]); break;
    case 54: statusValue = highByte(currentStatus.canin[6]); break;
    case 55: statusValue = lowByte(currentStatus.canin[7]); break;
    case 56: statusValue = highByte(currentStatus.canin[7]); break;
    case 57: statusValue = lowByte(currentStatus.canin[8]); break;
    case 58: statusValue = highByte(currentStatus.canin[8]); break;
    case 59: statusValue = lowByte(currentStatus.canin[9]); break;
    case 60: statusValue = highByte(currentStatus.canin[9]); break;
    case 61: statusValue = lowByte(currentStatus.canin[10]); break;
    case 62: statusValue = highByte(currentStatus.canin[10]); break;
    case 63: statusValue = lowByte(currentStatus.canin[11]); break;
    case 64: statusValue = highByte(currentStatus.canin[11]); break;
    case 65: statusValue = lowByte(currentStatus.canin[12]); break;
    case 66: statusValue = highByte(currentStatus.canin[12]); break;
    case 67: statusValue = lowByte(currentStatus.canin[13]); break;
    case 68: statusValue = highByte(currentStatus.canin[13]); break;
    case 69: statusValue = lowByte(currentStatus.canin[14]); break;
    case 70: statusValue = highByte(currentStatus.canin[14]); break;
    case 71: statusValue = lowByte(currentStatus.canin[15]); break;
    case 72: statusValue = highByte(currentStatus.canin[15]); break;

    case 73: statusValue = currentStatus.tpsADC; break;
    case 74: statusValue = 0U /*getNextError()*/; break; // errorNum (0:1), currentError(2:7)

    case 75: statusValue = currentStatus.launchCorrection; break;
    case 76: statusValue = lowByte(currentStatus.PW2); break; //Pulsewidth 2 multiplied by 10 in ms. Have to convert from uS to mS.
    case 77: statusValue = highByte(currentStatus.PW2); break; //Pulsewidth 2 multiplied by 10 in ms. Have to convert from uS to mS.
    case 78: statusValue = lowByte(currentStatus.PW3); break; //Pulsewidth 3 multiplied by 10 in ms. Have to convert from uS to mS.
    case 79: statusValue = highByte(currentStatus.PW3); break; //Pulsewidth 3 multiplied by 10 in ms. Have to convert from uS to mS.
    case 80: statusValue = lowByte(currentStatus.PW4); break; //Pulsewidth 4 multiplied by 10 in ms. Have to convert from uS to mS.
    case 81: statusValue = highByte(currentStatus.PW4); break; //Pulsewidth 4 multiplied by 10 in ms. Have to convert from uS to mS.

    case 82: statusValue = currentStatus.status3; break; // resentLockOn(0), nitrousOn(1), fuel2Active(2), vssRefresh(3), halfSync(4), nSquirts(6:7)
    case 83: statusValue = currentStatus.engineProtectStatus; break; //RPM(0), MAP(1), OIL(2), AFR(3), Unused(4:7)
    case 84: statusValue = lowByte(currentStatus.fuelLoad); break;
    case 85: statusValue = highByte(currentStatus.fuelLoad); break;
    case 86: statusValue = lowByte(currentStatus.ignLoad); break;
    case 87: statusValue = highByte(currentStatus.ignLoad); break;
    case 88: statusValue = lowByte(currentStatus.injAngle); break; 
    case 89: statusValue = highByte(currentStatus.injAngle); break; 
    case 90: statusValue = currentStatus.idleLoad; break;
    case 91: statusValue = currentStatus.CLIdleTarget; break; //closed loop idle target
    case 92: statusValue = (uint8_t)(currentStatus.mapDOT / 10); break; //rate of change of the map 
    case 93: statusValue = (int8_t)currentStatus.vvt1Angle; break;
    case 94: statusValue = currentStatus.vvt1TargetAngle; break;
    case 95: statusValue = currentStatus.vvt1Duty; break;
    case 96: statusValue = lowByte(currentStatus.flexBoostCorrection); break;
    case 97: statusValue = highByte(currentStatus.flexBoostCorrection); break;
    case 98: statusValue = currentStatus.baroCorrection; break;
    case 99: statusValue = currentStatus.ASEValue; break; //Current ASE (%)
    case 100: statusValue = lowByte(currentStatus.vss); break; //speed reading from the speed sensor
    case 101: statusValue = highByte(currentStatus.vss); break;
    case 102: statusValue = currentStatus.gear; break; 
    case 103: statusValue = currentStatus.fuelPressure; break;
    case 104: statusValue = currentStatus.oilPressure; break;
    case 105: statusValue = currentStatus.wmiPW; break;
    case 106: statusValue = currentStatus.status4; break; // wmiEmptyBit(0), vvt1Error(1), vvt2Error(2), fanStatus(3), UnusedBits(4:7)
    case 107: statusValue = (int8_t)currentStatus.vvt2Angle; break;
    case 108: statusValue = currentStatus.vvt2TargetAngle; break;
    case 109: statusValue = currentStatus.vvt2Duty; break;
    case 110: statusValue = currentStatus.outputsStatus; break;
    case 111: statusValue = (byte)(currentStatus.fuelTemp + CALIBRATION_TEMPERATURE_OFFSET); break; //Fuel temperature from flex sensor
    case 112: statusValue = currentStatus.fuelTempCorrection; break; //Fuel temperature Correction (%)
    case 113: statusValue = currentStatus.VE1; break; //VE 1 (%)
    case 114: statusValue = currentStatus.VE2; break; //VE 2 (%)
    case 115: statusValue = currentStatus.advance1; break; //advance 1 
    case 116: statusValue = currentStatus.advance2; break; //advance 2 
    case 117: statusValue = currentStatus.nitrous_status; break;
    case 118: statusValue = currentStatus.TS_SD_Status; break; //SD card status
    case 119: statusValue = lowByte(currentStatus.EMAP); break; //2 bytes for EMAP
    case 120: statusValue = highByte(currentStatus.EMAP); break;
    case 121: statusValue = currentStatus.fanDuty; break;
    case 122: statusValue = currentStatus.airConStatus; break;
  }

  return statusValue;
}

/** 
 * Searches the log 2 byte array to determine whether a given index is a regular single byte or a 2 byte field
 * Uses a boundless binary search for improved performance, but requires the fsIntIndex to remain in order
 * 
 * @param key - Index in the log array to check
 * @return True if the index is a 2 byte log field. False if it is a single byte
 */
bool is2ByteEntry(uint8_t key)
{
  // This array indicates which index values from the log are 2 byte values
  // This array MUST remain in ascending order
  // !!!! WARNING: If any value above 255 is required in this array, changes MUST be made to is2ByteEntry() function !!!!
  static constexpr byte PROGMEM fsIntIndex[] = {4, 14, 17, 22, 26, 28, 33, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 76, 78, 80, 82, 86, 88, 90, 93, 95, 99, 104, 111, 121, 125 };

  unsigned int bot = 0U;
  unsigned int mid = _countof(fsIntIndex);
  
  while (mid > 1U)
  {  
    if (key >= pgm_read_byte( &fsIntIndex[bot + mid / 2U]) )
    {
      bot += mid++ / 2U;
    }
    mid /= 2U;
  }

  return key == pgm_read_byte(&fsIntIndex[bot]);
}

void startToothLogger(void)
{
  currentStatus.toothLogEnabled = true;
  currentStatus.compositeTriggerUsed = 0U; //Safety first (Should never be required)
  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
  toothHistoryIndex = 0U;

  //Disconnect the standard interrupt and add the logger version
  detachInterrupt( digitalPinToInterrupt(pinTrigger) );
  attachInterrupt( digitalPinToInterrupt(pinTrigger), loggerPrimaryISR, CHANGE );

  if(VSS_USES_RPM2() != true)
  {
    detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
    attachInterrupt( digitalPinToInterrupt(pinTrigger2), loggerSecondaryISR, CHANGE );  
  }
  
}

void stopToothLogger(void)
{
  currentStatus.toothLogEnabled = false;

  //Disconnect the logger interrupts and attach the normal ones
  detachInterrupt( digitalPinToInterrupt(pinTrigger) );
  attachInterrupt( digitalPinToInterrupt(pinTrigger), triggerHandler, primaryTriggerEdge );

  if(VSS_USES_RPM2() != true)
  {
    detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
    attachInterrupt( digitalPinToInterrupt(pinTrigger2), triggerSecondaryHandler, secondaryTriggerEdge );  
  }
}

void startCompositeLogger(void)
{
  currentStatus.compositeTriggerUsed = 2U;
  currentStatus.toothLogEnabled = false; //Safety first (Should never be required)
  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
  toothHistoryIndex = 0U;

  //Disconnect the standard interrupt and add the logger version
  detachInterrupt( digitalPinToInterrupt(pinTrigger) );
  attachInterrupt( digitalPinToInterrupt(pinTrigger), loggerPrimaryISR, CHANGE );

  if( (VSS_USES_RPM2() != true) && (FLEX_USES_RPM2() != true) )
  {
    detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
    attachInterrupt( digitalPinToInterrupt(pinTrigger2), loggerSecondaryISR, CHANGE );
  }
}

void stopCompositeLogger(void)
{
  currentStatus.compositeTriggerUsed = 0U;

  //Disconnect the logger interrupts and attach the normal ones
  detachInterrupt( digitalPinToInterrupt(pinTrigger) );
  attachInterrupt( digitalPinToInterrupt(pinTrigger), triggerHandler, primaryTriggerEdge );

  if( (VSS_USES_RPM2() != true) && (FLEX_USES_RPM2() != true) )
  {
    detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
    attachInterrupt( digitalPinToInterrupt(pinTrigger2), triggerSecondaryHandler, secondaryTriggerEdge );
  }
}

void startCompositeLoggerTertiary(void)
{
  currentStatus.compositeTriggerUsed = 3U;
  currentStatus.toothLogEnabled = false; //Safety first (Should never be required)
  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
  toothHistoryIndex = 0U;

  //Disconnect the standard interrupt and add the logger version
  detachInterrupt( digitalPinToInterrupt(pinTrigger) );
  attachInterrupt( digitalPinToInterrupt(pinTrigger), loggerPrimaryISR, CHANGE );

  detachInterrupt( digitalPinToInterrupt(pinTrigger3) );
  attachInterrupt( digitalPinToInterrupt(pinTrigger3), loggerTertiaryISR, CHANGE );
}

void stopCompositeLoggerTertiary(void)
{
  currentStatus.compositeTriggerUsed = 0;

  //Disconnect the logger interrupts and attach the normal ones
  detachInterrupt( digitalPinToInterrupt(pinTrigger) );
  attachInterrupt( digitalPinToInterrupt(pinTrigger), triggerHandler, primaryTriggerEdge );

  detachInterrupt( digitalPinToInterrupt(pinTrigger3) );
  attachInterrupt( digitalPinToInterrupt(pinTrigger3), triggerTertiaryHandler, tertiaryTriggerEdge );
}


void startCompositeLoggerCams(void)
{
  currentStatus.compositeTriggerUsed = 4;
  currentStatus.toothLogEnabled = false; //Safety first (Should never be required)
  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
  toothHistoryIndex = 0;

  //Disconnect the standard interrupt and add the logger version
  if( (VSS_USES_RPM2() != true) && (FLEX_USES_RPM2() != true) )
  {
    detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
    attachInterrupt( digitalPinToInterrupt(pinTrigger2), loggerSecondaryISR, CHANGE );
  }

  detachInterrupt( digitalPinToInterrupt(pinTrigger3) );
  attachInterrupt( digitalPinToInterrupt(pinTrigger3), loggerTertiaryISR, CHANGE );
}

void stopCompositeLoggerCams(void)
{
  currentStatus.compositeTriggerUsed = false;

  //Disconnect the logger interrupts and attach the normal ones
  if( (VSS_USES_RPM2() != true) && (FLEX_USES_RPM2() != true) )
  {
    detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
    attachInterrupt( digitalPinToInterrupt(pinTrigger2), triggerSecondaryHandler, secondaryTriggerEdge );
  }

  detachInterrupt( digitalPinToInterrupt(pinTrigger3) );
  attachInterrupt( digitalPinToInterrupt(pinTrigger3), triggerTertiaryHandler, tertiaryTriggerEdge );
}
