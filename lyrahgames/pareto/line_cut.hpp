#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iomanip>
#include <iostream>
#include <limits>
#include <ranges>
#include <vector>
//
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto {

template <generic::frontier T>
class line_cut {
 public:
  using frontier_type = T;
  // using real = typename frontier_type::real;
  using real = double;

  line_cut() = default;
  line_cut(const frontier_type& frontier) {
    using namespace std;
    assert(frontier.objective_count() == 2);

    const auto n = frontier.sample_count();

    // Create indices with sorted entries.
    indices.resize(n);
    iota(begin(indices), end(indices), 0);
    ranges::sort(indices, [&frontier](auto i, auto j) {
      return *frontier.objectives_iterator(i) <
             *frontier.objectives_iterator(j);
    });

    auto it = frontier.objectives_iterator(indices.front());
    const real left = *it++;
    const real top = *it;
    it = frontier.objectives_iterator(indices.back());
    const real right = *it++;
    const real bottom = *it;

    constexpr auto infinity = std::numeric_limits<real>::infinity();
    const auto xscale = 1 / (right - left);
    const auto yscale = 1 / (top - bottom);
    vector<real> distances(n - 1);

    const auto square = [](real x) { return x * x; };

    const auto distance = [&](size_t i) {
      auto it = frontier.objectives_iterator(indices[i]);
      const real x1 = *it++;
      const real y1 = *it;
      it = frontier.objectives_iterator(indices[i + 1]);
      const real x2 = *it++;
      const real y2 = *it;
      const auto dx = xscale * (x2 - x1);
      const auto dy = yscale * (y1 - y2);
      const auto ddxdy2 = square(dx - dy);
      const auto denom = ddxdy2 + dx * dy;
      const auto slope_penalty = (denom < 1e-6) ? (0) : (ddxdy2 / denom);
      return sqrt(dx * dx + dy * dy) * (0.9 + 0.8 * slope_penalty);
    };

    // Compute mean distance of neighbors.
    // real mean_distance = 0;
    for (size_t i = 0; i < n - 1; ++i) {
      distances[i] = distance(i);
      mean_distance += distances[i];
    }
    mean_distance /= n - 1;

    // Compute the variance and standard deviation of distance of neighbors.
    // real var_distance = 0;
    for (size_t i = 0; i < n - 1; ++i)
      var_distance += square(distances[i] - mean_distance);
    var_distance /= n - 2;
    stddev_distance = sqrt(var_distance);

    // Estimate maximum allowed distance.
    ranges::sort(distances);
    auto max_allowed_distance = infinity;
    for (size_t i = 0; i < distances.size() - 1; ++i) {
      if (distances[i] < mean_distance) continue;
      // if (distances[i] / distances[i + 1] > 0.8) continue;
      if (distances[i] / distances[i + 1] > 0.8) continue;
      max_allowed_distance = distances[i];
      // std::cout << "index = " << i << '\n'
      //           << "max allowed dist = " << max_allowed_distance << '\n';
      break;
    }

    // Cut Pareto frontier into line segments.
    size_t line_start = 0;
    for (size_t i = 0; i < n - 1; ++i) {
      if (distance(i) > max_allowed_distance) {
        edges.push_back({line_start, i + 1});
        line_start = i + 1;
      }
    }
    edges.push_back({line_start, n});
  }

  auto permutation(size_t index) const -> size_t { return indices[index]; }

  const auto& lines() const { return edges; }

  real mean_neighbor_distance() const { return mean_distance; }
  real var_neighbor_distance() const { return var_distance; }
  real stddev_neighbor_distance() const { return stddev_distance; }

 private:
  std::vector<size_t> indices{};
  std::vector<std::pair<size_t, size_t>> edges{};
  real mean_distance = 0;
  real var_distance = 0;
  real stddev_distance = 0;
};

template <generic::frontier frontier_type>
line_cut(const frontier_type&) -> line_cut<frontier_type>;

}  // namespace lyrahgames::pareto
