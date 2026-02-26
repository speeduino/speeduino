#pragma once

#include "table3d.h"

table3d8RpmLoad getDummyTable(void);

static inline uint16_t getXMin(const table3d8RpmLoad &table)
{
    return table.axisX.axis[table.values.num_rows-1U]*axis_domain_to_factor(table.XDomain);
}

static inline uint16_t getXMax(const table3d8RpmLoad &table)
{
    return table.axisX.axis[0U]*axis_domain_to_factor(table.XDomain);
}

static inline uint16_t getYMin(const table3d8RpmLoad &table)
{
    return table.axisY.axis[table.values.num_rows-1U]*axis_domain_to_factor(table.YDomain);
}

static inline uint16_t getYMax(const table3d8RpmLoad &table)
{
    return table.axisY.axis[0U]*axis_domain_to_factor(table.YDomain);
}
