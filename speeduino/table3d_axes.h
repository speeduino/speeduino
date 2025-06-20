/** 
 * @addtogroup table_3d 
 *  @{
 */

/** \file
 * @brief 3D table axis types and iterators
 */

#pragma once

#include "table3d_typedefs.h"

/**\enum axis_domain
 * @brief Encodes the real world measurement that a table axis captures
 * */
enum axis_domain {
    /** RPM (engine speed) */
    axis_domain_Rpm,
    /** Load */
    axis_domain_Load,
};

static inline constexpr uint16_t axis_domain_to_factor(axis_domain domain)
{
    return domain==axis_domain_Rpm ? 100U : 2U;
}

/** @brief Iterate over table axis elements */
class table_axis_iterator
{
public:

    /** @brief Construct */
    table_axis_iterator(table3d_axis_t *pStart, const table3d_axis_t *pEnd, axis_domain domain)
     : _stride(pEnd>pStart ? stride_inc : stride_dec)
     , _pStart(pStart)
     , _pEnd(pEnd + _stride)
     , _domain(domain) //cppcheck-suppress misra-c2012-10.4
    {
    }

    axis_domain get_domain(void) const { return _domain; }

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
    const axis_domain _domain;
};

#define TABLE3D_TYPENAME_AXIS(size, domain) table3d ## size ## domain ## _axis

#define TABLE3D_GEN_AXIS(size, dom) \
    /** @brief The dxis for a 3D table with size x size dimensions and domain 'domain' */ \
    struct TABLE3D_TYPENAME_AXIS(size, dom) { \
        /** @brief The length of the axis in elements */ \
        static constexpr table3d_dim_t length = (size); \
        /** @brief The domain the axis represents */ \
        static constexpr axis_domain domain = axis_domain_ ## dom; \
        /**
          @brief The axis elements\
        */ \
        table3d_axis_t axis[(size)]; \
        \
        /** @brief Iterate over the axis elements */ \
        table_axis_iterator begin(void) \
        {  \
            return table_axis_iterator(axis+(size)-1, axis, domain); \
        } \
        /** @brief Iterate over the axis elements, from largest to smallest */ \
        table_axis_iterator rbegin(void) \
        {  \
            return table_axis_iterator(axis, axis+(size)-1, domain); \
        } \
    };

// This generates the axis types for the following sizes & domains:
TABLE3D_GEN_AXIS(6, Rpm)
TABLE3D_GEN_AXIS(6, Load)
TABLE3D_GEN_AXIS(4, Rpm)
TABLE3D_GEN_AXIS(4, Load)
TABLE3D_GEN_AXIS(8, Rpm)
TABLE3D_GEN_AXIS(8, Load)
TABLE3D_GEN_AXIS(16, Rpm)
TABLE3D_GEN_AXIS(16, Load)

/** @} */