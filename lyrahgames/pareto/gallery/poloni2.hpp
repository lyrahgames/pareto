#pragma once
#include <cassert>
#include <cmath>
#include <concepts>
//
#include <lyrahgames/meta.hpp>

namespace lyrahgames::pareto {

template <typename Y, typename X>
requires generic::compatible_vectors<X, Y>  //
    constexpr auto poloni2(const X& x) {
  using namespace std;
  assert(size(x) == 2);
  const auto square = [](auto x) { return x * x; };
  using real = meta::value<X>;
  constexpr real a1 = 0.5 * sin(1) - 2 * cos(1) + sin(2) - 1.5 * cos(2);
  constexpr real a2 = 1.5 * sin(1) - cos(1) + 2 * sin(2) - 0.5 * cos(2);
  const auto b1 = real{0.5} * sin(x[0]) - real{2.0} * cos(x[0]) + sin(x[1]) -
                  real{1.5} * cos(x[1]);
  const auto b2 = real{1.5} * sin(x[0]) - cos(x[0]) + real{2.0} * sin(x[1]) -
                  real{0.5} * cos(x[1]);
  return Y{real{1} + square(a1 - b1) + square(a2 - b2),
           square(x[0] + 3) + square(x[1] + 1)};
}

}  // namespace lyrahgames::pareto