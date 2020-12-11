#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <ranges>
#include <set>
#include <span>
#include <type_traits>
#include <vector>
//
#include <lyrahgames/gpp.hpp>
#include <lyrahgames/meta.hpp>

using namespace std;

template <floating_point real>
struct kursawe_problem {
  static constexpr size_t configurations = 3;
  static constexpr size_t objectives = 2;
  // static constexpr array<real, 2> box[] = {{-5, 5}, {-5, 5}, {-5, 5}};
  static constexpr auto box(size_t i) { return array<real, 2>{-5, 5}; }
  void operator()(const real* x, real* y) {
    using namespace std;
    y[0] = -10 * (exp(real(-0.2) * sqrt(x[0] * x[0] + x[1] * x[1])) +
                  exp(real(-0.2) * sqrt(x[1] * x[1] + x[2] * x[2])));
    y[1] = pow(abs(x[0]), real(0.8)) + 5 * sin(x[0] * x[0] * x[0]) +
           pow(abs(x[1]), real(0.8)) + 5 * sin(x[1] * x[1] * x[1]) +
           pow(abs(x[2]), real(0.8)) + 5 * sin(x[2] * x[2] * x[2]);
  }
};
template <floating_point real>
kursawe_problem<real> kursawe{};

template <floating_point real>
struct poloni2_problem {
  static constexpr size_t configurations = 2;
  static constexpr size_t objectives = 2;
  // static constexpr array<real, 2> box[] = {{-M_PI, M_PI}, {-M_PI, M_PI}};
  auto box(size_t i) { return pair<real, real>{-M_PI, M_PI}; }
  void operator()(const real* x, real* y) {
    using namespace std;
    const auto square = [](auto x) { return x * x; };
    constexpr real a1 = 0.5 * sin(1) - 2 * cos(1) + sin(2) - 1.5 * cos(2);
    constexpr real a2 = 1.5 * sin(1) - cos(1) + 2 * sin(2) - 0.5 * cos(2);
    const auto b1 = real{0.5} * sin(x[0]) - real{2.0} * cos(x[0]) + sin(x[1]) -
                    real{1.5} * cos(x[1]);
    const auto b2 = real{1.5} * sin(x[0]) - cos(x[0]) + real{2.0} * sin(x[1]) -
                    real{0.5} * cos(x[1]);
    y[0] = real{1} + square(a1 - b1) + square(a2 - b2);
    y[1] = square(x[0] + 3) + square(x[1] + 1);
  }
};
template <floating_point real>
poloni2_problem<real> poloni2{};

template <floating_point real>
struct zdt3_problem {
  static constexpr size_t configurations = 30;
  static constexpr size_t objectives = 2;
  auto box(size_t i) { return pair<real, real>{0, 1}; }
  void operator()(const real* x, real* y) {
    using namespace std;

    real g = 0;
    for (size_t i = 1; i < configurations; ++i) g += x[i];
    g = 1 + 9 * g / 29;

    real h = x[0] / g;
    h = 1 - sqrt(h) - h * sin(10 * M_PI * x[0]);

    y[0] = x[0];
    y[1] = h * g;
  }
};
template <floating_point real>
zdt3_problem<real> zdt3{};

template <typename vector>
constexpr auto domination(const vector& x, const vector& y) {
  assert(size(x) == size(y));
  for (size_t i = 0; i < size(x); ++i)
    if (x[i] > y[i]) return false;
  for (size_t i = 0; i < size(x); ++i)
    if (x[i] < y[i]) return true;
  return false;
}

template <typename pareto_problem, typename RNG>
inline auto pareto_optimization(pareto_problem problem, size_t n, RNG&& rng) {
  using namespace std;

  using real = remove_pointer_t<lyrahgames::meta::argument<pareto_problem, 1>>;
  using configurations_type = vector<real>;
  using objectives_type = vector<real>;

  uniform_real_distribution<real> distribution{0, 1};
  set<objectives_type> pareto_set{};

  configurations_type x(problem.configurations);
  objectives_type v(problem.objectives);

  // Use naive Monte-Carlo approach to estimate Pareto front.
  for (size_t i = 0; i < n; ++i) {
    // Get random argument vector in AABB.
    for (size_t k = 0; k < problem.configurations; ++k) {
      const auto random = distribution(rng);
      const auto [a, b] = problem.box(k);
      x[k] = random * a + (1 - random) * b;
    }

    // Check if it is a non-dominated point.
    problem(x.data(), v.data());

    bool to_add = true;
    for (auto it = begin(pareto_set); it != end(pareto_set);) {
      const auto& p = *it;
      if (domination(p, v)) {
        to_add = false;
        break;
      }
      if (domination(v, p))
        it = pareto_set.erase(it);
      else
        ++it;
    }
    if (to_add) pareto_set.insert(v);
  }
  // Copy results into a vector and return it.
  vector<objectives_type> result(pareto_set.size());
  size_t i = 0;
  for (auto& p : pareto_set) {
    result[i] = p;
    ++i;
  }
  return result;
}

int main() {
  using namespace std;
  using namespace lyrahgames;

  using real = float;

  mt19937 rng{random_device{}()};

  // auto pareto_front = pareto_optimization(kursawe<float>, 10'000'000, rng);
  // auto pareto_front = pareto_optimization(poloni2<float>, 100'000, rng);
  auto pareto_front = pareto_optimization(zdt3<float>, 1'000'000'000, rng);

  fstream pareto_file{"pareto.dat", ios::out};
  for (auto& p : pareto_front) pareto_file << p[0] << '\t' << p[1] << '\n';
  pareto_file << flush;

  gpp plot{};
  plot << "plot 'pareto.dat' u 1:2 w p lt rgb '#ff3333' pt 13\n";
}