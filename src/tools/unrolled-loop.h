#pragma once

#include <type_traits>
#include <utility>

namespace detail
{
    template<class T, T... Indexes, class F>
    constexpr void unrolled_loop(std::integer_sequence<T, Indexes...>, F&& f)
    {
        (f(std::integral_constant<T, Indexes>{}), ...);
    }
}

template<class T, T count, class F>
constexpr void unrolled_loop(F&& f)
{
    detail::unrolled_loop(std::make_integer_sequence<T, count>{}, std::forward<F>(f));
}
