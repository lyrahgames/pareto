#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
//
#include <lyrahgames/xstd/chrono.hpp>
//
#include <lyrahgames/gnuplot_pipe.hpp>
//
#include <lyrahgames/pareto/frontier.hpp>
#include <lyrahgames/pareto/nsga2.hpp>
//
#include <lyrahgames/pareto/gallery/pawellek.hpp>

using namespace std;
using namespace lyrahgames;
using namespace lyrahgames::pareto;

using real = float;

int main() {
  mt19937 rng{random_device{}()};

  using clock = chrono::high_resolution_clock;
  const auto start = clock::now();

  // Choose the problem from the gallery.
  const auto problem = gallery::pawellek<real>;

  // Estimate the pareto frontier by using the NSGA2 algorithm.
  nsga2::optimizer optimizer(problem, rng,
                             {.iterations = 100,
                              .population = 1000,
                              .kill_ratio = 0.2,
                              .crossover_ratio = 0.1});
  optimizer.optimize(rng);

  // Cast the estimated Pareto frontier to a usable output format.
  const auto pareto_front = frontier_cast<frontier<real>>(optimizer);

  const auto end = clock::now();
  const auto time = chrono::duration<double>(end - start).count();
  cout << setw(20) << "time = " << setw(20) << time << " s\n";

// Plot the data.
#include "../pawellek_plot.ipp"
}
