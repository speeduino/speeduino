#ifndef MATH_H
#define MATH_H

#include <stdint.h>
#include "globals.h"
#include "bit_shifts.h"
#include "utilities.h"

#ifdef USE_LIBDIVIDE
// We use pre-computed constant parameters with libdivide where possible. 
// Using predefined constants saves flash and RAM (.bss) versus calling the 
// libdivide generator functions (E.g. libdivide_s32_gen)
// 32-bit constants generated here: https://godbolt.org/z/vP8Kfejo9
#include "src/libdivide/libdivide.h"
#include "src/libdivide/constant_fast_div.h"
#endif

uint8_t random1to100(void);

/**
 * @defgroup group-rounded-div Rounding integer division
 * 
 * @brief Integer division returns the quotient. I.e. rounds to zero. This
 * code will round the result to nearest integer. Rounding behavior is
 * controlled by #DIV_ROUND_BEHAVIOR
 * 
 * @{
 */

/**
 * @defgroup group-rounded-div-behavior Rounding behavior
 * @{
 */


/** @brief Rounding behavior: always round down */
#define DIV_ROUND_DOWN -1

/** @brief Rounding behavior: always round up */
#define DIV_ROUND_UP 1

/** @brief Rounding behavior: round to nearest 
 * 
 * This rounds 0.5 away from zero. This is the same behavior
 * as the standard library round() function.
 */
#define DIV_ROUND_NEAREST 0

/** @brief Integer division rounding behavior. */
#define DIV_ROUND_BEHAVIOR DIV_ROUND_NEAREST
// (Unit tests expect DIV_ROUND_NEAREST behavior)

/**
 * @brief Computes the denominator correction for rounding division
 * based on our rounding behavior.
 * 
 * @param d The divisor (an integer)
 * @param t The type of the result. E.g. uint16_t
 */
#define DIV_ROUND_CORRECT(d, t) ((t)(((d)>>1U)+(t)DIV_ROUND_BEHAVIOR))
///@}

/**
 * @brief Rounded integer division
 * 
 * Integer division returns the quotient. I.e. rounds to zero. This
 * macro will round the result to nearest integer. Rounding behavior
 * is controlled by #DIV_ROUND_BEHAVIOR
 * 
 * @warning For performance reasons, this macro does not promote integers.
 * So it will overflow if n>MAX(t)-(d/2).
 * 
 * @param n The numerator (dividee) (an integer)
 * @param d The denominator (divider) (an integer)
 * @param t The type of the result. E.g. uint16_t
 */
#define DIV_ROUND_CLOSEST(n, d, t) ( \
    (((n) < (t)(0)) ^ ((d) < (t)(0))) ? \
        ((t)((n) - DIV_ROUND_CORRECT(d, t))/(t)(d)) : \
        ((t)((n) + DIV_ROUND_CORRECT(d, t))/(t)(d)))

/**
 * @brief Rounded \em unsigned integer division
 * 
 * This is slightly faster than the signed version (DIV_ROUND_CLOSEST(n, d, t))
 * 
 * @warning For performance reasons, this macro does not promote integers.
 * So it will overflow if n>MAX(t)-(d/2).
 * 
 * @param n The numerator (dividee) (an \em unsigned integer)
 * @param d The denominator (divider) (an \em unsigned integer)
 * @param t The type of the result. E.g. uint16_t
 */
#define UDIV_ROUND_CLOSEST(n, d, t) ((t)((n) + DIV_ROUND_CORRECT(d, t))/(t)(d))

///@}

/** @brief Test whether the parameter is an integer or not. */
#define IS_INTEGER(d) ((d) == (int32_t)(d))

/** 
 * @{
 * @brief Performance optimised integer division by 100. I.e. same as n/100
 * 
 * Uses the rounding behaviour controlled by @ref DIV_ROUND_BEHAVIOR
 * 
 * @param n Dividend to divide by 100
 * @return n/100, with rounding behavior applied
 */
static inline uint16_t div100(uint16_t n) {
    // As of avr-gcc 5.4.0, the compiler will optimize this to a multiply/shift
    // (unlike the signed integer overload, where __divmodhi4 is still called
    // see https://godbolt.org/z/c5bs5noT1)
    return UDIV_ROUND_CLOSEST(n, UINT16_C(100), uint16_t);
}

static inline int16_t div100(int16_t n) {
#ifdef USE_LIBDIVIDE
    // Try faster unsigned path first
    if (n>0) {
        return div100((uint16_t)n);
    }
    // Negative values here, so adjust pre-division to get same
    // behavior as roundf(float)
    return libdivide::libdivide_s16_do_raw(n - DIV_ROUND_CORRECT(UINT16_C(100), uint16_t), S16_MAGIC(100), S16_MORE(100));
#else
    return DIV_ROUND_CLOSEST(n, UINT16_C(100), int16_t);
#endif
}

static inline uint32_t div100(uint32_t n) {
#ifdef USE_LIBDIVIDE
    if (n<=(uint32_t)UINT16_MAX) {
        return div100((uint16_t)n);
    }
    return libdivide::libdivide_u32_do_raw(n + DIV_ROUND_CORRECT(UINT32_C(100), uint32_t), 2748779070L, 6);
#else
    return UDIV_ROUND_CLOSEST(n, UINT32_C(100), uint32_t);
#endif
}

#if defined(__arm__)
static inline int div100(int n) {
    return DIV_ROUND_CLOSEST(n, 100U, int);
}
#else
static inline int32_t div100(int32_t n) {
#ifdef USE_LIBDIVIDE    
    if (n<=INT16_MAX && n>=INT16_MIN) {
        return div100((int16_t)n);            
    }
    return libdivide::libdivide_s32_do_raw(n + (DIV_ROUND_CORRECT(UINT16_C(100), uint32_t) * (n<0 ? -1 : 1)), 1374389535L, 5);
#else
    return DIV_ROUND_CLOSEST(n, INT32_C(100), int32_t);
#endif
}
#endif
///@}

/**
 * @brief Optimised integer division by 360
 * 
 * @param n The numerator (dividee) (an integer)
 * @return uint32_t 
 */
static inline uint32_t div360(uint32_t n) {
#ifdef USE_LIBDIVIDE
    return libdivide::libdivide_u32_do_raw(n + DIV_ROUND_CORRECT(UINT32_C(360), uint32_t), 1813430637L, 72);
#else
    return (uint32_t)UDIV_ROUND_CLOSEST(n, UINT32_C(360), uint32_t);
#endif
}


/// @cond
// Helper function, private to percentageApprox - do not use directly.
// Computes value * (percent/100) using bitsPrecision
template <uint8_t bitsPrecision>
static inline uint32_t _percentageApprox(uint16_t percent, uint32_t value) {
  uint16_t iPercent = div100((uint16_t)(percent << bitsPrecision));
  return rshift<bitsPrecision>(value * (uint32_t)iPercent);
}

struct PctApproxPrecisionThreshold {
    uint16_t percentThreshold;
    uint8_t bitsPrecision;
};

// To keep the percentage within 16-bits (for performance), we have to scale the precision based on the percentage.
// I.e. the larger the percentage, the smaller the precision has to be (and vice-versa).
//
// We could use __builtin_clz() and use the leading zero count as the precision: but that is slow:
//  * AVR doesn't have a clz ASM instruction, so __builtin_clz() is implemented in software
//  * The algorithm below applies some compile time optimizations via templates.
//
// Note that this array is constexpr & only referenced at compile time.
static constexpr PctApproxPrecisionThreshold PCT_APPROX_PRECISIONS[] = { 
    // These must be in ascending order
    // More array elememnts = slower, but more accurate.
    // I ran various test cases to find the optimal balance of speed and
    // accuracy: 4 thresholds seemed to be the sweet spot.
    { 128U, 9U },
    { 256U, 8U },
    { 512U, 7U },
    { 1024U, 6U },
};

#define TRY_SHIFT_THRESHOLD(threshold, percent, value) \
    if ((percent)<(threshold).percentThreshold) { \
        return _percentageApprox<(threshold).bitsPrecision>((percent), (value)); \
    }

/// @endcond

/**
 * @brief Integer based percentage calculation: faster, but less accurate, than percentage()
 * 
 * Recommended use case is when dealing with percentages >50%.
 * 
 * @param percent The percent to apply to value
 * @param value The value to operate on
 *
 * @note Performance unit test shows a 25% speed improvement over percentage. 
 * However, accuracy decreases as the percentage decreases, compared to percentage():
 * Percent | Maximum Error | Example
 * ------- | :-----------: | :----------------------------------
 * 1%-6%   | 9%            | <c>percentage(4, 563)</c>         -> 23
 * ^       | ^             | <c>percentageApprox(4, 563)</c>   -> 21
 * 7%-40%  | 1%            | <c>percentage(10, 1806)</c>       -> 181
 * ^       | ^             | <c>percentageApprox(10, 1806)</c> -> 179
 * 41%+    | <0.3%         | <c>percentage(79, 2371)</c>       -> 1873
 * ^       | ^             | <c>percentageApprox(79, 2371)</c> -> 1870
 */
static inline uint32_t percentageApprox(uint16_t percent, uint32_t value) {
    // Manual loop unrolling so that PCT_APPROX_PRECISIONS is only referenced at compile time.
    TRY_SHIFT_THRESHOLD(PCT_APPROX_PRECISIONS[0], percent, value)
    TRY_SHIFT_THRESHOLD(PCT_APPROX_PRECISIONS[1], percent, value)
    TRY_SHIFT_THRESHOLD(PCT_APPROX_PRECISIONS[2], percent, value)
    TRY_SHIFT_THRESHOLD(PCT_APPROX_PRECISIONS[3], percent, value)

    static constexpr uint8_t LAST_ARRAY_INDEX = _countof(PCT_APPROX_PRECISIONS)-1U;
    static constexpr uint8_t FINAL_PRECISION = PCT_APPROX_PRECISIONS[LAST_ARRAY_INDEX].bitsPrecision-1U;
    return _percentageApprox<FINAL_PRECISION>(percent, value);
}

/**
 * @brief Slightly faster version of percentageApprox(uint16_t, uint32_t), since we know percent<256.
 */
static inline uint32_t percentageApprox(uint8_t percent, uint32_t value) {
    static_assert((PCT_APPROX_PRECISIONS[1U].percentThreshold-1U)==(uint16_t)UINT8_MAX, "");
    TRY_SHIFT_THRESHOLD(PCT_APPROX_PRECISIONS[0U], percent, value)
    return _percentageApprox<PCT_APPROX_PRECISIONS[1U].bitsPrecision>(percent, value);
}

/**
 * @brief Integer based percentage calculation.
 * 
 * @param percent The percent to apply to value
 * @param value The value to operate on
 */
static inline uint32_t percentage(uint16_t percent, uint32_t value) 
{
    return (uint32_t)div100((uint32_t)value * (uint32_t)percent);
}


/**
 * @brief Integer based half-percentage calculation.
 * 
 * @param percent The percent to calculate ([0, 100])
 * @param value The value to operate on
 * @return uint16_t 
 */
static inline uint16_t halfPercentage(uint16_t percent, uint16_t value) {
    uint32_t x200 = (uint32_t)percent * (uint32_t)value;
#ifdef USE_LIBDIVIDE    
    return (uint16_t)libdivide::libdivide_u32_do_raw(x200 + DIV_ROUND_CORRECT(UINT32_C(200), uint32_t), 2748779070L, 7);
#else
    return (uint16_t)UDIV_ROUND_CLOSEST(x200, UINT16_C(200), uint32_t);
#endif
}

/**
 * @brief Make one pass at correcting the value into the range [min, max)
 * 
 * @param min Minimum value (inclusive)
 * @param max Maximum value (exclusive)
 * @param value Value to nudge
 * @param nudgeAmount Amount to change value by 
 * @return int16_t 
 */
static inline int16_t nudge(int16_t min, int16_t max, int16_t value, int16_t nudgeAmount)
{
    if (value<min) { return value + nudgeAmount; }
    if (value>max) { return value - nudgeAmount; }
    return value;
}

#if defined(CORE_AVR) || defined(ARDUINO_ARCH_AVR)

static inline bool udiv_is16bit_result(uint32_t dividend, uint16_t divisor) {
  return divisor>(uint16_t)(dividend>>16U);
}

#endif
/**
 * @brief Optimised division: uint32_t/uint16_t => uint16_t
 * 
 * Optimised division of unsigned 32-bit by unsigned 16-bit when it is known
 * that the result fits into unsigned 16-bit.
 * 
 * ~60% quicker than raw 32/32 => 32 division on ATMega
 * 
 * @note Bad things will likely happen if the result doesn't fit into 16-bits.
 * @note Copied from https://stackoverflow.com/a/66593564
 * 
 * @param dividend The dividend (numerator)
 * @param divisor The divisor (denominator)
 * @return uint16_t 
 */
static inline uint16_t udiv_32_16 (uint32_t dividend, uint16_t divisor)
{
#if defined(CORE_AVR) || defined(ARDUINO_ARCH_AVR)

    if (divisor==0U || !udiv_is16bit_result(dividend, divisor)) { return UINT16_MAX; }

    #define INDEX_REG "r16"

    asm(
        "    ldi " INDEX_REG ", 16 ; bits = 16\n\t"
        "0:\n\t"
        "    lsl  %A0     ; shift\n\t"
        "    rol  %B0     ;  rem:quot\n\t"
        "    rol  %C0     ;   left\n\t"
        "    rol  %D0     ;    by 1\n\t"
        "    brcs 1f     ; if carry out, rem > divisor\n\t"
        "    cp   %C0, %A1 ; is rem less\n\t"
        "    cpc  %D0, %B1 ;  than divisor ?\n\t"
        "    brcs 2f     ; yes, when carry out\n\t"
        "1:\n\t"
        "    sub  %C0, %A1 ; compute\n\t"
        "    sbc  %D0, %B1 ;  rem -= divisor\n\t"
        "    ori  %A0, 1  ; record quotient bit as 1\n\t"
        "2:\n\t"
        "    dec  " INDEX_REG "     ; bits--\n\t"
        "    brne 0b     ; until bits == 0"
        : "=d" (dividend) 
        : "d" (divisor) , "0" (dividend) 
        : INDEX_REG
    );

    // Lower word contains the quotient, upper word contains the remainder.
    return dividend & 0xFFFFU;
#else
    // The non-AVR platforms are all fast enough (or have built in hardware dividers)
    // so just fall back to regular 32-bit division.
    return dividend / divisor;
#endif
}


/**
 * @brief Same as udiv_32_16(), except this will round to nearest integer 
 * instead of truncating.
 * 
 * Minor performance drop compared to non-rounding version.
 **/
static inline uint16_t udiv_32_16_closest(uint32_t dividend, uint16_t divisor)
{
#if defined(CORE_AVR) || defined(ARDUINO_ARCH_AVR)
    dividend = dividend + (uint32_t)(DIV_ROUND_CORRECT(divisor, uint16_t));
    return udiv_32_16(dividend, divisor);
#else
    return (uint16_t)UDIV_ROUND_CLOSEST(dividend, (uint32_t)divisor, uint32_t);
#endif
}

/**
 * @brief clamps a given value between the minimum and maximum thresholds.
 * 
 * Uses operator< to compare the values.
 * 
 * @tparam T Any type that supports operator<
 * @param v The value to clamp 
 * @param lo The minimum threshold
 * @param hi The maximum threshold
 * @return if v compares less than lo, returns lo; otherwise if hi compares less than v, returns hi; otherwise returns v.
 */
template<class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi){
    return v<lo ? lo : hi<v ? hi : v;
}

/// @cond

template <typename T, typename TPrime>
static inline T LOW_PASS_FILTER_8BIT(T input, uint8_t alpha, T prior) {
  // Intermediate steps are for MISRA compliance
  // Equivalent to: (input * (256 - alpha) + (prior * alpha)) >> 8
  static constexpr T ALPHA_MAX = (T)256;
  T inv_alpha = ALPHA_MAX - (T)alpha;
  TPrime prior_alpha = (prior * (TPrime)alpha);
  TPrime preshift = (input * (TPrime)inv_alpha) + prior_alpha;
  return (T)(preshift / ALPHA_MAX); // Division should resolve to a shift & avoids a MISRA violation
}

/// @endcond

/**
 * @brief Simple low pass IIR filter 16-bit values
 * 
 * This is effectively implementing the smooth filter from playground.arduino.cc/Main/Smooth
 * But removes the use of floats and uses 8 bits of fixed precision.
 * 
 * @param input incoming unfiltered value
 * @param alpha filter factor. 0=off, 255=full smoothing (0.00 to 0.99 in float, 0-99%)
 * @param prior previous *filtered* value.
 * @return uint16_t The filtered input
 */
static inline uint16_t LOW_PASS_FILTER(uint16_t input, uint8_t alpha, uint16_t prior) {
    return LOW_PASS_FILTER_8BIT<uint16_t, uint32_t>(input, alpha, prior);
}

/** @brief Simple low pass IIR filter for S16 values */
static inline int16_t LOW_PASS_FILTER(int16_t input, uint8_t alpha, int16_t prior) {
    return LOW_PASS_FILTER_8BIT<int16_t, int32_t>(input, alpha, prior);
}

#endif