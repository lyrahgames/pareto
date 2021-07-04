#pragma once
#include <cassert>
#include <cmath>
#include <concepts>
#include <numbers>
#include <ranges>
//
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto::gallery {

template <std::floating_point T>
struct poloni_problem {
  using real = T;

  static constexpr size_t parameter_count() { return 2; }
  static constexpr size_t objective_count() { return 2; }

  static constexpr real box_min(size_t index) {
    return -std::numbers::pi_v<real>;
  }
  static constexpr real box_max(size_t index) {
    return std::numbers::pi_v<real>;
  }

  void evaluate(const generic::range<real> auto& x,
                generic::range<real> auto&& y) {
    using namespace std;
    assert(ranges::size(x) == parameter_count());
    assert(ranges::size(y) == objective_count());

    const auto square = [](real x) { return x * x; };

    const auto f1 = [](real x, real y) {
      return real{0.5} * sin(x) - real{2} * cos(x) + sin(y) -
             real{1.5} * cos(y);
    };
    const auto f2 = [](real x, real y) {
      return real{1.5} * sin(x) - cos(x) + real{2} * sin(y) -
             real{0.5} * cos(y);
    };

    const auto a1 = f1(1, 2);
    const auto a2 = f2(1, 2);

    y[0] = 1 + square(a1 - f1(x[0], x[1])) + square(a2 - f2(x[0], x[1]));
    y[1] = square(x[0] + 3) + square(x[1] + 1);
  }
};

template <std::floating_point real>
poloni_problem<real> poloni{};

}  // namespace lyrahgames::pareto::gallery
