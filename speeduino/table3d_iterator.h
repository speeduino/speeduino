/** 
 * @defgroup table_3d 3D Tables
 * @addtogroup table_3d 
 *  @{
 */

/** \file
 * @brief 3D table axes and value iterators
 */

#pragma once

#include "table3d_typedefs.h"

// ========================= AXIS ITERATION =========================  

/** @brief Byte type. This is not defined in any C or C++ standard header. */
typedef uint8_t byte;

/** @brief Represents a 16-bit value as a byte. Useful for I/O. */
class int16_ref
{
public:

    /**
     * @brief Construct
     * @param value The \c int16_t to encapsulate.
     * @param factor The factor to scale the \c int16_t value by when converting to/from a \c byte
     */
    int16_ref(int16_t &value, uint8_t factor) 
        : _value(value), _factor(factor)
    {
    }

    /** Convert to a \c byte */
    inline byte operator*() const { return (byte)(_value/_factor); }

    /** Convert to a \c byte */
    inline explicit operator byte() const { return **this; }

    /** Convert from a \c byte */
    inline int16_ref &operator=( byte in )  { _value = (int16_t)in * (int16_t)_factor; return *this;  }

private:
    int16_t &_value;
    uint8_t _factor;
};

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

/**  @brief Describes an axis. Multiple axes from different tables will have the same metadata */
struct axis_metadata {
    /**  @brief Axis length in number of elements */
    table3d_dim_t axis_length;

    /**  @brief Factor used during I/O when converting 16-bit to 8 bit */
    uint8_t io_factor;

    /** @brief Construct */
    constexpr axis_metadata(table3d_dim_t size, axis_domain domain)
        : axis_length(size), io_factor(domain_to_iofactor(domain))
    {
    // Note that this is constexpr so can be evaluated at compile time
    }

    /** @brief Get the I/O factor given the domain */
    static constexpr uint8_t domain_to_iofactor(axis_domain domain) {
        // This really, really needs to be done at compile time, hence the contexpr
        return domain==axis_domain_Rpm ? 100 :
            domain==axis_domain_Load ? 2 : 1;
    }
};


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
    inline table_axis_iterator& advance(uint8_t steps)
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


// ========================= INTRA-ROW ITERATION ========================= 

/**  @brief Iterate through a table row. I.e. constant Y, changing X 
 * 
 * Instances of this class are normally created via a table_value_iterator instance.
*/
class table_row_iterator {
public:

    /** 
     * @brief Construct
     * @param pRowStart Pointer to the 1st element in the row
     * @param rowWidth The number of elements to in the row
    */
    table_row_iterator(table3d_value_t *pRowStart,  uint8_t rowWidth)
        : pValue(pRowStart), pEnd(pRowStart+rowWidth)
    {
    }

    /** Pointer to the end of the row */
    inline const table3d_value_t* end() const { return pEnd; }

    /** Pointer to the end of the row */
    inline table3d_value_t* end() { return pEnd; }

    /** @brief Advance the iterator
     * @param steps The number of elements to move the iterator
    */
    inline table_row_iterator& advance(uint8_t steps)
    { 
        pValue  = pValue + steps;
        return *this;
    }

    /** @brief Increment the iterator by one element*/
    inline table_row_iterator& operator++()
    {
        return advance(1);
    }

    /** @brief Test for end of iteration */
    inline bool at_end() const
    {
        return pValue == pEnd;
    }

    /** @brief Dereference the iterator */
    inline table3d_value_t& operator*() const
    {
        return *pValue;
    }

    /** @brief Number of elements available */
    inline uint8_t size() const { return pEnd-pValue; }

private:
    table3d_value_t *pValue;
    table3d_value_t *pEnd;
};


// ========================= INTER-ROW ITERATION ========================= 

/**  @brief Iterate through a tables values, row by row. */
class table_value_iterator
{
public:

    /** 
     * @brief Construct
     * @param pValues Pointer to the 1st value in a 1-d array
     * @param axisSize The number of columns & elements per row (square tables only)
    */
    table_value_iterator(const table3d_value_t *pValues, table3d_dim_t axisSize)
    : pRowsStart(const_cast<table3d_value_t*>(pValues) + (axisSize*(axisSize-1))),
      pRowsEnd(const_cast<table3d_value_t*>(pValues) - axisSize),
      rowWidth(axisSize)
    {
        // Table values are not linear in memory - rows are in reverse order
        // E.g. a 4x4 table with logical element [0][0] at the bottom left
        // (normal cartesian coordinates) has this layout.
        //  0	1	2	3
        //  4   5   6   7
        //  8   9   10  11
        //  12  13  14  15
        // So we start at row 3 (index 12 of the array) and iterate towards
        // the start of the array
        //
        // This all supports fast 3d interpolation.
    }

    /** @brief Advance the iterator
     * @param rows The number of \b rows to move
    */
    inline table_value_iterator& advance(uint8_t rows)
    {
        pRowsStart = pRowsStart - (rowWidth * rows);
        return *this;
    }

    /** @brief Increment the iterator by one \b row */
    inline table_value_iterator& operator++()
    {
        return advance(1);
    }

    /** @brief Dereference the iterator to access a row of data */
    inline table_row_iterator operator*() const
    {
        return table_row_iterator(pRowsStart, rowWidth);
    }

    /** @brief Test for end of iteration */
    inline bool at_end() const
    {
        return pRowsStart == pRowsEnd;
    }

private:
    table3d_value_t *pRowsStart;
    table3d_value_t *pRowsEnd;
    uint8_t rowWidth;
};

/** @} */