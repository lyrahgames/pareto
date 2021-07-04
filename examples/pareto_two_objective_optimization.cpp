#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
//
#include <lyrahgames/gnuplot/gnuplot.hpp>
#include <lyrahgames/pareto/gallery/gallery.hpp>
#include <lyrahgames/pareto/pareto.hpp>

using namespace std;
using namespace lyrahgames;

using real = float;

int main() {
  mt19937 rng{random_device{}()};

  using clock = chrono::high_resolution_clock;
  const auto start = clock::now();

  // Choose problem, estimate the Pareto frontier, and cast it to a usable
  // output format in one step.
  const auto problem = PROBLEM;
  const auto pareto_front = OPTIMIZATION;

  const auto end = clock::now();
  const auto time = chrono::duration<double>(end - start).count();
  cout << setw(20) << "time = " << setw(20) << time << " s\n";

  // Store the data into a file for plotting.
  gnuplot::temporary_file file{};
  for (size_t i = 0; i < pareto_front.sample_count(); ++i) {
    for (auto y : pareto_front.objectives(i))
      file << setw(20) << y << ' ';
    file << '\n';
  }
  file.flush();

  // Plot the data.
  gnuplot::pipe plot{};
  plot << "plot '" << file.path().string()
       << "' u 1:2 w p lt rgb '#ff3333' pt 13\n";
}
