#pragma once

#include <type_traits>
#include <tuple>

namespace mbgl {

template<typename T>
constexpr auto underlying_type(T t) -> typename std::underlying_type_t<T> {
    return typename std::underlying_type_t<T>(t);
}

template <std::size_t I = 0, typename T, typename Fn>
std::enable_if_t<I == std::tuple_size<std::decay_t<T>>::value, void> apply(T&&, Fn&&) {
}

template <std::size_t I = 0, typename T, typename Fn>
std::enable_if_t<I != std::tuple_size<std::decay_t<T>>::value, void> apply(T&& t, Fn&& fn) {
    fn(std::get<I>(std::forward<T>(t)));
    apply<I + 1>(std::forward<T>(t), std::forward<Fn>(fn));
}

// Checks whether the template parameter pack contains T
template <typename T, typename Tuple>
struct tuple_contains;

template <typename T>
struct tuple_contains<T, std::tuple<>> : std::false_type {};

template <typename T, typename A, typename... Args>
struct tuple_contains<T, std::tuple<A, Args...>> : tuple_contains<T, std::tuple<Args...>> {};

template <typename T, typename... Args>
struct tuple_contains<T, std::tuple<T, Args...>> : std::true_type {};

// Obtains the type T from the Tuple, or the first (default) parameter otherwise.
template <typename T, typename Tuple>
constexpr std::enable_if_t<tuple_contains<T, Tuple>::value, T&&> get_tuple_type(T&&, Tuple&& tuple) {
    return std::forward<T>(std::get<T>(std::forward<Tuple>(tuple)));
}

template <typename T, typename Tuple>
constexpr std::enable_if_t<!tuple_contains<T, Tuple>::value, T&&> get_tuple_type(T&& t, Tuple&&) {
    return std::forward<T>(t);
}

} // namespace mbgl
