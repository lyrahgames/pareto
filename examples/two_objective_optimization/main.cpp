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
  const auto line_cut = pareto::line_cut(pareto_front);

  const auto end = clock::now();
  const auto time = chrono::duration<double>(end - start).count();
  cout << setw(20) << "time = " << setw(20) << time << " s\n"
       << setw(20) << "mean distance = " << setw(20)
       << line_cut.mean_neighbor_distance() << '\n'
       << setw(20) << "var distance = " << setw(20)
       << line_cut.var_neighbor_distance() << '\n'
       << setw(20) << "stddev distance = " << setw(20)
       << line_cut.stddev_neighbor_distance() << '\n'
       << setw(20) << "line count = " << setw(20) << line_cut.lines().size()
       << '\n';
  ;

  // Store the data into a file for plotting.
  gnuplot::temporary_file file{};

  // for (size_t i = 0; i < pareto_front.sample_count(); ++i) {
  //   for (auto y : pareto_front.objectives(i))
  //     file << setw(20) << y << ' ';
  //   file << '\n';
  // }

  for (const auto& [first, last] : line_cut.lines()) {
    for (size_t i = first; i < last; ++i) {
      for (auto y : pareto_front.objectives(line_cut.permutation(i)))
        file << setw(20) << y << ' ';
      file << '\n';
    }
    file << '\n';
  }

  file.flush();

  // Plot the data.
  gnuplot::pipe plot{};
  plot << "set title '" PLOT_TITLE "'\n"
       << "plot '" << file.path().string()
       << "' u 1:2 w p pt 2 ps 0.5 lt rgb '#222222' title 'Point Estimation', "
          "'' u 1:2 w l lw 2 lt rgb '#ff3333' title 'Frontier Estimation'\n";
}
