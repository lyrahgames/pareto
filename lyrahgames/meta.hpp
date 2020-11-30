#pragma once
#include <concepts>
#include <functional>

namespace lyrahgames {

namespace generic {

template <typename T>
concept callable = requires(T v) {
  std::function{v};
};

template <typename T>
concept vector = requires(T v) {
  requires std::is_floating_point_v<std::decay_t<decltype(v[0])>>;
  { size(v) }
  ->std::same_as<size_t>;
};

}  // namespace generic

namespace meta {

template <generic::vector T>
using value = std::decay_t<decltype(std::declval<T>()[0])>;

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

template <generic::callable T>
using result = typename decltype(function{std::declval<T>()})::result;
template <generic::callable T>
constexpr auto argument_count =
    decltype(function{std::declval<T>()})::argument_count;
template <generic::callable T>
using arguments = typename decltype(function{std::declval<T>()})::arguments;
template <generic::callable T, size_t N>
using argument = typename decltype(function{std::declval<T>()})::argument<N>;

}  // namespace meta

namespace generic {

template <typename T, typename U>
concept compatible_vectors =   //
    generic::vector<T>         //
        && generic::vector<U>  //
            && std::same_as<meta::value<T>, meta::value<U>>;

template <typename T>
concept vector_field =                 //
    callable<T>                        //
    && (meta::argument_count<T> == 1)  //
    && compatible_vectors<meta::result<T>, meta::argument<T, 0>>;

}  // namespace generic

}  // namespace lyrahgames