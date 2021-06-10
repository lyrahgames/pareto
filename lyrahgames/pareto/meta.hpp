#pragma once
#include <concepts>
#include <functional>
#include <ranges>
//
#include <lyrahgames/xstd/forward.hpp>
#include <lyrahgames/xstd/meta.hpp>

namespace lyrahgames::pareto {

namespace generic {

using namespace lyrahgames::xstd::generic;

template <typename T>
concept random_number_generator = true;

template <typename T>
concept real = std::floating_point<T>;

template <typename T>
concept problem = real<typename T::real> &&
    requires(T& p, size_t i, T::real& a, T::real& b) {
  { p.parameter_count() } -> identical<size_t>;
  { p.objective_count() } -> identical<size_t>;
  std::tie(a, b) = p.box(i);
};

template <typename T>
concept optimizer = true;

template <typename T, typename real>
concept range = (std::ranges::random_access_range<T> &&
                 std::same_as<real, std::ranges::range_value_t<T>>);

template <typename T>
concept frontier = true;

}  // namespace generic

namespace generic {

template <typename T>
concept array =      //
    std::regular<T>  //
    &&               //
    requires(T v, size_t i) {
  { size(v) } -> std::unsigned_integral;
  { size(v) } -> std::convertible_to<size_t>;
  v[i];
  requires std::is_constructible_v<
      T, std::initializer_list<std::decay_t<decltype(v[i])>>>;
};

template <typename T>
concept static_array = array<T> &&  //
    std::enable_if_t<(size(T{}) > 0), std::true_type>::value;
template <typename T>
concept dynamic_array = array<T> && !static_array<T>;

template <typename T>
concept callable = requires(T v) {
  std::function{v};
};

template <typename T>
concept vector = requires(T v) {
  requires std::is_floating_point_v<std::decay_t<decltype(v[0])>>;
  { size(v) } -> std::same_as<size_t>;
};

}  // namespace generic

namespace meta {

template <generic::vector T>
using qualified_value = decltype(std::declval<T>()[0]);
template <generic::vector T>
using value = std::decay_t<qualified_value<T>>;

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
struct function;
template <typename R, typename... Args>
struct function<std::function<R(Args...)>> {
  using result = R;
  static constexpr size_t argument_count = sizeof...(Args);
  using arguments = std::tuple<Args...>;
  template <size_t n>
  using argument = std::tuple_element_t<n, std::tuple<Args...>>;
  explicit function(std::function<R(Args...)>) {}
};
template <generic::callable T>
function(T) -> function<decltype(std::function{std::declval<T>()})>;

// We have to introduce a second using declaration.
// Otherwise, strange compiler error seem to happen.
// We do not know why this happens.
template <generic::callable T>
using qualified_result = typename decltype(function{std::declval<T>()})::result;
template <generic::callable T>
using result = std::decay_t<qualified_result<T>>;

template <generic::callable T>
constexpr auto argument_count = decltype(function{
    std::declval<T>()})::argument_count;

template <generic::callable T>
using arguments = typename decltype(function{std::declval<T>()})::arguments;

template <generic::callable T, size_t N>
using qualified_argument =
    std::decay_t<typename decltype(function{std::declval<T>()})::argument<N>>;
template <generic::callable T, size_t N>
using argument = std::decay_t<qualified_argument<T, N>>;

template <typename T>
constexpr auto print_type() = delete;
template <typename T>
constexpr auto print_type(T) = delete;

}  // namespace meta

namespace generic {

template <typename T, typename U>
concept compatible_vectors =  //
    generic::vector<T>        //
    && generic::vector<U>     //
    && std::same_as<meta::value<T>, meta::value<U>>;

template <typename T>
concept vector_field =                //
    callable<T>                       //
    &&(meta::argument_count<T> == 1)  //
    && compatible_vectors<meta::result<T>, meta::argument<T, 0>>;

}  // namespace generic

}  // namespace lyrahgames::pareto