#pragma once

#include <inttypes.h>

#if defined(__AVR__)
#include <Arduino.h>
#else
#include <sys/time.h>
#endif

#if !defined(MILLIS_PER_SEC)
#define MILLIS_PER_SEC 1000ULL
#endif
#if !defined(MICROS_PER_SEC)
#define MICROS_PER_SEC (MILLIS_PER_SEC*1000)
#endif
#if !defined(NANOS_PER_SEC)
#define NANOS_PER_SEC (MICROS_PER_SEC*1000)
#endif

class timer {
private:
#if defined(__AVR__)
    uint32_t start_time;
    uint32_t end_time;
#else
    struct timeval start_time;
    struct timeval end_time;
#endif

public:

    timer() {
    }

    void start() {
#if defined(__AVR__)
        start_time = micros();
#else 
        gettimeofday(&start_time, NULL);
#endif
    }

    void stop() {
#if defined(__AVR__)
        end_time = micros();
#else 
        gettimeofday(&end_time, NULL);
#endif        
    }

    uint32_t duration_micros() {
#if defined(__AVR__)
        return end_time-start_time;
#else 
        return (uint32_t)(((end_time.tv_sec - start_time.tv_sec) * MICROS_PER_SEC) + (end_time.tv_usec - start_time.tv_usec));
#endif 
    }
};

template <typename TLoop, typename TParam>
void measure_executiontime(uint16_t iterations, TLoop from, TLoop to, TLoop step, timer &measure, TParam param, void (*pTestFun)(TLoop, TParam)) {
    measure.start();
    for (uint16_t loop=0; loop<iterations; ++loop)
    {
      for (TLoop a = from; a < to; a+=step)
      {
        pTestFun(a, param);
      }
    }
    measure.stop();
}

template <typename TParam>
struct execution_time {
    TParam result;
    uint32_t durationMicros;
};
template <typename TParam>
struct comparative_execution_times {
    execution_time<TParam> timeA;
    execution_time<TParam> timeB;
};
template <typename TLoop, typename TParam>
comparative_execution_times<TParam> compare_executiontime(uint16_t iterations, TLoop from, TLoop to, TLoop step, void (*pTestFunA)(TLoop, TParam&), void (*pTestFunB)(TLoop, TParam&)) {

    timer timerA;
    TParam paramA = 0;
    measure_executiontime<TLoop, TParam&>(iterations, from, to, step, timerA, paramA, pTestFunA);

    timer timerB;
    TParam paramB = 0;
    measure_executiontime<TLoop, TParam&>(iterations, from, to, step, timerB, paramB, pTestFunB);

    char buffer[128];
    sprintf(buffer, "Timing: %" PRIu32 ", %" PRIu32, timerA.duration_micros(), timerB.duration_micros());
    TEST_MESSAGE(buffer);

    return comparative_execution_times<TParam> {
        .timeA = execution_time<TParam> { .result = paramA, .durationMicros = timerA.duration_micros()},
        .timeB = execution_time<TParam> { .result = paramB, .durationMicros = timerB.duration_micros()}
    };
}