/**
 * @file 
 * @brief Functionality shared by the idle stepper control modes 
 */
#pragma once

#include "config_pages.h"
#include "statuses.h"

namespace idleController {

namespace stepper {

  uint16_t lookupCrankingSteps(const statuses &current);

  uint16_t lookupRunningSteps(const statuses &current);

}
}