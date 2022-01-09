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
    /** Throttle position */ 
    axis_domain_Tps 
};

/** @brief Iterate over table axis elements */
class table_axis_iterator
{
public:

    /** @brief Construct */
    table_axis_iterator(const table3d_axis_t *pStart, const table3d_axis_t *pEnd, int8_t stride, axis_domain domain)
     : _pAxis(pStart), _pAxisEnd(pEnd), _stride(stride), _domain(domain)
    {
    }

    axis_domain domain(void) const { return _domain; }

    /** @brief Advance the iterator
     * @param steps The number of elements to move the iterator
    */
    inline table_axis_iterator& advance(table3d_dim_t steps)
    {
        _pAxis = _pAxis + (_stride * steps);
        return *this;
    }

    /** @brief Increment the iterator by one element*/
    inline table_axis_iterator& operator++(void)
    {
        return advance(1);
    }

    /** @brief Test for end of iteration */
    inline bool at_end(void) const
    {
        return _pAxis == _pAxisEnd;
    }

    /** @brief Dereference the iterator */
    inline table3d_axis_t& operator*(void)
    {
        return *const_cast<table3d_axis_t *>(_pAxis);
    }
    /** @copydoc table_axis_iterator::operator*()  */
    inline const table3d_axis_t& operator*(void) const
    {
        return *_pAxis;
    }    
    
    /** @brief Reverse the iterator direction
     * 
     * Iterate from the end to the start. <b>This is only meant to be called on a freshly constructed iterator.</b>
     */
    inline table_axis_iterator& reverse(void)
    {
        const table3d_axis_t *_pOldAxis = _pAxis;
        _pAxis = _pAxisEnd - _stride;
        _pAxisEnd = _pOldAxis - _stride;
        _stride = (int8_t)(_stride * -1);
        return *this;
    }

private:
    const table3d_axis_t *_pAxis;
    const table3d_axis_t *_pAxisEnd;
    int8_t _stride;
    axis_domain _domain;
};

#define TABLE3D_TYPENAME_AXIS(size, domain) table3d ## size ## domain ## _axis

#define TABLE3D_GEN_AXIS(size, dom) \
    /** @brief The dxis for a 3D table with size x size dimensions and domain 'domain' */ \
    struct TABLE3D_TYPENAME_AXIS(size, dom) { \
        /** @brief The length of the axis in elements */ \
        static constexpr table3d_dim_t length = size; \
        /** @brief The domain the axis represents */ \
        static constexpr axis_domain domain = axis_domain_ ## dom; \
        /**
          @brief The axis elements\
        */ \
        table3d_axis_t axis[size]; \
        \
        /** @brief Iterate over the axis elements */ \
        inline table_axis_iterator begin(void) \
        {  \
            return table_axis_iterator(axis+(size-1), axis-1, -1, domain); \
        } \
    };

// This generates the axis types for the following sizes & domains:
TABLE3D_GEN_AXIS(6, Rpm)
TABLE3D_GEN_AXIS(6, Load)
TABLE3D_GEN_AXIS(4, Rpm)
TABLE3D_GEN_AXIS(4, Load)
TABLE3D_GEN_AXIS(8, Rpm)
TABLE3D_GEN_AXIS(8, Load)
TABLE3D_GEN_AXIS(8, Tps)
TABLE3D_GEN_AXIS(16, Rpm)
TABLE3D_GEN_AXIS(16, Load)

/** @} */