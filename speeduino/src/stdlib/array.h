/**
 * @file
 * @brief Minimal polyfill for the standard library array, since it isn't
 * available with avr-gcc.
 */
#pragma once

#include <stdint.h>

// LCOV_EXCL_START

// 1. Primary template for N > 0
template <typename T, size_t N>
struct array {
    // Public member data allows aggregate initialization
    T _elements[N] = {};

    // Types
    using value_type      = T;
    using size_type       = size_t;
    using reference       = T&;
    using const_reference = const T&;
    using iterator        = T*;
    using const_iterator  = const T*;

    // Constructors
    constexpr array() = default;

    // Element Access
    constexpr reference operator[](size_type index) noexcept { 
        return _elements[index]; 
    }
    
    constexpr const_reference operator[](size_type index) const noexcept { 
        return _elements[index]; 
    }

    constexpr reference at(size_type index) {
        return _elements[index];
    }

    constexpr const_reference at(size_type index) const {
        return _elements[index];
    }

    constexpr reference front() noexcept { return _elements[0]; }
    constexpr const_reference front() const noexcept { return _elements[0]; }
    
    constexpr reference back() noexcept { return _elements[N - 1]; }
    constexpr const_reference back() const noexcept { return _elements[N - 1]; }

    constexpr T* data() noexcept { return _elements; }
    constexpr const T* data() const noexcept { return _elements; }

    // Iterators
    constexpr iterator begin() noexcept { return _elements; }
    constexpr const_iterator begin() const noexcept { return _elements; }
    constexpr iterator end() noexcept { return _elements + N; }
    constexpr const_iterator end() const noexcept { return _elements + N; }

    // Capacity
    constexpr bool empty() const noexcept { return false; }
    constexpr size_type size() const noexcept { return N; }
    constexpr size_type max_size() const noexcept { return N; }

    // Operations
    constexpr void fill(const T& value) {
        for (size_t i = 0; i < N; ++i) {
            _elements[i] = value;
        }
    }
};

/** @brief A free function to populate a C-style array from a source C-style array*/
template <typename T>
static constexpr void copy(T *target, size_t N, const T *src, size_t M) {
    (void)memcpy(target, src, N<M ? N : M);
}

/** @brief A free function to populate a C-style array from a source C-style array*/
template <typename T, size_t N, size_t M>
static constexpr void copy(T (&target)[N], const T (&src)[M]) {
    copy(target, N, src, M);
}

// LCOV_EXCL_STOP
