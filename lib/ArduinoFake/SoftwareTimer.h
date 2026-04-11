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

    /** @brief Microseconds per tick */
    static constexpr uint32_t TIMER_RESOLUTION = 100UL;

    static constexpr counter_t microsToTicks(unsigned long micros)
    {
        return (counter_t)(micros/TIMER_RESOLUTION);
    }

    static constexpr uint32_t ticksToMicros(counter_t ticks)
    {
    return ticks * TIMER_RESOLUTION;
    }

private:
    callback_t callback = {nullptr};
    uint16_t tickCallbackId;
    std::atomic<bool> enabled = {false};

    void onNextTick(counter_t nextTick);
};

/** @brief An RAII class to start/stop tick event generation */
class TickEventGuard
{
public:
    /** @brief Halt tick event generation */
    TickEventGuard(void);
    /** @brief Resume tick event generation */
    ~TickEventGuard();
private:
    bool _stopped = false;
};

#endif