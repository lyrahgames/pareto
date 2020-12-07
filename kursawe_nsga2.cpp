#include <concepts>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
//
#include <lyrahgames/chrono.hpp>
#include <lyrahgames/gpp.hpp>
#include <lyrahgames/nsga2.hpp>
#include <lyrahgames/pareto/gallery/kursawe.hpp>

using namespace std;
using namespace lyrahgames;
using namespace pareto;
using real = float;

int main() {
  mt19937 rng{random_device{}()};
  uniform_real_distribution<real> dist{-5, 5};
  const auto random = [&] { return dist(rng); };

  nsga2 pareto_optimization{};
  const auto t = time([&] { pareto_optimization.advance(150); });
  cout << "Computation took " << t << "s for "
       << pareto_optimization.fronts.front().size()
       << " points on the pareto front."
       << "\n";

  fstream pareto_file{"pareto.dat", ios::out};
  for (auto index : pareto_optimization.fronts.front()) {
    const auto& p = pareto_optimization.values[index];
    const auto& x = pareto_optimization.inputs[index];
    pareto_file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0] << '\t'
                << p[1] << '\n';
  }
  pareto_file << flush;

  gpp plot{};
  plot << "set xr [-21:-14]\n"
       << "set yr [-12:2]\n"
       << "plot 'pareto.dat' u 4:5 w p lt rgb '#ff3333' pt 13\n";
}
