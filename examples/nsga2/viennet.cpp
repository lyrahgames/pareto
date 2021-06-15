#include <chrono>
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
#include <lyrahgames/pareto/gallery/viennet.hpp>

using namespace std;
using namespace lyrahgames;
using namespace lyrahgames::pareto;

using real = float;

int main() {
  mt19937 rng{random_device{}()};

  using clock = chrono::high_resolution_clock;
  const auto start = clock::now();

  // Choose the problem from the gallery.
  const auto problem = gallery::viennet<real>;

  // Estimate the pareto frontier by using the NSGA2 algorithm.
  nsga2::optimizer optimizer(problem, rng, {.population = 1000});
  optimizer.optimize(rng, 1000);

  // Cast the estimated Pareto frontier to a usable output format.
  const auto pareto_front = frontier_cast<frontier<real>>(optimizer);

  const auto end = clock::now();
  const auto time = chrono::duration<double>(end - start).count();
  cout << setw(20) << "time = " << setw(20) << time << " s\n";

// Plot the data.
#include "../viennet_plot.ipp"
}