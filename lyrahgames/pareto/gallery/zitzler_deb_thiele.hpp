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
struct zitzler_deb_thiele_1_problem {
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

    real h = 1 - sqrt(x[0] / g);

    y[0] = x[0];
    y[1] = h * g;
  }
};

template <std::floating_point real>
zitzler_deb_thiele_1_problem<real> zdt1{};

template <std::floating_point T>
struct zitzler_deb_thiele_2_problem {
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

    const auto square = [](real x) { return x * x; };
    real h = 1 - square(x[0] / g);

    y[0] = x[0];
    y[1] = h * g;
  }
};

template <std::floating_point real>
zitzler_deb_thiele_2_problem<real> zdt2{};

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

template <std::floating_point T>
struct zitzler_deb_thiele_4_problem {
  using real = T;

  static constexpr size_t parameter_count() { return 10; }
  static constexpr size_t objective_count() { return 2; }

  static constexpr real box_min(size_t index) {
    return (index == 0) ? (0) : (-5);
  }
  static constexpr real box_max(size_t index) {
    return (index == 0) ? (1) : (5);
  }

  void evaluate(const generic::range<real> auto& x,
                generic::range<real> auto&& y) {
    using namespace std;
    assert(ranges::size(x) == parameter_count());
    assert(ranges::size(y) == objective_count());

    constexpr auto pi = std::numbers::pi_v<real>;

    real g = 0;
    for (size_t i = 1; i < parameter_count(); ++i)
      g += x[i] * x[i] - 10 * cos(4 * pi * x[i]);
    g += 91;

    real h = x[0] / g;
    h = 1 - sqrt(h);

    y[0] = x[0];
    y[1] = h * g;
  }
};

template <std::floating_point real>
zitzler_deb_thiele_4_problem<real> zdt4{};

template <std::floating_point T>
struct zitzler_deb_thiele_6_problem {
  using real = T;

  static constexpr size_t parameter_count() { return 10; }
  static constexpr size_t objective_count() { return 2; }

  static constexpr real box_min(size_t index) { return 0; }
  static constexpr real box_max(size_t index) { return 1; }

  void evaluate(const generic::range<real> auto& x,
                generic::range<real> auto&& y) {
    using namespace std;
    assert(ranges::size(x) == parameter_count());
    assert(ranges::size(y) == objective_count());

    constexpr auto pi = std::numbers::pi_v<real>;
    const auto square = [](real x) { return x * x; };

    real f = 1 - exp(-4 * x[0]) * pow(sin(6 * pi * x[0]), 6);

    real g = 0;
    for (size_t i = 1; i < parameter_count(); ++i)
      g += x[i];
    g = 1 + 9 * pow(g / 9, real{0.25});

    real h = 1 - square(f / g);

    y[0] = f;
    y[1] = h * g;
  }
};

template <std::floating_point real>
zitzler_deb_thiele_6_problem<real> zdt6{};

}  // namespace lyrahgames::pareto::gallery
