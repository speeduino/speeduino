#pragma once

#include <stdint.h>
#include <avr-fast-shift.h>

/** 
 * @brief Fractional math based on shifts 
 * 
 * Using multiply + a compile time shift is much quicker than using runtime division. 
 * However, this restricts the denominator to a power of 2.
 */
template <uint8_t denominator>
class FastFraction
{
public:
  static_assert(denominator > 0 && (denominator & (denominator - 1)) == 0, "Denominator must be an exact power of 2");

  static constexpr uint8_t log2_exact(uint8_t n) noexcept {
    uint8_t exp = 0;
    while (n > 1) {
        n >>= 1;
        ++exp;
    }
    return exp;
  }

  explicit constexpr FastFraction(uint8_t numerator)
  : numerator_(numerator)
  {
  }

  /** 
   * @brief Integer multiplication by a fraction
   *
   * @note This uses integer math, so bigger fraction are more accurate.
   * E.g. (n * 5/8) is more accurate than (n + (n * 1/8))
   */  
  /// @{
  friend uint32_t operator*(uint32_t lhs, const FastFraction &fraction)
  {
    return rshift<log2_exact(denominator)>(lhs*fraction.numerator_);
  }
  friend uint32_t operator*(const FastFraction &fraction, uint32_t rhs)
  {
    return rhs * fraction;
  }
  /// @}

// private:
  uint8_t numerator_;
};

template <uint8_t D1, uint8_t D2>
constexpr auto operator*(const FastFraction<D1> &f1, const FastFraction<D2> &f2)
{
  return FastFraction<D1*D2>(f1.numerator_*f2.numerator_);
}

/** @brief Predefined fractions */  
/// @{
constexpr FastFraction<2> ONE_HALF(1);
constexpr auto ONE_QUARTER = ONE_HALF*ONE_HALF;
constexpr auto ONE_EIGTH = ONE_QUARTER*ONE_HALF;
constexpr auto ONE_SIXTEENTH = ONE_EIGTH*ONE_HALF;
constexpr auto THREE_EIGTHS = ONE_EIGTH*FastFraction<1>(3);
constexpr auto THREE_QUARTERS = ONE_QUARTER*FastFraction<1>(3);
constexpr auto FIVE_EIGTHS = ONE_EIGTH*FastFraction<1>(5);
constexpr auto NINE_EIGTHS = ONE_EIGTH*FastFraction<1>(9);
constexpr auto ELEVEN_EIGTHS = ONE_EIGTH*FastFraction<1>(11);
constexpr auto NINE_SIXTEENTHS = ONE_SIXTEENTH*FastFraction<1>(9);
constexpr auto FIVE_QUARTERS = ONE_QUARTER*FastFraction<1>(5);
constexpr auto ONE_AND_HALF = ONE_HALF*FastFraction<1>(3);
/// @}
