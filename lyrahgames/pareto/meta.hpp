#pragma once
#include <concepts>
#include <functional>
#include <iterator>
#include <ranges>
//
#include <lyrahgames/xstd/forward.hpp>
#include <lyrahgames/xstd/meta.hpp>

namespace lyrahgames::pareto {

namespace generic {

using namespace lyrahgames::xstd::generic;

template <typename T>
concept random_number_generator = true;

template <typename T>
concept real = std::floating_point<T>;

template <typename T>
concept problem = real<typename T::real> &&
    requires(T& p, size_t i, T::real& a, T::real& b) {
  { p.parameter_count() } -> identical<size_t>;
  { p.objective_count() } -> identical<size_t>;
  { p.box_min(i) } -> identical<typename T::real>;
  { p.box_max(i) } -> identical<typename T::real>;
};

template <typename T, typename X, typename Y>
concept evaluatable_problem = problem<T> &&
    requires(T& problem, const X& x, Y&& y) {
  problem.evaluate(x, std::forward<Y>(y));
};

template <typename T>
concept optimizer = true;

template <typename T, typename real>
concept range = (std::ranges::random_access_range<T> &&
                 std::same_as<real, std::ranges::range_value_t<T>>);

template <typename T, typename U>
concept input_iterator =
    std::input_iterator<T> && identical<U, std::iter_value_t<T>>;

template <typename T, typename U>
concept output_iterator = std::output_iterator<T, U>;

template <typename T, typename U>
concept output_range = std::ranges::output_range<T, U>;

template <typename T>
concept frontier = true;

}  // namespace generic

}  // namespace lyrahgames::pareto