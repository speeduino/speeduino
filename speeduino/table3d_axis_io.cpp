#include "table3d_axis_io.h"
#include "src/libdivide/constant_fast_div.h"

static constexpr libdivide::libdivide_s16_t divider_100 = { S16_MAGIC(100), S16_MORE(100) };
int16_byte converter_100(100, divider_100);

static constexpr libdivide::libdivide_s16_t divider_2 = { S16_MAGIC(2), S16_MORE(2) };
int16_byte converter_2(2, divider_2);

static constexpr libdivide::libdivide_s16_t divider_1 = { S16_MAGIC(1), S16_MORE(1) };
int16_byte converter_1(1, divider_1);