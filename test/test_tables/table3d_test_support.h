#pragma once

#include "table3d.h"

table3d8RpmLoad getDummyTable(void);

static inline uint16_t getXMin(const table3d8RpmLoad &table)
{
    return table.axisX.front()*getConversionFactor(table.XDomain);
}

static inline uint16_t getXMax(const table3d8RpmLoad &table)
{
    return table.axisX.back()*getConversionFactor(table.XDomain);
}

static inline uint16_t getYMin(const table3d8RpmLoad &table)
{
    return table.axisY.front()*getConversionFactor(table.YDomain);
}

static inline uint16_t getYMax(const table3d8RpmLoad &table)
{
    return table.axisY.back()*getConversionFactor(table.YDomain);
}
