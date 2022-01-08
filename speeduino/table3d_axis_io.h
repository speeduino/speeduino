#pragma once

#include "int16_byte.h"
#include "table3d_axes.h"

extern int16_byte converter_100;
extern int16_byte converter_2;
extern int16_byte converter_1;

constexpr const int16_byte& get_axis_io_converter(axis_domain domain)  {
    return domain==axis_domain_Rpm ? converter_100 :
            domain==axis_domain_Load ? converter_2 : converter_1;
}