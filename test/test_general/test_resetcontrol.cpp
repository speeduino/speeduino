#include "../test_utils.h"
#include "resetControl.h"

static decoder_status_t fakeDecoderStatus = {};
static decoder_status_t fakeGetStatus(void)
{
    return fakeDecoderStatus;
}

static void assert_matchResetControlToEngineState_On(statuses &current,uint16_t rpm, SyncStatus syncStatus, uint8_t resetPin)
{
    setRpm(current, rpm);

    current.decoder.getStatus = fakeGetStatus;
    fakeDecoderStatus.syncStatus = syncStatus;

    matchResetControlToEngineState(current);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(resetPin));
    TEST_ASSERT_TRUE(current.resetPreventActive);
}

static void assert_matchResetControlToEngineState_Off(statuses &current, uint16_t rpm, SyncStatus syncStatus, uint8_t resetPin)
{
    setRpm(current, rpm);

    current.decoder.getStatus = fakeGetStatus;
    fakeDecoderStatus.syncStatus = syncStatus;

    matchResetControlToEngineState(current);
    TEST_ASSERT_EQUAL(LOW, digitalRead(resetPin));
    TEST_ASSERT_FALSE(current.resetPreventActive);
}

static void assert_matchResetControlToEngineState_NoChange(statuses &current, uint16_t rpm, SyncStatus syncStatus, uint8_t resetPin, uint8_t expectedState)
{
    bool oldState = current.resetPreventActive;
    setRpm(current, rpm);

    current.decoder.getStatus = fakeGetStatus;
    fakeDecoderStatus.syncStatus = syncStatus;

    matchResetControlToEngineState(current);
    TEST_ASSERT_EQUAL(expectedState, digitalRead(resetPin));
    TEST_ASSERT_EQUAL(oldState, current.resetPreventActive);
}


static void test_matchResetControlToEngineState_PreventWhenRunning(void)
{
    constexpr uint8_t resetPin = 42;
    statuses current{};

    initialiseResetControl(current, RESET_CONTROL_PREVENT_WHEN_RUNNING, resetPin);

    TEST_ASSERT_EQUAL(RESET_CONTROL_PREVENT_WHEN_RUNNING, getResetControl());
    TEST_ASSERT_EQUAL(LOW, digitalRead(resetPin));
    TEST_ASSERT_FALSE(current.resetPreventActive);

    assert_matchResetControlToEngineState_On(current, 900, SyncStatus::Full, resetPin);
    current.resetPreventActive = true; // Simulate that the reset prevention is now active
    assert_matchResetControlToEngineState_Off(current, 0, SyncStatus::Full, resetPin);
    current.resetPreventActive = true; // Simulate that the reset prevention is now active
    assert_matchResetControlToEngineState_Off(current, 900, SyncStatus::None, resetPin);
    current.resetPreventActive = true; // Simulate that the reset prevention is now active
    assert_matchResetControlToEngineState_Off(current, 0, SyncStatus::None, resetPin);
    current.resetPreventActive = false; // Simulate that the reset prevention is off
    assert_matchResetControlToEngineState_On(current, 900, SyncStatus::Partial, resetPin);
    current.resetPreventActive = true; // Simulate that the reset prevention is now active
    assert_matchResetControlToEngineState_Off(current, 0, SyncStatus::Partial, resetPin);
}

static void test_matchResetControlToEngineState_PreventAlways(void)
{
    constexpr uint8_t resetPin = 42;
    statuses current{};

    initialiseResetControl(current, RESET_CONTROL_PREVENT_ALWAYS, resetPin);

    TEST_ASSERT_EQUAL(RESET_CONTROL_PREVENT_ALWAYS, getResetControl());
    TEST_ASSERT_EQUAL(HIGH, digitalRead(resetPin));
    TEST_ASSERT_TRUE(current.resetPreventActive);

    assert_matchResetControlToEngineState_NoChange(current, 0, SyncStatus::None, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(current, 900, SyncStatus::None, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(current, 0, SyncStatus::Full, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(current, 900, SyncStatus::Full, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(current, 0, SyncStatus::Partial, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(current, 900, SyncStatus::Partial, resetPin, HIGH);
}

static void test_matchResetControlToEngineState_SerialCommand(void)
{
    constexpr uint8_t resetPin = 42;
    statuses current{};

    initialiseResetControl(current, RESET_CONTROL_SERIAL_COMMAND, resetPin);

    TEST_ASSERT_EQUAL(RESET_CONTROL_SERIAL_COMMAND, getResetControl());
    TEST_ASSERT_EQUAL(HIGH, digitalRead(resetPin));
    TEST_ASSERT_FALSE(current.resetPreventActive);

    assert_matchResetControlToEngineState_NoChange(current, 0, SyncStatus::None, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(current, 900, SyncStatus::None, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(current, 0, SyncStatus::Full, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(current, 900, SyncStatus::Full, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(current, 0, SyncStatus::Partial, resetPin, HIGH);
    assert_matchResetControlToEngineState_NoChange(current, 900, SyncStatus::Partial, resetPin, HIGH);
}

void testResetControl(void) 
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_matchResetControlToEngineState_PreventWhenRunning);
        RUN_TEST_P(test_matchResetControlToEngineState_PreventAlways);
        RUN_TEST_P(test_matchResetControlToEngineState_SerialCommand);
    }
}
