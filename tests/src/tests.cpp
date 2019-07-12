#include <iostream>
#include <gtest/gtest.h>
#include <list>

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

TEST(SpeeduinoTestClass, ConfigUpdates)
{
//    EEPROM.write(0, 2); // this line make the tests crash
    setup();
    EXPECT_EQ(12, EEPROM.read(0));
    EXPECT_EQ(true, hasInterrupts);
}

TEST(SpeeduinoInjTestClass, PairedSimTest)
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

TEST(SpeeduinoInjTestClass, SemiSequentialSimTest)
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

TEST(SpeeduinoInjTestClass, SequentialSimTest)
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

TEST(SpeeduinoInjTestClass, PairedAltTest)
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

TEST(SpeeduinoInjTestClass, SemiSequentialAltTest)
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

TEST(SpeeduinoInjTestClass, SequentialAltTest)
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

TEST(SpeeduinoInjTestClass, 2CylSemiSeqAltTest)
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

TEST(SpeeduinoInjTestClass, 2CylSeqAltTest)
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

TEST(SpeeduinoInjTestClass, 3CylPairedSimTest)
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

TEST(SpeeduinoInjTestClass, 3CylSeqAltTest)
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

TEST(SpeeduinoInjTestClass, 4CylPairedSimTest)
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

TEST(SpeeduinoInjTestClass, 4CylSemiSeqAltTest)
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

TEST(SpeeduinoInjTestClass, 4CylSeqAltTest)
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

int main(int pArgCount, char *pArgValues[])
{
    ::testing::InitGoogleTest(&pArgCount, pArgValues);
    return RUN_ALL_TESTS();
}
