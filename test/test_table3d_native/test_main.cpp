#include <unity.h>
#include "table3d_interpolate.cpp"
#include "..\test_misc\tests_tables.cpp"

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_tableLookup_50pct);
  RUN_TEST(test_tableLookup_exact1Axis);
  RUN_TEST(test_tableLookup_exact2Axis);
  RUN_TEST(test_tableLookup_overMaxX);
  RUN_TEST(test_tableLookup_overMaxY);
  RUN_TEST(test_tableLookup_underMinX);
  RUN_TEST(test_tableLookup_underMinY);
  RUN_TEST(test_tableLookup_roundUp);
  RUN_TEST(test_all_incrementing);
  UNITY_END();

    return 0;
}