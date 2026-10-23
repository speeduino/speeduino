#include "../../test_utils.h"
#include "src/decoders/basicDistributor.h"

using namespace decoders::basic_distributor;

static void test_tooth_angle(void)
{
    config2 page2{};
    page2.nCylinders = 3;

    page2.strokes = FOUR_STROKE;
    {
        auto subject = initialise(page2);
        TEST_ASSERT_EQUAL(720/3, subject.triggerToothAngle);
    }

    page2.strokes = TWO_STROKE;
    {
        auto subject = initialise(page2);
        TEST_ASSERT_EQUAL(360/3, subject.triggerToothAngle);
    }
}

static void test_stall_time(void)
{
    config2 page2{};
    page2.strokes = FOUR_STROKE;

    page2.nCylinders = 3;
    {
        auto subject = initialise(page2);
        TEST_ASSERT_EQUAL(444240, subject.MAX_STALL_TIME);
    }

    page2.nCylinders = 5;
    {
        auto subject = initialise(page2);
        TEST_ASSERT_EQUAL(479952, subject.MAX_STALL_TIME);
    }
}


static void test_noteeth(void)
{
    config2 page2{};
    page2.strokes = FOUR_STROKE;

    page2.nCylinders = 0;
    auto subject = initialise(page2);
    TEST_ASSERT_EQUAL(1, subject.triggerActualTeeth);
}

void testBasicDistribInitialise(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_tooth_angle);
    RUN_TEST_P(test_stall_time);
    RUN_TEST_P(test_noteeth);
}