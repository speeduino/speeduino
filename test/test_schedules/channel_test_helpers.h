#pragma once

// Capture boilerplate code into these macros
// Avoid repeated code in tests

#if INJ_CHANNELS >= 1
#define RUNIF_INJCHANNEL1(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_INJCHANNEL1(ignored, codeBlock) (codeBlock);
#endif
#if INJ_CHANNELS >= 2
#define RUNIF_INJCHANNEL2(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_INJCHANNEL2(ignored, codeBlock) (codeBlock);
#endif
#if INJ_CHANNELS >= 3
#define RUNIF_INJCHANNEL3(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_INJCHANNEL3(ignored, codeBlock) (codeBlock);
#endif
#if INJ_CHANNELS >= 4
#define RUNIF_INJCHANNEL4(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_INJCHANNEL4(ignored, codeBlock) (codeBlock);
#endif
#if INJ_CHANNELS >= 5
#define RUNIF_INJCHANNEL5(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_INJCHANNEL5(ignored, codeBlock) (codeBlock);
#endif
#if INJ_CHANNELS >= 6
#define RUNIF_INJCHANNEL6(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_INJCHANNEL6(ignored, codeBlock) (codeBlock);
#endif
#if INJ_CHANNELS >= 7
#define RUNIF_INJCHANNEL7(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_INJCHANNEL7(ignored, codeBlock) (codeBlock);
#endif
#if INJ_CHANNELS >= 8
#define RUNIF_INJCHANNEL8(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_INJCHANNEL8(ignored, codeBlock) (codeBlock);
#endif

#define INJCHANNEL_TEST_HELPER1(codeBlock) RUNIF_INJCHANNEL1(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define INJCHANNEL_TEST_HELPER2(codeBlock) RUNIF_INJCHANNEL2(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define INJCHANNEL_TEST_HELPER3(codeBlock) RUNIF_INJCHANNEL3(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define INJCHANNEL_TEST_HELPER4(codeBlock) RUNIF_INJCHANNEL4(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define INJCHANNEL_TEST_HELPER5(codeBlock) RUNIF_INJCHANNEL5(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define INJCHANNEL_TEST_HELPER6(codeBlock) RUNIF_INJCHANNEL6(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define INJCHANNEL_TEST_HELPER7(codeBlock) RUNIF_INJCHANNEL7(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define INJCHANNEL_TEST_HELPER8(codeBlock) RUNIF_INJCHANNEL8(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))

#if IGN_CHANNELS >= 1
#define RUNIF_IGNCHANNEL1(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_IGNCHANNEL1(ignored, codeBlock) (codeBlock);
#endif
#if IGN_CHANNELS >= 2
#define RUNIF_IGNCHANNEL2(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_IGNCHANNEL2(ignored, codeBlock) (codeBlock);
#endif
#if IGN_CHANNELS >= 3
#define RUNIF_IGNCHANNEL3(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_IGNCHANNEL3(ignored, codeBlock) (codeBlock);
#endif
#if IGN_CHANNELS >= 4
#define RUNIF_IGNCHANNEL4(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_IGNCHANNEL4(ignored, codeBlock) (codeBlock);
#endif
#if IGN_CHANNELS >= 5
#define RUNIF_IGNCHANNEL5(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_IGNCHANNEL5(ignored, codeBlock) (codeBlock);
#endif
#if IGN_CHANNELS >= 6
#define RUNIF_IGNCHANNEL6(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_IGNCHANNEL6(ignored, codeBlock) (codeBlock);
#endif
#if IGN_CHANNELS >= 7
#define RUNIF_IGNCHANNEL7(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_IGNCHANNEL7(ignored, codeBlock) (codeBlock);
#endif
#if IGN_CHANNELS >= 8
#define RUNIF_IGNCHANNEL8(codeBlock, ignored) (codeBlock);
#else
#define RUNIF_IGNCHANNEL8(ignored, codeBlock) (codeBlock);
#endif

#define IGNCHANNEL_TEST_HELPER1(codeBlock) RUNIF_IGNCHANNEL1(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define IGNCHANNEL_TEST_HELPER2(codeBlock) RUNIF_IGNCHANNEL2(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define IGNCHANNEL_TEST_HELPER3(codeBlock) RUNIF_IGNCHANNEL3(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define IGNCHANNEL_TEST_HELPER4(codeBlock) RUNIF_IGNCHANNEL4(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define IGNCHANNEL_TEST_HELPER5(codeBlock) RUNIF_IGNCHANNEL5(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define IGNCHANNEL_TEST_HELPER6(codeBlock) RUNIF_IGNCHANNEL6(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define IGNCHANNEL_TEST_HELPER7(codeBlock) RUNIF_IGNCHANNEL7(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
#define IGNCHANNEL_TEST_HELPER8(codeBlock) RUNIF_IGNCHANNEL8(codeBlock, TEST_IGNORE_MESSAGE("Not enough channels"))
