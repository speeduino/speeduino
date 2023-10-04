#include "fuel_tables_setup.h"
#include <globals.h>

void test_fuel_set_WUE_tables(void)
{
  //Set mock values for WUE table. Scale is accounted for with WUETable2 values
  ((uint8_t*)WUETable.axisX)[0] = ((uint8_t*)WUETable2.axisX)[0] = 0 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable.axisX)[1] = ((uint8_t*)WUETable2.axisX)[1] = 20 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable.axisX)[2] = ((uint8_t*)WUETable2.axisX)[2] = 30 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable.axisX)[3] = ((uint8_t*)WUETable2.axisX)[3] = 40 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable.axisX)[4] = ((uint8_t*)WUETable2.axisX)[4] = 50 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable.axisX)[5] = ((uint8_t*)WUETable2.axisX)[5] = 60 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable.axisX)[6] = ((uint8_t*)WUETable2.axisX)[6] = 70 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable.axisX)[7] = ((uint8_t*)WUETable2.axisX)[7] = 90 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable.axisX)[8] = ((uint8_t*)WUETable2.axisX)[8] = 100 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable.axisX)[9] = ((uint8_t*)WUETable2.axisX)[9] = 120 + CALIBRATION_TEMPERATURE_OFFSET;

  ((uint8_t*)WUETable.values)[0] = 200;
  ((uint8_t*)WUETable.values)[1] = 190;
  ((uint8_t*)WUETable.values)[2] = 180;
  ((uint8_t*)WUETable.values)[3] = 170;
  ((uint8_t*)WUETable.values)[4] = 155;
  ((uint8_t*)WUETable.values)[5] = 140;
  ((uint8_t*)WUETable.values)[6] = 130;
  ((uint8_t*)WUETable.values)[7] = 120;
  ((uint8_t*)WUETable.values)[8] = 110;
  ((uint8_t*)WUETable.values)[9] = 100;

  ((uint8_t*)WUETable2.values)[0] = 800 / WUETABLE2_VALUE_SCALE;
  ((uint8_t*)WUETable2.values)[1] = 700 / WUETABLE2_VALUE_SCALE;
  ((uint8_t*)WUETable2.values)[2] = 500 / WUETABLE2_VALUE_SCALE;
  ((uint8_t*)WUETable2.values)[3] = 300 / WUETABLE2_VALUE_SCALE;
  ((uint8_t*)WUETable2.values)[4] = 300 / WUETABLE2_VALUE_SCALE;
  ((uint8_t*)WUETable2.values)[5] = 275 / WUETABLE2_VALUE_SCALE;
  ((uint8_t*)WUETable2.values)[6] = 240 / WUETABLE2_VALUE_SCALE;
  ((uint8_t*)WUETable2.values)[7] = 200 / WUETABLE2_VALUE_SCALE;
  ((uint8_t*)WUETable2.values)[8] = 150 / WUETABLE2_VALUE_SCALE;
  ((uint8_t*)WUETable2.values)[9] = 100 / WUETABLE2_VALUE_SCALE;

}

void test_fuel_set_cranking_tables(void)
{
  //set mock values for cranking tables. Scale is accounted for
  ((uint8_t*)crankingEnrichTable.axisX)[0] = ((uint8_t*)crankingEnrichTable2.axisX)[0] = 0 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)crankingEnrichTable.axisX)[1] = ((uint8_t*)crankingEnrichTable2.axisX)[1] = 60 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)crankingEnrichTable.axisX)[2] = ((uint8_t*)crankingEnrichTable2.axisX)[2] = 120 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)crankingEnrichTable.axisX)[3] = ((uint8_t*)crankingEnrichTable2.axisX)[3] = 180 + CALIBRATION_TEMPERATURE_OFFSET;
  
  ((uint8_t*)crankingEnrichTable.values)[0] = 225 / CRANKINGENRICHTABLE_VALUE_SCALE; //Table 1 values are scaled by 5 in storage
  ((uint8_t*)crankingEnrichTable.values)[1] = 190 / CRANKINGENRICHTABLE_VALUE_SCALE;
  ((uint8_t*)crankingEnrichTable.values)[2] = 140 / CRANKINGENRICHTABLE_VALUE_SCALE;
  ((uint8_t*)crankingEnrichTable.values)[3] = 120 / CRANKINGENRICHTABLE_VALUE_SCALE;

  ((uint8_t*)crankingEnrichTable2.values)[0] = 1600 / CRANKINGENRICHTABLE2_VALUE_SCALE; //Table 2 values are scaled by 10 in storage
  ((uint8_t*)crankingEnrichTable2.values)[1] = 400  / CRANKINGENRICHTABLE2_VALUE_SCALE;
  ((uint8_t*)crankingEnrichTable2.values)[2] = 250  / CRANKINGENRICHTABLE2_VALUE_SCALE;
  ((uint8_t*)crankingEnrichTable2.values)[3] = 130  / CRANKINGENRICHTABLE2_VALUE_SCALE;
}

void test_fuel_set_ASE_tables(void)
{
  //set mock values for ASE tables. Scale is accounted for with ASETable2 values
  //set duration table
  ((uint8_t*)ASECountTable.axisX)[0] = ((uint8_t*)ASETable.axisX)[0] = ((uint8_t*)ASETable2.axisX)[0] = 0 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)ASECountTable.axisX)[1] = ((uint8_t*)ASETable.axisX)[1] = ((uint8_t*)ASETable2.axisX)[1] = 40 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)ASECountTable.axisX)[2] = ((uint8_t*)ASETable.axisX)[2] = ((uint8_t*)ASETable2.axisX)[2] = 120 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)ASECountTable.axisX)[3] = ((uint8_t*)ASETable.axisX)[3] = ((uint8_t*)ASETable2.axisX)[3] = 180 + CALIBRATION_TEMPERATURE_OFFSET;

  ((uint8_t*)ASECountTable.values)[0] = 16;
  ((uint8_t*)ASECountTable.values)[1] = 12;
  ((uint8_t*)ASECountTable.values)[2] = 3;
  ((uint8_t*)ASECountTable.values)[3] = 1;

  ((uint8_t*)ASETable.values)[0] = 100;
  ((uint8_t*)ASETable.values)[1] = 40;
  ((uint8_t*)ASETable.values)[2] = 20;
  ((uint8_t*)ASETable.values)[3] = 5;

  ((uint8_t*)ASETable2.values)[0] = 1000 / ASETABLE2_VALUE_SCALE; //Table2 values are scaled by 5 in storage
  ((uint8_t*)ASETable2.values)[1] = 400  / ASETABLE2_VALUE_SCALE;
  ((uint8_t*)ASETable2.values)[2] = 50   / ASETABLE2_VALUE_SCALE;
  ((uint8_t*)ASETable2.values)[3] = 20   / ASETABLE2_VALUE_SCALE;
}

void test_fuel_set_flex_tables(void)
{
  //set mock values for flex fuel table
  ((uint8_t*)flexFuelTable.axisX)[0] = 0;
  ((uint8_t*)flexFuelTable.axisX)[1] = 20;
  ((uint8_t*)flexFuelTable.axisX)[2] = 40;
  ((uint8_t*)flexFuelTable.axisX)[3] = 60;
  ((uint8_t*)flexFuelTable.axisX)[4] = 85;
  ((uint8_t*)flexFuelTable.axisX)[5] = 100;

  ((uint8_t*)flexFuelTable.values)[0] = 0;
  ((uint8_t*)flexFuelTable.values)[1] = 24;
  ((uint8_t*)flexFuelTable.values)[2] = 47;
  ((uint8_t*)flexFuelTable.values)[3] = 70;
  ((uint8_t*)flexFuelTable.values)[4] = 100;
  ((uint8_t*)flexFuelTable.values)[5] = 110;

  //set flex ignition table
  ((uint8_t*)flexAdvTable.axisX)[0] = 0;
  ((uint8_t*)flexAdvTable.axisX)[1] = 20;
  ((uint8_t*)flexAdvTable.axisX)[2] = 40;
  ((uint8_t*)flexAdvTable.axisX)[3] = 60;
  ((uint8_t*)flexAdvTable.axisX)[4] = 85;
  ((uint8_t*)flexAdvTable.axisX)[5] = 100;

  ((uint8_t*)flexAdvTable.values)[0] = 0;
  ((uint8_t*)flexAdvTable.values)[1] = 23;
  ((uint8_t*)flexAdvTable.values)[2] = 46;
  ((uint8_t*)flexAdvTable.values)[3] = 69;
  ((uint8_t*)flexAdvTable.values)[4] = 100;
  ((uint8_t*)flexAdvTable.values)[5] = 120;
}

//Setup a basic TAE enrichment curve, threshold etc that are common to all tests. Specific values maybe updated in each individual test
void test_corrections_TAE_setup(void)
{
    configPage2.aeMode = AE_MODE_TPS; //Set AE to TPS

    configPage4.taeValues[0] = 70;
    configPage4.taeValues[1] = 103; 
    configPage4.taeValues[2] = 124;
    configPage4.taeValues[3] = 136; 

    //Note: These values are divided by 10
    configPage4.taeBins[0] = 0;
    configPage4.taeBins[1] = 8; 
    configPage4.taeBins[2] = 22;
    configPage4.taeBins[3] = 97; 

    configPage2.taeThresh = 0;
    configPage2.taeMinChange = 0;

    //Divided by 100
    configPage2.aeTaperMin = 10; //1000
    configPage2.aeTaperMax = 50; //5000
        
    //Set the coolant to be above the warmup AE taper
    configPage2.aeColdTaperMax = 60;
    configPage2.aeColdTaperMin = 0;
    currentStatus.coolant = (int)(configPage2.aeColdTaperMax - CALIBRATION_TEMPERATURE_OFFSET) + 1;

    BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC); //Make sure AE is turned off
    BIT_CLEAR(currentStatus.engine, BIT_ENGINE_DCC); //Make sure AE is turned off
}