#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iomanip>
#include <iostream>
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

    // Compute mean distance of neighbors.
    // real mean_distance = 0;
    for (size_t i = 0; i < n - 1; ++i) {
      auto it = frontier.objectives_iterator(indices[i]);
      const real x1 = *it++;
      const real y1 = *it;
      it = frontier.objectives_iterator(indices[i + 1]);
      const real x2 = *it++;
      const real y2 = *it;

      mean_distance += sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    }
    mean_distance /= n - 1;

    // cout << setw(30) << "mean distance = " << mean_distance << '\n';

    // Compute the variance and standard deviation of distance of neighbors.
    // real var_distance = 0;
    for (size_t i = 0; i < n - 1; ++i) {
      auto it = frontier.objectives_iterator(indices[i]);
      const real x1 = *it++;
      const real y1 = *it;
      it = frontier.objectives_iterator(indices[i + 1]);
      const real x2 = *it++;
      const real y2 = *it;

      const auto distance = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
      var_distance += (distance - mean_distance) * (distance - mean_distance);
    }
    var_distance /= n - 2;
    stddev_distance = sqrt(var_distance);

    // cout << setw(30) << "distance variance = " << var_distance << '\n'
    //      << setw(30) << "distance stddev = " << stddev_distance << '\n';

    size_t line_start = 0;
    for (size_t i = 0; i < n - 1; ++i) {
      auto it = frontier.objectives_iterator(indices[i]);
      const real x1 = *it++;
      const real y1 = *it;
      it = frontier.objectives_iterator(indices[i + 1]);
      const real x2 = *it++;
      const real y2 = *it;

      const auto distance = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
      if (distance > mean_distance + 3 * stddev_distance) {
        edges.push_back({line_start, i + 1});
        line_start = i + 1;
        // cout << "Cut connection (" << i << ", " << i + 1 << ").\n";
        // output << '\n';
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
