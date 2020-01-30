#if !defined(__TEST_SCHEDULE_H__)
#define __TEST_SCHEDULE_H__

typedef uint32_t micros_t;

void test_status_initial_off(void);
void test_status_off_to_pending(void);
void test_status_pending_to_running(void);
void test_status_running_to_off(void);
void test_status_running_to_pending(void);

void test_accuracy_timeout(void);
void test_accuracy_duration(void);

#endif // __TEST_SCHEDULE_H__