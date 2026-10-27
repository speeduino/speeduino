/**
 * @file 
 * @brief Functionality shared by the idle PWM control modes 
 */
#pragma once

#include "config_pages.h"
#include "statuses.h"

namespace idleController {

namespace pwm {

  uint8_t lookupCrankingLoad(const statuses &current);

  uint8_t lookupRunningLoad(const statuses &current);

}
}