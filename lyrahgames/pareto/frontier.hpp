#pragma once
#include <span>
#include <vector>
//
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto {

template <std::floating_point T>
struct frontier {
  using real = T;

  frontier() = default;

  frontier(size_t count, size_t input, size_t output)
      : s{count},
        n{input},
        m{output},
        parameters_data(s * n),
        objectives_data(s * m) {}

  auto sample_count() const noexcept { return s; }

  auto parameter_count() const noexcept { return n; }

  auto objective_count() const noexcept { return m; }

  auto parameters(size_t index) noexcept {
    return std::span{&parameters_data[n * index],
                     &parameters_data[n * (index + 1)]};
  }

  auto parameters(size_t index) const noexcept {
    return std::span{&parameters_data[n * index],
                     &parameters_data[n * (index + 1)]};
  }

  auto objectives(size_t index) noexcept {
    return std::span{&objectives_data[m * index],
                     &objectives_data[m * (index + 1)]};
  }

  auto objectives(size_t index) const noexcept {
    return std::span{&objectives_data[m * index],
                     &objectives_data[m * (index + 1)]};
  }

  auto parameters_iterator(size_t index) noexcept {
    return &parameters_data[n * index];
  }

  auto parameters_iterator(size_t index) const noexcept {
    return &parameters_data[n * index];
  }

  auto objectives_iterator(size_t index) noexcept {
    return &objectives_data[m * index];
  }

  auto objectives_iterator(size_t index) const noexcept {
    return &objectives_data[m * index];
  }

  size_t s{};
  size_t n{};
  size_t m{};

  std::vector<real> parameters_data{};
  std::vector<real> objectives_data{};
};

static_assert(generic::frontier<frontier<float>>);
static_assert(generic::frontier<frontier<double>>);

}  // namespace lyrahgames::pareto