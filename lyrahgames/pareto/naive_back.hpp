#pragma once
#include <random>
#include <set>
#include <vector>
//
#include <lyrahgames/meta.hpp>
#include <lyrahgames/pareto/aabb.hpp>
#include <lyrahgames/pareto/domination.hpp>

namespace lyrahgames::pareto {

// pareto_front_estimation
template <generic::vector_field T, typename RNG>
inline auto monte_carlo_pareto_front(T f,
                                     const aabb<meta::argument<T, 0>>& box,
                                     size_t n,
                                     RNG&& rng) {
  using namespace std;

  using result_type = meta::result<T>;
  using argument_type = meta::argument<T, 0>;
  using argument_value_type = meta::value<argument_type>;
  // meta::print_type<result_type>();
  // meta::print_type<argument_type>();
  // meta::print_type<argument_value_type>();
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

// pareto_front_estimation
template <generic::vector_field T, generic::vector_field U, typename RNG>
inline auto monte_carlo_constrained_pareto_front(
    T f,
    const aabb<meta::argument<T, 0>>& box,
    U constraints,
    size_t n,
    RNG&& rng) {
  using namespace std;

  using result_type = meta::result<T>;
  using argument_type = meta::argument<T, 0>;
  using argument_value_type = meta::value<argument_type>;
  // meta::print_type<result_type>();
  // meta::print_type<argument_type>();
  // meta::print_type<argument_value_type>();
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

    // Check if configuration is feasible.
    const auto c = constraints(x);
    bool feasible = true;
    for (size_t k = 0; k < size(c); ++k) {
      if (c[k] < 0) {
        feasible = false;
        break;
      }
    }
    if (!feasible) continue;

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

}  // namespace lyrahgames::pareto
