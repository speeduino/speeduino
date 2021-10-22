#include <stdlib.h>
#include "table3d.h"

#define DEFINE_TABLE_STATICS(size, xDom, yDom) \
  constexpr table3d_metadata DECLARE_3DTABLE_TYPENAME(size, xDom, yDom)::_metadata;
TABLE_GENERATOR(DEFINE_TABLE_STATICS)

// =============================== Iterators =========================

table_value_iterator rows_begin(const void *pTable, table_type_t key)
{
  #define GET_ROW_ITERATOR(size, xDomain, yDomain, pTable) \
      return rows_begin(((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable));
  CONCRETE_TABLE_ACTION(key, GET_ROW_ITERATOR, pTable);
}


/**
 * Convert page iterator to table x axis iterator.
 */
table_axis_iterator x_begin(const void *pTable, table_type_t key)
{
  #define GET_X_ITERATOR(size, xDomain, yDomain, pTable) \
      return x_begin(((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable));
  CONCRETE_TABLE_ACTION(key, GET_X_ITERATOR, pTable);
}


/**
 * Convert page iterator to table y axis iterator.
 */
table_axis_iterator y_begin(const void *pTable, table_type_t key)
{
  #define GET_Y_ITERATOR(size, xDomain, yDomain, pTable) \
      return y_begin(((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable));
  CONCRETE_TABLE_ACTION(key, GET_Y_ITERATOR, pTable);
}