#include <globals.h>
#include <init.h>
#include <unity.h>
#include "tests_tables.h"

void setup_FuelTable(void)
{
  //Setup the fuel table with some sane values for testing
  //Table is setup per the below
  /*
  100 |  109 |  111 |  112 |  113 |  114 |  114 |  114 |  115 |  115 |  115 |  114 |  114 |  113 |  112 |  111 |  111
  96  |  104 |  106 |  107 |  108 |  109 |  109 |  110 |  110 |  110 |  110 |  110 |  109 |  108 |  107 |  107 |  106
  90  |   98 |  101 |  103 |  103 |  104 |  105 |  105 |  105 |  105 |  105 |  105 |  104 |  104 |  103 |  102 |  102
  86  |   93 |   96 |   98 |   99 |   99 |  100 |  100 |  101 |  101 |  101 |  100 |  100 |   99 |   98 |   98 |   97
  76  |   81 |   86 |   88 |   89 |   90 |   91 |   91 |   91 |   91 |   91 |   91 |   90 |   90 |   89 |   89 |   88
  70  |   74 |   80 |   83 |   84 |   85 |   86 |   86 |   86 |   87 |   86 |   86 |   86 |   85 |   84 |   84 |   84
  65  |   68 |   75 |   78 |   79 |   81 |   81 |   81 |   82 |   82 |   82 |   82 |   81 |   81 |   80 |   79 |   79
  60  |   61 |   69 |   72 |   74 |   76 |   76 |   77 |   77 |   77 |   77 |   77 |   76 |   76 |   75 |   75 |   74
  56  |   54 |   62 |   66 |   69 |   71 |   71 |   72 |   72 |   72 |   72 |   72 |   72 |   71 |   71 |   70 |   70
  50  |   48 |   56 |   60 |   64 |   66 |   66 |   68 |   68 |   68 |   68 |   67 |   67 |   67 |   66 |   66 |   65
  46  |   42 |   49 |   54 |   58 |   61 |   62 |   62 |   63 |   63 |   63 |   63 |   62 |   62 |   61 |   61 |   61
  40  |   38 |   43 |   48 |   52 |   55 |   56 |   57 |   58 |   58 |   58 |   58 |   58 |   57 |   57 |   57 |   56
  36  |   36 |   39 |   42 |   46 |   50 |   51 |   52 |   53 |   53 |   53 |   53 |   53 |   53 |   52 |   52 |   52
  30  |   35 |   36 |   38 |   41 |   44 |   46 |   47 |   48 |   48 |   49 |   48 |   48 |   48 |   48 |   47 |   47
  26  |   34 |   35 |   36 |   37 |   39 |   41 |   42 |   43 |   43 |   44 |   44 |   44 |   43 |   43 |   43 |   43
  16  |   34 |   34 |   34 |   34 |   34 |   34 |   34 |   34 |   34 |   35 |   34 |   34 |   34 |   34 |   34 |   34
      ----------------------------------------------------------------------------------------------------------------
         500 |  700 |  900 | 1200 | 1600 | 2000 | 2500 | 3100 | 3500 | 4100 | 4700 | 5300 | 5900 | 6500 | 6750 | 7000
  */
  
  int tempXAxis[16] = {500,700, 900, 1200, 1600, 2000, 2500, 3100, 3500, 4100, 4700, 5300, 5900, 6500, 6750, 7000};
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.axisX[x] = tempXAxis[x]; }
  int tempYAxis[16] = {100, 96, 90, 86, 76, 70, 66, 60, 56, 50, 46, 40, 36, 30, 26, 16};
  for (byte x = 0; x< fuelTable.ySize; x++) { fuelTable.axisY[x] = tempYAxis[x]; }


  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[0][x] = pgm_read_byte_near(tempRow1 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[1][x] = pgm_read_byte_near(tempRow2 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[2][x] = pgm_read_byte_near(tempRow3 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[3][x] = pgm_read_byte_near(tempRow4 + x);}
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[4][x] = pgm_read_byte_near(tempRow5 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[5][x] = pgm_read_byte_near(tempRow6 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[6][x] = pgm_read_byte_near(tempRow7 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[7][x] = pgm_read_byte_near(tempRow8 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[8][x] = pgm_read_byte_near(tempRow9 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[9][x] = pgm_read_byte_near(tempRow10 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[10][x] = pgm_read_byte_near(tempRow11 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[11][x] = pgm_read_byte_near(tempRow12 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[12][x] = pgm_read_byte_near(tempRow13 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[13][x] = pgm_read_byte_near(tempRow14 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[14][x] = pgm_read_byte_near(tempRow15 + x); }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[15][x] = pgm_read_byte_near(tempRow16 + x); }
  /*
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[0][x] = tempRow1[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[1][x] = tempRow2[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[2][x] = tempRow3[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[3][x] = tempRow4[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[4][x] = tempRow5[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[5][x] = tempRow6[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[6][x] = tempRow7[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[7][x] = tempRow8[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[8][x] = tempRow9[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[9][x] = tempRow10[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[10][x] = tempRow11[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[11][x] = tempRow12[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[12][x] = tempRow13[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[13][x] = tempRow14[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[14][x] = tempRow15[x]; }
  for (byte x = 0; x< fuelTable.xSize; x++) { fuelTable.values[15][x] = tempRow16[x]; }
  */
  
}

void testTables()
{
  RUN_TEST(test_tableLookup_50pct);
  RUN_TEST(test_tableLookup_exact1Axis);
  RUN_TEST(test_tableLookup_exact2Axis);
  RUN_TEST(test_tableLookup_overMaxX);
  RUN_TEST(test_tableLookup_overMaxY);
  RUN_TEST(test_tableLookup_underMinX);
  RUN_TEST(test_tableLookup_underMinY);
  //RUN_TEST(test_all_incrementing);
  
}

void test_tableLookup_50pct(void)
{
  //Tests a lookup that is exactly 50% of the way between cells on both the X and Y axis
  //initialiseAll(); //Run the main initialise function
  setup_FuelTable();

  currentStatus.RPM = 2250;
  currentStatus.fuelLoad = 53;

  uint16_t tempVE = get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(tempVE, 69);
}

void test_tableLookup_exact1Axis(void)
{
  //Tests a lookup that exactly matches on the X axis and 50% of the way between cells on the Y axis
  initialiseAll(); //Run the main initialise function
  setup_FuelTable();

  currentStatus.RPM = 2500;
  currentStatus.fuelLoad = 48;

  uint16_t tempVE = get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(tempVE, 65);
}

void test_tableLookup_exact2Axis(void)
{
  //Tests a lookup that exactly matches on both the X and Y axis
  initialiseAll(); //Run the main initialise function
  setup_FuelTable();

  currentStatus.RPM = 2500;
  currentStatus.fuelLoad = 70;

  uint16_t tempVE = get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(tempVE, 86);
}

void test_tableLookup_overMaxX(void)
{
  //Tests a lookup where the RPM exceeds the highest value in the table. The Y value is a 50% match
  initialiseAll(); //Run the main initialise function
  setup_FuelTable();

  currentStatus.RPM = 10000;
  currentStatus.fuelLoad = 73;

  uint16_t tempVE = get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(tempVE, 86);
}

void test_tableLookup_overMaxY(void)
{
  //Tests a lookup where the load value exceeds the highest value in the table. The X value is a 50% match
  initialiseAll(); //Run the main initialise function
  setup_FuelTable();

  currentStatus.RPM = 600;
  currentStatus.fuelLoad = 110;

  uint16_t tempVE = get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(tempVE, 110);
}

void test_tableLookup_underMinX(void)
{
  //Tests a lookup where the RPM value is below the lowest value in the table. The Y value is a 50% match
  initialiseAll(); //Run the main initialise function
  setup_FuelTable();

  currentStatus.RPM = 300;
  currentStatus.fuelLoad = 38;

  uint16_t tempVE = get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(tempVE, 37);
}

void test_tableLookup_underMinY(void)
{
  //Tests a lookup where the load value is below the lowest value in the table. The X value is a 50% match
  initialiseAll(); //Run the main initialise function
  setup_FuelTable();

  currentStatus.RPM = 600;
  currentStatus.fuelLoad = 8;

  uint16_t tempVE = get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(tempVE, 34);
}

void test_all_incrementing(void)
{
  //Test the when going up both the load and RPM axis that the returned value is always equal or higher to the previous one
  //Tests all combinations of load/rpm from between 0-200 load and 0-9000 rpm
  //WARNING: This can take a LONG time to run. It is disabled by default for this reason
  uint16_t tempVE = 0;
  
  for(uint16_t rpm = 0; rpm<8000; rpm+=100)
  {
    tempVE = 0;
    for(uint8_t load = 0; load<120; load++)
    {
      uint16_t newVE = get3DTableValue(&fuelTable, load, rpm);
      TEST_ASSERT_GREATER_OR_EQUAL(tempVE, newVE);
      tempVE = newVE;
    }
  }
}