#include "table3d_axis_io.h"
#include "maths.h"

static byte to_byte_1(int16_t value) { return (byte)value; }
static int16_t from_byte_1(byte in) { return (int16_t)in; }

table3d_axis_io_converter get_table3d_axis_converter(axis_domain)  {
    return table3d_axis_io_converter { &to_byte_1, &from_byte_1 };
}