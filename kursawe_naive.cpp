#include <concepts>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <typeinfo>
#include <vector>
//
#include <lyrahgames/chrono.hpp>
#include <lyrahgames/gpp.hpp>
#include <lyrahgames/kursawe.hpp>
#include <lyrahgames/nsga2.hpp>
// #include <lyrahgames/pareto_optimization.hpp>
#include <lyrahgames/meta.hpp>

using namespace std;
using namespace lyrahgames;
using real = float;

void test(auto x) { cout << "The argument uses the default implementation.\n"; }
void test(generic::vector auto x) {
  cout << "The argument fulfills the concept vector.\n";
}
void test(generic::callable auto f) {
  cout << "The argument fulfills the callable concept.\n";
}
void test(generic::vector_field auto f) {
  cout << "The argument fulfills the concept vector_field.\n";
}
// constexpr void callable_test(auto callable) {
//   // cout << typeid(typename detail::function_traits<decltype(
//   //                    std::function{callable})>::result_type)
//   //             .name()
//   //      << '(';

//   using traits = decltype(detail::function_traits{callable});

//   cout << typeid(typename traits::result_type).name() << '(';
//   if constexpr (traits::argc > 0) {
//     // cout << typeid(typename traits::arg<0>).name();
//     std::apply(
//         [](auto&&... args) { ((cout << typeid(args).name() << ", "), ...); },
//         typename traits::args{});
//   }

//   // for (size_t i = 0; i < typename detail::function_traits<decltype(
//   //                            std::function{callable})>::argc;
//   //      ++i)
//   // if constexpr (detail::function_traits<decltype(
//   //                   std::function{callable})>::argc > 0)
//   //   cout << typeid(typename detail::function_traits<decltype(
//   //                      std::function{callable})>::arg<0>)
//   //               .name()
//   //        << ", ";
//   cout << ')' << '\n';
// }

struct functor {
  int operator()() const { return a; }
  int a;
};

struct A {};

int main() {
  test(1.0f);
  test(array<float, 2>{});
  test(array<float, 3>{});
  test(array<float, 4>{});
  test(std::vector<float>{});
  test(std::vector<int>{});

  test([]() {});
  test([](array<float, 3> x) { return array<float, 2>{}; });
  test([](array<float, 3> x) { return std::vector<float>{}; });
  test(kursawe<float>);

  // callable_test([](float x) { return x * x; });
  // callable_test([](float x, double y, int z) { return x * x + y + z; });
  // callable_test(test<float>);
  // functor f{2};
  // callable_test(f);

  // aabb<array<float, 2>> box{{-5, -3}, {1, 2}};

  mt19937 rng{random_device{}()};
  uniform_real_distribution<real> dist{-5, 5};
  const auto random = [&] { return dist(rng); };

  const int n = 10'000'000;
  set<array<real, 2>> pareto_front{};
  const auto t = time([&] {
    for (size_t i = 1; i < n; ++i) {
      const array<real, 3> x{random(), random(), random()};
      const auto v = kursawe(x);
      bool to_add = true;
      for (auto it = begin(pareto_front); it != end(pareto_front);) {
        const auto& p = *it;
        if (domination(p, v)) {
          to_add = false;
          break;
        }
        if (domination(v, p))
          it = pareto_front.erase(it);
        else
          ++it;
      }
      if (to_add) pareto_front.insert(v);
    }
  });
  cout << "Computation took " << t << "s for " << n << " random points with "
       << pareto_front.size() << " points on the pareto front."
       << "\n";

  fstream pareto_file{"pareto.dat", ios::out};
  for (auto& p : pareto_front) pareto_file << p[0] << '\t' << p[1] << '\n';
  pareto_file << flush;

  gpp plot{};
  plot << "set xr [-21:-14]\n"
       << "set yr [-12:2]\n"
       << "plot 'pareto.dat' u 1:2 w p lt rgb '#ff3333' pt 13\n";
}
