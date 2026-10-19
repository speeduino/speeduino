#include "../test_utils.h"
#include "globals.h"
#include "comms.h"
#include "comms_legacy.h"
#include "comms_secondary.h"
#include "pages.h"
#include "logger.h"

#if defined(NATIVE_BOARD)
// Independent RX/TX buffers catch accidental reads/writes through global Serial.
class TestPrimaryStream : public Stream
{
public:
    uint8_t rx[16] = {};
    uint8_t tx[32] = {};
    uint8_t rxLength = 0;
    uint8_t rxIndex = 0;
    uint8_t txLength = 0;
    int capacity = 32;
    int available() override { return rxLength - rxIndex; }
    int peek() override { return available() ? rx[rxIndex] : -1; }
    int read() override { return available() ? rx[rxIndex++] : -1; }
    void flush() override {}
    int availableForWrite() override { return capacity; }
    size_t write(uint8_t value) override
    {
        if (capacity == 0 || txLength == sizeof(tx)) { return 0; }
        tx[txLength++] = value;
        --capacity;
        return 1;
    }
};

static void test_primary_burn_uses_selected_stream(void)
{
    TestPrimaryStream port;
    pPrimarySerial = &port;
    for (uint8_t cmd : {'b', 'B'})
    {
        port.rx[0] = cmd;
        port.rx[1] = 0;
        port.rx[2] = 2;
        port.rxLength = 3;
        port.rxIndex = 0;
        serialStatusFlag = SERIAL_INACTIVE;
        legacySerialCommand();
        TEST_ASSERT_EQUAL(SERIAL_INACTIVE, serialStatusFlag);
        TEST_ASSERT_EQUAL_UINT8(2, port.rxIndex);
    }
    currentStatus.commCompat = false;
    pPrimarySerial = &Serial;
}

static void test_primary_button_reads_selected_stream(void)
{
    TestPrimaryStream port;
    pPrimarySerial = &port;
    port.rx[0] = 'E';
    port.rx[1] = 1;
    port.rx[2] = 1; // TS_CMD_TEST_ENBL = 257
    port.rxLength = 2;
    currentStatus.RPM = 0;
    currentStatus.isTestModeActive = false;
    serialStatusFlag = SERIAL_INACTIVE;
    legacySerialCommand();
    TEST_ASSERT_FALSE(currentStatus.isTestModeActive);
    TEST_ASSERT_EQUAL(SERIAL_COMMAND_INPROGRESS_LEGACY, serialStatusFlag);
    port.rxLength = 3;
    legacySerialCommand();
    TEST_ASSERT_TRUE(currentStatus.isTestModeActive);
    TEST_ASSERT_EQUAL_UINT8(3, port.rxIndex);
    TEST_ASSERT_EQUAL(SERIAL_INACTIVE, serialStatusFlag);
    currentStatus.isTestModeActive = false;
    pPrimarySerial = &Serial;
}

static void test_primary_page_write_uses_selected_stream(void)
{
    TestPrimaryStream port;
    pPrimarySerial = &port;
    const uint8_t saved = getPageValue(2, 0);
    const uint8_t request[] = {'M', 0, 2, 0, 0, 1, 0, 42};
    memcpy(port.rx, request, sizeof(request));
    port.rxLength = sizeof(request);
    serialStatusFlag = SERIAL_INACTIVE;
    legacySerialCommand();
    TEST_ASSERT_EQUAL_UINT8(42, getPageValue(2, 0));
    TEST_ASSERT_EQUAL_UINT8(sizeof(request), port.rxIndex);
    TEST_ASSERT_EQUAL(SERIAL_INACTIVE, serialStatusFlag);
    setPageValue(2, 0, saved);
    pPrimarySerial = &Serial;
}

static void test_primary_realtime_response_resumes_on_selected_stream(void)
{
    TestPrimaryStream port;
    pPrimarySerial = &port;
    pSecondarySerial = &Serial;
    const uint8_t request[] = {'r', 0, 0x30, 0, 0, 3, 0};
    memcpy(port.rx, request, sizeof(request));
    port.rxLength = sizeof(request);
    port.capacity = 1;
    serialStatusFlag = SERIAL_INACTIVE;
    legacySerialCommand();
    TEST_ASSERT_EQUAL(SERIAL_TRANSMIT_INPROGRESS_LEGACY, serialStatusFlag);
    TEST_ASSERT_EQUAL_UINT8(1, port.txLength);
    const uint8_t expected[] = {getTSLogEntry(0), getTSLogEntry(1), getTSLogEntry(2)};
    port.capacity = 32;
    serialTransmit();
    TEST_ASSERT_EQUAL(SERIAL_INACTIVE, serialStatusFlag);
    TEST_ASSERT_EQUAL_UINT8(3, port.txLength);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, port.tx, sizeof(expected));
    pPrimarySerial = &Serial;
}
#endif

void testPrimarySerial(void)
{
#if defined(NATIVE_BOARD)
    RUN_TEST(test_primary_burn_uses_selected_stream);
    RUN_TEST(test_primary_button_reads_selected_stream);
    RUN_TEST(test_primary_page_write_uses_selected_stream);
    RUN_TEST(test_primary_realtime_response_resumes_on_selected_stream);
#endif
}
