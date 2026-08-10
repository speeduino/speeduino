/**
 * @file 
 * @brief Workaround when min() & max() are defined as macros, which generates compile errors in
 * most C++ standard library implementations. 
 * 
 * This file will workaround this. It should probably be included *before* any standard library headers.
 * 
 */
#pragma once

#if !defined(ARDUINO_ARCH_STM32)

#undef max

template<typename _Tp>
constexpr const _Tp& max(const _Tp& __a, const _Tp& __b) {
    if (__b > __a) {
        return __b;
    }
    return __a;
}
#endif

#if !defined(ARDUINO_ARCH_STM32)

#undef min

template<typename _Tp>
constexpr const _Tp& min(const _Tp& __a, const _Tp& __b) {
    if (__b < __a) {
        return __b;
    }
    return __a;
}

#endif