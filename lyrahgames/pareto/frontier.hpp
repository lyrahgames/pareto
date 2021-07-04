#pragma once
#include <span>
#include <vector>
//
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto {

/// One possible frontier structure used to store the information of an
/// estimated Pareto frontier. It can be used by frontier-casting an optimizer
/// after an optimization to this structure. This structure uses an SOA layout.
template <generic::real T>
struct frontier {
  using real = T;

  frontier() = default;

  frontier(size_t count, size_t input, size_t output)
      : s{count},
        n{input},
        m{output},
        parameters_data(s * n),
        objectives_data(s * m) {}

  /// Returns number of stored samples.
  auto sample_count() const noexcept { return s; }

  /// Returns the number of parameters per sample.
  auto parameter_count() const noexcept { return n; }

  /// Returns the number of objectives per sample.
  auto objective_count() const noexcept { return m; }

  /// Returns range of parameters to the sample identified by 'index'.
  auto parameters(size_t index) noexcept {
    return std::span{&parameters_data[n * index],
                     &parameters_data[n * (index + 1)]};
  }

  /// Returns range of parameters to the sample identified by 'index'.
  /// Constant overload.
  auto parameters(size_t index) const noexcept {
    return std::span{&parameters_data[n * index],
                     &parameters_data[n * (index + 1)]};
  }

  /// Returns range of objectives to the sample identified by 'index'.
  auto objectives(size_t index) noexcept {
    return std::span{&objectives_data[m * index],
                     &objectives_data[m * (index + 1)]};
  }

  /// Returns range of parameters to the sample identified by 'index'.
  /// Constant overload.
  auto objectives(size_t index) const noexcept {
    return std::span{&objectives_data[m * index],
                     &objectives_data[m * (index + 1)]};
  }

  /// Returns iterator to the beginning of the parameters
  /// of the sample identified by 'index'.
  auto parameters_iterator(size_t index) noexcept {
    return &parameters_data[n * index];
  }

  /// Returns iterator to the beginning of the parameters
  /// of the sample identified by 'index'. Constant overload.
  auto parameters_iterator(size_t index) const noexcept {
    return &parameters_data[n * index];
  }

  /// Returns iterator to the beginning of the objectives
  /// of the sample identified by 'index'.
  auto objectives_iterator(size_t index) noexcept {
    return &objectives_data[m * index];
  }

  /// Returns iterator to the beginning of the objectives
  /// of the sample identified by 'index'. Constant overload.
  auto objectives_iterator(size_t index) const noexcept {
    return &objectives_data[m * index];
  }

  /// Sample Count
  size_t s{};
  /// Parameter Count
  size_t n{};
  /// Objective Count
  size_t m{};

  std::vector<real> parameters_data{};
  std::vector<real> objectives_data{};
};

static_assert(generic::frontier<frontier<float>>);
static_assert(generic::frontier<frontier<double>>);

}  // namespace lyrahgames::pareto
