// Standalone native regression: no firmware or Arduino mocks are needed.
#include "SoftwareTimer.h"
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

int main()
{
    std::promise<void> entered;
    std::promise<void> release;
    auto releaseSignal = release.get_future();
    auto enteredSignal = entered.get_future();
    auto *timer = new software_timer_t();
    timer->setCallback([&]() {
        timer->disableTimer();
        entered.set_value();
        releaseSignal.wait();
    });
    timer->compare = 0;
    timer->enableTimer();
    if (enteredSignal.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        // Release a late callback before waiting for destruction.
        release.set_value();
        delete timer;
        std::cerr << "Timer did not deliver a callback\n";
        return 1;
    }

    std::promise<void> deleting;
    auto deletingSignal = deleting.get_future();
    auto destroyed = std::async(std::launch::async, [&]() {
        deleting.set_value();
        delete timer;
    });
    deletingSignal.wait();
    const bool waited = destroyed.wait_for(std::chrono::milliseconds(50)) == std::future_status::timeout;
    release.set_value();
    destroyed.get();
    if (!waited)
    {
        std::cerr << "Timer destruction did not wait for its in-flight callback\n";
        return 1;
    }

    // Exercise registration and removal while the ticker iterates its callbacks.
    for (unsigned i = 0; i < 20000; ++i)
    {
        software_timer_t temporary;
        if (i % 10 == 0)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
    std::cout << "Timer lifetime and registration stress tests passed\n";
}
