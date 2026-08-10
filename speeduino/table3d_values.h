/** 
 * @addtogroup table_3d 
 *  @{
 */

/** \file
 * @brief 3D table value structs and iterators
 */

#pragma once

#include "table3d_typedefs.h"
#include "preprocessor.h"

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
    table_row_iterator(const table3d_value_t *pRowStart, table3d_dim_t rowWidth)
        : pValue(pRowStart), pEnd(pRowStart+rowWidth)  //cppcheck-suppress misra-c2012-10.4
    {
    }

    // LCOV_EXCL_START
    /** @brief Pointer to the end of the row */
    const table3d_value_t* end(void) const { return pEnd; }
    /** @copydoc table_row_iterator::end() const */
    table3d_value_t* end(void) { return const_cast<table3d_value_t *>(pEnd); }
    // LCOV_EXCL_STOP

    /** @brief Advance the iterator
     * @param steps The number of elements to move the iterator
    */
    table_row_iterator& advance(table3d_dim_t steps)
    { 
        pValue  = pValue + steps;
        return *this;
    }

    /** @brief Increment the iterator by one element*/
    table_row_iterator& operator++(void)
    {
        return advance(1);
    }

    /** @brief Test for end of iteration */
    bool at_end(void) const
    {
        return pValue == pEnd;
    }

    // LCOV_EXCL_START
    /** @brief Dereference the iterator */
    const table3d_value_t& operator*(void) const
    {
        return *pValue;
    }
    /** @copydoc table_row_iterator::operator*() const */
    table3d_value_t& operator*(void)
    {
        return *const_cast<table3d_value_t *>(pValue);
    }
    // LCOV_EXCL_STOP

    /** @brief Number of elements available */
    table3d_dim_t size(void) const { return pEnd-pValue; }

private:
    const table3d_value_t *pValue;
    const table3d_value_t *pEnd;
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
        : pRowsStart(pValues),  //cppcheck-suppress misra-c2012-10.4
        rowWidth(axisSize)
    {
        // Table values are laid out conventionally - rows are in order
        // E.g. a 4x4 table with logical element [0][0] at the bottom left
        // (normal cartesian coordinates) has this layout.
        //  12  13  14  15
        //  8   9   10  11
        //  4   5   6   7
        //  0	1	2	3
        // So we start at row 0 (index 0 of the array) and iterate forward
    }

    /** @brief Advance the iterator
     * @param rows The number of \b rows to move
    */
    table_value_iterator& advance(table3d_dim_t rows)
    {
        _row = _row + rows;
        return *this;
    }

    /** @brief Increment the iterator by one \b row */
    table_value_iterator& operator++(void)
    {
        return advance(1U);
    }

    /** @brief Dereference the iterator to access a row of data */
    table_row_iterator operator*(void) const
    {
        return table_row_iterator(pRowsStart + (_row*rowWidth), rowWidth);
    }
    /** @copydoc table_value_iterator::operator*() const */
    table_row_iterator operator*(void)
    {
        return table_row_iterator(pRowsStart + (_row*rowWidth), rowWidth);
    }    

    /** @brief Test for end of iteration */
    bool at_end(void) const
    {
        return _row == rowWidth;
    }

private:
    const table3d_value_t *pRowsStart;
    table3d_dim_t _row = 0;
    table3d_dim_t rowWidth;
};

#define TABLE3D_TYPENAME_VALUE(size, xDom, yDom) CONCAT(TABLE3D_TYPENAME_BASE(size, xDom, yDom), _values)

#define TABLE3D_GEN_VALUES(size, xDom, yDom) \
    /** @brief The values for a 3D table with size x size dimensions, xDom x-axis and yDom y-axis */ \
    struct TABLE3D_TYPENAME_VALUE(size, xDom, yDom) { \
        /** @brief The number of items in a row. I.e. it's length  */ \
        static constexpr table3d_dim_t row_size = (size); \
        /** @brief The number of rows */ \
        static constexpr table3d_dim_t num_rows = (size); \
        /** @brief The row values */ \
        table3d_value_t values[(uint16_t)row_size*num_rows]; \
        \
        /** @brief Iterate over the values */ \
        table_value_iterator begin(void) \
        {  \
            return table_value_iterator(values, row_size); \
        } \
        \
        /** @brief Direct access to table value element from a linear index */ \
        table3d_value_t& value_at(table3d_dim_t linear_index) \
        { \
            return values[linear_index]; \
        } \
    };
TABLE3D_GENERATOR(TABLE3D_GEN_VALUES)

/** @} */
