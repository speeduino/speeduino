#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "../test_utils.h"
#include "globals.h"
#include "comms.h"
#include "comms_legacy.h"
#include "comms_secondary.h"

extern bool processSecondaryCanReply(Stream &port);

#if defined(NATIVE_BOARD)
using reply_stream_base_t = SECONDARY_SERIAL_T;
#else
using reply_stream_base_t = Stream;
#endif

class reply_stream_t : public reply_stream_base_t
{
public:
    uint8_t data[22] = {1, 0, 0, 0, 0, 0, 0, 0, 0x34, 0x12};
    uint8_t cursor = 0;
    uint8_t received = 0;
    uint8_t emptyReads = 0;
    int available(void) override { return received - cursor; }
    int peek(void) override { return available() ? data[cursor] : -1; }
    int read(void) override { if (available()) { return data[cursor++]; } ++emptyReads; return -1; }
    void flush(void) override {}
    size_t write(uint8_t) override { return 1; }
};

static void test_fragmented_success(void)
{
    reply_stream_t port;
    configPage9.caninput_source_start_byte[0] = 6;
    configPage9.caninput_source_num_bytes = 1;
    currentStatus.canin[0] = 42;
    for (uint8_t n = 0; n < 10; ++n)
    {
        port.received = n;
        TEST_ASSERT_FALSE(processSecondaryCanReply(port));
        TEST_ASSERT_EQUAL_UINT8(0, port.cursor);
        TEST_ASSERT_EQUAL_UINT16(42, currentStatus.canin[0]);
    }
    port.received = 10;
    TEST_ASSERT_TRUE(processSecondaryCanReply(port));
    TEST_ASSERT_EQUAL_UINT16(0x1234, currentStatus.canin[0]);
    TEST_ASSERT_EQUAL_UINT8(10, port.cursor);
    TEST_ASSERT_EQUAL_UINT8(0, port.emptyReads);
}

static void test_short_failure(void)
{
    reply_stream_t port;
    port.data[0] = 0;
    port.received = 1;
    TEST_ASSERT_FALSE(processSecondaryCanReply(port));
    port.received = 2;
    TEST_ASSERT_TRUE(processSecondaryCanReply(port));
    TEST_ASSERT_EQUAL_UINT8(2, port.cursor);
    TEST_ASSERT_EQUAL_UINT8(0, port.emptyReads);
}

static void test_invalid_channel_consumes_reply(void)
{
    reply_stream_t port;
    port.data[1] = 255;
    port.received = 10;
    currentStatus.canin[0] = 42;
    TEST_ASSERT_TRUE(processSecondaryCanReply(port));
    TEST_ASSERT_EQUAL_UINT8(10, port.cursor);
    TEST_ASSERT_EQUAL_UINT16(42, currentStatus.canin[0]);
}

static void test_single_byte_and_last_byte(void)
{
    for (uint8_t twoBytes = 0; twoBytes < 2; ++twoBytes)
    {
        reply_stream_t port;
        port.received = 10;
        configPage9.caninput_source_num_bytes = twoBytes;
        configPage9.caninput_source_start_byte[0] = 7;
        TEST_ASSERT_TRUE(processSecondaryCanReply(port));
        TEST_ASSERT_EQUAL_UINT16(0x12, currentStatus.canin[0]);
        TEST_ASSERT_EQUAL_UINT8(0, port.emptyReads);
    }
}

#if defined(NATIVE_BOARD)
static void pump_secondary(reply_stream_t &port)
{
    SECONDARY_SERIAL_T *previousPort = pSecondarySerial;
    const uint8_t previousProtocol = configPage9.secondarySerialProtocol;
    pSecondarySerial = &port;
    configPage9.secondarySerialProtocol = SECONDARY_SERIAL_PROTO_CAN;
    secondserial_Command();
    pSecondarySerial = previousPort;
    configPage9.secondarySerialProtocol = previousProtocol;
}

static void test_dispatch_fragmented_and_consecutive_replies(void)
{
    reply_stream_t port;
    for (uint8_t i = 10; i > 0; --i) { port.data[i] = port.data[i - 1]; }
    port.data[0] = 'G';
    port.data[11] = 'G'; port.data[12] = 0; port.data[13] = 0;
    configPage9.caninput_source_num_bytes = 1;
    configPage9.caninput_source_start_byte[0] = 6;
    currentStatus.canin[0] = 42;
    serialSecondaryStatusFlag = SERIAL_INACTIVE;
    for (uint8_t n = 1; n < 11; ++n)
    {
        port.received = n;
        pump_secondary(port);
        TEST_ASSERT_EQUAL(SERIAL_COMMAND_INPROGRESS_LEGACY, serialSecondaryStatusFlag);
        TEST_ASSERT_EQUAL_UINT8(1, port.cursor);
        TEST_ASSERT_EQUAL_UINT16(42, currentStatus.canin[0]);
    }
    port.received = 11;
    pump_secondary(port);
    TEST_ASSERT_EQUAL(SERIAL_INACTIVE, serialSecondaryStatusFlag);
    TEST_ASSERT_EQUAL_UINT16(0x1234, currentStatus.canin[0]);
    port.received = 14;
    pump_secondary(port);
    TEST_ASSERT_EQUAL(SERIAL_INACTIVE, serialSecondaryStatusFlag);
    TEST_ASSERT_EQUAL_UINT8(14, port.cursor);
    TEST_ASSERT_EQUAL_UINT16(0x1234, currentStatus.canin[0]);
    TEST_ASSERT_EQUAL_UINT8(0, port.emptyReads);
}
#endif

void runAllTests(void)
{
    RUN_TEST_P(test_fragmented_success);
    RUN_TEST_P(test_short_failure);
    RUN_TEST_P(test_invalid_channel_consumes_reply);
    RUN_TEST_P(test_single_byte_and_last_byte);
#if defined(NATIVE_BOARD)
    RUN_TEST_P(test_dispatch_fragmented_and_consecutive_replies);
#endif
}
TEST_HARNESS(runAllTests)
