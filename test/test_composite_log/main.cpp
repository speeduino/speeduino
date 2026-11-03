#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "../test_utils.h"
#include "globals.h"
#include "comms.h"
#include "comms_legacy.h"

extern void sendCompositeLog(void);

class log_test_stream_t : public Stream
{
public:
    uint16_t written = 0;
    int available(void) override { return 0; }
    int availableForWrite(void) override { return 1024; }
    int read(void) override { return -1; }
    int peek(void) override { return -1; }
    void flush(void) override {}
    size_t write(uint8_t) override { ++written; return 1; }
};

static void assert_padded_log(uint16_t samples, uint32_t timestamp)
{
    log_test_stream_t stream;
    Stream *oldSerial = pPrimarySerial;
    pPrimarySerial = &stream;
    currentStatus.isToothLog1Full = false;
    toothHistoryIndex = samples;
    logItemsTransmitted = 0;
    toothHistory[0] = timestamp;
    compositeLogHistory[0] = 1;
    sendCompositeLog();
    pPrimarySerial = oldSerial;

    const uint32_t expected = samples == 0 ? 0 : timestamp;
    for (uint16_t i = samples; i < TOOTH_LOG_SIZE; ++i)
    {
        TEST_ASSERT_EQUAL_UINT32(expected, toothHistory[i]);
        TEST_ASSERT_EQUAL_UINT8(0, compositeLogHistory[i]);
    }
    TEST_ASSERT_EQUAL_UINT16(2U + 1U + (TOOTH_LOG_SIZE * 5U) + 4U, stream.written);
    TEST_ASSERT_EQUAL_UINT16(0, toothHistoryIndex);
}

static void test_empty_log(void) { assert_padded_log(0, 12345); }
static void test_one_sample(void) { assert_padded_log(1, 12345); }

void runAllTests(void)
{
    RUN_TEST_P(test_empty_log);
    RUN_TEST_P(test_one_sample);
}
TEST_HARNESS(runAllTests)
