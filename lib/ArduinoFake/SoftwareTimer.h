#pragma once

#if defined(NATIVE_BOARD)

#include <cstdint>
#include <atomic>
#include <functional>
#include <limits>

class software_timer_t {
public:

    using counter_t = uint32_t;
    using callback_t = std::function<void()>;

    std::atomic<counter_t> counter = {0U};
    std::atomic<counter_t> compare = {std::numeric_limits<counter_t>::max()};
    
    software_timer_t();
    ~software_timer_t();

    void setCallback(const callback_t &callback);

    void enableTimer(void);
    void disableTimer(void);

    static counter_t microsToTicks(unsigned long micros);

private:
    callback_t callback = {nullptr};
    uint16_t tickCallbackId;
    std::atomic<bool> enabled = {false};

    void onNextTick(counter_t nextTick);
};

#endif