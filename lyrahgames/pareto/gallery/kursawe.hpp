#pragma once
#include <cassert>
#include <cmath>
#include <concepts>
#include <ranges>
//
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto::gallery {

template <std::floating_point T>
struct kursawe_problem {
  using real = T;

  static constexpr size_t parameter_count() { return 3; }
  static constexpr size_t objective_count() { return 2; }

  static constexpr auto box(size_t i) { return std::pair<real, real>{-5, 5}; }

  void evaluate(const generic::range<real> auto& x,
                generic::range<real> auto&& y) {
    using namespace std;
    assert(ranges::size(x) == parameter_count());
    assert(ranges::size(y) == objective_count());

    y[0] = -10 * (exp(real(-0.2) * sqrt(x[0] * x[0] + x[1] * x[1])) +
                  exp(real(-0.2) * sqrt(x[1] * x[1] + x[2] * x[2])));
    y[1] = pow(abs(x[0]), real(0.8)) + 5 * sin(x[0] * x[0] * x[0]) +
           pow(abs(x[1]), real(0.8)) + 5 * sin(x[1] * x[1] * x[1]) +
           pow(abs(x[2]), real(0.8)) + 5 * sin(x[2] * x[2] * x[2]);
  }
};

template <std::floating_point real>
kursawe_problem<real> kursawe{};

}  // namespace lyrahgames::pareto::gallery