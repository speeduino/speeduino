
void test_shared_variableMissingToothPatterns(void (*triggerSetEndTeeth_TestMethod)(), bool isDualWheel);
void test_shared_triggerSetup(int teeth, int missingTeeth, int ignitionMode, int ignitionChannels);
static String test_local_createMessage(int channel, int teeth, int missingTeeth, int ignitionAngle, int triggerAngleOffset);
static int test_local_calculateEndTooth(int ignitionAngle, int triggerOffset, int totalTeeth, int missingTeeth, bool isDualWheel);
