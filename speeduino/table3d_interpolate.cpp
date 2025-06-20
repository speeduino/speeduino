#include "table3d_interpolate.h"
#include "maths.h"


// ============================= Axis Bin Searching =========================

static inline bool is_in_bin(const table3d_axis_t &testValue, const table3d_axis_t &min, const table3d_axis_t &max)
{
  return testValue > min && testValue <= max;
}

// Find the axis index for the top of the bin that covers the test value.
// E.g. 4 in { 1, 3, 5, 7, 9 } would be 2
// We assume the axis is in order.
static inline table3d_dim_t find_bin_max(
  const table3d_axis_t &value,        // Value to search for
  const table3d_axis_t *pAxis,  // The axis to search
  table3d_dim_t minElement,     // Axis index of the element with the lowest value (at one end of the array)
  table3d_dim_t maxElement,     // Axis index of the element with the highest value (at the other end of the array)
  table3d_dim_t lastBinMax)     // The last result from this call - used to speed up searches
{
  // It's quicker to increment/adjust this pointer than to repeatedly 
  // index the array - minimum 2%, often >5%
  const table3d_axis_t *pMax = nullptr;
  // minElement is at one end of the array, so the "lowest" bin 
  // is [minElement, minElement+stride]. Since we're working with the upper
  // index of the bin pair, we can't go below minElement + stride.
  table3d_dim_t minBinIndex = minElement - 1U;

  // Check the cached last bin and either side first - it's likely that this will give a hit under
  // real world conditions

  // Check if we're still in the same bin as last time
  pMax = pAxis + lastBinMax;
  if (is_in_bin(value, *(pMax + 1U), *pMax))
  {
    return lastBinMax;
  }
  // Check the bin above the last one
  pMax = pMax + 1U;
  if (lastBinMax!=minBinIndex && is_in_bin(value, *(pMax + 1U), *pMax))
  {
    return lastBinMax + 1U;    
  }
  // Check the bin below the last one
  pMax -= 2U;
  if (lastBinMax!=maxElement && is_in_bin(value, *(pMax + 1U), *pMax))
  {
    return lastBinMax - 1U;
  }

  // Check if outside array limits - won't happen often in the real world
  // so check after the cache check
  // At or above maximum - clamp to final value
  if (value>=pAxis[maxElement])
  {
    return maxElement;
  }
  // At or below minimum - clamp to lowest value
  if (value<=pAxis[minElement])
  {
    return minElement - 1U;
  }

  // No hits above, so run a linear search.
  // We start at the maximum & work down, rather than looping from [0] up to [max]
  // This is because the important tables (fuel and injection) will have the highest
  // RPM at the top of the X axis, so starting there will mean the best case occurs 
  // when the RPM is highest (and hence the CPU is needed most)
  lastBinMax = maxElement;
  pMax = pAxis + lastBinMax;
  while (lastBinMax!=minBinIndex && !is_in_bin(value, *(pMax + 1U), *pMax))
  {
    ++lastBinMax;
    ++pMax;
  }
  return lastBinMax;
}

table3d_dim_t find_xbin(const table3d_axis_t &value, const table3d_axis_t *pAxis, table3d_dim_t size, table3d_dim_t lastBin)
{
  return find_bin_max(value, pAxis, size-1U, 0U, lastBin);
}

table3d_dim_t find_ybin(const table3d_axis_t &value, const table3d_axis_t *pAxis, table3d_dim_t size, table3d_dim_t lastBin)
{
  // Y axis is stored in reverse for performance purposes (not sure that's still valid). 
  // The minimum value is at the end & max at the start. So need to adjust for that. 
  return find_bin_max(value, pAxis, size-1U, 0U, lastBin);
}

// ========================= Fixed point math =========================

// An unsigned fixed point number type with 1 integer bit & 8 fractional bits.
// See https://en.wikipedia.org/wiki/Q_(number_format).
// This is specialised for the number range 0..1 - a generic fixed point
// class would miss some important optimisations. Specifically, we can avoid
// type promotion during multiplication.
typedef uint16_t QU1X8_t;
static constexpr QU1X8_t QU1X8_INTEGER_SHIFT = 8;
static constexpr QU1X8_t QU1X8_ONE = (QU1X8_t)1U << QU1X8_INTEGER_SHIFT;
static constexpr QU1X8_t QU1X8_HALF = (QU1X8_t)1U << (QU1X8_INTEGER_SHIFT-1U);

static inline QU1X8_t mulQU1X8(QU1X8_t a, QU1X8_t b)
{
    // 1x1 == 1....but the real reason for this is to avoid 16-bit multiplication overflow.
    //
    // We are using uint16_t as our underlying fixed point type. If we follow the regular
    // code path, we'd need to promote to uint32_t to avoid overflow.
    //
    // The overflow can only happen when *both* the X & Y inputs
    // are at the edge of a bin. 
    //
    // This is a rare condition, so most of the time we can use 16-bit multiplication and gain performance
    if (a==QU1X8_ONE && b==QU1X8_ONE)
    {
        return QU1X8_ONE;
    }
  // Add the equivalent of 0.5 to the final calculation pre-rounding.
  // This will have the effect of rounding to the nearest integer, rather
  // than always rounding down.
  return ((a * b) + QU1X8_HALF) >> QU1X8_INTEGER_SHIFT;
}

// ============================= Axis value to bin % =========================

static inline QU1X8_t compute_bin_position(const uint16_t value, const table3d_dim_t &bin, const table3d_axis_t *pAxis, const uint16_t &multiplier)
{
  uint16_t binMinValue = pAxis[bin+1U]*multiplier;
  if (value<=binMinValue) { return 0U; }
  uint16_t binMaxValue = pAxis[bin]*multiplier;
  if (value>=binMaxValue) { return QU1X8_ONE; }
  uint16_t binWidth = binMaxValue-binMinValue;

  // Since we can have bins of any width, we need to use 
  // 24.8 fixed point to avoid overflow
  uint16_t binPosition = value - binMinValue;
  uint32_t p = (uint32_t)binPosition << QU1X8_INTEGER_SHIFT;
  // But since we are computing the ratio (0 to 1), p is guaranteed to be
  // less than binWidth and thus the division below will result in a value
  // <=1. So we can reduce the data type from 24.8 (uint32_t) to 1.8 (uint16_t)
  return udiv_32_16(p, (uint16_t)binWidth);  
}

template <uint16_t xMultiplier, uint16_t yMultiplier>
static inline table3d_value_t interpolate_3d_value(const table3DGetValueCache *pValueCache, 
                    const table3d_dim_t axisSize,
                    const table3d_value_t *pValues,
                    const table3d_axis_t *pXAxis,
                    const table3d_axis_t *pYAxis)
{
    table3d_dim_t rowMax = pValueCache->lastYBinMax * axisSize;
    table3d_dim_t rowMin = rowMax + axisSize;
    table3d_dim_t colMax = axisSize - pValueCache->lastXBinMax - 1U;
    table3d_dim_t colMin = colMax - 1U;
    table3d_value_t A = pValues[rowMax + colMin];
    table3d_value_t B = pValues[rowMax + colMax];
    table3d_value_t C = pValues[rowMin + colMin];
    table3d_value_t D = pValues[rowMin + colMax];  
  
    //Check that all values aren't just the same (This regularly happens with things like the fuel trim maps)
    if( (A == B) && (A == C) && (A == D) ) 
    { 
      return A;
    }
    else
    {
      //Create some normalised position values
      //These are essentially percentages (between 0 and 1) of where the desired value falls between the nearest bins on each axis
      const QU1X8_t p = compute_bin_position(pValueCache->last_lookup.x, pValueCache->lastXBinMax, pXAxis, xMultiplier);
      const QU1X8_t q = compute_bin_position(pValueCache->last_lookup.y, pValueCache->lastYBinMax, pYAxis, yMultiplier);

      const QU1X8_t m = mulQU1X8(QU1X8_ONE-p, q);
      const QU1X8_t n = mulQU1X8(p, q);
      const QU1X8_t o = mulQU1X8(QU1X8_ONE-p, QU1X8_ONE-q);
      const QU1X8_t r = mulQU1X8(p, QU1X8_ONE-q);
      return ( (A * m) + (B * n) + (C * o) + (D * r) ) >> QU1X8_INTEGER_SHIFT;
    }
}

// ============================= End internal support functions =========================

//This function pulls a value from a 3D table given a target for X and Y coordinates.
//It performs a 2D linear interpolation as described in: www.megamanual.com/v22manual/ve_tuner.pdf
template <>
table3d_value_t __attribute__((noclone)) get3DTableValue<100U, 2U>(struct table3DGetValueCache *pValueCache, 
                    table3d_dim_t axisSize,
                    const table3d_value_t *pValues,
                    const table3d_axis_t *pXAxis,
                    const table3d_axis_t *pYAxis,
                    const uint16_t Y_in, const uint16_t X_in)
{
    //0th check is whether the same X and Y values are being sent as last time. 
    // If they are, this not only prevents a lookup of the axis, but prevents the 
    //interpolation calcs being performed
    if( X_in == pValueCache->last_lookup.x && 
        Y_in == pValueCache->last_lookup.y)
    {
      return pValueCache->lastOutput;
    }

    // Figure out where on the axes the incoming coord are
    pValueCache->lastXBinMax = find_xbin(div100(X_in), pXAxis, axisSize, pValueCache->lastXBinMax);
    pValueCache->lastYBinMax = find_ybin(Y_in>>1U, pYAxis, axisSize, pValueCache->lastYBinMax);
    pValueCache->last_lookup.x = X_in;
    pValueCache->last_lookup.y = Y_in;
    pValueCache->lastOutput = interpolate_3d_value<100U, 2U>(pValueCache, axisSize, pValues, pXAxis, pYAxis);

    return pValueCache->lastOutput;
}
