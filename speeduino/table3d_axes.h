/** 
 * @addtogroup table_3d 
 *  @{
 */

/** \file
 * @brief 3D table axis types and iterators
 */

#pragma once

#include "table3d_typedefs.h"

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
    
private:

    static constexpr int8_t stride_inc = 1;
    static constexpr int8_t stride_dec = -1;
    int8_t _stride;
    table3d_axis_t *_pStart;
    const table3d_axis_t *_pEnd;
};

#define TABLE3D_TYPENAME_AXIS(size) table3d ## size ## _axis

#define TABLE3D_GEN_AXIS(size) \
    /** @brief The dxis for a 3D table with size x size dimensions */ \
    struct TABLE3D_TYPENAME_AXIS(size) { \
        /** @brief The length of the axis in elements */ \
        static constexpr table3d_dim_t length = (size); \
        /**
          @brief The axis elements\
        */ \
        table3d_axis_t axis[(size)]; \
        \
        /** @brief Iterate over the axis elements */ \
        table_axis_iterator begin(void) \
        {  \
            return table_axis_iterator(axis+(size)-1, axis); \
        } \
        /** @brief Iterate over the axis elements, from largest to smallest */ \
        table_axis_iterator rbegin(void) \
        {  \
            return table_axis_iterator(axis, axis+(size)-1); \
        } \
    };

// This generates the axis types for the following sizes & domains:
TABLE3D_GEN_AXIS(6)
TABLE3D_GEN_AXIS(4)
TABLE3D_GEN_AXIS(8)
TABLE3D_GEN_AXIS(16)

/** @} */