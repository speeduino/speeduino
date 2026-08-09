/** 
 * @addtogroup table_3d 
 *  @{
 */

/** \file
 * @brief 3D table axis types and iterators
 */

#pragma once

#include "table3d_typedefs.h"

/**\enum AxisDomain
 * @brief Encodes the real world measurement that a table axis captures
 * */
enum class AxisDomain : uint8_t {
    /** RPM (engine speed) */
    Rpm,
    /** Load */
    Load,
};

static constexpr uint16_t getConversionFactor(AxisDomain domain)
{
    return domain==AxisDomain::Rpm ? 100U : 2U;
}

/** @brief Iterate over table axis elements */
class table_axis_iterator
{
public:

    /** @brief Construct */
    table_axis_iterator(table3d_axis_t *pStart, const table3d_axis_t *pEnd)
     : _stride(pEnd>pStart ? stride_inc : stride_dec)
     , _pStart(pStart)
     , _pEnd(pEnd + _stride)
    {
    }

    /** @brief Advance the iterator
     * @param steps The number of elements to move the iterator
    */
    table_axis_iterator& advance(int8_t steps)
    {
        _pStart = _pStart + ((int16_t)_stride * steps);
        return *this;
    }

    /** @brief Increment the iterator by one element*/
    table_axis_iterator& operator++(void)
    {
        return advance(1);
    }

    /** @brief Test for end of iteration */
    bool at_end(void) const
    {
        return _pStart == _pEnd;
    }

    // LCOV_EXCL_START
    /** @brief Dereference the iterator */
    table3d_axis_t& operator*(void)
    {
        return *_pStart;
    }
    /** @copydoc table_axis_iterator::operator*()  */
    const table3d_axis_t& operator*(void) const
    {
        return *_pStart;
    }    
    // LCOV_EXCL_STOP
    
private:

    static constexpr int8_t stride_inc = 1;
    static constexpr int8_t stride_dec = -1;
    int8_t _stride;
    table3d_axis_t *_pStart;
    const table3d_axis_t *_pEnd;
};

/** @} */