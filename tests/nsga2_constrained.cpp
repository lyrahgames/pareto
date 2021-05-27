#include <concepts>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
//
#include <lyrahgames/chrono.hpp>
#include <lyrahgames/gpp.hpp>
#include <lyrahgames/pareto/gallery/kursawe.hpp>
#include <lyrahgames/pareto/gallery/poloni2.hpp>
#include <lyrahgames/pareto/nsga2_constrained.hpp>

using namespace std;
using namespace lyrahgames;
using namespace pareto;
using real = float;

int main() {
  mt19937 rng{random_device{}()};
  uniform_real_distribution<real> dist{-5, 5};
  const auto random = [&] { return dist(rng); };

  using config_vector = array<real, 2>;
  using object_vector = array<real, 2>;

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

  vector<config_vector> inputs{};
  vector<object_vector> values{};
  vector<vector<size_t>> fronts{};

  const auto t = time([&] {
    tie(inputs, values, fronts) =
        pareto::nsga2::constrained_optimization(f, box, g, rng, 1000);
  });
  cout << "Computation took " << t << "s for " << fronts.front().size()
       << " points on the pareto front."
       << "\n";

  fstream pareto_file{"pareto.dat", ios::out};
  for (size_t index = 0; index < fronts.front().size(); ++index) {
    const auto& p = values[index];
    const auto& x = inputs[index];
    pareto_file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0] << '\t'
                << p[1] << '\n';
  }
  pareto_file << flush;

  gpp plot{};
  plot << "plot 'pareto.dat' u 4:5 w p lt rgb '#ff3333' pt 13\n";
}
