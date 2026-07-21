#include <unity.h>
#include "../test_utils.h"
#include "decoder_t.h"

static void nullCallback(void) { }

static void test_attach_return(void)
{
    interrupt_t subject;
    TEST_ASSERT_FALSE(subject.isValid());
    TEST_ASSERT_EQUAL(NOT_A_PIN, subject.attach(33));

    subject.callback = nullCallback;
    TEST_ASSERT_FALSE(subject.isValid());
    TEST_ASSERT_EQUAL(NOT_A_PIN, subject.attach(33));

    subject.edge = CHANGE;
    TEST_ASSERT_TRUE(subject.isValid());
    TEST_ASSERT_EQUAL(33, subject.attach(33));
}

void testinterrupt_t()
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_attach_return);
  }
}