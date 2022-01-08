/*

Ignition advance degree calculation (AdvanceDC) tests.

General overview:
1. We replicate the settings you would set in tunerstudio by setting the tables directly in the code. The tables are
   smaller than in the actual firmware because we aren't testing the actual tables, we just need to be able to get stable values.
2. Every value in the tables is tested and the values are usually set to minimum, 0 , a normal value and maximum.
3. The tests are performed multiple times with different input values from the engineParameters array.

*/

#include <globals.h>
#include <speeduino.h>
#include <unity.h>
#include <corrections.h>
#include "secondaryTables.h"

#define CONSTRAINCAST_INT8(value) (int8_t)constrain(value, -127, 127)
#define _countof(x) (sizeof(x) / sizeof (x[0]))
#define OI OFFSET_IGNITION

enum ignitionTableType {
    TABLE_1,
    TABLE_2,
    TABLE_Multiplied
};

void setupAdvanceDCignitionTable(table3d16RpmLoad * ignTable, ignitionTableType tableType)
{
    const table3d_axis_t tempXAxis[] = {500, 1000, 2000, 2500, 3000, 3500, 4000, 5000, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 10000};
    const byte tempXAxisSize = _countof(tempXAxis);
    const table3d_axis_t tempYAxis[] = {510, 350, 250, 150, 100, 85, 75, 65, 50, 45, 37, 30, 23, 17, 9, 0};
    
    uint8_t const * valuesPerXAxis;
    //X axis values we care about                                 510                              100                             50                              23                      0
    const uint8_t valuesPerXAxisTable1[tempXAxisSize] =          { 70+OI,  70+OI,  70+OI,  30+OI,  30+OI,  30+OI,  30+OI,  0+OI,   0+OI,   0+OI,   0+OI,   -40+OI, -40+OI, -40+OI, -40+OI, -40+OI };
    const uint8_t valuesPerXAxisTable2[tempXAxisSize] =          { 69+OI,  69+OI,  69+OI,  20+OI,  20+OI,  20+OI,  20+OI,  1+OI,   1+OI,   1+OI,   1+OI,   -39+OI, -39+OI, -39+OI, -39+OI, -39+OI };
    const uint8_t valuesPerXAxisTableMultiplied[tempXAxisSize] = { 215+OI, 215+OI, 215+OI, 120+OI, 120+OI, 120+OI, 120+OI, 100+OI, 100+OI, 100+OI, 100+OI, 80+OI,  80+OI,  80+OI,  0+OI,   0+OI };
    switch (tableType) {
        case TABLE_1:
        valuesPerXAxis = valuesPerXAxisTable1;
        break;
        case TABLE_2:
        valuesPerXAxis = valuesPerXAxisTable2;
        break;
        case TABLE_Multiplied:
        valuesPerXAxis = valuesPerXAxisTableMultiplied;
        break;
    }

    memcpy(ignTable->axisX.axis, tempXAxis, sizeof(ignTable->axisX));
    memcpy(ignTable->axisY.axis, tempYAxis, sizeof(ignTable->axisY));

    byte valuesPos = 0;
    for (byte x = 0; x < tempXAxisSize*3; x++) { ignTable->values.values[valuesPos] = valuesPerXAxis[valuesPos/tempXAxisSize]; valuesPos++; }
    for (byte x = 0; x < tempXAxisSize*4; x++) { ignTable->values.values[valuesPos] = valuesPerXAxis[valuesPos/tempXAxisSize]; valuesPos++; }
    for (byte x = 0; x < tempXAxisSize*4; x++) { ignTable->values.values[valuesPos] = valuesPerXAxis[valuesPos/tempXAxisSize]; valuesPos++; }
    for (byte x = 0; x < tempXAxisSize*5; x++) { ignTable->values.values[valuesPos] = valuesPerXAxis[valuesPos/tempXAxisSize]; valuesPos++; }
}

byte iatRetBins[3] = {13, 82, 124};
byte iatRetValues[3] = {0, 10, 30};

#define CLT_VAL_OFFSET 15
byte cltAdvBins[4] = {-40+CALIBRATION_TEMPERATURE_OFFSET, 0+CALIBRATION_TEMPERATURE_OFFSET, 50+CALIBRATION_TEMPERATURE_OFFSET, 102+CALIBRATION_TEMPERATURE_OFFSET};
byte cltAdvValues[4] = {-13+CLT_VAL_OFFSET, -3+CLT_VAL_OFFSET, 0+CLT_VAL_OFFSET, 13+CLT_VAL_OFFSET};

byte cltAdvBinsZero[4] = {0+CALIBRATION_TEMPERATURE_OFFSET, 0+CALIBRATION_TEMPERATURE_OFFSET, 0+CALIBRATION_TEMPERATURE_OFFSET, 0+CALIBRATION_TEMPERATURE_OFFSET};
byte cltAdvValuesZero[4] = {0+CLT_VAL_OFFSET, 0+CLT_VAL_OFFSET, 0+CLT_VAL_OFFSET, 0+CLT_VAL_OFFSET};

byte flexAdvBins[3] = {0, 40, 100};
byte flexAdvAdj[3] = {0+OFFSET_IGNITION, 20+OFFSET_IGNITION, 50+OFFSET_IGNITION};

int16_t wmiAdvIgnitionTableValues[4] = {-40, 0, 30, 70};
byte wmiAdvBins[4] = {0/2, 50/2, 100/2, 510/2};
byte wmiAdvAdj[4] = {0+OFFSET_IGNITION, 0+OFFSET_IGNITION, 10+OFFSET_IGNITION, 50+OFFSET_IGNITION};

#define IAC_RPM_DIV 10
byte iacBins[4] = {-40+CALIBRATION_TEMPERATURE_OFFSET, 0+CALIBRATION_TEMPERATURE_OFFSET, 95+CALIBRATION_TEMPERATURE_OFFSET, 143+CALIBRATION_TEMPERATURE_OFFSET};
byte iacCLValues[4] = {2000/IAC_RPM_DIV, 1000/IAC_RPM_DIV, 500/IAC_RPM_DIV, 0/IAC_RPM_DIV};

#define IDLE_RPM_DIV 10
#define IDLE_RPM_OFFSET 500
#define IDLE_ADV_OFFSET 15
byte idleAdvBins[3] = {(-500+IDLE_RPM_OFFSET)/IDLE_RPM_DIV, (0+IDLE_RPM_OFFSET)/IDLE_RPM_DIV, (500+IDLE_RPM_OFFSET)/IDLE_RPM_DIV};
byte idleAdvValues[3] = {-15+IDLE_ADV_OFFSET, 0+IDLE_ADV_OFFSET, 50+IDLE_ADV_OFFSET};
const int idleAdvZeroRpmBin = 1;

//For tracking position in tables during tests
byte tablePosition1;
byte tablePosition2;

//Test input array
struct engineParams {
    long MAP;
    byte TPS;
    int16_t ignitionTable1Value;
    int16_t ignitionTable2Value;
    byte spark2Mode;
};
uint8_t engineParametersPos = 0;
const uint8_t engineParametersSize = 20;
engineParams engineParameters[engineParametersSize];

void setupAdvanceDC() { // One time table/test input setup
    setupAdvanceDCignitionTable(&ignitionTable, TABLE_1);

    engineParameters[0].MAP = 510;
    engineParameters[0].TPS = 0;
    engineParameters[0].ignitionTable1Value = 70;
    engineParameters[0].ignitionTable2Value = 69;

    engineParameters[1].MAP = 100;
    engineParameters[1].TPS = 0;
    engineParameters[1].ignitionTable1Value = 30;
    engineParameters[1].ignitionTable2Value = 20;

    engineParameters[2].MAP = 50;
    engineParameters[2].TPS = 0;
    engineParameters[2].ignitionTable1Value = 0;
    engineParameters[2].ignitionTable2Value = 1;

    engineParameters[3].MAP = 23;
    engineParameters[3].TPS = 0;
    engineParameters[3].ignitionTable1Value = -40;
    engineParameters[3].ignitionTable2Value = -39;

    engineParameters[4].MAP = 0;
    engineParameters[4].TPS = 0;
    engineParameters[4].ignitionTable1Value = -40;
    engineParameters[4].ignitionTable2Value = -39;

    engineParameters[15] = engineParameters[10] = engineParameters[5] = engineParameters[0];
    engineParameters[16] = engineParameters[11] = engineParameters[6] = engineParameters[1];
    engineParameters[17] = engineParameters[12] = engineParameters[7] = engineParameters[2];
    engineParameters[18] = engineParameters[13] = engineParameters[8] = engineParameters[3];
    engineParameters[19] = engineParameters[14] = engineParameters[9] = engineParameters[4];
    
    // 0-4 should use spark table 1 but spark table 2 conditional is enabled but the conditions are not met
    engineParameters[0].spark2Mode = engineParameters[1].spark2Mode = engineParameters[2].spark2Mode = engineParameters[3].spark2Mode = engineParameters[4].spark2Mode = SPARK2_MODE_CONDITIONAL_SWITCH;
    
    // 5-9 should use spark table 2 multiplied
    engineParameters[5].spark2Mode = engineParameters[6].spark2Mode = engineParameters[7].spark2Mode = engineParameters[8].spark2Mode = engineParameters[9].spark2Mode = SPARK2_MODE_MULTIPLY;
    engineParameters[5].ignitionTable2Value = 215;
    engineParameters[6].ignitionTable2Value = 120;
    engineParameters[7].ignitionTable2Value = 100;
    engineParameters[8].ignitionTable2Value = 80;
    engineParameters[9].ignitionTable2Value = 0;
    
    // 10-14 should use spark table 2 added to spark table 1
    engineParameters[10].spark2Mode = engineParameters[11].spark2Mode = engineParameters[12].spark2Mode = engineParameters[13].spark2Mode = engineParameters[14].spark2Mode = SPARK2_MODE_ADD;

    // 15-19 should use spark table 2, spark table 2 conditional is enabled and the conditions are met
    engineParameters[15].spark2Mode = engineParameters[16].spark2Mode = engineParameters[17].spark2Mode = engineParameters[18].spark2Mode = engineParameters[19].spark2Mode = SPARK2_MODE_CONDITIONAL_SWITCH;
    engineParameters[15].TPS = engineParameters[16].TPS = engineParameters[17].TPS = engineParameters[18].TPS = engineParameters[19].TPS = 97*2;
}

void resetAdvanceDC() { //Resets configuration data
    // Setup base values which disables all advance except ignition tables

    //General
    configPage2.ignAlgorithm = LOAD_SOURCE_MAP;
    currentStatus.MAP = engineParameters[engineParametersPos].MAP;
    currentStatus.RPM = 1000;
    currentStatus.RPMdiv100 = currentStatus.RPM/100;
    currentStatus.TPS = engineParameters[engineParametersPos].TPS;
    runSecsX10 = 0;
    

    //Secondary tables
    configPage10.spark2Mode = engineParameters[engineParametersPos].spark2Mode;
    configPage10.spark2Algorithm = LOAD_SOURCE_MAP;
    configPage10.spark2SwitchVariable = SPARK2_CONDITION_TPS;
    configPage10.spark2SwitchValue = 96*2;
    if (configPage10.spark2Mode == SPARK2_MODE_MULTIPLY) {
        setupAdvanceDCignitionTable(&ignitionTable2, TABLE_Multiplied);
    }
    else {
        setupAdvanceDCignitionTable(&ignitionTable2, TABLE_2);
    }

    //Flex fuel
    configPage2.flexEnabled = 0;
    flexAdvTable.valueSize = SIZE_BYTE;
    flexAdvTable.axisSize = SIZE_BYTE;
    flexAdvTable.xSize = 3;
    flexAdvTable.values = &flexAdvAdj;
    flexAdvTable.axisX = &flexAdvBins;
    table2D_getValue(&flexAdvTable, 1); // This is to prevent use of incorrect cached value when first access is for value 0

    //WMI
    configPage10.wmiEnabled = 0;
    configPage10.wmiAdvEnabled = 1;
    BIT_CLEAR(currentStatus.status4, BIT_STATUS4_WMI_EMPTY);
    configPage10.wmiTPS = 40*2;
    configPage10.wmiRPM = 8;
    configPage10.wmiMAP = 0;
    configPage10.wmiIAT = 40;
    wmiAdvTable.valueSize = SIZE_BYTE;
    wmiAdvTable.axisSize = SIZE_BYTE;
    wmiAdvTable.xSize = 4;
    wmiAdvTable.values = &wmiAdvAdj;
    wmiAdvTable.axisX = &wmiAdvBins;
    table2D_getValue(&wmiAdvTable, 1); // This is to prevent use of incorrect cached value when first access is for value 0

    //IAT
    currentStatus.IAT = 13;
    IATRetardTable.valueSize = SIZE_BYTE;
    IATRetardTable.axisSize = SIZE_BYTE;
    IATRetardTable.xSize = 3;
    IATRetardTable.values = &iatRetValues;
    IATRetardTable.axisX = &iatRetBins;

    //CLT
    currentStatus.coolant = 50;
    CLTAdvanceTable.valueSize = SIZE_BYTE;
    CLTAdvanceTable.axisSize = SIZE_BYTE;
    CLTAdvanceTable.xSize = 4;
    CLTAdvanceTable.values = &cltAdvValues;
    CLTAdvanceTable.axisX = &cltAdvBins;

    //Idle
    configPage2.idleAdvEnabled = 0;
    configPage2.idleAdvDelay = 0;
    configPage2.idleAdvRPM = 3000/100;
    configPage2.vssMode = 0;
    configPage2.idleAdvAlgorithm = 0;
    configPage2.idleAdvTPS = 98*2;
    idleAdvStart = 0;
    configPage9.idleAdvStartDelay = 0;
    idleTargetTable.valueSize = SIZE_BYTE;
    idleTargetTable.axisSize = SIZE_BYTE;
    idleTargetTable.xSize = 4;
    idleTargetTable.values = &iacCLValues;
    idleTargetTable.axisX = &iacBins;
    idleAdvanceTable.valueSize = SIZE_BYTE;
    idleAdvanceTable.axisSize = SIZE_BYTE;
    idleAdvanceTable.xSize = 3;
    idleAdvanceTable.values = &idleAdvValues;
    idleAdvanceTable.axisX = &idleAdvBins;
    table2D_getValue(&idleTargetTable, 1); // This is to prevent use of incorrect cached value when first access is for value 0
    table2D_getValue(&idleAdvanceTable, 1); // This is to prevent use of incorrect cached value when first access is for value 0

    //Soft rev limit
    configPage6.engineProtectType = PROTECT_CUT_OFF;
    configPage4.SoftRevLim = 100/100;
    softStartTime = 0;
    configPage4.SoftLimMax = 10;

    //Nitrous
    configPage10.n2o_enable = 0;

    //Soft Launch + Soft Flat Shift
    clutchTrigger = true;
    configPage6.flatSArm = 500/100;
    //Soft Launch
    configPage6.launchEnabled = 0;
    configPage6.lnchSoftLim = 800/100;
    configPage10.lnchCtrlTPS = 90*2;
    //Soft Flat Shift
    configPage6.flatSEnable = 0;
    configPage6.flatSSoftWin = 400/100;

    //Knock (not used)
    configPage10.knock_mode = KNOCK_MODE_OFF;

    //Fixed angle
    configPage2.fixAngEnable = 0;

    //Crank angle
    BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
}

int16_t getAdvanceDCbaseAdvance() { //Returns correct base advance from ignition table(s)
    int16_t result;
    int16_t ignitionTable1Value = engineParameters[engineParametersPos].ignitionTable1Value;
    int16_t ignitionTable2Value = engineParameters[engineParametersPos].ignitionTable2Value;
    if(configPage10.spark2Mode == SPARK2_MODE_MULTIPLY) {
        result = ignitionTable1Value*ignitionTable2Value/(int16_t)100;
    }
    else if(configPage10.spark2Mode == SPARK2_MODE_ADD) {
        result = ignitionTable1Value+ignitionTable2Value;
    }
    else if(configPage10.spark2Mode == SPARK2_MODE_CONDITIONAL_SWITCH && currentStatus.TPS > configPage10.spark2SwitchValue) {
        result = ignitionTable2Value;
    }
    else {
        result = ignitionTable1Value;
    }
    return result;
}

void getAdvanceDCspeeduinoAdvance() { //Gets the actual advance calculated by Speeduino
    currentStatus.advance = getAdvance1();
    calculateSecondarySpark();
}

void testAdvanceDCNone() {
    getAdvanceDCspeeduinoAdvance();
    int16_t result = getAdvanceDCbaseAdvance();
    TEST_ASSERT_EQUAL(CONSTRAINCAST_INT8(result), currentStatus.advance);
}

void testAdvanceDCIAT() {
    currentStatus.IAT = iatRetBins[tablePosition1];
    getAdvanceDCspeeduinoAdvance();
    int16_t result = getAdvanceDCbaseAdvance()-iatRetValues[tablePosition1];
    TEST_ASSERT_EQUAL(CONSTRAINCAST_INT8(result), currentStatus.advance);
}

void testAdvanceDCCLT() {
    currentStatus.coolant = cltAdvBins[tablePosition1]-CALIBRATION_TEMPERATURE_OFFSET;
    getAdvanceDCspeeduinoAdvance();
    int16_t result = getAdvanceDCbaseAdvance()+cltAdvValues[tablePosition1]-CLT_VAL_OFFSET;
    TEST_ASSERT_EQUAL(CONSTRAINCAST_INT8(result), currentStatus.advance);
}

void testAdvanceDCFlex() {
    currentStatus.ethanolPct = flexAdvBins[tablePosition1];
    getAdvanceDCspeeduinoAdvance();
    int16_t result = getAdvanceDCbaseAdvance()+flexAdvAdj[tablePosition1]-OFFSET_IGNITION;
    TEST_ASSERT_EQUAL(CONSTRAINCAST_INT8(result), currentStatus.advance);
}

void testAdvanceDCWMI() {
    currentStatus.MAP = wmiAdvBins[tablePosition1]*2;
    getAdvanceDCspeeduinoAdvance();
    int16_t result = wmiAdvIgnitionTableValues[tablePosition1]+wmiAdvAdj[tablePosition1]-OFFSET_IGNITION;
    TEST_ASSERT_EQUAL(CONSTRAINCAST_INT8(result), currentStatus.advance);
}

void testAdvanceIdleDelta() {
    int16_t delta = (idleAdvBins[tablePosition1]*IDLE_RPM_DIV)-IDLE_RPM_OFFSET;

    //Special case since RPM doesn't go negative
    bool zeroRPMzeroDelta = false;
    if (currentStatus.RPM == 0 && delta > 0) {
        zeroRPMzeroDelta = true;
    }

    if (delta > (int16_t)currentStatus.RPM) { currentStatus.RPM = 0; }
    else {
        currentStatus.RPM = currentStatus.RPM - delta;
    }
    currentStatus.RPMdiv100 = currentStatus.RPM/100;

    getAdvanceDCspeeduinoAdvance();
    
    int16_t result = 0;
    if (configPage2.idleAdvEnabled == 1) { result = getAdvanceDCbaseAdvance(); }

    if (zeroRPMzeroDelta) {
        result += idleAdvValues[idleAdvZeroRpmBin]-IDLE_ADV_OFFSET;
    }
    else {
        result += idleAdvValues[tablePosition1]-IDLE_ADV_OFFSET;
    }
    TEST_ASSERT_EQUAL(CONSTRAINCAST_INT8(result), currentStatus.advance);
}

void testAdvanceDCIdleTargets(byte tablePosition2) {
    currentStatus.coolant = (int16_t)iacBins[tablePosition2]-CALIBRATION_TEMPERATURE_OFFSET;
    char testName[40];
    for (tablePosition1 = 0; tablePosition1 < idleAdvanceTable.xSize; tablePosition1++) {
        currentStatus.RPM = (uint16_t)iacCLValues[tablePosition2]*IAC_RPM_DIV; //idle target RPM
        sprintf(testName, "testAdvanceIdle %u-%u", tablePosition2, tablePosition1);
        UnityDefaultTestRun(testAdvanceIdleDelta, testName, __LINE__);
    }
}

void testAdvanceDCSoftRevLimit() {
    getAdvanceDCspeeduinoAdvance();
    int16_t result = getAdvanceDCbaseAdvance();
    if (configPage2.SoftLimitMode == SOFT_LIMIT_RELATIVE) {
        result -= configPage4.SoftLimRetard;
    }
    else if (configPage2.SoftLimitMode == SOFT_LIMIT_FIXED) {
        result = configPage4.SoftLimRetard;
    }
    TEST_ASSERT_EQUAL(CONSTRAINCAST_INT8(result), currentStatus.advance);
}

void testAdvanceDCNitrous() {
    getAdvanceDCspeeduinoAdvance();
    int16_t result = getAdvanceDCbaseAdvance();
    if (currentStatus.nitrous_status == NITROUS_STAGE1) {
        result -= configPage10.n2o_stage1_retard;
    }
    else if (currentStatus.nitrous_status == NITROUS_STAGE2) {
        result -= configPage10.n2o_stage2_retard;
    }
    else if (currentStatus.nitrous_status == NITROUS_BOTH) {
        result -= configPage10.n2o_stage1_retard+configPage10.n2o_stage2_retard;
    }
    TEST_ASSERT_EQUAL(CONSTRAINCAST_INT8(result), currentStatus.advance);
}

void testAdvanceDCSoftLaunch() {
    getAdvanceDCspeeduinoAdvance();
    TEST_ASSERT_EQUAL(CONSTRAINCAST_INT8(configPage6.lnchRetard), currentStatus.advance);
}

void testAdvanceDCSoftFlatShift() {
    getAdvanceDCspeeduinoAdvance();
    TEST_ASSERT_EQUAL(CONSTRAINCAST_INT8(configPage6.flatSRetard), currentStatus.advance);
}

void testAdvanceDCFixed() {
    getAdvanceDCspeeduinoAdvance();
    TEST_ASSERT_EQUAL(CONSTRAINCAST_INT8(configPage4.FixAng), currentStatus.advance);
}

void testAdvanceDCCranking() {
    getAdvanceDCspeeduinoAdvance();
    TEST_ASSERT_EQUAL(CONSTRAINCAST_INT8(configPage4.CrankAng), currentStatus.advance);
}

void testAdvanceDC() {
    setupAdvanceDC();

    char engineParamMessage[100];

    for(engineParametersPos = 0; engineParametersPos < engineParametersSize; engineParametersPos++) {

        sprintf(engineParamMessage, "Beginning tests with engineParameters %u", engineParametersPos);
        UnityMessage(engineParamMessage, __LINE__);

        //Just from Ignition Table
        resetAdvanceDC();
        RUN_TEST(testAdvanceDCNone);

        //IAT
        resetAdvanceDC();
        for (tablePosition1 = 0; tablePosition1 < IATRetardTable.xSize; tablePosition1++) {
            RUN_TEST(testAdvanceDCIAT);
        }

        //CLT
        resetAdvanceDC();
        for (tablePosition1 = 0; tablePosition1 < CLTAdvanceTable.xSize; tablePosition1++) {
            RUN_TEST(testAdvanceDCCLT);
        }

        //Flex
        resetAdvanceDC();
        configPage2.flexEnabled = 1;
        for (tablePosition1 = 0; tablePosition1 < flexAdvTable.xSize; tablePosition1++) {
            RUN_TEST(testAdvanceDCFlex);
        }

        //Idle
        resetAdvanceDC();
        configPage2.idleAdvEnabled = 1;
        //Disable coolant adjustments as idle tests will change coolant temperature
        CLTAdvanceTable.values = &cltAdvValuesZero;
        CLTAdvanceTable.axisX = &cltAdvBinsZero;
        for (tablePosition2 = 0; tablePosition2 < idleTargetTable.xSize; tablePosition2++) {
            testAdvanceDCIdleTargets(tablePosition2);
        }
        configPage2.idleAdvEnabled = 2;
        for (tablePosition2 = 0; tablePosition2 < idleTargetTable.xSize; tablePosition2++) {
            testAdvanceDCIdleTargets(tablePosition2);
        }

        //Soft Rev Limit
        resetAdvanceDC();
        configPage6.engineProtectType = PROTECT_CUT_IGN;
        configPage2.SoftLimitMode = SOFT_LIMIT_RELATIVE;
        configPage4.SoftLimRetard = 0;
        RUN_TEST(testAdvanceDCSoftRevLimit);
        configPage4.SoftLimRetard = 30;
        RUN_TEST(testAdvanceDCSoftRevLimit);
        configPage4.SoftLimRetard = 80;
        RUN_TEST(testAdvanceDCSoftRevLimit);

        configPage2.SoftLimitMode = SOFT_LIMIT_FIXED;
        configPage4.SoftLimRetard = 0;
        RUN_TEST(testAdvanceDCSoftRevLimit);
        configPage4.SoftLimRetard = 30;
        RUN_TEST(testAdvanceDCSoftRevLimit);
        configPage4.SoftLimRetard = 80;
        RUN_TEST(testAdvanceDCSoftRevLimit);

        //Nitrous
        resetAdvanceDC();
        configPage10.n2o_enable = 2;
        currentStatus.nitrous_status = NITROUS_STAGE1;
        configPage10.n2o_stage1_retard = 0;
        RUN_TEST(testAdvanceDCNitrous);
        configPage10.n2o_stage1_retard = 20;
        RUN_TEST(testAdvanceDCNitrous);
        configPage10.n2o_stage1_retard = 40;
        RUN_TEST(testAdvanceDCNitrous);
        
        currentStatus.nitrous_status = NITROUS_STAGE2;
        configPage10.n2o_stage2_retard = 0;
        RUN_TEST(testAdvanceDCNitrous);
        configPage10.n2o_stage2_retard = 20;
        RUN_TEST(testAdvanceDCNitrous);
        configPage10.n2o_stage2_retard = 40;
        RUN_TEST(testAdvanceDCNitrous);

        currentStatus.nitrous_status = NITROUS_BOTH;
        configPage10.n2o_stage1_retard = 0;
        configPage10.n2o_stage2_retard = 0;
        RUN_TEST(testAdvanceDCNitrous);
        configPage10.n2o_stage1_retard = 20;
        configPage10.n2o_stage2_retard = 20;
        RUN_TEST(testAdvanceDCNitrous);
        configPage10.n2o_stage1_retard = 40;
        configPage10.n2o_stage2_retard = 40;
        RUN_TEST(testAdvanceDCNitrous);

        //Soft Launch
        resetAdvanceDC();
        configPage6.launchEnabled = 1;
        currentStatus.clutchEngagedRPM = 400;
        currentStatus.TPS = 95*2;
        configPage6.lnchRetard = -30;
        RUN_TEST(testAdvanceDCSoftLaunch);
        configPage6.lnchRetard = 0;
        RUN_TEST(testAdvanceDCSoftLaunch);
        configPage6.lnchRetard = 40;
        RUN_TEST(testAdvanceDCSoftLaunch);

        //Soft Flat Shift
        resetAdvanceDC();
        configPage6.flatSEnable = 1;
        currentStatus.RPM = 4900;
        currentStatus.clutchEngagedRPM = 5000;
        configPage6.flatSRetard = -30;
        RUN_TEST(testAdvanceDCSoftFlatShift);
        configPage6.flatSRetard = 0;
        RUN_TEST(testAdvanceDCSoftFlatShift);
        configPage6.flatSRetard = 80;
        RUN_TEST(testAdvanceDCSoftFlatShift);

        //Knock
        // Can't test knock as it isn't implemented yet

        //Fixed
        resetAdvanceDC();
        configPage2.fixAngEnable = 1;
        configPage4.FixAng = -64;
        RUN_TEST(testAdvanceDCFixed);
        configPage4.FixAng = 0;
        RUN_TEST(testAdvanceDCFixed);
        configPage4.FixAng = 64;
        RUN_TEST(testAdvanceDCFixed);

        //Cranking
        resetAdvanceDC();
        BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
        configPage4.CrankAng = -10;
        RUN_TEST(testAdvanceDCCranking);
        configPage4.CrankAng = 0;
        RUN_TEST(testAdvanceDCCranking);
        configPage4.CrankAng = 80;
        RUN_TEST(testAdvanceDCCranking);
    }

    //WMI - This test needs specific MAP values so run it separately
    engineParametersPos = 0;
    resetAdvanceDC();
    configPage10.wmiEnabled = 1;
    currentStatus.TPS = 50*2;
    for (tablePosition1 = 0; tablePosition1 < wmiAdvTable.xSize; tablePosition1++) {
        RUN_TEST(testAdvanceDCWMI);
    }
}