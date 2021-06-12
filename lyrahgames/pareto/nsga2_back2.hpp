#pragma once
#include <array>
#include <concepts>
#include <random>
#include <tuple>
#include <vector>
//
#include <lyrahgames/meta.hpp>
#include <lyrahgames/pareto/aabb.hpp>
#include <lyrahgames/pareto/domination.hpp>

namespace lyrahgames::pareto {

namespace nsga2 {
// currently only for static arrays
template <generic::vector_field field, typename RNG>
auto optimization(field f,
                  const aabb<meta::argument<field, 0>>& box,
                  RNG&& rng,
                  size_t iterations) {
  using namespace std;

  using result_type = meta::result<field>;
  using argument_type = meta::argument<field, 0>;
  using real = meta::value<argument_type>;

  size_t m = 100 * 4;
  size_t n = 2 * m;
  vector<argument_type> inputs(n);
  vector<result_type> values(n);
  vector<vector<size_t>> fronts{};
  vector<real> crowding_distances(n);
  // vector<argument_type> tmp_inputs(n);
  // vector<result_type> tmp_values(n);

  uniform_real_distribution<real> distribution{0, 1};

  // Create initial population.
  auto x = box.min;  // constructor not allowed
  // Get random argument vector in AABB.
  for (size_t i = 0; i < m; ++i) {
    for (size_t k = 0; k < size(x); ++k) {
      const auto random = distribution(rng);
      x[k] = random * box.max[k] + (1 - random) * box.min[k];
    }
    inputs[i] = x;
    values[i] = f(x);
  }

  const auto make_new_population = [&]() {
    // crossover
    size_t i = 0;
    std::uniform_int_distribution<size_t> population_dist{0, m - 1};
    for (; i < m / 2; i += 2) {
      auto x = inputs[population_dist(rng)];
      auto y = inputs[population_dist(rng)];
      const auto crossings =
          std::uniform_int_distribution<size_t>{1, size(x) - 1}(rng);
      for (size_t j = 0; j < crossings; ++j) {
        const auto index =
            std::uniform_int_distribution<size_t>{0, size(x) - 1}(rng);
        std::swap(x[index], y[index]);
        x[index] = clamp(x[index], box.min[index], box.max[index]);
        y[index] = clamp(y[index], box.min[index], box.max[index]);
      }
      inputs[m + i + 0] = x;
      inputs[m + i + 1] = y;
      values[m + i + 0] = f(x);
      values[m + i + 1] = f(y);
    }
    // mutation
    for (; i < m; ++i) {
      auto x = inputs[population_dist(rng)];
      constexpr real stepsize = 0.1;
      for (size_t k = 0; k < x.size(); ++k) {
        const auto step = std::uniform_real_distribution<real>{-1, 1}(rng);
        x[k] += step * stepsize * (box.max[k] - box.min[k]);
        x[k] = clamp(x[k], box.min[k], box.max[k]);
      }
      inputs[m + i] = x;
      values[m + i] = f(x);
    }
  };

  const auto non_dominated_sort = [&]() {
    fronts.resize(1);
    fronts.front().resize(0);

    vector<size_t> ranks(n, 0);
    vector<size_t> dominations(n, 0);
    vector<vector<size_t>> dominated_sets(n);

    for (size_t i = 0; i < n; ++i) {
      const auto& p = values[i];
      for (size_t j = 0; j < n; ++j) {
        const auto& q = values[j];
        if (domination(p, q))
          dominated_sets[i].push_back(j);
        else if (domination(q, p))
          ++dominations[i];
      }
      if (dominations[i] == 0) {
        ranks[i] = 1;
        fronts[0].push_back(i);
      }
    }
    size_t i = 0;
    while (!fronts[i].empty()) {
      vector<size_t> buffer{};
      for (auto pid : fronts[i]) {
        for (auto qid : dominated_sets[pid]) {
          --dominations[qid];
          if (dominations[qid] == 0) {
            ranks[qid] = i + 2;
            buffer.push_back(qid);
          }
        }
      }
      ++i;
      fronts.push_back({});
      fronts[i] = buffer;
    }
  };

  const auto assign_crowding_distances = [&]() {
    for (size_t i = 0; i < n; ++i)
      crowding_distances[i] = 0;
    for (auto& front : fronts) {
      if (front.empty()) break;
      for (size_t v = 0; v < size(values[0]); ++v) {
        sort(begin(front), end(front),
             [&](auto x, auto y) { return values[x][v] < values[y][v]; });
        crowding_distances[front.front()] = INFINITY;
        crowding_distances[front.back()] = INFINITY;
        for (size_t i = 1; i < front.size() - 1; ++i)
          crowding_distances[front[i]] +=
              (values[front[i + 1]][v] - values[front[i - 1]][v]) /
              (values[front.back()][v] - values[front.front()][v]);
      }
    }
  };

  // Advance the population.
  for (size_t it = 0; it < iterations; ++it) {
    make_new_population();
    non_dominated_sort();
    assign_crowding_distances();
    size_t last_front_index = 0;
    size_t s = fronts.front().size();
    while (s < m) {
      ++last_front_index;
      s += fronts[last_front_index].size();
    }
    if (s != m) {
      sort(begin(fronts[last_front_index]), end(fronts[last_front_index]),
           [&](auto x, auto y) {
             return crowding_distances[x] > crowding_distances[y];
           });
    }

    vector<argument_type> tmp_inputs(n);
    vector<result_type> tmp_values(n);
    size_t index = 0;
    for (size_t i = 0; i < last_front_index; ++i) {
      for (auto j : fronts[i]) {
        tmp_inputs[index] = inputs[j];
        tmp_values[index] = values[j];
        ++index;
      }
    }
    for (size_t j = 0; index < m; ++index, ++j) {
      tmp_inputs[index] = inputs[fronts[last_front_index][j]];
      tmp_values[index] = values[fronts[last_front_index][j]];
    }
    std::swap(inputs, tmp_inputs);
    std::swap(values, tmp_values);
  }

  return tuple{inputs, values, fronts};
}
}  // namespace nsga2

}  // namespace lyrahgames::pareto