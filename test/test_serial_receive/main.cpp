#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "../test_utils.h"
#include "globals.h"
#include "comms.h"
#include "comms_legacy.h"
#include "pages.h"
#include "sensors.h"

#if defined(NATIVE_BOARD)
#include <vector>
#include <FastCRC.h>

extern uint32_t serialReceiveStartTime;

class packet_stream_t : public Stream
{
public:
    std::vector<uint8_t> input;
    std::vector<uint8_t> output;
    size_t cursor = 0;
    size_t received = 0;
    int available(void) override { return received - cursor; }
    int availableForWrite(void) override { return 4096; }
    int peek(void) override { return available() ? input[cursor] : -1; }
    int read(void) override { return available() ? input[cursor++] : -1; }
    void flush(void) override {}
    size_t write(uint8_t value) override { output.push_back(value); return 1; }
    void frame(std::vector<uint8_t> payload)
    {
        input.clear(); output.clear(); cursor = 0;
        input.push_back(payload.size() >> 8);
        input.push_back(payload.size());
        input.insert(input.end(), payload.begin(), payload.end());
        FastCRC32 crc;
        uint8_t empty = 0;
        uint32_t checksum = crc.crc32(payload.empty() ? &empty : payload.data(), payload.size());
        for (int8_t shift = 24; shift >= 0; shift -= 8) { input.push_back(checksum >> shift); }
        received = input.size();
    }
    void pump(void)
    {
        pPrimarySerial = this;
        currentStatus.allowLegacyComms = false;
        serialReceive();
        pPrimarySerial = &Serial;
    }
    void expect(uint8_t code)
    {
        TEST_ASSERT_TRUE(output.size() >= 3);
        TEST_ASSERT_EQUAL_UINT8(code, output[2]);
        TEST_ASSERT_EQUAL(SERIAL_INACTIVE, serialStatusFlag);
    }
};

static void test_empty_frame(void)
{
    packet_stream_t port;
    port.frame({}); port.pump(); port.expect(0x84);
}

static void test_capacity_boundary(void)
{
    packet_stream_t port;
    std::vector<uint8_t> payload(TS_SERIAL_BUFFER_SIZE, 0);
    payload[0] = 'C';
    port.frame(payload); port.pump(); port.expect(0);
    payload.push_back(0);
    port.frame(payload); port.pump(); port.expect(0x84);
    port.frame({'C'}); port.pump(); port.expect(0);
}

static void test_fragmented_oversized_frame(void)
{
    packet_stream_t port;
    port.frame(std::vector<uint8_t>(TS_SERIAL_BUFFER_SIZE + 1, 0xFF));
    port.received = 66;
    port.pump();
    TEST_ASSERT_TRUE(port.output.empty());
    TEST_ASSERT_EQUAL(SERIAL_RECEIVE_INPROGRESS, serialStatusFlag);
    port.received = port.input.size();
    port.pump(); port.expect(0x84);
    port.frame({'C'}); port.pump(); port.expect(0);
}

static void test_bad_crc_does_not_write_and_recovers(void)
{
    packet_stream_t port;
    const uint8_t previous = getPageValue(veSetPage, 0);
    port.frame({'M', 0, veSetPage, 0, 0, 1, 0, static_cast<uint8_t>(previous ^ 0xFF)});
    port.input.back() ^= 1U;
    port.pump(); port.expect(0x82);
    TEST_ASSERT_EQUAL_UINT8(previous, getPageValue(veSetPage, 0));
    port.frame({'C'}); port.pump(); port.expect(0);
}

static void test_oversized_frame_timeout_recovers(void)
{
    packet_stream_t port;
    port.frame(std::vector<uint8_t>(TS_SERIAL_BUFFER_SIZE + 1, 0xFF));
    port.received = 66;
    port.pump();
    TEST_ASSERT_EQUAL(SERIAL_RECEIVE_INPROGRESS, serialStatusFlag);
    // Expire the existing 400 ms receive timeout without a wall-clock sleep.
    serialReceiveStartTime = millis() - 401U;
    port.pump(); port.expect(0x80);
    port.frame({'C'}); port.pump(); port.expect(0);
}

static void test_short_command_headers(void)
{
    packet_stream_t port;
    const char commands[] = {'b', 'B', 'd', 'E', 'k', 'M', 'p', 'r', 't', 'w'};
    for (char command : commands)
    {
        port.frame({static_cast<uint8_t>(command)});
        port.pump(); port.expect(0x84);
    }
}

static void test_page_write_requires_received_bytes(void)
{
    packet_stream_t port;
    const uint8_t previous = getPageValue(veSetPage, 0);
    port.frame({'M', 0, veSetPage, 0, 0, 2, 0, 0x5A});
    port.pump(); port.expect(0x84);
    TEST_ASSERT_EQUAL_UINT8(previous, getPageValue(veSetPage, 0));
    port.frame({'M', 0, veSetPage, 0, 0, 1, 0, 0x5A});
    port.pump(); port.expect(0);
    TEST_ASSERT_EQUAL_UINT8(0x5A, getPageValue(veSetPage, 0));
    setPageValue(veSetPage, 0, previous);
}

static void test_calibration_bounds(void)
{
    packet_stream_t port;
    const uint8_t previous = o2CalibrationTable.values[0];
    port.frame({'t', 0, 2, 4, 0, 0, 1, 0x55}); // offset 1024, length 1
    port.pump(); port.expect(0x84);
    port.frame({'t', 0, 2, 3, 255, 0, 2, 0x55, 0x55}); // crosses 1024
    port.pump(); port.expect(0x84);
    port.frame({'t', 0, 2, 0, 0, 0, 0}); // empty calibration chunk
    port.pump(); port.expect(0x84);
    port.frame({'t', 0, 2, 0, 0, 0, 32}); // absent body
    port.pump(); port.expect(0x84);
    port.frame({'t', 0, 0, 0, 0, 0, 64}); // truncated CLT calibration
    port.pump(); port.expect(0x84);
    TEST_ASSERT_EQUAL_UINT8(previous, o2CalibrationTable.values[0]);
}

static void test_valid_first_calibration_chunk(void)
{
    packet_stream_t port;
    std::vector<uint8_t> payload(263, 0x55);
    payload[0] = 't'; payload[1] = 0; payload[2] = 2;
    payload[3] = 0; payload[4] = 0; payload[5] = 1; payload[6] = 0;
    port.frame(payload); port.pump(); port.expect(0);
    for (uint8_t i = 0; i < 8; ++i)
    {
        TEST_ASSERT_EQUAL_UINT8(0x55, o2CalibrationTable.values[i]);
        TEST_ASSERT_EQUAL_UINT16(i * 32U, o2CalibrationTable.axis[i]);
    }
}
#else
static void test_serial_receive_requires_native(void)
{
    TEST_IGNORE_MESSAGE("Packet buffers are host-only; firmware is built by the board matrix.");
}
#endif

void runAllTests(void)
{
#if defined(NATIVE_BOARD)
    RUN_TEST_P(test_empty_frame);
    RUN_TEST_P(test_capacity_boundary);
    RUN_TEST_P(test_fragmented_oversized_frame);
    RUN_TEST_P(test_bad_crc_does_not_write_and_recovers);
    RUN_TEST_P(test_oversized_frame_timeout_recovers);
    RUN_TEST_P(test_short_command_headers);
    RUN_TEST_P(test_page_write_requires_received_bytes);
    RUN_TEST_P(test_calibration_bounds);
    RUN_TEST_P(test_valid_first_calibration_chunk);
#else
    RUN_TEST_P(test_serial_receive_requires_native);
#endif
}
TEST_HARNESS(runAllTests)
