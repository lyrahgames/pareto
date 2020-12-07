#pragma once
#include <array>
#include <cmath>
#include <concepts>

namespace lyrahgames::pareto {

template <std::floating_point Real>
constexpr auto viennet(const std::array<Real, 2>& x) {
  using namespace std;
  const auto tmp = x[0] * x[0] + x[1] * x[1];
  const auto square = [](auto x) { return x * x; };
  return array<Real, 3>{
      Real{0.5} * tmp + sin(tmp),
      square(Real{3} * x[0] - Real{2} * x[1] + Real{4}) / Real{8} +
          square(x[0] - x[1] + Real{1}) / Real{27} + 15,
      Real{1} / (tmp + Real{1}) - Real{1.1} * exp(-tmp)};
}

}  // namespace lyrahgames::pareto