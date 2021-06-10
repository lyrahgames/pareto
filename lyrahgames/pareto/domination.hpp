#pragma once
#include <cassert>
//
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto {

template <std::ranges::random_access_range T,
          std::ranges::random_access_range U>
inline auto domination(const T& x, const U& y)  //
    requires(std::same_as<std::ranges::range_value_t<T>,
                          std::ranges::range_value_t<U>>&&
                 std::totally_ordered<std::ranges::range_value_t<T>>) {
  assert(std::ranges::size(x) == std::ranges::size(y));
  const auto n = std::ranges::size(x);
  for (size_t i = 0; i < n; ++i)
    if (x[i] > y[i]) return false;
  for (size_t i = 0; i < n; ++i)
    if (x[i] < y[i]) return true;
  return false;
}

}  // namespace lyrahgames::pareto