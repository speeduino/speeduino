#include "../test_utils.h"
#include "resetControl.h"

static decoder_status_t fakeDecoderStatus = {};
static decoder_status_t fakeGetStatus(void)
{
    return fakeDecoderStatus;
}

static void assert_matchResetControlToEngineState_On(uint16_t rpm, SyncStatus syncStatus, uint8_t resetPin)
{
    statuses current = {};
    current.setRpm( rpm);

    current.decoder.getStatus = fakeGetStatus;
    fakeDecoderStatus.syncStatus = syncStatus;

    matchResetControlToEngineState(current);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(resetPin));
    TEST_ASSERT_TRUE(isResetPreventActive());
}

static void assert_matchResetControlToEngineState_Off(uint16_t rpm, SyncStatus syncStatus, uint8_t resetPin)
{
    statuses current = {};
    current.setRpm( rpm);

    current.decoder.getStatus = fakeGetStatus;
    fakeDecoderStatus.syncStatus = syncStatus;

    matchResetControlToEngineState(current);
    TEST_ASSERT_EQUAL(LOW, digitalRead(resetPin));
    TEST_ASSERT_FALSE(isResetPreventActive());
}

static void assert_matchResetControlToEngineState_NoChange(uint16_t rpm, SyncStatus syncStatus, uint8_t resetPin, uint8_t expectedState)
{
    bool oldState = isResetPreventActive();
    statuses current = {};
    current.setRpm( rpm);

    current.decoder.getStatus = fakeGetStatus;
    fakeDecoderStatus.syncStatus = syncStatus;

    matchResetControlToEngineState(current);
    TEST_ASSERT_EQUAL(expectedState, digitalRead(resetPin));
    TEST_ASSERT_EQUAL(oldState, isResetPreventActive());
}


static void test_matchResetControlToEngineState_PreventWhenRunning(void)
{
    constexpr uint8_t resetPin = 42;

    initialiseResetControl(ResetControlMode::PreventWhenRunning, resetPin);

    TEST_ASSERT_EQUAL(ResetControlMode::PreventWhenRunning, getResetControlMode());
    TEST_ASSERT_EQUAL(LOW, digitalRead(resetPin));
    TEST_ASSERT_FALSE(isResetPreventActive());

    assert_matchResetControlToEngineState_On(900, SyncStatus::Full, resetPin);
    assert_matchResetControlToEngineState_Off(0, SyncStatus::Full, resetPin);
    assert_matchResetControlToEngineState_Off(900, SyncStatus::None, resetPin);
    assert_matchResetControlToEngineState_Off(0, SyncStatus::None, resetPin);
    assert_matchResetControlToEngineState_On(900, SyncStatus::Partial, resetPin);
    assert_matchResetControlToEngineState_Off(0, SyncStatus::Partial, resetPin);
}

static void test_matchResetControlToEngineState_PreventAlways(void)
{
    constexpr uint8_t resetPin = 42;

    initialiseResetControl(ResetControlMode::PreventAlways, resetPin);

    TEST_ASSERT_EQUAL(ResetControlMode::PreventAlways, getResetControlMode());
    TEST_ASSERT_EQUAL(HIGH, digitalRead(resetPin));
    TEST_ASSERT_TRUE(isResetPreventActive());

    assert_matchResetControlToEngineState_NoChange(0, SyncStatus::None, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(900, SyncStatus::None, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(0, SyncStatus::Full, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(900, SyncStatus::Full, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(0, SyncStatus::Partial, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(900, SyncStatus::Partial, resetPin, HIGH);
}

static void test_matchResetControlToEngineState_SerialCommand(void)
{
    constexpr uint8_t resetPin = 42;

    initialiseResetControl(ResetControlMode::SerialCommand, resetPin);

    TEST_ASSERT_EQUAL(ResetControlMode::SerialCommand, getResetControlMode());
    TEST_ASSERT_EQUAL(HIGH, digitalRead(resetPin));
    TEST_ASSERT_FALSE(isResetPreventActive());

    assert_matchResetControlToEngineState_NoChange(0, SyncStatus::None, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(900, SyncStatus::None, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(0, SyncStatus::Full, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(900, SyncStatus::Full, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(0, SyncStatus::Partial, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(900, SyncStatus::Partial, resetPin, HIGH);
}

void testResetControl(void) 
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_matchResetControlToEngineState_PreventWhenRunning);
        RUN_TEST_P(test_matchResetControlToEngineState_PreventAlways);
        RUN_TEST_P(test_matchResetControlToEngineState_SerialCommand);
    }
}
