#pragma once

#include <unity.h>
struct unity_filename_guard_t
{
    unity_filename_guard_t(const char *szNew)
    : _old(Unity.TestFile)
    {
        Unity.TestFile = szNew;
    }
    ~unity_filename_guard_t()
    {
        Unity.TestFile = _old;
    }
    const char *_old;
};
