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
 * @tparam from The lower bound
 * @tparam to The upper bound (exclusive)
 */
template <uint8_t from, uint8_t to> 
struct static_for {

    /**
     * @brief Unroll, calling a free function
     * 
     * @attention The free function must have a uint8_t first parameter. 
     * This is the "loop" index, which will be injected automatically
     * 
     * @tparam funcType Function signature (should be automatically deduced)
     * @tparam Args Function argument pack (should be automatically deduced)
     * @param f The function to call
     * @param args Function arguments in addition to the "loop" index
     */
    template <typename funcType, typename...Args>
    static inline __attribute__((always_inline)) void repeat_n (funcType f, Args...args){
        f(from, args...); // Call the supplied free function
        //cppcheck-suppress misra-c2012-17.7
        static_for<from + 1U, to>::repeat_n(f, args...); // Call next iteration
    }
};

/// @cond 
// The loop termination specialization for static_for<uint8_t, uint8_t> (you can ignore this)
template <uint8_t to>
struct static_for<to, to> {

    template <typename funcType, typename...Args>
    //cppcheck-suppress misra-c2012-17.7
    //cppcheck-suppress misra-c2012-8.2
    static inline void repeat_n(funcType, Args...){
    }    
};
/// @endcond 