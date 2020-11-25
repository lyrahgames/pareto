#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
//
#include <gpp.hpp>

using namespace std;
using real = float;

constexpr auto kursawe(const array<real, 3>& x) {
  return array<real, 2>{
      real{-10} * (exp(real{-0.2} * sqrt(x[0] * x[0] + x[1] * x[1])) +
                   exp(real{-0.2} * sqrt(x[1] * x[1] + x[2] * x[2]))),
      pow(abs(x[0]), real{0.8}) + 5 * sin(x[0] * x[0] * x[0]) +
          pow(abs(x[1]), real{0.8}) + 5 * sin(x[1] * x[1] * x[1]) +
          pow(abs(x[2]), real{0.8}) + 5 * sin(x[2] * x[2] * x[2])};
}

int main() {
  gpp::pipe plot{};
  fstream file{"tmp.dat", ios::out};

  vector<array<real, 3>> inputs{};
  vector<array<real, 2>> values{};

  const size_t n = 100;
  for (size_t i = 0; i < n; ++i) {
    const float x = 10 * real(i) / (n - 1) - 5;
    for (size_t j = 0; j < n; ++j) {
      const float y = 10 * real(j) / (n - 1) - 5;
      for (size_t k = 0; k < n; ++k) {
        const float z = 10 * real(k) / (n - 1) - 5;
        const auto v = kursawe({x, y, z});
        inputs.push_back({x, y, z});
        values.push_back(v);
      }
    }
  }

  // vector<array<real, 3>> pareto_inputs{};
  // vector<array<real, 2>> pareto_values{};

  for (size_t i = 0; i < values.size(); ++i) {
    const auto& p = values[i];
    bool pareto_dominated = false;
    for (size_t j = 0; j < values.size(); ++j) {
      const auto& q = values[j];
      if ((pareto_dominated = ((q[0] <= p[0]) && (q[1] <= p[1]) &&
                               ((q[0] < p[0]) || (q[1] < p[1])))))
        break;
    }
    if (!pareto_dominated) {
      // pareto_values.push_back(p);
      // pareto_inputs.push_back(inputs[i]);
      const auto& x = inputs[i];
      file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0] << '\t'
           << p[1] << '\n';
    }
  }
  file << flush;

  plot << "plot 'tmp.dat' u 4:5 w p\n";
}