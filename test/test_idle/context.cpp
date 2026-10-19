#include "context.h"
#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "units.h"

extern table2D_u8_u8_4 iacCrankDutyTable;
extern table2D_u8_u8_10 iacPWMTable;
extern table2D_u8_u8_4 iacCrankStepsTable;
extern table2D_u8_u8_10 iacStepTable;
extern idleController::detail::state_t _idleState;
extern idleController::detail::fnCurMicros_t _idleCurMicros;

static void prepare_pwm_crank_table(void)
{
  TEST_DATA_P uint8_t bins[] = {
    temperatureAddOffset(-20),
    temperatureAddOffset(0),
    temperatureAddOffset(75),
    temperatureAddOffset(110)
  };
  TEST_DATA_P uint8_t values[] = { 0, 30, 70, 100 };
  populate_2dtable_P(&iacCrankDutyTable, values, bins);
}

static void prepare_pwm_running_table(void)
{
  TEST_DATA_P uint8_t runBins[] = { 0, 40, 60, 80, 90, 100, 120, 160, 200, 255 };
  TEST_DATA_P uint8_t runValues[] = { 1, 5, 10, 14, 19, 23, 28, 32, 37, 41 };
  populate_2dtable_P(&iacPWMTable, runValues, runBins);
}

static void prepare_stepper_crank_table(void)
{
  TEST_DATA_P uint8_t bins[] = {
    temperatureAddOffset(-20),
    temperatureAddOffset(0),
    temperatureAddOffset(75),
    temperatureAddOffset(110)
  };
  TEST_DATA_P uint8_t values[] = { 0, 30, 70, 100 };
  populate_2dtable_P(&iacCrankStepsTable, values, bins);
}

static void prepare_stepper_run_table(void)
{
  TEST_DATA_P uint8_t runBins[] = { 0, 40, 60, 80, 90, 100, 120, 160, 200, 255 };
  TEST_DATA_P uint8_t runValues[] = { 0, 10, 20, 30, 60, 70, 80, 100, 120, 140 };
  populate_2dtable_P(&iacStepTable, runValues, runBins);
}

void context_t::prepare_idle(uint8_t algorithm)
{
    // Idle test pins; 
    constexpr uint8_t TEST_IDLE1_PIN = 18U;
    constexpr uint8_t TEST_IDLE2_PIN = 19U;
    constexpr uint8_t TEST_IDLEUP_INPUT_PIN = 20U;
    constexpr uint8_t TEST_IDLEUP_OUTPUT_PIN = 21U;

    pins.pinIdle1 = TEST_IDLE1_PIN;
    pins.pinIdle2 = TEST_IDLE2_PIN;
    pins.pinIdleUp = TEST_IDLEUP_INPUT_PIN;
    pins.pinIdleUpOutput = TEST_IDLEUP_OUTPUT_PIN;
    page6.iacAlgorithm = algorithm;
    page6.idleKP = 100U;
    page6.idleKI = 50U;
    page6.idleKD = 0U;
    page6.iacStepTime = 1U;
    page9.iacCoolTime = 1U;
    page2.iacCLmaxValue = 100U;
    current.CLIdleTarget = 80U; // Set outside of idleControl() so set a default here
    BIT_SET(current.LOOP_TIMER, BIT_TIMER_1HZ);
    BIT_SET(current.LOOP_TIMER, BIT_TIMER_10HZ);

    prepare_pwm_crank_table();
    prepare_pwm_running_table();
    prepare_stepper_crank_table();
    prepare_stepper_run_table();
}

void context_t::init(void)
{
    _idleState = idleController::detail::state_t();
    initialiseIdle(false);
}

uint32_t context_t::idleControl(uint32_t startTime, uint32_t increment, uint8_t stepCount)
{
  idle_micros_override_t microsOverride;
  for (uint8_t i=0; i<stepCount; ++i)
  {
    idle_micros_override_t::mockIdleMicrosValue = startTime; ::idleControl(); startTime += increment;
  }
  return startTime;
}

idle_micros_override_t::idle_micros_override_t(void) : original(_idleCurMicros)
{
    _idleCurMicros = &mockIdleMicros;
}

idle_micros_override_t::~idle_micros_override_t(void)
{
    _idleCurMicros = original;
}

// Definition and initialization outside the class
unsigned long idle_micros_override_t::mockIdleMicrosValue = 0; 
