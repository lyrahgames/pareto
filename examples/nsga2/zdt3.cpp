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
#include <lyrahgames/pareto/gallery/zdt3.hpp>

using namespace std;
using namespace lyrahgames;
using namespace lyrahgames::pareto;

using real = float;

int main() {
  mt19937 rng{random_device{}()};

  using clock = chrono::high_resolution_clock;
  const auto start = clock::now();

  // Choose problem, estimate the Pareto frontier, and cast it to a usable
  // output format in one step.
  const auto pareto_front = nsga2::optimization<frontier<real>>(
      gallery::zdt3<real>, rng, {.iterations = 1000, .population = 1000});

  const auto end = clock::now();
  const auto time = chrono::duration<double>(end - start).count();
  cout << setw(20) << "time = " << setw(20) << time << " s\n";

// Plot the data.
#include "../zdt3_plot.ipp"
}