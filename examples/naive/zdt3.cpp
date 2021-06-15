#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
//
#include <lyrahgames/gnuplot_pipe.hpp>
//
#include <lyrahgames/pareto/frontier.hpp>
#include <lyrahgames/pareto/naive.hpp>
//
#include <lyrahgames/pareto/gallery/zdt3.hpp>

using namespace std;
using namespace lyrahgames;
using namespace lyrahgames::pareto;

using real = float;

int main() {
  mt19937 rng{random_device{}()};

  // Choose problem, estimate the Pareto frontier, and cast it to a usable
  // output format in one step.
  const auto pareto_front =
      naive::optimization<frontier<real>>(gallery::zdt3<real>, 10'000'000, rng);

// Plot the data.
#include "../zdt3_plot.ipp"
}