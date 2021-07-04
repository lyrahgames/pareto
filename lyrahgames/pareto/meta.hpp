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

template <typename T, typename real>
concept range = (std::ranges::random_access_range<T> &&
                 std::same_as<real, std::ranges::range_value_t<T>>);

template <typename T, typename U>
concept input_iterator =
    std::input_iterator<T> && identical<U, std::iter_value_t<T>>;

template <typename T, typename U>
concept input_range =
    std::ranges::input_range<T> && identical<U, std::ranges::range_value_t<T>>;

template <typename T, typename U>
concept output_iterator = std::output_iterator<T, U>;

template <typename T, typename U>
concept output_range = std::ranges::output_range<T, U>;

/// Basic Concept for General Pareto Problems
template <typename T>
concept problem = real<typename T::real> &&
    requires(T& p, size_t i, T::real& a, T::real& b) {
  { p.parameter_count() } -> identical<size_t>;
  { p.objective_count() } -> identical<size_t>;
  { p.box_min(i) } -> identical<typename T::real>;
  { p.box_max(i) } -> identical<typename T::real>;
};

/// General Pareto Problems Evaluatable on a Given Range Type
template <typename T, typename X, typename Y>
concept evaluatable_problem = problem<T> &&
    requires(T& problem, const X& x, Y&& y) {
  problem.evaluate(x, std::forward<Y>(y));
};

/// Concept for General Pareto Frontiers
template <typename T>
concept frontier = real<typename T::real> && requires(T& v,
                                                      const T& c,
                                                      size_t count,
                                                      size_t parameters,
                                                      size_t objectives,
                                                      size_t index) {
  { T(count, parameters, objectives) } -> identical<T>;

  { c.sample_count() } -> identical<size_t>;
  { c.parameter_count() } -> identical<size_t>;
  { c.objective_count() } -> identical<size_t>;

  { v.parameters(index) } -> output_range<typename T::real>;
  { v.objectives(index) } -> output_range<typename T::real>;
  { v.parameters_iterator(index) } -> output_iterator<typename T::real>;
  { v.objectives_iterator(index) } -> output_iterator<typename T::real>;

  { c.parameters(index) } -> input_range<typename T::real>;
  { c.objectives(index) } -> input_range<typename T::real>;
  { c.parameters_iterator(index) } -> input_iterator<typename T::real>;
  { c.objectives_iterator(index) } -> input_iterator<typename T::real>;
};

/// Defines that a given type provides a member function template
/// 'frontier_cast' and therefore is castable to the given frontier.
template <typename T, typename U>
concept frontier_castable = requires(T& t) {
  { t.template frontier_cast<U>() } -> identical<U>;
};

/// Idea of Concept of General Optimizers
template <typename T>
concept optimizer = problem<typename T::problem_type>;

}  // namespace generic

}  // namespace lyrahgames::pareto
