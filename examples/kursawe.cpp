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
#include <lyrahgames/pareto/gallery/kursawe.hpp>

using namespace std;
using namespace lyrahgames;
using namespace lyrahgames::pareto;

using real = float;

int main() {
  mt19937 rng{random_device{}()};

  // naive::optimizer<gallery::kursawe_problem<real>> optimizer{};
  // optimizer.optimize(10'000'000, rng);
  // const auto pareto_front = frontier_cast<frontier<real>>(optimizer);

  const auto pareto_front = naive::optimization<frontier<real>>(
      gallery::kursawe<real>, 10'000'000, rng);

  // const auto optimizer =
  //     naive::optimization(gallery::kursawe<real>, 10'000'000, rng);
  // const auto pareto_front = frontier_cast<frontier<real>>(optimizer);

  fstream file{"kursawe.dat", ios::out};
  for (size_t i = 0; i < pareto_front.sample_count(); ++i) {
    for (auto x : pareto_front.parameters(i))
      file << setw(20) << x << ' ';
    for (auto y : pareto_front.objectives(i))
      file << setw(20) << y << ' ';
    file << '\n';
  }
  file << flush;

  gnuplot_pipe xplot{};
  xplot << "splot 'kursawe.dat' u 1:2:3 w p lt rgb '#ff3333' pt 13\n";

  gnuplot_pipe yplot{};
  yplot << "plot 'kursawe.dat' u 4:5 w p lt rgb '#ff3333' pt 13\n";
}