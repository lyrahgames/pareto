#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
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

constexpr auto domination(const array<real, 2>& x, const array<real, 2>& y) {
  return (x[0] <= y[0]) && (x[1] <= y[1]) && ((x[0] < y[0]) || (x[1] < y[1]));
  // return ((x[0] < y[0]) && (x[1] <= y[1])) || ((x[0] <= y[0]) && (x[1] <
  // y[1]));
}

int main() {
  gpp::pipe plot{};
  fstream file{"tmp.dat", ios::out};

  vector<array<real, 3>> inputs{};
  vector<array<real, 2>> values{};

  {
    const auto start = chrono::high_resolution_clock::now();

    const size_t n = 300;
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

    const auto end = chrono::high_resolution_clock::now();
    const auto time = chrono::duration<float>(end - start).count();
    cout << "generation " << time << " s\n";
  }

  {
    const auto start = chrono::high_resolution_clock::now();

    vector<size_t> buffer{};
    set<size_t> pareto_indices{0};

    for (size_t i = 1; i < values.size(); ++i) {
      const auto& v = values[i];
      bool to_add = true;
      buffer.resize(0);
      for (auto index : pareto_indices) {
        const auto& p = values[index];
        if (domination(p, v)) {
          to_add = false;
          break;
        }
        if (domination(v, p)) buffer.push_back(index);
      }
      for (auto index : buffer) pareto_indices.erase(index);
      if (to_add) pareto_indices.insert(i);
    }

    for (auto index : pareto_indices) {
      const auto& p = values[index];
      const auto& x = inputs[index];
      file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0] << '\t'
           << p[1] << '\n';
    }

    file << flush;

    const auto end = chrono::high_resolution_clock::now();
    const auto time = chrono::duration<float>(end - start).count();
    cout << "pareto " << time << " s\n";
  }

  plot << "plot 'tmp.dat' u 4:5 w p\n";
}