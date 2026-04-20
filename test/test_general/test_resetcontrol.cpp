#include "../test_utils.h"
#include "resetControl.h"

static decoder_status_t fakeDecoderStatus = {};
static decoder_status_t fakeGetStatus(void)
{
    return fakeDecoderStatus;
}

static void test_matchResetControlToEngineState(void)
{
    constexpr uint8_t resetPin = 42;
    statuses current{};

    current.decoder.getStatus = fakeGetStatus;
    current.RPM = 0;
    fakeDecoderStatus.syncStatus = SyncStatus::None;

    initialiseResetControl(current, RESET_CONTROL_PREVENT_WHEN_RUNNING, resetPin);

    TEST_ASSERT_EQUAL(RESET_CONTROL_PREVENT_WHEN_RUNNING, getResetControl());
    TEST_ASSERT_EQUAL(LOW, digitalRead(resetPin));
    TEST_ASSERT_FALSE(current.resetPreventActive);

    current.RPM = 900;
    fakeDecoderStatus.syncStatus = SyncStatus::Full;
    matchResetControlToEngineState(current);

    TEST_ASSERT_EQUAL(HIGH, digitalRead(resetPin));
    TEST_ASSERT_TRUE(current.resetPreventActive);

    current.RPM = 0;
    fakeDecoderStatus.syncStatus = SyncStatus::None;
    matchResetControlToEngineState(current);

    TEST_ASSERT_EQUAL(LOW, digitalRead(resetPin));
    TEST_ASSERT_FALSE(current.resetPreventActive);
}

void testResetControl(void) 
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_matchResetControlToEngineState);
    }
}
