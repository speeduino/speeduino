#if !defined(__TEST_DECODERS_H__)
#define __TEST_DECODERS_H__

void testDecoders();

void test_addToothLogEntry_toothLog();
void test_addToothLogEntry_compositeLog();
void test_addToothLogEntry();

void test_loggerPrimaryISR();
void test_loggerSecondaryISR();

void test_decoderStdGetRPM();

void test_decoderSetFilter();

void test_decoderCrankingGetRPM();


#endif // __TEST_DECODERS_H__