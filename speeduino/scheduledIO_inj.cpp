#include "scheduledIO_inj.h"
#include "acc_mc33810.h"
#include "scheduledIO_direct_inj.h"

/** @file
 * Injector and Coil (toggle/open/close) control (under various situations, eg with particular cylinder count, rotary engine type or wasted spark ign, etc.).
 * Also accounts for presence of MC33810 injector/ignition (dwell, etc.) control circuit.
 * Functions here are typically assigned (at initialisation) to callback function variables (e.g. inj1StartFunction or inj1EndFunction) 
 * form where they are called (by scheduler.ino).
 */

static volatile byte injStatusMask = 0;
static InjIoControlMode _controlMode = InjIoControlMode::Direct;

void initInjIoControl(InjIoControlMode controlMode)
{
    _controlMode = controlMode;
}

/** @brief Injector open/close status bits */
char getInjectorStatus(void)
{
    return injStatusMask;
}

// LCOV_EXCL_START
// Exclude from code coverage, since this is all board output control

void openInjector(uint8_t channel)
{
#if defined(MC33810_SUPPORT)
    if(_controlMode==InjIoControlMode::Direct) {
        openInjector_DIRECT(channel);
    } else {
        openInjector_MC33810(channel);
    };
#else
    openInjector_DIRECT(channel);
#endif
    BIT_SET(injStatusMask, (channel)-1U);
}

void closeInjector(uint8_t channel)
{
#if defined(MC33810_SUPPORT)
    if(_controlMode==InjIoControlMode::Direct) {
        closeInjector_DIRECT(channel);
    } else {
        closeInjector_MC33810(channel);
    };
#else
    closeInjector_DIRECT(channel);
#endif
    BIT_CLEAR(injStatusMask, (channel)-1U); 
}

void openInjector1(void)   { openInjector(1); }
void closeInjector1(void)  { closeInjector(1); }
void openInjector2(void)   { openInjector(2); }
void closeInjector2(void)  { closeInjector(2); }
void openInjector3(void)   { openInjector(3); }
void closeInjector3(void)  { closeInjector(3); }
void openInjector4(void)   { openInjector(4); }
void closeInjector4(void)  { closeInjector(4); }
void openInjector5(void)   { openInjector(5); }
void closeInjector5(void)  { closeInjector(5); }
void openInjector6(void)   { openInjector(6); }
void closeInjector6(void)  { closeInjector(6); }
void openInjector7(void)   { openInjector(7); }
void closeInjector7(void)  { closeInjector(7); }
void openInjector8(void)   { openInjector(8); }
void closeInjector8(void)  { closeInjector(8); }

// These are for Semi-Sequential and 5 Cylinder injection
//Standard 4 cylinder pairings
void openInjector1and3(void) { openInjector(1U); openInjector(3U); }
void closeInjector1and3(void) { closeInjector(1U); closeInjector(3U); }
void openInjector2and4(void) { openInjector(2U); openInjector(4U); }
void closeInjector2and4(void) { closeInjector(2U); closeInjector(4U); }
//Alternative output pairings
void openInjector1and4(void) { openInjector(1U); openInjector(4U); }
void closeInjector1and4(void) { closeInjector(1U); closeInjector(4U); }
void openInjector2and3(void) { openInjector(2U); openInjector(3U); }
void closeInjector2and3(void) { closeInjector(2U); closeInjector(3U); }

void openInjector3and5(void) { openInjector(3U); openInjector(5U); }
void closeInjector3and5(void) { closeInjector(3U); closeInjector(5U); }

void openInjector2and5(void) { openInjector(2U); openInjector(5U); }
void closeInjector2and5(void) { closeInjector(2U); closeInjector(5U); }
void openInjector3and6(void) { openInjector(3U); openInjector(6U); }
void closeInjector3and6(void) { closeInjector(3U); closeInjector(6U); }

void openInjector1and5(void) { openInjector(1U); openInjector(5U); }
void closeInjector1and5(void) { closeInjector(1U); closeInjector(5U); }
void openInjector2and6(void) { openInjector(2U); openInjector(6U); }
void closeInjector2and6(void) { closeInjector(2U); closeInjector(6U); }
void openInjector3and7(void) { openInjector(3U); openInjector(7U); }
void closeInjector3and7(void) { closeInjector(3U); closeInjector(7U); }
void openInjector4and8(void) { openInjector(4U); openInjector(8U); }
void closeInjector4and8(void) { closeInjector(4U); closeInjector(8U); }

// LCOV_EXCL_STOP