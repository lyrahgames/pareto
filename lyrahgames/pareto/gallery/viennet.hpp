#pragma once
#include <cassert>
#include <cmath>
#include <concepts>
#include <ranges>
//
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto::gallery {

template <std::floating_point T>
struct viennet_problem {
  using real = T;

  static constexpr size_t parameter_count() { return 2; }
  static constexpr size_t objective_count() { return 3; }

  static constexpr auto box(size_t i) { return std::pair<real, real>{-3, 3}; }

  void evaluate(const generic::range<real> auto& x,
                generic::range<real> auto&& y) {
    using namespace std;
    assert(ranges::size(x) == parameter_count());
    assert(ranges::size(y) == objective_count());

    const auto t = x[0] * x[0] + x[1] * x[1];
    y[0] = t / 2 + sin(t);
    const auto p = 3 * x[0] - 2 * x[1] + 4;
    const auto q = x[0] - x[1] + 1;
    y[1] = p * p / 8 + q * q / 27 + 15;
    y[2] = 1 / (1 + t) - real(1.1) * exp(-t);
  }
};

template <std::floating_point real>
viennet_problem<real> viennet{};

}  // namespace lyrahgames::pareto::gallery