#include <concepts>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <typeinfo>
#include <vector>
//
#include <lyrahgames/chrono.hpp>
#include <lyrahgames/gpp.hpp>
#include <lyrahgames/meta.hpp>
#include <lyrahgames/nsga2.hpp>
#include <lyrahgames/pareto/gallery/kursawe.hpp>
#include <lyrahgames/pareto/gallery/poloni2.hpp>
#include <lyrahgames/pareto/gallery/schaffer.hpp>
#include <lyrahgames/pareto/naive.hpp>

int main() {
  using namespace std;
  using namespace lyrahgames;
  using namespace pareto;

  using real = float;
  using config_vector = array<real, 3>;
  using object_vector = array<real, 2>;

  mt19937 rng{random_device{}()};
  vector<object_vector> pareto_front{};
  // const auto f = schaffer<float>;
  // const auto f = poloni2<array<float, 2>, array<float, 2>>;
  // const auto box = aabb<array<float, 2>>{{-M_PI, -M_PI}, {M_PI, M_PI}};

  const auto t = time([&] {
    pareto_front = monte_carlo_pareto_front(
        kursawe<float>, aabb<array<float, 3>>{{-5, -5, -5}, {5, 5, 5}},
        10'000'000, rng);
    // pareto_front = monte_carlo_pareto_front(f, aabb<array<float, 1>>{{-5},
    // {5}},
    //                                         1000, rng);
    // pareto_front = monte_carlo_pareto_front(f, box, 100'000, rng);
  });
  cout << "Computation took " << t << "s for " << pareto_front.size()
       << " points on the pareto front."
       << "\n";

  fstream pareto_file{"pareto.dat", ios::out};
  for (auto& p : pareto_front) pareto_file << p[0] << '\t' << p[1] << '\n';
  pareto_file << flush;

  gpp plot{};
  plot << "plot 'pareto.dat' u 1:2 w p lt rgb '#ff3333' pt 13\n";
}
