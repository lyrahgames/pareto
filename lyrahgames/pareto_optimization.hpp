#pragma once
#include <array>
#include <cassert>
#include <iostream>
#include <random>
#include <set>
#include <vector>
//
#include <lyrahgames/meta.hpp>

namespace lyrahgames {

template <generic::vector T>
struct aabb {
  T min;
  T max;
};

template <std::totally_ordered Real>
constexpr auto domination(const std::array<Real, 2>& x,
                          const std::array<Real, 2>& y) {
  return (x[0] <= y[0]) && (x[1] <= y[1]) && ((x[0] < y[0]) || (x[1] < y[1]));
}

template <generic::vector T>
constexpr auto domination(const T& x, const T& y) {
  assert(size(x) == size(y));
  for (size_t i = 0; i < size(x); ++i)
    if (x[i] > y[i]) return false;
  for (size_t i = 0; i < size(x); ++i)
    if (x[i] < y[i]) return true;
  return false;
}

// pareto_front_estimation
template <generic::vector_field T, typename RNG>
inline auto monte_carlo_pareto_front(
    T f, const aabb<std::decay_t<meta::argument<T, 0>>>& box, size_t n,
    RNG&& rng) {
  using namespace std;
  using argument_type = std::decay_t<meta::argument<T, 0>>;
  using result_type = std::decay_t<meta::result<T>>;
  // using argument_value_type = meta::value<argument_type>;
  using argument_value_type = std::decay_t<decltype(box.min[0])>;
  //
  uniform_real_distribution<argument_value_type> distribution{0, 1};
  set<result_type> pareto_set{};
  auto x = box.min;  // constructor not allowed
  // Use naive Monte-Carlo approach to estimate Pareto front.
  for (size_t i = 0; i < n; ++i) {
    // Get random argument vector in AABB.
    for (size_t k = 0; k < size(x); ++k) {
      const auto random = distribution(rng);
      x[k] = random * box.max[k] + (1 - random) * box.min[k];
    }
    // Check if it is a non-dominated point.
    const auto v = f(x);
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
  vector<result_type> result(pareto_set.size());
  size_t i = 0;
  for (auto& p : pareto_set) {
    result[i] = p;
    ++i;
  }
  return result;
}

}  // namespace lyrahgames