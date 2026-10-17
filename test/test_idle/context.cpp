#include "context.h"
#include "../test_utils.h"
#include "units.h"

extern table2D_u8_u8_4 iacCrankDutyTable;
extern table2D_u8_u8_10 iacPWMTable;
extern table2D_u8_u8_4 iacCrankStepsTable;
extern table2D_u8_u8_10 iacStepTable;

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
    pins.pinIdle1 = TEST_IDLE1_PIN;
    pins.pinIdle2 = TEST_IDLE2_PIN;
    pins.pinIdleUp = TEST_IDLEUP_INPUT_PIN;
    pins.pinIdleUpOutput = TEST_IDLEUP_OUTPUT_PIN;
    page6.iacAlgorithm = algorithm;
    page6.iacChannels = 0U;
    page6.iacPWMdir = 0U;
    page6.iacPWMrun = 0U;
    page6.iacFastTemp = 0U;            // Skip ON branch in ON_OFF unless we set coolant low
    page6.idleKP = 100U;
    page6.idleKI = 50U;
    page6.idleKD = 0U;
    page6.iacStepTime = 1U;
    page9.iacCoolTime = 1U;
    page2.iacCLminValue = 0U;
    page2.iacCLmaxValue = 100U;
    page2.idleUpAdder = 0U;
    current.coolant = 80;
    current.idleUpActive = false;
    current.CLIdleTarget = 80U;

    prepare_pwm_crank_table();
    prepare_pwm_running_table();
    prepare_stepper_crank_table();
    prepare_stepper_run_table();
}
