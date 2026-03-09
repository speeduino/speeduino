#if defined(NATIVE_BOARD)
#include <thread>
#include <chrono>
#include <functional>
#include <map>
#include "SoftwareTimer.h"

namespace
{
static inline std::chrono::microseconds getCurMicros(void)
{
    // static std::atomic<std::chrono::steady_clock::time_point> startTime(std::chrono::steady_clock::now());
    // auto nowTime = std::chrono::steady_clock::now();
    // // Handle timer overflow by reseting start time
    // if (nowTime<startTime.load())
    // {
    //     startTime = std::chrono::steady_clock::time_point();
    // }
    // // Microseconds since program startup
    // return std::chrono::duration_cast<std::chrono::microseconds>(nowTime - startTime.load());
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch());
}

static std::chrono::microseconds ticksToMicros(software_timer_t::counter_t ticks)
{
    return std::chrono::microseconds(ticks * software_timer_t::TIMER_RESOLUTION);
}

}

/** @brief A monotonic ticker event source. */
class TickEventSource
{
public:
    /** @brief Callback type */
    using callback_t = std::function<void(software_timer_t::counter_t)>;

    TickEventSource() : tickCounter(1U)
    {
        start();
    }
    ~TickEventSource()
    {
        stop();
    }

    /** @brief External access to the current tick */
    software_timer_t::counter_t currentTick(void) const {
        return tickCounter;
    }

    /** @brief Register a tick callback
     * 
     * @param cb The callback, called once per tick
     * @return uint16_t Callback id that can be passed to unregisterCallback
     */
    uint16_t registerCallback(callback_t cb) {
        uint16_t id = nextId_++;
        callbacks_[id] = std::move(cb);
        return id;
    }

    /** @brief Remove a previously registered callback */
    void unregisterCallback(uint16_t id) {
        callbacks_.erase(id);
    }    

    /** @brief Begin event notification */
    void start(void)
    {
        halt = false;
        tickThread = std::move(std::thread([this]() { tickTimerTask(); }));
    }

    /** @brief Halt event notification */
    void stop(void)
    {
        halt = true;
        if (tickThread.joinable())
        {
            tickThread.join();
        }
    }

private:

    void notifyNextTickEvent(void) {
        for (auto& cb : callbacks_) {
            if (cb.second) {
                cb.second(currentTick());
            }
        }
    }

    // This could definitely be made more efficient, but for testing purposes this will do
    void waitForNextTick(void) {
        auto nextTime = getCurMicros()+ticksToMicros(1);
        
        while (getCurMicros()<nextTime && !halt) {
            // Busy-wait loop
        }
    }

    void tickTimerTask(void) {
        while (!halt) {
            waitForNextTick();
            // Tell callbacks about next tick
            if (!halt)
            {
                ++tickCounter;
                notifyNextTickEvent();
            }
        }
    }

    std::map<uint16_t, callback_t> callbacks_;
    uint16_t nextId_ = 0;
    std::thread tickThread;
    software_timer_t::counter_t tickCounter;
    std::atomic<bool> halt = {false};
};

/// @brief  Our global tick event source
static TickEventSource& getTicker(void) {
    static TickEventSource theTicker;
    return theTicker;
}

software_timer_t::software_timer_t()
: counter(getTicker().currentTick())
{
    TickEventSource::callback_t thisCallback(std::bind(&software_timer_t::onNextTick, this, std::placeholders::_1));
    tickCallbackId = getTicker().registerCallback(thisCallback);
}
software_timer_t::~software_timer_t()
{
    getTicker().unregisterCallback(tickCallbackId);
}

void software_timer_t::setCallback(const callback_t &cb)
{
    callback = cb;
}

void software_timer_t::enableTimer(void) 
{
    enabled = true;
}
void software_timer_t::disableTimer(void) 
{
    enabled = false;
}

void software_timer_t::onNextTick(counter_t nextTick)
{
    counter = nextTick; 

    if (enabled.load() && (callback != nullptr) && (counter.load()>=compare.load())) {
        callback();
    }
}

TickEventGuard::TickEventGuard(void)
{
    getTicker().stop();
}
TickEventGuard::~TickEventGuard()
{
    getTicker().start();
}

#endif