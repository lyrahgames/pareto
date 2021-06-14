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

  nsga2::optimizer optimizer(gallery::viennet<real>, 1000);
  optimizer.optimize(1000, rng);
  const auto pareto_front = frontier_cast<frontier<real>>(optimizer);

  // const auto pareto_front =
  //     naive::optimization<frontier<real>>(gallery::viennet<real>, 10'000,
  //     rng);

  fstream file{"viennet.dat", ios::out};
  for (size_t i = 0; i < pareto_front.sample_count(); ++i) {
    for (auto x : pareto_front.parameters(i))
      file << setw(20) << x << ' ';
    for (auto y : pareto_front.objectives(i))
      file << setw(20) << y << ' ';
    file << '\n';
  }
  file << flush;

  gnuplot_pipe xplot{};
  xplot << "set title \"Viennet Parameters\"\n"
           "plot 'viennet.dat' u 1:2 w p lt rgb '#ff3333' pt 13\n";

  gnuplot_pipe yplot{};
  yplot << "set title \"Viennet Objectives\"\n"
           "splot 'viennet.dat' u 3:4:5 w p lt rgb '#ff3333' pt 13\n";

  string wait;
  getline(cin, wait);
}