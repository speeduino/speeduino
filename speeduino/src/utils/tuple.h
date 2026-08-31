#pragma once

#include <tuple>

namespace stdext {

///@cond 
namespace details {
template <class F, size_t... Is>
constexpr auto index_apply_impl(F f,
                                std::index_sequence<Is...>) {
    return f(std::integral_constant<size_t, Is> {}...);
}

template <size_t N, class F>
constexpr auto index_apply(F f) {
    return index_apply_impl(f, std::make_index_sequence<N>{});
}
}
///@endcond 

/**
 * @brief Remove the last element(s) from a tuple
 * 
 * @tparam Tuple Tuple type
 * @param t The tuple
 * @return constexpr auto 
 */
template <typename Tuple>
constexpr auto pop_back(Tuple&& t) {
    constexpr std::size_t size = std::tuple_size<std::decay_t<Tuple>>::value;
    static_assert(size > 0, "Cannot pop from an empty tuple!");
    
    return details::index_apply<size-1>([&](auto... Is) {
        return std::make_tuple(std::get<Is>(std::forward<Tuple>(t))...);
    });
}

} // namespace stdext