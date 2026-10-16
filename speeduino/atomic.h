#pragma once

#include "src/utils/nominmax.h"
#include <tuple>
#include "board_definition.h"

#if !defined(ATOMIC)

#include <SimplyAtomic.h>

#endif

/**
 * @brief Atomically copy the **forwarding reference** arguments into a std::tuple<>
 * 
 * Using a forwarding reference (Args&&...) preserves the arguments value catgeory (lvalue, rvalue)
 * and cv-qualifier. E.g.
 *  volatile uint32_t foo;
 *  volatile uint8_t bar;
 *  // This calls atomic_copy(volatile uint32_t &, volatile uint8_t &)
 *  auto copy = atomic_copy(foo, bar); 
 */
template <typename... Args>
static inline auto atomic_copy(Args&&... args)
{
  ATOMIC()
  {
    // At this point we make copies of all the arguments.
    // Since the arguments are references, we are not making copies of copies
    return std::make_tuple(std::forward<Args>(args)...);
  }
  // LCOV_EXCL_START
  __builtin_unreachable(); 
  // LCOV_EXCL_STOP
}

