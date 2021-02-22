extern void testTables();
void setup_FuelTable(void);
void test_tableLookup_50pct(void);
void test_tableLookup_exact1Axis(void);
void test_tableLookup_exact2Axis(void);
void test_tableLookup_overMaxX(void);
void test_tableLookup_overMaxY(void);
void test_tableLookup_underMinX(void);
void test_tableLookup_underMinY(void);
void test_all_incrementing(void);

  //Go through the 8 rows and add the column values
  const PROGMEM byte tempRow1[16] = {109, 111, 112, 113, 114, 114, 114, 115, 115, 115, 114, 114, 113, 112, 111, 111};
  const PROGMEM byte tempRow2[16] = {104, 106, 107, 108, 109, 109, 110, 110, 110, 110, 110, 109, 108, 107, 107, 106};
  const PROGMEM byte tempRow3[16] = {98, 101, 103, 103, 104, 105, 105, 105, 105, 105, 105, 104, 104, 103, 102, 102};
  const PROGMEM byte tempRow4[16] = {93, 96, 98, 99, 99, 100, 100, 101, 101, 101, 100, 100, 99, 98, 98, 97};
  const PROGMEM byte tempRow5[16] = {81, 86, 88, 89, 90, 91, 91, 91, 91, 91, 91, 90, 90, 89, 89, 88};
  const PROGMEM byte tempRow6[16] = {74, 80, 83, 84, 85, 86, 86, 86, 87, 86, 86, 86, 85, 84, 84, 84};
  const PROGMEM byte tempRow7[16] = {68, 75, 78, 79, 81, 81, 81, 82, 82, 82, 82, 81, 81, 80, 79, 79};
  const PROGMEM byte tempRow8[16] = {61, 69, 72, 74, 76, 76, 77, 77, 77, 77, 77, 76, 76, 75, 75, 74};
  const PROGMEM byte tempRow9[16] = {54, 62, 66, 69, 71, 71, 72, 72, 72, 72, 72, 72, 71, 71, 70, 70};
  const PROGMEM byte tempRow10[16] = {48, 56, 60, 64, 66, 66, 68, 68, 68, 68, 67, 67, 67, 66, 66, 65};
  const PROGMEM byte tempRow11[16] = {42, 49, 54, 58, 61, 62, 62, 63, 63, 63, 63, 62, 62, 61, 61, 61};
  const PROGMEM byte tempRow12[16] = {38, 43, 48, 52, 55, 56, 57, 58, 58, 58, 58, 58, 57, 57, 57, 56};
  const PROGMEM byte tempRow13[16] = {36, 39, 42, 46, 50, 51, 52, 53, 53, 53, 53, 53, 53, 52, 52, 52};
  const PROGMEM byte tempRow14[16] = {35, 36, 38, 41, 44, 46, 47, 48, 48, 49, 48, 48, 48, 48, 47, 47};
  const PROGMEM byte tempRow15[16] = {34, 35, 36, 37, 39, 41, 42, 43, 43, 44, 44, 44, 43, 43, 43, 43};
  const PROGMEM byte tempRow16[16] = {34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 34, 34, 34, 34, 34, 34};