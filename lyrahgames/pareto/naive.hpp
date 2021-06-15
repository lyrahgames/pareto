#pragma once
#include <algorithm>
#include <cmath>
#include <concepts>
#include <map>
#include <ranges>
#include <span>
//
#include <lyrahgames/pareto/domination.hpp>
#include <lyrahgames/pareto/frontier_cast.hpp>
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto {

namespace naive {

template <typename T>
concept problem = generic::evaluatable_problem<T,
                                               std::vector<typename T::real>,
                                               std::vector<typename T::real>>;

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

  void optimize(size_t samples, generic::random_number_generator auto&& rng) {
    using namespace std;

    // Generate oracle for random numbers.
    uniform_real_distribution<real> distribution{0, 1};
    const auto random = [&] { return distribution(rng); };

    const auto n = problem.parameter_count();
    const auto m = problem.objective_count();

    parameter_vector x(n);
    objective_vector y(m);

    // Use naive Monte-Carlo approach to estimate Pareto front.
    for (size_t s = 0; s < samples; ++s) {
      // Get random parameter vector inside box constaints.
      for (size_t k = 0; k < n; ++k) {
        // const auto random = uniform();
        // const auto [a, b] = problem.box(k);
        // x[k] = random * a + (1 - random) * b;
        x[k] = lerp(problem.box_min(k), problem.box_max(k), random());
      }

      // Evaluate its objective values.
      problem.evaluate(x, y);

      // Check if it is a non-dominated point.
      bool non_dominated = true;
      for (auto it = begin(pareto_optima); it != end(pareto_optima);) {
        const auto& [p, _] = *it;
        if (domination(p, y)) {
          non_dominated = false;
          break;
        }
        if (domination(y, p))
          it = pareto_optima.erase(it);
        else
          ++it;
      }
      if (non_dominated) pareto_optima.emplace(y, x);
    }
  }

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

auto optimization(problem auto problem,
                  size_t samples,
                  generic::random_number_generator auto&& rng) {
  optimizer result(problem);
  result.optimize(samples, std::forward<decltype(rng)>(rng));
  return result;
}

template <generic::frontier frontier_type>
auto optimization(problem auto problem,
                  size_t samples,
                  generic::random_number_generator auto&& rng) {
  return frontier_cast<frontier_type>(
      optimization(problem, samples, std::forward<decltype(rng)>(rng)));
}

}  // namespace naive

}  // namespace lyrahgames::pareto