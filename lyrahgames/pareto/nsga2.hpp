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
#include <lyrahgames/pareto/frontier_cast.hpp>
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto {

namespace nsga2 {

/// Specialized Pareto Problems for the NSGA2 Algorithm
template <typename T>
concept problem = generic::evaluatable_problem<T,
                                               std::span<typename T::real>,
                                               std::span<typename T::real>>;

/// NSGA2 Optimization Algorithm with Custom Implementation of the Non-Dominated
/// Sorting Algorithm, Simulated Binary Crossovers, and Alternate Random
/// Mutations
template <problem T>
class optimizer {
 public:
  using problem_type = T;
  using real = typename problem_type::real;

  /// Structure to provide easy intialization of the parameters of the
  /// algorithm. By using designated initializers, named function arguments can
  /// be simulated.
  struct configuration {
    size_t iterations = 1000;
    size_t population = 1000;
    float kill_ratio = 0.5;
    float crossover_ratio = 0.3;
  };

  optimizer() = default;
  explicit optimizer(problem_type p,
                     generic::random_number_generator auto&& rng,
                     configuration config = {})
      : problem(p),
        s(config.population),
        select(std::floor((1 - config.kill_ratio) * config.population)),
        iter(config.iterations),
        crossover_probability(config.crossover_ratio) {
    init();
    init_population(std::forward<decltype(rng)>(rng));
  }

  void init() {
    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();
    parameters.resize(n * s);
    objectives.resize(m * s);
    permutation.resize(s);
    crowding_distances.resize(s);
    pareto_indices.reserve(s);
  }

  /// Clamp the parameters referenced by the given index to the box constraints
  /// defined by the current problem.
  void clamp(size_t index) {
    using std::clamp;
    const auto n = problem.parameter_count();
    for (size_t i = 0; i < n; ++i) {
      // const auto [a, b] = problem.box(i);
      const auto x = parameters[n * index + i];
      // parameters[n * index + i] = clamp(x, a, b);
      parameters[n * index + i] =
          clamp(x, problem.box_min(i), problem.box_max(i));
    }
  }

  /// Evaluate all objectives at the given index by using the parameters
  /// referenced by the given index.
  void evaluate(size_t index) {
    using std::span;
    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();
    problem.evaluate(
        span{&parameters[n * index], &parameters[n * (index + 1)]},
        span{&objectives[m * index], &objectives[m * (index + 1)]});
  }

  /// Generates a random population to start with the optimization algorithm.
  void init_population(generic::random_number_generator auto&& rng) {
    using namespace std;

    // Introduce short-hand notations.
    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();

    // Generate oracle for random numbers.
    uniform_real_distribution<real> distribution{0, 1};
    const auto random = [&] { return distribution(rng); };

    // Iterate over all possible samples.
    for (size_t i = 0; i < s; ++i) {
      // Generate uniformly distributed parameter samples.
      for (size_t j = 0; j < n; ++j)
        parameters[n * i + j] =
            lerp(problem.box_min(j), problem.box_max(j), random());
      evaluate(i);
    }

    // Pre-sort the randomly generated population.
    non_dominated_sort();
    crowding_distance_sort();
  }

  /// Sort the current population into their layers of domination.
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
      size_t front = 0;

      // Already known pareto points have already been put at the end.
      // So iterate only over dominated points from the last iteration.
      for (size_t i = 1; i < s - fronts.back(); ++i) {
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
          if (dominates(q, p)) {
            non_dominated = false;
            permutation[front] = index;
            ++front;
            break;
          }

          // If it dominates other assumed Pareto points then remove the assumed
          // Pareto point and put its index at the front of the permutation.
          if (dominates(p, q)) {
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
      for (size_t i = 0; i < front / 2; ++i)
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

  /// Sort a specific domination layer of the current population with respect to
  /// their crowding distance by computing it first.
  void crowding_distance_sort() {
    using namespace std;

    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();

    // If we exactly the amount of needed points then no crowding distance sort
    // is required.
    if (fronts.back() == select) return;

    // Compute range for the last front to only sort the last front.
    const auto first = s - fronts[fronts.size() - 1];
    const auto last = s - fronts[fronts.size() - 2];

    // Set used crowding distances to zero to able to accumulate afterwards.
    // for (size_t i = offset; i < offset + count; ++i)
    for (size_t i = first; i < last; ++i)
      crowding_distances[i] = 0;

    // Compute the crowding distance for every point
    // by iterating over all objectives.
    for (size_t v = 0; v < m; ++v) {
      // First, sort points based on their current objective value
      // to be able to easily compute neighboring distances.
      sort(&permutation[first], &permutation[last], [&](auto x, auto y) {
        return objectives[m * x + v] < objectives[m * y + v];
      });

      // Points at the boundary are always important and should never be
      // discarded. Set their distance to infinity.
      constexpr auto inf = numeric_limits<real>::infinity();
      crowding_distances[permutation[first]] = inf;
      crowding_distances[permutation[last - 1]] = inf;

      // Compute the scaling factor of the current objective for all points.
      const auto scale = 1 / (objectives[m * permutation[last - 1] + v] -
                              objectives[m * permutation[first] + v]);

      // Accumulate scaled distances to neighbors.
      for (size_t i = first + 1; i < last - 1; ++i)
        crowding_distances[permutation[i]] +=
            scale * (objectives[m * permutation[i + 1] + v] -
                     objectives[m * permutation[i - 1] + v]);
    }

    // Sort the respective points based on their computed crowding distance.
    sort(&permutation[first], &permutation[last], [&](auto x, auto y) {
      return crowding_distances[x] < crowding_distances[y];
    });
  }

  /// Crossover Scheme
  void simulated_binary_crossover(size_t parent1,
                                  size_t parent2,
                                  size_t offspring1,
                                  size_t offspring2,
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

      parameters[n * offspring1 + i] = tmp1;
      parameters[n * offspring2 + i] = tmp2;
    }
  }

  /// Mutation Scheme
  void alternate_random_mutation(size_t parent,
                                 size_t offspring,
                                 generic::random_number_generator auto&& rng) {
    using namespace std;

    // Introduce short-hand notations.
    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();

    constexpr real stepsize = 0.1;

    // Construct oracle for random numbers.
    uniform_real_distribution<real> distribution{-1, 1};
    const auto uniform = [&] { return distribution(rng); };

    // For all parameters of the parent, draw new parameters.
    for (size_t k = 0; k < n; ++k) {
      const auto random = uniform();
      const auto a = problem.box_min(k);
      const auto b = problem.box_max(k);
      const auto value =
          parameters[n * parent + k] + random * stepsize * (b - a);
      parameters[n * offspring + k] = value;
    }
  }

  /// Discards the bad part of the population and fills it up again by using
  /// crossovers and mutations.
  void populate(generic::random_number_generator auto&& rng) {
    using namespace std;

    // Introduce short-hand notations.
    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();

    // Add oracle for random index.
    // All good points are stored at the end of the permutation.
    uniform_int_distribution<size_t> distribution{s - select, s - 1};
    const auto random = [&] { return distribution(rng); };

    // Compute count of crossovers.
    const size_t count = s - select;
    const size_t crossover_count =
        2 * size_t(crossover_probability * (count / 2));

    size_t i = 0;

    // Crossover
    for (; i < crossover_count; i += 2) {
      const auto parent1 = permutation[random()];
      const auto parent2 = permutation[random()];
      const auto offspring1 = permutation[i + 0];
      const auto offspring2 = permutation[i + 1];

      simulated_binary_crossover(parent1, parent2, offspring1, offspring2, rng);
      // Make sure newly generated parameters fulfill the box constraints.
      clamp(offspring1);
      clamp(offspring2);
      evaluate(offspring1);
      evaluate(offspring2);
    }

    // Mutation
    for (; i < count; ++i) {
      const auto parent = permutation[random()];
      const auto offspring = permutation[i];

      alternate_random_mutation(parent, offspring, rng);
      // Make sure newly generated parameters fulfill the box constraints.
      clamp(offspring);
      evaluate(offspring);
    }
  }

  /// This function can be applied multiple times to further improve the
  /// estimation of the Pareto frontier.
  void optimize(generic::random_number_generator auto&& rng,
                size_t iterations) {
    for (size_t i = 0; i < iterations; ++i) {
      populate(rng);
      non_dominated_sort();
      crowding_distance_sort();
    }
  }

  /// Uses the iterations count given by construction.
  void optimize(generic::random_number_generator auto&& rng) {
    optimize(std::forward<decltype(rng)>(rng), iter);
  }

  /// Casts the estimated Pareto points stored as an implementation detail into
  /// a usable frontier data structure.
  template <generic::frontier frontier_type>
  auto frontier_cast() const {
    using namespace std;
    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();
    frontier_type frontier{fronts[1], n, m};
    for (size_t i = 0; i < fronts[1]; ++i) {
      const auto index = permutation[s - 1 - i];
      {
        auto it = frontier.parameters_iterator(i);
        for (size_t j = 0; j < n; ++j, ++it)
          *it = parameters[n * index + j];
      }
      {
        auto it = frontier.objectives_iterator(i);
        for (size_t j = 0; j < m; ++j, ++it)
          *it = objectives[m * index + j];
      }
    }
    return frontier;
  }

 private:
  problem_type problem{};

  std::vector<real> parameters{};
  std::vector<real> objectives{};
  std::vector<size_t> permutation{};
  std::vector<real> crowding_distances{};
  std::vector<size_t> fronts{};
  std::unordered_set<size_t> pareto_indices{};

  /// Population Size
  size_t s;
  /// Number of samples kept alive after one iteration.
  size_t select;
  /// Number of iterations given by initialization.
  size_t iter;
  /// Crossover/Mutation Ratio per Iteration
  float crossover_probability;
};

template <problem problem_type>
optimizer(problem_type, size_t) -> optimizer<problem_type>;

/// Short-hand function to set the parameters and optimize in one step. This
/// function returns an instance to the NSGA2 optimizer.
auto optimization(
    problem auto problem,
    generic::random_number_generator auto&& rng,
    typename optimizer<decltype(problem)>::configuration config = {}) {
  optimizer result(problem, rng, config);
  result.optimize(std::forward<decltype(rng)>(rng));
  return result;
}

/// Short-hand function overload to additionally make a frontier cast after
/// optimization and discard the optimizer instance in one step.
template <generic::frontier frontier_type>
auto optimization(
    problem auto problem,
    generic::random_number_generator auto&& rng,
    typename optimizer<decltype(problem)>::configuration config = {}) {
  return frontier_cast<frontier_type>(
      optimization(problem, std::forward<decltype(rng)>(rng), config));
}

}  // namespace nsga2

}  // namespace lyrahgames::pareto
