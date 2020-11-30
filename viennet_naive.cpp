#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
//
#include <lyrahgames/chrono.hpp>
#include <lyrahgames/gpp.hpp>
#include <lyrahgames/pareto_optimization.hpp>
#include <lyrahgames/viennet.hpp>

int main() {
  using namespace std;
  using namespace lyrahgames;

  using real = float;
  using config_vector = array<real, 2>;
  using object_vector = array<real, 3>;

  mt19937 rng{random_device{}()};
  vector<object_vector> pareto_front{};

  const auto f = viennet<float>;
  const auto box = aabb<config_vector>{{-3, -3}, {3, 3}};

  const auto t = time(
      [&] { pareto_front = monte_carlo_pareto_front(f, box, 10'000, rng); });
  cout << "Computation took " << t << "s for " << pareto_front.size()
       << " points on the pareto front."
       << "\n";

  fstream pareto_file{"pareto.dat", ios::out};
  for (auto& p : pareto_front)
    pareto_file << p[0] << '\t' << p[1] << '\t' << p[2] << '\n';
  pareto_file << flush;

  gpp plot{};
  plot << "splot 'pareto.dat' u 1:2:3 w p lt rgb '#ff3333' pt 13\n";
  string input;
  getline(cin, input);
}
