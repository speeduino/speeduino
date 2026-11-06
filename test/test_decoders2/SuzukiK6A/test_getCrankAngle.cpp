#include "../../test_utils.h"
#include "test_context.h"

static void test_getCrankAngle(uint8_t toothNum, uint16_t expectedCrankAngle, uint16_t expectedToothAngle) 
{
  context_k6a_t ctx;
  ctx.init();
  ctx.setup_primaryfilter(150, 100);
  ctx.decoderState.toothCurrentCount = toothNum;
  CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
  setAngleConverterRevolutionTime(1);

  TEST_ASSERT_INT16_WITHIN(1, expectedCrankAngle, ctx.getCrankAngle(BASE_1ST_TOOTH_TIME));
  TEST_ASSERT_EQUAL(expectedToothAngle, ctx.decoderState.triggerToothAngle);
}

static void test_getCrankAngle(void) {
    // Zero isn't a valid tooth, but just in case....
    test_getCrankAngle(0, 650, 90);
    test_getCrankAngle(1, 0, 70);
    test_getCrankAngle(2, 170, 170);
    test_getCrankAngle(3, 240, 70);
    test_getCrankAngle(4, 410, 170);
    test_getCrankAngle(5, 480, 70);
    test_getCrankAngle(6, 515, 35);
    test_getCrankAngle(7, 650, 135);
    // 8 isn't a valid tooth, but just in case....
    test_getCrankAngle(8, 0, 70);
}

static void test_getRevolutionTime(void)
{
  context_k6a_t ctx;
  ctx.init();
  ctx.setup_primaryfilter(120000UL, 1000);
  
  // Standard calculation at cam speed: half the time between the last 2 tooth #1
  ctx.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  ctx.current.startRevolutions = 1; // not cranking
  TEST_ASSERT_EQUAL_UINT32(60000UL, ctx.getRevolutionTime());
}

void testSuzukiK6AgetCrankAngle()
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_getCrankAngle);
        RUN_TEST_P(test_getRevolutionTime);
    }
}