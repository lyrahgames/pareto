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
#include <lyrahgames/meta.hpp>
#include <lyrahgames/nsga2.hpp>
#include <lyrahgames/pareto/gallery/kursawe.hpp>
#include <lyrahgames/pareto/gallery/poloni2.hpp>
#include <lyrahgames/pareto/gallery/schaffer.hpp>
#include <lyrahgames/pareto/naive.hpp>

int main() {
  using namespace std;
  using namespace lyrahgames;
  using namespace pareto;

  using real = float;
  using config_vector = array<real, 2>;
  using object_vector = array<real, 2>;

  mt19937 rng{random_device{}()};
  vector<object_vector> pareto_front{};

  // const auto f = [](const config_vector& x) {
  //   return object_vector{x[0], (1 + x[1]) / x[0]};
  // };
  // const auto box = aabb<config_vector>{{0.1, 0.0}, {1.0, 5.0}};
  // const auto g = [](const config_vector& x) {   //
  //   return array<real, 2>{9 * x[0] + x[1] - 6,  //
  //                         9 * x[0] - x[1] - 1};
  // };

  // const auto f = [](const config_vector& x) {  //
  //   const auto square = [](auto y) { return y * y; };
  //   return object_vector{square(x[0] - 2) + square(x[1] - 1) + 2,  //
  //                        9 * x[0] - square(x[1] - 1)};
  // };
  // const auto box = aabb<config_vector>{{-20, -20}, {20, 20}};
  // const auto g = [](const config_vector& x) {               //
  //   return array<real, 2>{225 - x[0] * x[0] - x[1] * x[1],  //
  //                         -10 - x[0] + 3 * x[1]};
  // };

  const auto f = [](const config_vector& x) {  //
    return object_vector{x[0], x[1]};
  };
  const auto box = aabb<config_vector>{{0, 0}, {M_PI, M_PI}};
  const auto g = [](const config_vector& x) {  //
    const auto square = [](auto y) { return y * y; };
    return array<real, 2>{
        x[0] * x[0] + x[1] * x[1] - 1 -
            real{0.1} * cos(16 * atan(x[0] / x[1])),  //
        real(0.5) - square(x[0] - real(0.5)) - square(x[1] - real(0.5))};
  };

  const auto t = time([&] {  //
    pareto_front =
        monte_carlo_constrained_pareto_front(f, box, g, 1'000'000, rng);
  });

  cout << "Computation took " << t << "s for " << pareto_front.size()
       << " points on the pareto front."
       << "\n";

  fstream pareto_file{"pareto.dat", ios::out};
  for (auto& p : pareto_front)
    pareto_file << p[0] << '\t' << p[1] << '\n';
  pareto_file << flush;

  gpp plot{};
  plot << "plot 'pareto.dat' u 1:2 w p lt rgb '#ff3333' pt 13\n";
}
