#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
//
#include <lyrahgames/chrono.hpp>
#include <lyrahgames/gpp.hpp>
#include <lyrahgames/pareto/naive.hpp>

template <std::floating_point Real>
constexpr auto pawellek(const std::array<Real, 3>& x) {
  using namespace std;

  const auto tmp = x[0] * x[0] + x[1] * x[1] + x[2] * x[2];
  return array<Real, 3>{
      Real{-10} * (exp(Real{-0.2} * sqrt(x[0] * x[0] + x[1] * x[1])) +
                   exp(Real{-0.2} * sqrt(x[1] * x[1] + x[2] * x[2]))),
      pow(abs(x[0]), Real{0.8}) + 5 * sin(x[0] * x[0] * x[0]) +
          pow(abs(x[1]), Real{0.8}) + 5 * sin(x[1] * x[1] * x[1]) +
          pow(abs(x[2]), Real{0.8}) + 5 * sin(x[2] * x[2] * x[2]),
      Real{0.5} * tmp + sin(tmp)};
}

int main() {
  using namespace std;
  using namespace lyrahgames;
  using namespace pareto;

  using real = float;
  using config_vector = array<real, 3>;
  using object_vector = array<real, 3>;

  mt19937 rng{random_device{}()};
  vector<object_vector> pareto_front{};

  const auto f = pawellek<float>;
  const auto box = aabb<config_vector>{{-3, -3, -3}, {3, 3, 3}};

  const auto t = time([&] {
    pareto_front = monte_carlo_pareto_front(f, box, 10'000'000, rng);
  });
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
