#pragma once
#include <algorithm>
#include <cmath>
#include <concepts>
#include <map>
#include <random>
#include <ranges>
#include <span>
//
#include <lyrahgames/pareto/domination.hpp>
#include <lyrahgames/pareto/frontier_cast.hpp>
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto {

namespace naive {

/// Specialized Pareto Problems for the Naive Algorithm
template <typename T>
concept problem = generic::evaluatable_problem<T,
                                               std::vector<typename T::real>,
                                               std::vector<typename T::real>>;

/// Naive Monte-Carlo-based Pareto Optimization Algorithm
/// Generates uniformly distributed random samples inside the box constraints of
/// the given problem and keeps all non-dominated points inside a map data
/// structure.
template <problem T>
class optimizer {
 public:
  using problem_type = T;
  using real = typename problem_type::real;
  using parameter_vector = std::vector<real>;
  using objective_vector = std::vector<real>;
  using container = std::map<objective_vector, parameter_vector>;

  optimizer() = default;

  explicit optimizer(problem_type p) : problem(p) {}

  /// Estimate the Pareto frontier of the given problem. This function can be
  /// called multiple times to further improve the estimate.
  void optimize(generic::random_number_generator auto&& rng,
                size_t iterations = 1000) {
    using namespace std;

    // Generate oracle for random numbers.
    uniform_real_distribution<real> distribution{0, 1};
    const auto random = [&] { return distribution(rng); };

    // Introduce short-hand notations.
    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();

    // Vectors for evaluating and storing temporary problem configurations.
    parameter_vector x(n);
    objective_vector y(m);

    // Use number of Monte-Carlo iterations to estimate the Pareto front.
    for (size_t s = 0; s < iterations; ++s) {
      // Get random parameter vector inside box constaints.
      for (size_t k = 0; k < n; ++k)
        x[k] = lerp(problem.box_min(k), problem.box_max(k), random());

      // Evaluate its objective values.
      problem.evaluate(x, y);

      // Check if it is a non-dominated point.
      bool non_dominated = true;
      for (auto it = begin(pareto_optima); it != end(pareto_optima);) {
        const auto& [p, _] = *it;
        if (dominates(p, y)) {
          non_dominated = false;
          break;
        }
        // Delete points that are dominated.
        if (dominates(y, p))
          it = pareto_optima.erase(it);
        else
          ++it;
      }

      // Insert non-dominated points into the map.
      if (non_dominated) pareto_optima.emplace(y, x);
    }
  }

  /// Casts the estimated Pareto points stored as an implementation detail into
  /// a usable frontier data structure.
  template <generic::frontier frontier_type>
  auto frontier_cast() const {
    using namespace std;
    frontier_type frontier{pareto_optima.size(), problem.parameter_count(),
                           problem.objective_count()};
    for (size_t i = 0; auto& [y, x] : pareto_optima) {
      ranges::copy(x, frontier.parameters_iterator(i));
      ranges::copy(y, frontier.objectives_iterator(i));
      ++i;
    }
    return frontier;
  }

 private:
  problem_type problem{};
  container pareto_optima{};
};

template <problem problem_type>
optimizer(problem_type) -> optimizer<problem_type>;

/// Short-hand function to set the parameters and optimize in one step. This
/// function returns an instance to the naive optimizer.
auto optimization(problem auto problem,
                  generic::random_number_generator auto&& rng,
                  size_t iterations) {
  optimizer result(problem);
  result.optimize(std::forward<decltype(rng)>(rng), iterations);
  return result;
}

/// Short-hand function overload to additionally make a frontier cast after
/// optimization and discard the optimizer instance in one step.
template <generic::frontier frontier_type>
auto optimization(problem auto problem,
                  generic::random_number_generator auto&& rng,
                  size_t iterations) {
  return frontier_cast<frontier_type>(
      optimization(problem, std::forward<decltype(rng)>(rng), iterations));
}

}  // namespace naive

}  // namespace lyrahgames::pareto
