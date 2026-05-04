#pragma once

/**
 * @file 
 * @brief Visitor pattern for the 3D table types. 
 * 
 * This allows us to write code that operates on the tables without having to know the concrete type of the table.
 */

#include "table3d.h"

/**
 * @brief Visit a 3D table with a visitor. 
 * 
 * The visitor is a struct that has a visit() method for each concrete table
 * type, and a default visit() method for the case where the table type is 
 * not recognised.
 * 
 * @param table 3d table instance to visit
 * @param visitor Visitor to apply to the table
 * @return TReturn Return value from the visitor, if any
 */
template <typename TConcreteVisitor, typename TReturn = void>
static inline TReturn visitTable3d(table3d_t &table, TableType key, TConcreteVisitor &visitor)
{
    switch (key)
    {
        // LCOV_EXCL_START
        default:
        case TableType::table_type_None:
        // LCOV_EXCL_STOP
/// @cond
        #define VISIT_CASE(size, xDom, yDom) \
            case TableType::TO_TYPE_KEY(size, xDom, yDom): \
                return visitor.visit(static_cast<TABLE3D_TYPENAME_BASE(size, xDom, yDom) &>(table)); 
/// @endcond

        TABLE3D_GENERATOR(VISIT_CASE)
    }
}