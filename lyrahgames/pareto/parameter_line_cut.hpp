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
class parameter_line_cut {
 public:
  using frontier_type = T;
  // using real = typename frontier_type::real;
  using real = typename frontier_type::real;

  parameter_line_cut() = default;
  parameter_line_cut(const frontier_type& frontier) {
    using namespace std;
    assert(frontier.objective_count() == 2);

    // Utility
    const auto square = [](real x) { return x * x; };

    const auto n = frontier.sample_count();
    const auto p = frontier.parameter_count();

    // Determine AABB in parameter space of frontier.
    const auto it = frontier.parameters(0);
    vector<real> parameter_aabb_min(begin(it), end(it));
    vector<real> parameter_aabb_max(begin(it), end(it));
    for (size_t i = 1; i < n; ++i) {
      for (size_t j = 0; const auto& x : frontier.parameters(i)) {
        parameter_aabb_min[j] = min(parameter_aabb_min[j], x);
        parameter_aabb_max[j] = max(parameter_aabb_max[j], x);
        ++j;
      }
    }
    // Compute AABB scales.
    vector<real> parameter_scale(p);
    for (size_t i = 0; i < p; ++i) {
      parameter_scale[i] = 1 / (parameter_aabb_max[i] - parameter_aabb_min[i]);
    }

    // Define distance metric for parameters with respect to AABB.
    const auto parameter_distance = [&](size_t i, size_t j) {
      const auto x = frontier.parameters(i);
      const auto y = frontier.parameters(j);
      real distance = 0;
      for (size_t k = 0; k < p; ++k)
        distance += square(parameter_scale[k] * (x[k] - y[k]));
      return sqrt(distance / p);
    };

    // Create indices with sorted entries of Pareto frontier.
    indices.resize(n);
    iota(begin(indices), end(indices), 0);
    ranges::sort(indices, [&frontier](auto i, auto j) {
      return *frontier.objectives_iterator(i) <
             *frontier.objectives_iterator(j);
    });

    // Cut Pareto frontier into line segments.
    size_t line_start = 0;
    for (size_t i = 0; i < n - 1; ++i) {
      const auto d = parameter_distance(indices[i], indices[i + 1]);
      const real max_allowed_distance = 0.2;
      // cout << setw(15) << d << '\n';
      if (d > max_allowed_distance) {
        cout << setw(15) << i << '\n';
        edges.push_back({line_start, i + 1});
        line_start = i + 1;
      }
    }
    edges.push_back({line_start, n});
  }

  auto permutation(size_t index) const -> size_t { return indices[index]; }

  const auto& lines() const { return edges; }

 private:
  std::vector<size_t> indices{};
  std::vector<std::pair<size_t, size_t>> edges{};
};

template <generic::frontier frontier_type>
parameter_line_cut(const frontier_type&) -> parameter_line_cut<frontier_type>;

}  // namespace lyrahgames::pareto
