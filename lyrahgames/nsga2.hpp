#pragma once
#include <array>
#include <concepts>
#include <random>
#include <vector>
//
#include <lyrahgames/pareto/aabb.hpp>
#include <lyrahgames/pareto/domination.hpp>
#include <lyrahgames/pareto/gallery/kursawe.hpp>

namespace lyrahgames {

struct nsga2 {
  using real = float;

  nsga2() : inputs(n), values(n), crowding_distances(n) { initialize(); }

  void make_new_population();
  void initialize();
  void non_dominated_sort();
  void assign_crowding_distances();
  void advance(size_t iterations = 1);

  std::mt19937 rng{std::random_device{}()};
  size_t m = 100 * 4;
  size_t n = 2 * m;
  std::vector<std::array<real, 3>> inputs;
  std::vector<std::array<real, 2>> values;
  std::vector<std::vector<size_t>> fronts;
  std::vector<real> crowding_distances;
};

inline void nsga2::make_new_population() {
  // crossover
  size_t i = 0;
  std::uniform_int_distribution<size_t> population_dist{0, m - 1};
  for (; i < m / 2; i += 2) {
    auto x = inputs[population_dist(rng)];
    auto y = inputs[population_dist(rng)];
    // const auto crossings = std::uniform_int_distribution<size_t>{1, 3 -
    // 1}(rng);
    const size_t crossings = 1;  // odd number ensures results are different
    for (size_t j = 0; j < crossings; ++j) {
      const auto index = std::uniform_int_distribution<size_t>{0, 3 - 1}(rng);
      std::swap(x[index], y[index]);
    }
    auto x_value = pareto::kursawe(x);
    auto y_value = pareto::kursawe(y);
    inputs[m + i + 0] = x;
    inputs[m + i + 1] = y;
    values[m + i + 0] = x_value;
    values[m + i + 1] = y_value;
  }
  // mutation
  for (; i < m; ++i) {
    auto x = inputs[population_dist(rng)];
    constexpr real stepsize = 0.1 * 5;
    for (size_t k = 0; k < x.size(); ++k) {
      const auto step = std::uniform_real_distribution<real>{-1, 1}(rng);
      x[k] += step * stepsize;
    }
    inputs[m + i] = x;
    values[m + i] = pareto::kursawe(x);
  }
}

inline void nsga2::initialize() {
  std::uniform_real_distribution<real> dist{-5, 5};
  for (size_t i = 0; i < m; ++i) {
    const std::array<real, 3> x{dist(rng), dist(rng), dist(rng)};
    const auto v = pareto::kursawe(x);
    inputs[i] = x;
    values[i] = v;
  }
}

inline void nsga2::non_dominated_sort() {
  fronts.resize(1);
  fronts.front().resize(0);

  std::vector<size_t> ranks(n, 0);
  std::vector<size_t> dominations(n, 0);
  std::vector<std::vector<size_t>> dominated_sets(n);

  for (size_t i = 0; i < n; ++i) {
    const auto& p = values[i];
    for (size_t j = 0; j < n; ++j) {
      const auto& q = values[j];
      if (pareto::domination(p, q))
        dominated_sets[i].push_back(j);
      else if (pareto::domination(q, p))
        ++dominations[i];
    }
    if (dominations[i] == 0) {
      ranks[i] = 1;
      fronts[0].push_back(i);
    }
  }
  size_t i = 0;
  while (!fronts[i].empty()) {
    std::vector<size_t> buffer{};
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
}

inline void nsga2::assign_crowding_distances() {
  for (size_t i = 0; i < n; ++i) crowding_distances[i] = 0;
  for (auto& front : fronts) {
    if (front.empty()) break;
    for (size_t v = 0; v < 2; ++v) {
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
}

inline void nsga2::advance(size_t iterations) {
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

    std::vector<std::array<real, 3>> tmp_inputs(n);
    std::vector<std::array<real, 2>> tmp_values(n);
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
}

}  // namespace lyrahgames