#pragma once
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <ranges>
#include <span>
#include <unordered_set>
#include <vector>
//
#include <lyrahgames/pareto/domination.hpp>
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto {

namespace nsga2 {

template <generic::problem T>
class optimizer {
 public:
  using problem_type = T;
  using real = typename problem_type::real;
  using size_type = size_t;

  optimizer(size_type samples) : s(samples), select(samples / 2) { init(); }

  void init() {
    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();
    parameters.resize(n * s);
    objectives.resize(m * s);
    permutation.resize(s);
    crowding_distances.resize(s);
    pareto_indices.reserve(s);
  }

  void init_population(generic::random_number_generator auto&& rng) {
    using namespace std;

    uniform_real_distribution<real> distribution{0, 1};
    const auto uniform = [&] { return distribution(rng); };

    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();

    // Iterate over all possible samples.
    for (size_type i = 0; i < s; ++i) {
      // Generate uniformly distributed parameter samples.
      for (size_type j = 0; j < n; ++j) {
        const auto rnd = uniform();
        const auto [a, b] = problem.box(j);
        parameters[n * i + j] = rnd * a + (1 - rnd) * b;
      }
      // Evaluate their respective objectives.
      auto dest = span{&objectives[m * i], &objectives[m * (i + 1)]};
      problem.evaluate(span{&parameters[n * i], &parameters[n * (i + 1)]},
                       dest);
    }
  }

  void non_dominated_sort() {
    using namespace std;

    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();

    // We use a permutation array to not have to copy all vectors.
    iota(permutation.begin(), permutation.end(), 0);

    // The elements of the fronts array mark the beginning of a new pareto front
    // beginning from the end with respect to the permutation.
    fronts.resize(1);
    fronts[0] = 0;

    // Sort front-wise until enough points are reached.
    while (fronts.back() < select) {
      pareto_indices.clear();
      pareto_indices.insert(permutation[0]);

      // Marks the number of currently dominated points.
      size_type front = 0;

      // Already known pareto points have already been put at the end.
      // So iterate only over dominated points from the last iteration.
      for (size_type i = 1; i < s - fronts.back(); ++i) {
        // Reference the objectives of the current point with respect to the
        // permutation.
        const auto index = permutation[i];
        auto p = span{&objectives[m * index], &objectives[m * (index + 1)]};

        // Check this point for domination against all other points currently
        // assumed to be Pareto points.
        bool non_dominated = true;
        auto it = begin(pareto_indices);
        for (; it != end(pareto_indices);) {
          // Reference the objectives of an assumed Pareto point.
          const auto j = *it;
          auto q = span{&objectives[m * j], &objectives[m * (j + 1)]};

          // Every dominated point will be put in the permutation front.
          // At the end, all pareto points will be put at the end of the current
          // permutation.

          // If the current point is dominated by the assumed Pareto point then
          // put it at the front of the permutation.
          if (domination(q, p)) {
            non_dominated = false;
            permutation[front] = index;
            ++front;
            break;
          }

          // If it dominates other assumed Pareto points then remove the assumed
          // Pareto point and put its index at the front of the permutation.
          if (domination(p, q)) {
            it = pareto_indices.erase(it);
            permutation[front] = j;
            ++front;
          } else {
            ++it;
          }
        }

        if (non_dominated) pareto_indices.insert(index);
      }

      // Order dominated points in reverse order. Heuristic to fasten up
      // sorting. Apart from performance, there is no logical reason to do this.
      // The algorithm would be able to work without it.
      for (size_type i = 0; i < front / 2; ++i)
        swap(permutation[i], permutation[front - 1 - i]);

      // Put all Pareto points after the dominated points.
      for (auto j : pareto_indices) {
        permutation[front] = j;
        ++front;
      }
      // Mark the current front.
      fronts.push_back(fronts.back() + pareto_indices.size());
    }
  }

  void crowding_distance_sort() {
    using namespace std;

    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();

    // If we exactly the amount of needed points then no crowding distance sort
    // is required.
    if (fronts.back() == select) return;

    // Compute offset and count for the last front to only sort the last front.
    const auto offset = s - fronts[fronts.size() - 1];
    const auto count = fronts[fronts.size() - 1] - fronts[fronts.size() - 2];

    // Set used crowding distances to zero to able to accumulate afterwards.
    // for (size_type i = offset; i < offset + count; ++i)
    for (size_type i = 0; i < s; ++i)
      crowding_distances[i] = 0;

    // Compute the crowding distance for every point
    // by iterating over all objectives.
    for (size_type v = 0; v < m; ++v) {
      // First, sort points based on their current objective value
      // to be able to easily compute neighboring distances.
      sort(&permutation[offset], &permutation[offset + count],
           [&](auto x, auto y) {
             return objectives[m * x + v] < objectives[m * y + v];
           });

      // Points at the boundary are always important and should never be
      // discarded. Set their distance to infinity.
      constexpr auto inf = numeric_limits<real>::infinity();
      crowding_distances[permutation[offset]] = inf;
      crowding_distances[permutation[offset + count - 1]] = inf;

      // Compute the scaling factor of the current objective for all points.
      const auto scale =
          1 / (objectives[m * permutation[offset + count - 1] + v] -
               objectives[m * permutation[offset] + v]);

      // Accumulate scaled distances to neighbors.
      for (size_type i = offset + 1; i < offset + count - 1; ++i)
        crowding_distances[permutation[i]] +=
            scale * (objectives[m * permutation[i + 1] + v] -
                     objectives[m * permutation[i - 1] + v]);
    }

    // Sort the respective points based on their computed crowding distance.
    sort(&permutation[offset], &permutation[offset + count],
         [&](auto x, auto y) {
           return crowding_distances[x] < crowding_distances[y];
         });
  }

  void simulated_binary_crossover(size_type parent1,
                                  size_type parent2,
                                  size_type offspring1,
                                  size_type offspring2,
                                  generic::random_number_generator auto&& rng) {
    using namespace std;

    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();

    constexpr real distribution_index = 2;
    constexpr auto distribution_index_coeff = 1 / (distribution_index + 1);

    uniform_real_distribution<real> distribution{0, 1};

    for (size_t i = 0; i < n; ++i) {
      const auto random = distribution(rng);
      const auto beta =
          (random <= real(0.5))
              ? (pow(2 * random, distribution_index_coeff))
              : (pow(1 / (2 * (1 - random)), distribution_index_coeff));

      const auto tmp1 = real(0.5) * ((1 + beta) * parameters[n * parent1 + i] +
                                     (1 - beta) * parameters[n * parent2 + i]);

      const auto tmp2 = real(0.5) * ((1 - beta) * parameters[n * parent1 + i] +
                                     (1 + beta) * parameters[n * parent2 + i]);

      const auto [a, b] = problem.box(i);
      parameters[n * offspring1 + i] = clamp(tmp1, a, b);
      parameters[n * offspring2 + i] = clamp(tmp2, a, b);
    }

    auto dest1 =
        span{&objectives[m * offspring1], &objectives[m * (offspring1 + 1)]};
    problem.evaluate(
        span{&parameters[n * offspring1], &parameters[n * (offspring1 + 1)]},
        dest1);

    auto dest2 =
        span{&objectives[m * offspring2], &objectives[m * (offspring2 + 1)]};
    problem.evaluate(
        span{&parameters[n * offspring2], &parameters[n * (offspring2 + 1)]},
        dest2);
  }

  void alternate_random_mutation(size_type parent,
                                 size_type offspring,
                                 generic::random_number_generator auto&& rng) {
    using namespace std;

    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();

    constexpr real stepsize = 0.1;

    // Construct oracle for random numbers.
    uniform_real_distribution<real> distribution{-1, 1};
    const auto uniform = [&] { return distribution(rng); };

    // For all parameters of the parent, draw new parameters.
    for (size_t k = 0; k < n; ++k) {
      const auto random = uniform();
      const auto [a, b] = problem.box(k);
      const auto value =
          parameters[n * parent + k] + random * stepsize * (b - a);
      // Make sure the new parameters fulfill the box constraints.
      parameters[n * offspring + k] = clamp(value, a, b);
    }

    // Evaluate the objectives of the new point.
    auto dest =
        span{&objectives[m * offspring], &objectives[m * (offspring + 1)]};
    problem.evaluate(
        span{&parameters[n * offspring], &parameters[n * (offspring + 1)]},
        dest);
  }

  void populate(generic::random_number_generator auto&& rng) {
    using namespace std;

    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();

    constexpr real probability_crossover = 0.3;
    const size_type count = s - select;
    const size_type crossovers =
        2 * size_type(probability_crossover * (count / 2));

    uniform_int_distribution<size_type> distribution{0, select - 1};

    size_type i = 0;

    for (; i < crossovers; i += 2) {
      const auto parent1 = permutation[s - 1 - distribution(rng)];
      const auto parent2 = permutation[s - 1 - distribution(rng)];
      const auto offspring1 = permutation[s - 1 - select - i];
      const auto offspring2 = permutation[s - 1 - select - i - 1];
      simulated_binary_crossover(parent1, parent2, offspring1, offspring2, rng);
    }

    for (; i < count; ++i) {
      const auto parent = permutation[s - 1 - distribution(rng)];
      const auto offspring = permutation[s - 1 - select - i];
      alternate_random_mutation(parent, offspring, rng);
    }
  }

  void optimize(generic::random_number_generator auto&& rng) {
    init_population(rng);
    for (size_type it = 0; it < max_it; ++it) {
      non_dominated_sort();
      crowding_distance_sort();
      populate(rng);
    }
    non_dominated_sort();
    crowding_distance_sort();
  }

  // private:
  problem_type problem{};
  size_type s;
  size_type max_it = 1000;
  size_type select;
  std::vector<real> parameters;
  std::vector<real> objectives;
  std::vector<size_type> permutation;
  std::vector<real> crowding_distances;
  std::vector<size_type> fronts;
  std::unordered_set<size_type> pareto_indices{};
};

}  // namespace nsga2

}  // namespace lyrahgames::pareto