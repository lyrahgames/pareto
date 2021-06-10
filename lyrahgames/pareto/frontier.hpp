#pragma once
#include <concepts>
#include <span>
#include <vector>

namespace lyrahgames::pareto {

template <std::floating_point T>
struct frontier {
  using real = T;
  using size_type = size_t;

  frontier() = default;

  frontier(size_type count, size_type input, size_type output)
      : s{count},
        n{input},
        m{output},
        parameters_data(s * n),
        objectives_data(s * m) {}

  auto sample_count() const noexcept { return s; }

  auto parameter_count() const noexcept { return n; }

  auto objective_count() const noexcept { return m; }

  auto parameters(size_type index) noexcept {
    return std::span{&parameters_data[n * index],
                     &parameters_data[n * (index + 1)]};
  }

  auto parameters(size_type index) const noexcept {
    return std::span{&parameters_data[n * index],
                     &parameters_data[n * (index + 1)]};
  }

  auto objectives(size_type index) noexcept {
    return std::span{&objectives_data[m * index],
                     &objectives_data[m * (index + 1)]};
  }

  auto objectives(size_type index) const noexcept {
    return std::span{&objectives_data[m * index],
                     &objectives_data[m * (index + 1)]};
  }

  auto parameters_iterator(size_type index) noexcept {
    return &parameters_data[n * index];
  }

  auto parameters_iterator(size_type index) const noexcept {
    return &parameters_data[n * index];
  }

  auto objectives_iterator(size_type index) noexcept {
    return &objectives_data[m * index];
  }

  auto objectives_iterator(size_type index) const noexcept {
    return &objectives_data[m * index];
  }

  size_type s{};
  size_type n{};
  size_type m{};

  std::vector<real> parameters_data{};
  std::vector<real> objectives_data{};
};

}  // namespace lyrahgames::pareto