#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
//
#include <lyrahgames/gnuplot_pipe.hpp>
//
#include <lyrahgames/pareto/frontier.hpp>
#include <lyrahgames/pareto/nsga2.hpp>
//
#include <lyrahgames/pareto/gallery/kursawe.hpp>
#include <lyrahgames/pareto/gallery/zdt3.hpp>

using namespace std;
using namespace lyrahgames;
using namespace lyrahgames::pareto;

using real = float;

int main() {
  mt19937 rng{random_device{}()};

  const size_t s = 1000;
  // const size_t n = 3;
  const size_t m = 2;
  nsga2::optimizer<gallery::zdt3_problem<real>> optimizer(s);
  optimizer.optimize(rng);

  fstream file{"kursawe_nsga2.dat", ios::out};

  for (size_t k = 0; k < /*optimizer.fronts.size() -*/ 1; ++k) {
    sort(begin(optimizer.permutation) + (s - optimizer.fronts[k + 1]),
         begin(optimizer.permutation) + (s - optimizer.fronts[k]),
         [&](auto x, auto y) {
           return optimizer.objectives[m * x] <= optimizer.objectives[m * y];
         });

    for (size_t i = optimizer.fronts[k]; i < optimizer.fronts[k + 1]; ++i) {
      const auto index = m * optimizer.permutation[s - 1 - i];
      file << optimizer.objectives[index + 0] << '\t'
           << optimizer.objectives[index + 1] << '\t'
           << optimizer.crowding_distances[optimizer.permutation[s - 1 - i]]
           << '\n';
    }
    file << '\n';
  }

  file << flush;

  gnuplot_pipe plot{};
  plot << "plot 'kursawe_nsga2.dat' u 1:2 w lp lt rgb '#ff3333' pt 13, "
          "'' u 1:2:3 w yerrorbars lt rgb '#999999'\n";
}