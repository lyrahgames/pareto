#pragma once
#include <array>
#include <concepts>
#include <functional>
//
#include <set>

namespace lyrahgames {

template <typename T>
concept vector = requires(T v) {
  // { (v[0]) }
  // ->std::floating_point;
  requires std::is_floating_point_v<std::decay_t<decltype(v[0])>>;
  { size(v) }
  ->std::same_as<size_t>;
};

template <vector T>
struct aabb {
  T min;
  T max;
};

template <std::totally_ordered Real>
constexpr auto domination(const std::array<Real, 2>& x,
                          const std::array<Real, 2>& y) {
  return (x[0] <= y[0]) && (x[1] <= y[1]) && ((x[0] < y[0]) || (x[1] < y[1]));
}

namespace detail {

template <typename T>
struct function_traits;

// template <typename R, typename... Args>
// struct function_traits<std::function<R(Args...)>> {
//   static const size_t nargs = sizeof...(Args);

//   typedef R result_type;

//   template <size_t i>
//   struct arg {
//     typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
//   };
// };

template <typename R, typename... Args>
// struct function_traits<R (*)(Args...)> {
struct function_traits<std::function<R(Args...)>> {
  static constexpr size_t argc = sizeof...(Args);

  using result_type = std::decay_t<R>;

  using args = std::tuple<Args...>;

  template <size_t n>
  using arg = std::decay_t<std::tuple_element_t<n, std::tuple<Args...>>>;

  explicit function_traits(std::function<R(Args...)>) {}
};

template <typename T>
function_traits(T)
    -> function_traits<decltype(std::function{std::declval<T>()})>;

// template <typename T>
// using function_traits_t =
//     function_traits<decltype(std::function{std::declval<T>()})>;
// struct function_traits<T>
//     : public function_traits<std::function{std::declval<T>()}> {};

}  // namespace detail

template <typename T>
concept vector_field = requires(T v) {
  requires decltype(detail::function_traits{v})::argc == 1;
  requires vector<typename decltype(detail::function_traits{v})::result_type>;
  requires vector<typename decltype(detail::function_traits{v})::arg<0>>;
};

// pareto_front_estimation
template <vector_field Functor>
inline void monte_carlo_pareto_front(
    Functor f,
    aabb<typename decltype(detail::function_traits{f})::arg<0>> box) {
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
}

}  // namespace lyrahgames