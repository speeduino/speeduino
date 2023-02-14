#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "decoders.h"

#ifndef UNIT_TEST 
  extern unsigned long micros_safe_injection; // Hack so that vscode sees the variable
#endif

struct isDecoderStalled_testdata {
  unsigned long micros;
  unsigned long toothLastToothTime;
  bool expected;
} *isDecoderStalled_testdata_current;

void testIsDecoderStalled_runTest() {
  isDecoderStalled_testdata *testdata = isDecoderStalled_testdata_current;

  micros_safe_injection = testdata->micros;
  toothLastToothTime = testdata->toothLastToothTime;

  if (testdata->expected) {
    TEST_ASSERT_TRUE(isDecoderStalled());
  }
  else {
    TEST_ASSERT_FALSE(isDecoderStalled());
  }
}

void testIsDecoderStalled() {
  MAX_STALL_TIME = 50000;

  const byte testNameLength = 200;
  char testName[testNameLength];

  const isDecoderStalled_testdata isDecoderStalled_testdatas[] = {

    // Should stall. Tooth was recorded longer ago than MAX_STALL_TIME.
    { .micros = 0UL,       .toothLastToothTime = 0UL - MAX_STALL_TIME - 1,       .expected = true },
    { .micros = 20000UL,   .toothLastToothTime = 20000UL - MAX_STALL_TIME - 1,   .expected = true },
    { .micros = 1000000UL, .toothLastToothTime = 1000000UL - MAX_STALL_TIME - 1, .expected = true },
    { .micros = ULONG_MAX, .toothLastToothTime = ULONG_MAX - MAX_STALL_TIME - 1, .expected = true },

    // Should not stall. Tooth was recorded just before.
    { .micros = 0UL,       .toothLastToothTime = 0UL - 1,       .expected = false },
    { .micros = 20000UL,   .toothLastToothTime = 20000UL - 1,   .expected = false },
    { .micros = 1000000UL, .toothLastToothTime = 1000000UL - 1, .expected = false },
    { .micros = ULONG_MAX, .toothLastToothTime = ULONG_MAX - 1, .expected = false },

    // Should not stall. Tooth was recorded at the same time (but just before).
    { .micros = 0UL,       .toothLastToothTime = 0UL,       .expected = false },
    { .micros = 20000UL,   .toothLastToothTime = 20000UL,   .expected = false },
    { .micros = 1000000UL, .toothLastToothTime = 1000000UL, .expected = false },
    { .micros = ULONG_MAX, .toothLastToothTime = ULONG_MAX, .expected = false },

    // Should stall. Tooth was recorded after function call. Is not possible as isDecoderStalled must have interrupts disabled before being called.
    { .micros = 0UL,       .toothLastToothTime = 0UL + 1,       .expected = true },
    { .micros = 20000UL,   .toothLastToothTime = 20000UL + 1,   .expected = true },
    { .micros = 1000000UL, .toothLastToothTime = 1000000UL + 1, .expected = true },
    { .micros = ULONG_MAX, .toothLastToothTime = ULONG_MAX + 1, .expected = true },

    // Should stall. Tooth was recorded after function call. Is not possible as isDecoderStalled must have interrupts disabled before being called.
    { .micros = 0UL,       .toothLastToothTime = 0UL + MAX_STALL_TIME + 1,       .expected = true },
    { .micros = 20000UL,   .toothLastToothTime = 20000UL + MAX_STALL_TIME + 1,   .expected = true },
    { .micros = 1000000UL, .toothLastToothTime = 1000000UL + MAX_STALL_TIME + 1, .expected = true },
    { .micros = ULONG_MAX, .toothLastToothTime = ULONG_MAX + MAX_STALL_TIME + 1, .expected = true },

  };

  for (auto testdata : isDecoderStalled_testdatas) {
    isDecoderStalled_testdata_current = &testdata;
    snprintf(testName, testNameLength, "isDecoderStalled / micros %lu / lastTooth %lu ", testdata.micros, testdata.toothLastToothTime);
    UnityDefaultTestRun(testIsDecoderStalled_runTest, testName, __LINE__);
  }

}