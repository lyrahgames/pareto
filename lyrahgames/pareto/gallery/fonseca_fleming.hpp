#pragma once
#include <cassert>
#include <cmath>
#include <concepts>
#include <ranges>
//
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto::gallery {

template <std::floating_point T>
struct fonseca_fleming_problem {
  using real = T;

  size_t parameter_count() const { return n; }
  static constexpr size_t objective_count() { return 2; }

  static constexpr real box_min(size_t index) { return -4; }
  static constexpr real box_max(size_t index) { return 4; }

  void evaluate(const generic::range<real> auto& x,
                generic::range<real> auto&& y) {
    using namespace std;
    assert(ranges::size(x) == parameter_count());
    assert(ranges::size(y) == objective_count());

    real p = 0;
    real q = 0;
    const auto t = 1 / sqrt(real(n));
    const auto square = [](auto x) { return x * x; };
    for (size_t i = 0; i < n; ++i) {
      p += square(x[i] - t);
      q += square(x[i] + t);
    }
    y[0] = 1 - exp(-p);
    y[1] = 1 - exp(-q);
  }

  size_t n{3};
};

template <std::floating_point real>
using fonseca_fleming = fonseca_fleming_problem<real>;

}  // namespace lyrahgames::pareto::gallery
