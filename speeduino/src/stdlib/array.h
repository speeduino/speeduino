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

/** @brief A free function to populate an array from a source C-style array*/
template <typename T, size_t N, size_t M>
constexpr array<T, N>& fill(array<T, N>& arr, const T (&src)[M]) {
    for (size_t i = 0; i < N && i < M; ++i) {
        arr[i] = src[i];
    }
    return arr;
}

/** @brief A free function to populate an array from a source array*/
template <typename T, size_t N, size_t M>
constexpr array<T, N>& fill(array<T, N>& arr, const array<T, M> &src) {
    for (size_t i = 0; i < N && i < M; ++i) {
        arr[i] = src[i];
    }
    return arr;
}

// LCOV_EXCL_STOP
