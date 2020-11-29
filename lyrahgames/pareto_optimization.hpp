#pragma once
#include <array>
#include <concepts>
#include <functional>
//
#include <set>
//
#include <lyrahgames/meta.hpp>

namespace lyrahgames {

// template <vector T>
// struct aabb {
//   T min;
//   T max;
// };

template <std::totally_ordered Real>
constexpr auto domination(const std::array<Real, 2>& x,
                          const std::array<Real, 2>& y) {
  return (x[0] <= y[0]) && (x[1] <= y[1]) && ((x[0] < y[0]) || (x[1] < y[1]));
}

// pareto_front_estimation
// template <vector_field Functor>
// inline void monte_carlo_pareto_front(
//     Functor f,
//     aabb<typename decltype(detail::function_traits{f})::arg<0>> box) {
// using namespace detail;
// using input_t = typename function_traits<Functor>::arg<0>;
// using output_t = typename function_traits<Functor>::result_type;
// using real_type = std::decay_t<decltype(std::declval<input_t>()[0])>;
// static_assert(std::is_same_v<input_t, std::array<float, 3>>);
// static_assert(std::is_same_v<output_t, std::array<float, 2>>);
// static_assert(std::is_same_v<real_type, float>);
// static_assert(
//     std::is_same_v<real_type,
//                    std::decay_t<decltype(std::declval<output_t>()[0])>>);

// const int n = 10'000'000;
// set<array<real, 2>> pareto_front{};
// for (size_t i = 1; i < n; ++i) {
//   const array<real, 3> x{random(), random(), random()};
//   const auto v = kursawe(x);
//   bool to_add = true;
//   for (auto it = begin(pareto_front); it != end(pareto_front);) {
//     const auto& p = *it;
//     if (domination(p, v)) {
//       to_add = false;
//       break;
//     }
//     if (domination(v, p))
//       it = pareto_front.erase(it);
//     else
//       ++it;
//   }
//   if (to_add) pareto_front.insert(v);
// }
// }

}  // namespace lyrahgames