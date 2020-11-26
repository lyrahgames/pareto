#pragma once
#include <array>
#include <concepts>
//
#include <set>

namespace lyrahgames {

template <std::totally_ordered Real>
constexpr auto domination(const std::array<Real, 2>& x,
                          const std::array<Real, 2>& y) {
  return (x[0] <= y[0]) && (x[1] <= y[1]) && ((x[0] < y[0]) || (x[1] < y[1]));
}

}  // namespace lyrahgames