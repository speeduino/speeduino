#pragma once

#include <unity.h>
#include "src/controllers/launch/launchController.h"
#include "src/pins/inputPin.h"

extern inputPin_t launchPin;

struct launch_fixture
{
    statuses current = {};
    config2 page2 = {};
    config6 page6 = {};
    config10 page10 = {};
    config15 page15 = {};
    pinNumbers_t pins = {};

    launch_fixture()
    {
        current.clutchTrigger = true;
        current.clutchEngagedRPM = 3000;
        current.setRpm(5000);
        current.TPS = 50;
        page6.launchEnabled = true;
        page6.flatSEnable = true;
        page6.launchHiLo = true;
        page6.flatSArm = 40;
        page6.lnchHardLim = 45;
        page10.lnchCtrlTPS = 50;
        page10.lnchCtrlVss = 50;
        page15.rollingProtRPMDelta[0] = -5;
        pins.pinLaunch = 13;
    }

    void setClutch(bool engaged)
    {
        if (engaged) {
            launchPin._pin.setPinHigh();
        } else {
            launchPin._pin.setPinLow();
        }
    }

    void init(void)
    {
        initialiseLaunchControl(page6, pins);        
    }

    void update()
    {
        updateLaunchAndFlatShift(current, page2, page6, page10, page15);
    }

    void assertState(bool launch, bool flatShift)
    {
        TEST_ASSERT_EQUAL(launch, current.launchingHard);
        TEST_ASSERT_EQUAL(launch, current.hardLaunchActive);
        TEST_ASSERT_EQUAL(flatShift, current.flatShiftingHard);
    }
};
