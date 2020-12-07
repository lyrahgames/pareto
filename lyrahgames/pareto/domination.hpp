#pragma once
#include <cassert>
//
#include <lyrahgames/meta.hpp>

namespace lyrahgames::pareto {

// template <std::totally_ordered Real>
// constexpr auto domination(const std::array<Real, 2>& x,
//                           const std::array<Real, 2>& y) {
//   return (x[0] <= y[0]) && (x[1] <= y[1]) && ((x[0] < y[0]) || (x[1] <
//   y[1]));
// }

template <generic::vector vector>
constexpr auto domination(const vector& x, const vector& y) {
  assert(size(x) == size(y));
  for (size_t i = 0; i < size(x); ++i)
    if (x[i] > y[i]) return false;
  for (size_t i = 0; i < size(x); ++i)
    if (x[i] < y[i]) return true;
  return false;
}

}  // namespace lyrahgames::pareto