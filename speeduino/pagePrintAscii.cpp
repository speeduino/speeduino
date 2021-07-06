#include "pagePrintAscii.h"

#ifdef SMALL_FLASH_MODE

void printPageAscii(byte pageNum, Print &target)
{
    ; // Noop
}

#else

#include "globals.h"
#include "table_iterator.h"
#include "utilities.h"

// =============================== Helpers - called by generated code =====================

#define PRINT_SPACE_DELIMITED(target, item) \
        target.print(item);                 \
        target.print(F(" ")); 

static void serial_print_space_delimited(Print &target, const uint8_t *first, const uint8_t *last)
{
    while (first!=last)
    {
        PRINT_SPACE_DELIMITED(target, *first)
        ++first;
    }
    target.println();
}

static void serial_print_space_delimited(Print &target, const int16_t *first, const int16_t *last)
{
    while (first!=last)
    {
        PRINT_SPACE_DELIMITED(target, *first)
        ++first;
    }
    target.println();
}

static void serial_print_space_delimited(Print &target, const uint16_t *first, const uint16_t *last)
{
    while (first!=last)
    {
        PRINT_SPACE_DELIMITED(target, *first)
        ++first;
    }
    target.println();
}

static void serial_print_prepadding(Print &target, uint16_t value)
{
    if (value < 100)
    {
        target.print(F(" "));
        if (value < 10)
        {
            target.print(F(" "));
        }
    }
}

static void serial_print_prepadded_value(Print &target, uint16_t value)
{
    serial_print_prepadding(target, value);
    target.print(value);
    target.print(F(" "));
}

static void print_row(Print &target, const table_axis_iterator_t &y_it, table_row_t row)
{
    serial_print_prepadded_value(target, get_value(y_it));

    while (!at_end(row))
    {
        serial_print_prepadded_value(target, *row.pValue++);
    }
    target.println();
}

static void print_x_axis(Print &target, table_axis_iterator_t x_it)
{
    target.print(F("    "));

    while(!at_end(x_it))
    {
        serial_print_prepadded_value(target, get_value(x_it));
        advance_axis(x_it);
    }
}

static void serial_print_3dtable(Print &target, table_row_iterator_t row_it, table_axis_iterator_t x_it, table_axis_iterator_t y_it)
{
    while (!at_end(row_it))
    {
        print_row(target, y_it, get_row(row_it));
        advance_axis(y_it);
        advance_row(row_it);
    }

    print_x_axis(target, x_it);
    target.println();
}

static void serial_print_3dtable(Print &target, const table3D &currentTable)
{
    serial_print_3dtable(target, rows_begin(&currentTable), x_begin(&currentTable), y_begin(&currentTable));
}

#define print_array(outputName, array) serial_print_space_delimited(outputName, array, array+_countof(array));

// Alias page 2 - it's page 1 in the INI file
#define configPage1 configPage2
// As per INI comment
// ;Has to be called algorithm for the req fuel calculator to work :(
#define algorithm fuelAlgorithm
#define alternate injTiming
#define twoStroke strokes

// Pull in the generated code.
#include "pagePrintAscii.g.hpp"

#endif // SMALL_FLASH_MODE