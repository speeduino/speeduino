#include "table3d_axis_io.h"
#include "maths.h"

static byte to_byte_100(int16_t value) { return (byte)div100(value); }
static int16_t from_byte_100(byte in) { return (int16_t)in * 100; }

static byte to_byte_1(int16_t value) { return (byte)value; }
static int16_t from_byte_1(byte in) { return (int16_t)in; }

static byte to_byte_2(int16_t value) { return (byte)(value/2); } //cppcheck-suppress misra-c2012-10.8
static int16_t from_byte_2(byte in) { return (int16_t)in*2; }

table3d_axis_io_converter get_table3d_axis_converter(axis_domain domain)  {
    return domain==axis_domain_Rpm ? table3d_axis_io_converter { &to_byte_100, &from_byte_100 } :
            domain==axis_domain_Load ? table3d_axis_io_converter { &to_byte_2, &from_byte_2 } : 
            table3d_axis_io_converter { &to_byte_1, &from_byte_1 };
}