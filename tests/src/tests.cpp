#include <iostream>
#include <gtest/gtest.h>
#include <list>
#include <type_traits>

#include "speeduino.cpp"

const float REQ_FUEL = 12.0;
const int REQ_FUEL_US = REQ_FUEL * 1000;

std::list<int> get_divisors(int n)
{
    if (n <= 1)
        return {n};

    std::list<int> result = {1, n};

    for (int i = 2, d = n; i < d; i++)
    {
        if (n % i == 0)
        {
            d = n / i;
            result.push_back(i);
            if (d != i)
                result.push_back(d);
        }
    }
    return result;
}

// This function takes the input parameters we would put in TunerStudio and converts them to what speeduino knows.
bool set_constants(float reqFuel, unsigned nCylinders, unsigned nSquirts, bool alternate, int injLayout)
{
    // The checks below were removed from TunerStudio.
    //    if(nCylinders < nSquirts) return false;
    //    if(alternate && (nSquirts & 1) == 1) return false; // Can not alternate odd number of squirts

    reqFuel /= nSquirts;
    if (alternate)
        reqFuel *= 2;

    configPage2.reqFuel = (byte)(reqFuel * 10);
    configPage2.nCylinders = nCylinders;
    configPage2.divider = nCylinders / nSquirts;
    configPage2.injTiming = alternate;
    configPage2.strokes = FOUR_STROKE;
    configPage2.engineType = EVEN_FIRE;
    configPage2.injLayout = injLayout;
    configPage10.stagingEnabled = false;

    do { writeAllConfig(); } while(eepromWritesPending);

    // Reset the variables below to their default value.
    channel1InjEnabled = false;
    channel2InjEnabled = false;
    channel3InjEnabled = false;
    channel4InjEnabled = false;
    channel5InjEnabled = false;
    channel6InjEnabled = false;
    channel7InjEnabled = false;
    channel8InjEnabled = false;

    return true;
}

TEST(SpeeduinoTests, ConfigUpdates)
{
//    EEPROM.write(0, 2); // this line make the tests crash
    setup();
    EXPECT_EQ(12, EEPROM.read(0));
    EXPECT_EQ(true, hasInterrupts);
}

TEST(SpeeduinoInjTests, PairedSimTest)
{
    for (int nCylinders : {1, 2, 3, 4, 5, 6, 8})
        for (int nSquirts : get_divisors(nCylinders))
        {
            ASSERT_TRUE(set_constants(REQ_FUEL, nCylinders, nSquirts, false, INJ_PAIRED));
            setup();
            EXPECT_EQ(nSquirts, currentStatus.nSquirts);
            EXPECT_EQ(REQ_FUEL_US / nSquirts / 2, req_fuel_uS); // thought: should be double that (aka EXPECT_EQ(REQ_FUEL_US/nSquirts, req_fuel_uS))
            EXPECT_EQ(720 / nSquirts, CRANK_ANGLE_MAX_INJ);
        }
}

TEST(SpeeduinoInjTests, SemiSequentialSimTest)
{
    for (int nCylinders : {1, 2, 3, 4, 5, 6, 8})
        for (int nSquirts : get_divisors(nCylinders))
        {
            ASSERT_TRUE(set_constants(REQ_FUEL, nCylinders, nSquirts, false, INJ_SEMISEQUENTIAL));
            setup();
            EXPECT_EQ(nSquirts, currentStatus.nSquirts);
            EXPECT_EQ(REQ_FUEL_US / nSquirts / 2, req_fuel_uS); // thought: should be double that (aka EXPECT_EQ(REQ_FUEL_US/nSquirts, req_fuel_uS))
            EXPECT_EQ(720 / nSquirts, CRANK_ANGLE_MAX_INJ);
        }
}

TEST(SpeeduinoInjTests, SequentialSimTest)
{
    for (int nCylinders : {1, 2, 3, 4}) // sequential doesn't support more than 4 cylinders
        for (int nSquirts : get_divisors(nCylinders))
        {
            ASSERT_TRUE(set_constants(REQ_FUEL, nCylinders, nSquirts, false, INJ_SEQUENTIAL));
            setup();
            EXPECT_EQ(1, currentStatus.nSquirts);
            EXPECT_EQ(REQ_FUEL_US / nSquirts, req_fuel_uS); // thought: should stay constant (aka EXPECT_EQ(REQ_FUEL_US, req_fuel_uS))
            EXPECT_EQ(720, CRANK_ANGLE_MAX_INJ);
        }
}

TEST(SpeeduinoInjTests, PairedAltTest)
{
    for (int nCylinders : {1, 2, 3, 4, 5, 6, 8})
        for (int nSquirts : get_divisors(nCylinders))
        {
            ASSERT_TRUE(set_constants(REQ_FUEL, nCylinders, nSquirts, true, INJ_PAIRED));
            setup();
            EXPECT_EQ(nSquirts, currentStatus.nSquirts);
            EXPECT_EQ(REQ_FUEL_US / nSquirts, req_fuel_uS);
            EXPECT_EQ(720 / nSquirts, CRANK_ANGLE_MAX_INJ);
        }
}

TEST(SpeeduinoInjTests, SemiSequentialAltTest)
{
    for (int nCylinders : {1, 2, 3, 4, 5, 6, 8})
        for (int nSquirts : get_divisors(nCylinders))
        {
            ASSERT_TRUE(set_constants(REQ_FUEL, nCylinders, nSquirts, true, INJ_SEMISEQUENTIAL));
            setup();
            EXPECT_EQ(nSquirts, currentStatus.nSquirts);
            EXPECT_EQ(REQ_FUEL_US / nSquirts, req_fuel_uS);
            EXPECT_EQ(720 / nSquirts, CRANK_ANGLE_MAX_INJ);
        }
}

TEST(SpeeduinoInjTests, SequentialAltTest)
{
    for (int nCylinders : {1, 2, 3, 4}) // sequential doesn't support more than 4 cylinders
        for (int nSquirts : get_divisors(nCylinders))
        {
            ASSERT_TRUE(set_constants(REQ_FUEL, nCylinders, nSquirts, true, INJ_SEQUENTIAL));
            setup();
            EXPECT_EQ(1, currentStatus.nSquirts);
            EXPECT_EQ(REQ_FUEL_US / nSquirts * 2, req_fuel_uS); // thought: should stay constant (aka EXPECT_EQ(REQ_FUEL_US, req_fuel_uS))
            EXPECT_EQ(720, CRANK_ANGLE_MAX_INJ);
        }
}

TEST(SpeeduinoInjTests, 2CylSemiSeqAltTest)
{
    //                      ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(REQ_FUEL, 2, 2, true, INJ_SEMISEQUENTIAL));
    setup();

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(false, channel3InjEnabled);
    EXPECT_EQ(false, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0, channel1InjDegrees);
    EXPECT_EQ(180, channel2InjDegrees);
}

TEST(SpeeduinoInjTests, 2CylSeqAltTest)
{
    //                      ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(REQ_FUEL, 2, 2, true, INJ_SEQUENTIAL));
    setup();

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(false, channel3InjEnabled);
    EXPECT_EQ(false, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0, channel1InjDegrees);
    EXPECT_EQ(180, channel2InjDegrees);
}

TEST(SpeeduinoInjTests, 3CylPairedSimTest)
{
    //                       ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(REQ_FUEL, 3, 1, false, INJ_PAIRED));
    setup();

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(true, channel3InjEnabled);
    EXPECT_EQ(false, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0, channel1InjDegrees);
    EXPECT_EQ(0, channel2InjDegrees);
    EXPECT_EQ(0, channel3InjDegrees);
}

TEST(SpeeduinoInjTests, 3CylSeqAltTest)
{
    //                      ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(REQ_FUEL, 3, 3, true, INJ_SEQUENTIAL));
    setup();

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(true, channel3InjEnabled);
    EXPECT_EQ(false, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0, channel1InjDegrees);
    EXPECT_EQ(240, channel2InjDegrees);
    EXPECT_EQ(480, channel3InjDegrees);
}

TEST(SpeeduinoInjTests, 4CylPairedSimTest)
{
    //                       ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(REQ_FUEL, 4, 1, false, INJ_PAIRED));
    setup();

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(false, channel3InjEnabled);
    EXPECT_EQ(false, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0, channel1InjDegrees);
    EXPECT_EQ(0, channel2InjDegrees);
}

TEST(SpeeduinoInjTests, 4CylSemiSeqAltTest)
{
    //                      ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(REQ_FUEL, 4, 2, true, INJ_SEMISEQUENTIAL));
    setup();

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(false, channel3InjEnabled);
    EXPECT_EQ(false, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0, channel1InjDegrees);
    EXPECT_EQ(180, channel2InjDegrees);
}

TEST(SpeeduinoInjTests, 4CylSeqAltTest)
{
    //                      ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(REQ_FUEL, 4, 2, true, INJ_SEQUENTIAL));
    setup();

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(true, channel3InjEnabled);
    EXPECT_EQ(true, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0, channel1InjDegrees);
    EXPECT_EQ(180, channel2InjDegrees);
    EXPECT_EQ(360, channel3InjDegrees);
    EXPECT_EQ(540, channel4InjDegrees);
}

TEST(SpeeduinoScheduleTests, ScheduleTest1)
{
  FUEL1_COUNTER = 0;
  FUEL1_COMPARE = 0;

  setFuelSchedule1(4800, 800);
  EXPECT_EQ(FUEL1_COUNTER + 1200, FUEL1_COMPARE);
  EXPECT_EQ(PENDING, fuelSchedule1.Status);

  FUEL1_COUNTER = FUEL1_COMPARE;
  TIMER3_COMPA_vect();
  EXPECT_EQ(FUEL1_COUNTER + 200, FUEL1_COMPARE);
  EXPECT_EQ(RUNNING, fuelSchedule1.Status);

  FUEL1_COUNTER = FUEL1_COMPARE;
  TIMER3_COMPA_vect();
  EXPECT_EQ(OFF, fuelSchedule1.Status);
}

TEST(SpeeduinoScheduleTests, ScheduleTest2)
{
  FUEL1_COUNTER = 0;

  setFuelSchedule1(3200, 1600);
  EXPECT_EQ(FUEL1_COUNTER + 800, FUEL1_COMPARE);
  EXPECT_EQ(PENDING, fuelSchedule1.Status);

  FUEL1_COUNTER = FUEL1_COMPARE;
  TIMER3_COMPA_vect();
  EXPECT_EQ(FUEL1_COUNTER + 400, FUEL1_COMPARE);
  EXPECT_EQ(RUNNING, fuelSchedule1.Status);

  FUEL1_COUNTER += 200;

  setFuelSchedule1(4000, 800);
  EXPECT_EQ(FUEL1_COUNTER + 200, FUEL1_COMPARE);
  EXPECT_EQ(RUNNING, fuelSchedule1.Status);

  FUEL1_COUNTER = FUEL1_COMPARE;
  TIMER3_COMPA_vect();
  EXPECT_EQ(FUEL1_COUNTER + 800, FUEL1_COMPARE);
  EXPECT_EQ(PENDING, fuelSchedule1.Status);

  FUEL1_COUNTER = FUEL1_COMPARE;
  TIMER3_COMPA_vect();
  EXPECT_EQ(FUEL1_COUNTER + 200, FUEL1_COMPARE);
  EXPECT_EQ(RUNNING, fuelSchedule1.Status);

  FUEL1_COUNTER = FUEL1_COMPARE;
  TIMER3_COMPA_vect();
  EXPECT_EQ(OFF, fuelSchedule1.Status);
}

TEST(SpeeduinoTests, TypeTraits)
{
  // Expecting no class overhead over C counterpart
  EXPECT_EQ(true, std::is_standard_layout<table2D>());
  EXPECT_EQ(true, std::is_standard_layout<table3D>());
  EXPECT_EQ(true, std::is_standard_layout<Schedule>());

  EXPECT_EQ(true, std::is_standard_layout<config2>());
  EXPECT_EQ(true, std::is_standard_layout<config4>());
  EXPECT_EQ(true, std::is_standard_layout<config6>());
  EXPECT_EQ(true, std::is_standard_layout<config9>());
  EXPECT_EQ(true, std::is_standard_layout<config10>());
}

TEST(SpeeduinoTests, Table2dGetValue)
{
  const int tableSize = 8;
  table2D table;

  table.axisX = nullptr;
  table.values = nullptr;
  table.valueSize = SIZE_BYTE;

  initialisationComplete = false;
  table2D_setSize(&table, tableSize);

  ASSERT_NE(nullptr, table.axisX);
  ASSERT_NE(nullptr, table.values);

  for(int i = 0; i < tableSize; i++)
  {
    table.axisX[i] = i*10;
    table.values[i] = i*10;
  }

  EXPECT_EQ(0,  table2D_getValue(&table, -10));
  EXPECT_EQ(0,  table2D_getValue(&table,   0));
  EXPECT_EQ(10, table2D_getValue(&table,  10));
  EXPECT_EQ(35, table2D_getValue(&table,  35));
  EXPECT_EQ(70, table2D_getValue(&table,  70));
  EXPECT_EQ(70, table2D_getValue(&table,  80));

}

TEST(SpeeduinoTests, Table3dGetValue)
{
  const int tableSize = 8;
  table3D table;

  table.axisX = nullptr;
  table.axisY = nullptr;
  table.values = nullptr;

  initialisationComplete = false;
  table3D_setSize(&table, tableSize);

  ASSERT_NE(nullptr, table.axisX);
  ASSERT_NE(nullptr, table.axisY);
  ASSERT_NE(nullptr, table.values);

  for(int i = 0; i < tableSize; i++)
  {
    table.axisX[i] = i*10;
    table.axisY[tableSize-1-i] = i*10;
    for(int j = 0; j < tableSize; j++)
    {
      table.values[tableSize-1-j][i] = i + j*10;
    }
  }

  //                            table   Y    X
  EXPECT_EQ(0,  get3DTableValue(&table, 0, -10));
  EXPECT_EQ(0,  get3DTableValue(&table, 0,   0));
  EXPECT_EQ(1,  get3DTableValue(&table, 0,  10));
  EXPECT_EQ(3,  get3DTableValue(&table, 0,  35));
  EXPECT_EQ(7,  get3DTableValue(&table, 0,  70));
  EXPECT_EQ(7,  get3DTableValue(&table, 0,  80));

  EXPECT_EQ(0,  get3DTableValue(&table, -10, 0));
  EXPECT_EQ(0,  get3DTableValue(&table,   0, 0));
  EXPECT_EQ(10, get3DTableValue(&table,  10, 0));
  EXPECT_EQ(35, get3DTableValue(&table,  35, 0));
  EXPECT_EQ(70, get3DTableValue(&table,  70, 0));
  EXPECT_EQ(70, get3DTableValue(&table,  80, 0));

  EXPECT_EQ(0,  get3DTableValue(&table, -10, -10));
  EXPECT_EQ(0,  get3DTableValue(&table,   0,   0));
  EXPECT_EQ(11, get3DTableValue(&table,  10,  10));
  EXPECT_EQ(38, get3DTableValue(&table,  35,  35));
  EXPECT_EQ(77, get3DTableValue(&table,  70,  70));
  EXPECT_EQ(77, get3DTableValue(&table,  80,  80));
}

int main(int pArgCount, char *pArgValues[])
{
    ::testing::InitGoogleTest(&pArgCount, pArgValues);
    return RUN_ALL_TESTS();
}
