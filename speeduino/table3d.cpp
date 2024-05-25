#include <stdlib.h>
#include "table3d.h"

// =============================== Iterators =========================

table_value_iterator rows_begin(void *pTable, table_type_t key)
{
  #define CTA_GET_ROW_ITERATOR(size, xDomain, yDomain, pTable) \
      return ((TABLE3D_TYPENAME_BASE(size, xDomain, yDomain)*)pTable)->values.begin();
  #define CTA_GET_ROW_ITERATOR_DEFAULT ({ return table_value_iterator(NULL, 0U); })      
  CONCRETE_TABLE_ACTION(key, CTA_GET_ROW_ITERATOR, CTA_GET_ROW_ITERATOR_DEFAULT, pTable);
}


/**
 * Convert page iterator to table x axis iterator.
 */
table_axis_iterator x_begin(void *pTable, table_type_t key)
{
  #define CTA_GET_X_ITERATOR(size, xDomain, yDomain, pTable) \
      return ((TABLE3D_TYPENAME_BASE(size, xDomain, yDomain)*)pTable)->axisX.begin();
  #define CTA_GET_X_ITERATOR_DEFAULT ({ return table_axis_iterator(NULL, NULL, axis_domain_Tps); })      
  CONCRETE_TABLE_ACTION(key, CTA_GET_X_ITERATOR, CTA_GET_X_ITERATOR_DEFAULT, pTable);
}

table_axis_iterator x_rbegin(void *pTable, table_type_t key)
{
  #define CTA_GET_X_RITERATOR(size, xDomain, yDomain, pTable) \
      return ((TABLE3D_TYPENAME_BASE(size, xDomain, yDomain)*)pTable)->axisX.rbegin();
  #define CTA_GET_X_ITERATOR_DEFAULT ({ return table_axis_iterator(NULL, NULL, axis_domain_Tps); })      
  CONCRETE_TABLE_ACTION(key, CTA_GET_X_RITERATOR, CTA_GET_X_ITERATOR_DEFAULT, pTable);
}

/**
 * Convert page iterator to table y axis iterator.
 */
table_axis_iterator y_begin(void *pTable, table_type_t key)
{
  #define CTA_GET_Y_ITERATOR(size, xDomain, yDomain, pTable) \
      return ((TABLE3D_TYPENAME_BASE(size, xDomain, yDomain)*)pTable)->axisY.begin();
  #define CTA_GET_Y_ITERATOR_DEFAULT ({ return table_axis_iterator(NULL, NULL, axis_domain_Tps); })      
  CONCRETE_TABLE_ACTION(key, CTA_GET_Y_ITERATOR, CTA_GET_Y_ITERATOR_DEFAULT, pTable);
}

table_axis_iterator y_rbegin(void *pTable, table_type_t key)
{
  #define CTA_GET_Y_RITERATOR(size, xDomain, yDomain, pTable) \
      return ((TABLE3D_TYPENAME_BASE(size, xDomain, yDomain)*)pTable)->axisY.rbegin();
  #define CTA_GET_Y_ITERATOR_DEFAULT ({ return table_axis_iterator(NULL, NULL, axis_domain_Tps); })      
  CONCRETE_TABLE_ACTION(key, CTA_GET_Y_RITERATOR, CTA_GET_Y_ITERATOR_DEFAULT, pTable);
}