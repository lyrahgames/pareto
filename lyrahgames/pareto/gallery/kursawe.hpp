#pragma once
#include <array>
#include <cmath>
#include <concepts>

namespace lyrahgames::pareto {

template <std::floating_point Real>
constexpr auto kursawe(const std::array<Real, 3>& x) {
  using namespace std;
  return array<Real, 2>{
      Real{-10} * (exp(Real{-0.2} * sqrt(x[0] * x[0] + x[1] * x[1])) +
                   exp(Real{-0.2} * sqrt(x[1] * x[1] + x[2] * x[2]))),
      pow(abs(x[0]), Real{0.8}) + 5 * sin(x[0] * x[0] * x[0]) +
          pow(abs(x[1]), Real{0.8}) + 5 * sin(x[1] * x[1] * x[1]) +
          pow(abs(x[2]), Real{0.8}) + 5 * sin(x[2] * x[2] * x[2])};
}

}  // namespace lyrahgames::pareto