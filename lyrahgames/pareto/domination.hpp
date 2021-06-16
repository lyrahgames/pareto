#pragma once
#include <cassert>
#include <concepts>
#include <iterator>
#include <ranges>

namespace lyrahgames::pareto {

/// Evaluates if the point given by the range 'x' Pareto-dominates the point
/// given by the range 'y'. Pareto domination does not define a total order.
template <std::ranges::input_range T, std::ranges::input_range U>
inline bool dominates(const T& x, const U& y) noexcept  //
    requires(std::totally_ordered_with<std::ranges::range_value_t<T>,
                                       std::ranges::range_value_t<U>>) {
  using namespace std;
  assert(ranges::size(x) == ranges::size(y));

  auto yit = ranges::begin(y);
  for (auto xit = ranges::begin(x); xit != ranges::end(x); ++xit, ++yit)
    if (*xit > *yit) return false;
  yit = ranges::begin(y);
  for (auto xit = ranges::begin(x); xit != ranges::end(x); ++xit, ++yit)
    if (*xit < *yit) return true;
  return false;

  // const auto n = std::ranges::size(x);
  // for (size_t i = 0; i < n; ++i)
  //   if (x[i] > y[i]) return false;
  // for (size_t i = 0; i < n; ++i)
  //   if (x[i] < y[i]) return true;
  // return false;
}

}  // namespace lyrahgames::pareto