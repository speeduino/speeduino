#if defined(NATIVE_BOARD)
#include <thread>
#include <chrono>
#include <functional>
#include <Arduino.h>
#include "SoftwareTimer.h"

// This could definitely be made more efficient, but for testing purposes this will do

software_timer_t::software_timer_t()
{
    timerThread = std::move(std::thread([this]() { backgroundTimerTask(); }));
}
software_timer_t::~software_timer_t()
{
    halt = true;
    timerThread.join();
}

void software_timer_t::setCallback(const callback_t &cb)
{
    callback = cb;
}

void software_timer_t::backgroundTimerTask(void)
{
    while (!halt) {
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
        counter = static_cast<counter_t>(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count()); 
        if (enabled.load() && (callback != nullptr) && (compare!=0U) && (counter>=compare)) {
            callback();
        }
    }
}

#endif