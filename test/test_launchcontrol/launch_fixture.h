#pragma once

#include <unity.h>
#include "src/controllers/launch/launchController.h"
#include "src/pins/inputPin.h"
#include "units.h"

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
        page2.vssMode = VSS_MODE_OFF;
        page2.hardCutType == HARD_CUT_FULL;
        page6.launchEnabled = true;
        page6.flatSEnable = true;
        page6.launchHiLo = true;
        page6.lnchHardLim = 35;
        page6.lnchSoftLim = page6.lnchHardLim;
        page6.flatSArm = page6.lnchHardLim + 5;
        page6.flatSSoftWin = 5;
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
        extern void updateLaunchFlagsCore(statuses &current, const config2 &page2, const config6 &page6, const config10 &page10, const config15 &page15);
        updateLaunchFlagsCore(current, page2, page6, page10, page15);
    }

    void armLaunch(void)
    {
        setClutch(true);
        page6.launchEnabled = true;
        current.setRpm(RPM_COARSE.toUser(page6.flatSArm)-3);
        current.TPS = page10.lnchCtrlTPS+3;
        current.vss = page10.lnchCtrlVss/3;
    }

    void armSoftLaunch(void)
    {
        armLaunch();
        current.setRpm(RPM_COARSE.toUser(page6.lnchSoftLim)+1);
    }

    void armHardLaunch(void)
    {
        armLaunch();
        current.setRpm(RPM_COARSE.toUser(page6.lnchHardLim)+1);
    }

    void armFlatShift(void)
    {
        setClutch(true);
        page6.flatSEnable = true;
        current.setRpm(RPM_COARSE.toUser(page6.flatSArm)+3);
    }

    void armSoftFlatShift(void)
    {
        armFlatShift();
        current.launchStatus.clutchTrigger = true;
        current.launchStatus.clutchEngagedRPM = ((page6.flatSArm) * 100) + 500;
        current.setRpm(current.launchStatus.clutchEngagedRPM + 600);
    }
};
