#pragma once

#include <stdint.h>
#include <iterator>
#include <array>

namespace mcu8 {
    /**
     * @brief A custom iterator optimised for small arrays on 8-bit MCUs
     * 
     * Standard C++ iterators usually compare two raw pointers (ptr != end_ptr). On an 8-bit MCU,
     * pointers are 16 bits (or 24 bits on larger microcontrollers). Comparing two 16-bit pointers 
     * takes multiple CPU cycles. This design compares a single byte (index != other.index), which 
     * executes in a single cycle.
     */
    template <typename T>
    class iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = int8_t; // 8-bit signed difference
        using pointer           = T*;
        using reference         = T&;

    private:
        T* ptr;
        uint8_t index; // The magic 1-byte counter
    
        // Grants access to private members of iterator<uint8_t> from iterator<const uint8_t>
        template <typename U> friend class iterator;

    public:
        explicit iterator(T* p, uint8_t idx) : ptr(p), index(idx) {}

        template <typename U>
        iterator(const iterator<U>& other) : ptr(other.ptr), index(other.index) {}

        // --- 1. Forward & Bidirectional (Increment / Decrement) ---
        iterator& operator++() { ++index; return *this; }
        iterator operator++(int) { iterator tmp = *this; ++index; return tmp; }
        
        iterator& operator--() { --index; return *this; }
        iterator operator--(int) { iterator tmp = *this; --index; return tmp; }

        // --- 2. Dereferencing & Subscript ---
        reference operator*() const { return ptr[index]; }
        pointer operator->() const { return &ptr[index]; }
        reference operator[](int8_t n) const { return ptr[index + n]; }

        // --- 3. Random Access Math (Using 8-bit offsets) ---
        iterator& operator+=(int8_t n) { index += n; return *this; }
        iterator& operator-=(int8_t n) { index -= n; return *this; }

        // --- Hidden Friends: Math Operators ---
        friend iterator operator+(iterator it, int8_t n) { it += n; return it; }
        friend iterator operator+(int8_t n, iterator it) { it += n; return it; }
        friend iterator operator-(iterator it, int8_t n) { it -= n; return it; }
        
        // Distance between two iterators (Hidden Friend allows iterator vs const_iterator)
        friend int8_t operator-(const iterator& lhs, const iterator& rhs) { 
            return static_cast<int8_t>(lhs.index - rhs.index); 
        }

        // --- Hidden Friends: Relational Operators ---
        // These are optimized to 8-bit register compares and perfectly support mixed const/non-const
        friend bool operator==(const iterator& lhs, const iterator& rhs) { return lhs.index == rhs.index; }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) { return lhs.index != rhs.index; }
        friend bool operator<(const iterator& lhs, const iterator& rhs)  { return lhs.index <  rhs.index; }
        friend bool operator>(const iterator& lhs, const iterator& rhs)  { return lhs.index >  rhs.index; }
        friend bool operator<=(const iterator& lhs, const iterator& rhs) { return lhs.index <= rhs.index; }
        friend bool operator>=(const iterator& lhs, const iterator& rhs) { return lhs.index >= rhs.index; }

        uint8_t pos(void) const { return index; }
    };

    /**
     *  @brief  Return an iterator pointing to the first element of the array.
     *  @param  arr  Array.
     */
    template <typename T, uint8_t N>
    static inline auto begin(T (&arr)[N]) {
        return iterator<T>(arr, 0);
    }

    /**
     *  @brief  Return an iterator pointing to one past the last element
     *          of the array.
     *  @param  arr  Array.
     */
    template <typename T, uint8_t N>
    static inline auto end(T (&arr)[N]) {
        return iterator<T>(arr, N);
    }

    // --- std::array Overloads ---

    /**
     *  @brief  Return an iterator pointing to the first element of a std::array.
     *          Saves pointer comparisons on 8-bit MCUs.
     */
    template <typename T, std::size_t N>
    static inline auto begin(std::array<T, N>& arr) {
        static_assert(N <= 255, "mcu8 only supports arrays up to 255 elements!");
        return iterator<T>(arr.data(), 0);
    }

    /**
     *  @brief  Return a const_iterator pointing to the first element of a const std::array.
     */
    template <typename T, std::size_t N>
    static inline auto begin(const std::array<T, N>& arr) {
        static_assert(N <= 255, "mcu8 only supports arrays up to 255 elements!");
        return iterator<const T>(arr.data(), 0);
    }

    /**
     *  @brief  Return an iterator pointing to one past the last element of a std::array.
     */
    template <typename T, std::size_t N>
    static inline auto end(std::array<T, N>& arr) {
        static_assert(N <= 255, "mcu8 only supports arrays up to 255 elements!");
        return iterator<T>(arr.data(), static_cast<uint8_t>(N));
    }

    /**
     *  @brief  Return a const_iterator pointing to one past the last element of a const std::array.
     */
    template <typename T, std::size_t N>
    static inline auto end(const std::array<T, N>& arr) {
        static_assert(N <= 255, "mcu8 only supports arrays up to 255 elements!");
        return iterator<const T>(arr.data(), static_cast<uint8_t>(N));
    }    
}
