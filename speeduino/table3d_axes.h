/** 
 * @addtogroup table_3d 
 *  @{
 */

/** \file
 * @brief 3D table axis types and iterators
 */

#pragma once

#include "table3d_typedefs.h"
#include "int16_ref.h"

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

constexpr uint8_t domain_to_iofactor(axis_domain domain) {
    // This really, really needs to be done at compile time, hence the contexpr
    return domain==axis_domain_Rpm ? 100 :
        domain==axis_domain_Load ? 2 : 1;
}

/** @brief Iterate over table axis elements */
class table_axis_iterator
{
public:

    /** @brief Construct */
    table_axis_iterator(const table3d_axis_t *pStart, const table3d_axis_t *pEnd, uint8_t io_factor, int8_t stride)
     : _pAxis(pStart), _pAxisEnd(pEnd), _axisFactor(io_factor), _stride(stride)
    {
    }

    /** @brief Advance the iterator
     * @param steps The number of elements to move the iterator
    */
    inline table_axis_iterator& advance(table3d_dim_t steps)
    {
        _pAxis = _pAxis + (_stride * steps);
        return *this;
    }

    /** @brief Increment the iterator by one element*/
    inline table_axis_iterator& operator++()
    {
        return advance(1);
    }

    /** @brief Test for end of iteration */
    inline bool at_end() const
    {
        return _pAxis == _pAxisEnd;
    }

    /** @brief Dereference the iterator */
    inline int16_ref operator *() const
    {
        return int16_ref(*const_cast<table3d_axis_t*>(_pAxis), _axisFactor);
    }
    
    /** @brief Reverse the iterator direction
     * 
     * Iterate from the end to the start
     */
    inline table_axis_iterator& reverse()
    {
        // This is only meant to be called on a freshly constructed iterator.
        const table3d_axis_t *_pOldAxis = _pAxis;
        _pAxis = _pAxisEnd - _stride;
        _pAxisEnd = _pOldAxis - _stride;
        _stride = (int8_t)(_stride * -1);
        return *this;
    }

private:
    const table3d_axis_t *_pAxis;
    const table3d_axis_t *_pAxisEnd;
    uint8_t _axisFactor;
    int8_t _stride;
};

#define XAXIS_TYPENAME(size, xDom, yDom) xaxis_ ##size ## xDom ## yDom

#define GEN_XAXIS(size, xDom, yDom) \
    struct XAXIS_TYPENAME(size, xDom, yDom) { \
        static constexpr table3d_dim_t length = size; \
        static constexpr axis_domain domain = axis_domain_ ## xDom; \
        table3d_axis_t axis[size]; \
        \
        inline table_axis_iterator begin() \
        {  \
            return table_axis_iterator(axis, axis+size, domain_to_iofactor(domain), 1); \
        } \
    };
TABLE_GENERATOR(GEN_XAXIS)

#define YAXIS_TYPENAME(size, xDom, yDom) yaxis_ ##size ## xDom ## yDom

#define GEN_YAXIS(size, xDom, yDom) \
    struct YAXIS_TYPENAME(size, xDom, yDom) { \
        static constexpr table3d_dim_t length = size; \
        static constexpr axis_domain domain = axis_domain_ ## yDom; \
        table3d_axis_t axis[size]; \
        \
        inline table_axis_iterator begin() \
        { \
            return table_axis_iterator(axis+(size-1), axis-1, domain_to_iofactor(domain), -1); \
        } \
    };
TABLE_GENERATOR(GEN_YAXIS)

