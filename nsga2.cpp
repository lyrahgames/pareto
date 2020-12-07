#include <concepts>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
//
#include <lyrahgames/chrono.hpp>
#include <lyrahgames/gpp.hpp>
#include <lyrahgames/kursawe.hpp>
#include <lyrahgames/nsga2_optimizer.hpp>
#include <lyrahgames/pareto_optimization.hpp>
#include <lyrahgames/poloni2.hpp>

using namespace std;
using namespace lyrahgames;
using real = float;

int main() {
  mt19937 rng{random_device{}()};
  uniform_real_distribution<real> dist{-5, 5};
  const auto random = [&] { return dist(rng); };

  vector<array<float, 2>> inputs{};
  vector<array<float, 2>> values{};
  vector<vector<size_t>> fronts{};

  // const auto f = kursawe<float>;
  // const auto box = aabb<array<float, 3>>{{-5, -5, -5}, {5, 5, 5}};
  const auto f = poloni2<array<float, 2>, array<float, 2>>;
  const auto box = aabb<array<float, 2>>{{-M_PI, -M_PI}, {M_PI, M_PI}};

  const auto t = time([&] {
    tie(inputs, values, fronts) = nsga2::optimization(f, box, rng, 100);
  });
  cout << "Computation took " << t << "s for " << fronts.front().size()
       << " points on the pareto front."
       << "\n";

  fstream pareto_file{"pareto.dat", ios::out};
  for (auto index : fronts.front()) {
    const auto& p = values[index];
    const auto& x = inputs[index];
    pareto_file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0] << '\t'
                << p[1] << '\n';
  }
  pareto_file << flush;

  gpp plot{};
  plot << "plot 'pareto.dat' u 4:5 w p lt rgb '#ff3333' pt 13\n";
}
