#pragma once
#include <cassert>
#include <cmath>
#include <concepts>
#include <ranges>
//
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto::gallery {

template <std::floating_point T>
struct zitzler_deb_thiele_3_problem {
  using real = T;

  static constexpr size_t parameter_count() { return 30; }
  static constexpr size_t objective_count() { return 2; }

  static constexpr real box_min(size_t index) { return 0; }
  static constexpr real box_max(size_t index) { return 1; }

  void evaluate(const generic::range<real> auto& x,
                generic::range<real> auto&& y) {
    using namespace std;
    assert(ranges::size(x) == parameter_count());
    assert(ranges::size(y) == objective_count());

    real g = 0;
    for (size_t i = 1; i < parameter_count(); ++i)
      g += x[i];
    g = 1 + 9 * g / 29;

    real h = x[0] / g;
    h = 1 - sqrt(h) - h * sin(10 * M_PI * x[0]);

    y[0] = x[0];
    y[1] = h * g;
  }
};

template <std::floating_point real>
zitzler_deb_thiele_3_problem<real> zdt3{};

}  // namespace lyrahgames::pareto::gallery
