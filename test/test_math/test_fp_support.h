#pragma once
#include <stdio.h>
#include <unity.h>
#include <fp64lib.h>

float64_t floatDivision(int32_t a, int32_t b);

void assert_rounded_div(int32_t a, int32_t b, int32_t actual);