#pragma once
#include <cassert>
#include <cmath>
#include <concepts>
#include <ranges>
#include <stdexcept>
//
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto::gallery {

template <std::floating_point T>
struct schaffer_1_problem {
  using real = T;

  static constexpr size_t parameter_count() { return 1; }
  static constexpr size_t objective_count() { return 2; }

  real box_min(size_t index) const { return -a; }
  real box_max(size_t index) const { return a; }

  schaffer_1_problem() = default;
  schaffer_1_problem(real aa) : a{aa} {
    if (a < 10)
      throw std::runtime_error(
          "Parameter of Schaffer 1 has to be bigger or equal than 10.");
  }

  void evaluate(const generic::range<real> auto& x,
                generic::range<real> auto&& y) {
    using namespace std;
    assert(ranges::size(x) == parameter_count());
    assert(ranges::size(y) == objective_count());

    const auto square = [](auto x) { return x * x; };
    y[0] = square(x[0]);
    y[1] = square(x[0] - 2);
  }

  real a;
};

template <std::floating_point real>
using schaffer1 = schaffer_1_problem<real>;

template <std::floating_point T>
struct schaffer_2_problem {
  using real = T;

  static constexpr size_t parameter_count() { return 1; }
  static constexpr size_t objective_count() { return 2; }

  static constexpr real box_min(size_t index) { return -5; }
  static constexpr real box_max(size_t index) { return 10; }

  void evaluate(const generic::range<real> auto& x,
                generic::range<real> auto&& y) {
    using namespace std;
    assert(ranges::size(x) == parameter_count());
    assert(ranges::size(y) == objective_count());

    const auto square = [](auto x) { return x * x; };
    y[0] = (x[0] <= 1)
               ? (-x[0])
               : ((x[0] <= 3) ? (x[0] - 2)
                              : ((x[0] <= 4) ? (4 - x[0]) : (x[0] - 4)));
    y[1] = square(x[0] - 5);
  }
};

template <std::floating_point real>
schaffer_2_problem<real> schaffer2;

}  // namespace lyrahgames::pareto::gallery
