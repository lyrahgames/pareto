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

  const auto pareto_front =
      naive::optimization<frontier<real>>(gallery::zdt3<real>, 10'000'000, rng);

  fstream file{"zdt3.dat", ios::out};
  for (size_t i = 0; i < pareto_front.sample_count(); ++i) {
    for (auto y : pareto_front.objectives(i))
      file << setw(20) << y << ' ';
    file << '\n';
  }
  file << flush;

  gnuplot_pipe yplot{};
  yplot << "plot 'zdt3.dat' u 1:2 w p lt rgb '#ff3333' pt 13\n";
}