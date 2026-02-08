#pragma once

#if defined(NATIVE_BOARD)

#include <cstdint>
#include <atomic>
#include <thread>
#include <functional>

class software_timer_t {
public:

    using counter_t = uint32_t;
    using callback_t = std::function<void()>;

    std::atomic<counter_t> counter = {0U};
    std::atomic<counter_t> compare = {0U};
    
    software_timer_t();
    ~software_timer_t();

    void setCallback(const callback_t &callback);

    void enableTimer(void) { enabled = true; }
    void disableTimer(void) { enabled = false; }

private:
    // Atomic flag to signal the thread to stop
    std::atomic<bool> enabled = {false}; 
    std::atomic<bool> halt = {false};
    callback_t callback = {nullptr};
    std::thread timerThread;

    void backgroundTimerTask(void);
};

#endif