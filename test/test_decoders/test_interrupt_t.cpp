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

static void test_isValid(void)
{
  interrupt_t subject;

  TEST_ASSERT_FALSE(subject.isValid());
  subject.edge = CHANGE;
  TEST_ASSERT_FALSE(subject.isValid());
  subject.callback = test_isValid;
  TEST_ASSERT_FALSE(subject.isValid());
  subject.attach(19);
  TEST_ASSERT_TRUE(subject.isValid());
  subject.edge = RISING;
  TEST_ASSERT_TRUE(subject.isValid());
  subject.edge = FALLING;
  TEST_ASSERT_TRUE(subject.isValid());
  subject.edge = TRIGGER_EDGE_NONE;
  TEST_ASSERT_FALSE(subject.isValid());
}

static void test_isTriggered(void)
{
  interrupt_t subject;

  TEST_ASSERT_FALSE(subject.isTriggered());

  subject.edge = CHANGE;
  TEST_ASSERT_FALSE(subject.isTriggered());

  subject.callback = test_isValid;
  TEST_ASSERT_FALSE(subject.isTriggered());

  subject.attach(19);
  TEST_ASSERT_TRUE(subject.isTriggered());

  subject.edge = RISING;
  subject._pin._pin.setPinLow();
  TEST_ASSERT_FALSE(subject.isTriggered());

  subject._pin._pin.setPinHigh();
  TEST_ASSERT_TRUE(subject.isTriggered());

  subject.edge = FALLING;
  TEST_ASSERT_FALSE(subject.isTriggered());

  subject._pin._pin.setPinLow();
  TEST_ASSERT_TRUE(subject.isTriggered());
}


void testinterrupt_t()
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_attach_return);
    RUN_TEST_P(test_isValid);
    RUN_TEST_P(test_isTriggered);
  }
}