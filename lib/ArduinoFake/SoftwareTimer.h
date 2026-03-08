#pragma once

#if defined(NATIVE_BOARD)

#include <cstdint>
#include <atomic>
#include <functional>

class software_timer_t {
public:

    using counter_t = uint32_t;
    using callback_t = std::function<void()>;

    std::atomic<counter_t> counter = {0U};
    std::atomic<counter_t> compare = {UINT32_MAX};
    
    software_timer_t();
    ~software_timer_t();

    void setCallback(const callback_t &callback);

    void enableTimer(void);
    void disableTimer(void);

    static counter_t microsToTicks(unsigned long micros);

private:
    callback_t callback = {nullptr};
    uint16_t tickCallbackId;

    void onNextTick(counter_t nextTick);
};

#endif