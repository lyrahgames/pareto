#include <concepts>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
//
#include <lyrahgames/chrono.hpp>
#include <lyrahgames/gpp.hpp>
#include <lyrahgames/kursawe.hpp>
#include <lyrahgames/pareto_optimization.hpp>

using namespace std;
using namespace lyrahgames;
using real = float;

int main() {
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
