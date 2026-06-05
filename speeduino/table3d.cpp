#include <stdlib.h>
#include "table3d.h"
#include "table3d_visitor.h"

// =============================== Iterators =========================

struct row_begin_visitor {
    template <typename TTable>
    table_value_iterator visit(TTable &table) {
        return table.values.begin();
    }
};

table_value_iterator rows_begin(table3d_t *pTable, TableType key)
{
  row_begin_visitor visitor;
  // LCOV_EXCL_BR_START
  return visitTable3d<row_begin_visitor, table_value_iterator>(*pTable, key, visitor);
  // LCOV_EXCL_BR_STOP
}

struct x_begin_visitor {
    template <typename TTable>
    table_axis_iterator visit(TTable &table) {
        return table.axisX.begin();
    }
};

table_axis_iterator x_begin(table3d_t *pTable, TableType key)
{
  x_begin_visitor visitor;
  // LCOV_EXCL_BR_START
  return visitTable3d<x_begin_visitor, table_axis_iterator>(*pTable, key, visitor);
  // LCOV_EXCL_BR_STOP
}


struct x_rbegin_visitor {
    template <typename TTable>
    table_axis_iterator visit(TTable &table) {
        return table.axisX.rbegin();
    }
};

table_axis_iterator x_rbegin(table3d_t *pTable, TableType key)
{
  x_rbegin_visitor visitor;
  // LCOV_EXCL_BR_START
  return visitTable3d<x_rbegin_visitor, table_axis_iterator>(*pTable, key, visitor);
  // LCOV_EXCL_BR_STOP
}

struct y_begin_visitor {
    template <typename TTable>
    table_axis_iterator visit(TTable &table) {
        return table.axisY.begin();
    }
};

table_axis_iterator y_begin(table3d_t *pTable, TableType key)
{
  y_begin_visitor visitor;
  // LCOV_EXCL_BR_START
  return visitTable3d<y_begin_visitor, table_axis_iterator>(*pTable, key, visitor);
  // LCOV_EXCL_BR_STOP
}


struct y_rbegin_visitor {
    template <typename TTable>
    table_axis_iterator visit(TTable &table) {
        return table.axisY.rbegin();
    }
};

table_axis_iterator y_rbegin(table3d_t *pTable, TableType key)
{
  y_rbegin_visitor visitor;
  // LCOV_EXCL_BR_START
  return visitTable3d<y_rbegin_visitor, table_axis_iterator>(*pTable, key, visitor);
  // LCOV_EXCL_BR_STOP
}
