#pragma once

/**
 * @file static_for.hpp
 * @brief A compile time *unrolled* loop
 * 
 * GCC is surprisingly conservative at unrolling loops with -funroll-loops (and even -funroll-all-loops): 
 *  - no nested loops (for, while), which works against inlining.
 *  - if it will increase code size by small margins
 *  - other factors I don't quite understand yet
 * 
 * In summary, it's designed for very simple cases where loop overhead is obviously high
 * @see https://godbolt.org/z/6KGWajPaY
 */

#include <stdint.h>
#include <utility>
#include <initializer_list>

///@cond 
namespace detail {

template <typename F, uint8_t... Is>
static inline __attribute__((always_inline)) void unroll_impl(F&& f, std::integer_sequence<uint8_t, Is...>) {
    // C++14 expansion using an initializer list to evaluate f(Is) in order
    (void)std::initializer_list<uint8_t>{ (f(std::integral_constant<uint8_t, Is>{}), (uint8_t)0)... };
}

} // detail
///@endcond 

/**
 * @brief Generate a fixed number of calls to a given free function *at compile time*.
 * 
 *  Usage:
 *  @code
 *    static inline void foo(uint8_t index, time_t *currTime, const char *message) {
 *      // Do something
 *    }
 * 
 *    time_t currTime = time(NULL);
 *    char message[] = "Unrolled!";
 *    // Equivalent of
 *    // foo(3, &currTime, message);
 *    // foo(4, &currTime, message);
 *    // foo(5, &currTime, message);
 *    // foo(6, &currTime, message);
 *    static_for<3, 7>::repeat_n(foo, &currTime, message);
 *  @endcode
 * 
 * @tparam N The upper bound (exclusive)
 * @tparam F Callable type 
 * @param f Callable instance
 */
template <uint8_t N, typename F>
static inline __attribute__((always_inline)) void static_for(F&& f) {
    detail::unroll_impl(std::forward<F>(f), std::make_integer_sequence<uint8_t, N>{});
}
