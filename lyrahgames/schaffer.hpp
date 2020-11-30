#pragma once
#include <array>
#include <concepts>

namespace lyrahgames {

template <std::floating_point Real>
constexpr auto schaffer(const std::array<Real, 1>& x) {
  return std::array<Real, 2>{x[0] * x[0], (x[0] - 2) * (x[0] - 2)};
}

}  // namespace lyrahgames