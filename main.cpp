#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
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
  mt19937 rng{random_device{}()};
  uniform_real_distribution<real> dist{-5, 5};
  const auto random = [&] { return dist(rng); };

  const int n = 100;
  vector<array<real, 3>> inputs(n);
  vector<array<real, 2>> values(n);
  for (size_t i = 0; i < n; ++i) {
    const array<real, 3> x{random(), random(), random()};
    const auto v = kursawe(x);
    inputs[i] = x;
    values[i] = v;
  }

  vector<size_t> ranks(n, 0);
  vector<size_t> dominations(n, 0);
  vector<vector<size_t>> dominated_sets(n);
  vector<vector<size_t>> fronts(1);
  for (size_t i = 0; i < n; ++i) {
    const auto& p = values[i];
    for (size_t j = 0; j < n; ++j) {
      const auto& q = values[j];
      if (domination(p, q))
        dominated_sets[i].push_back(j);
      else if (domination(q, p))
        ++dominations[i];
    }
    if (dominations[i] == 0) {
      ranks[i] = 1;
      fronts[0].push_back(i);
    }
  }
  size_t i = 0;
  while (!fronts[i].empty()) {
    vector<size_t> buffer{};
    for (auto pid : fronts[i]) {
      for (auto qid : dominated_sets[pid]) {
        --dominations[qid];
        if (dominations[qid] == 0) {
          ranks[qid] = i + 2;
          buffer.push_back(qid);
        }
      }
    }
    ++i;
    fronts.push_back({});
    fronts[i] = buffer;
  }

  vector<array<real, 3>> tmp_inputs(n);
  vector<array<real, 2>> tmp_values(n);
  i = 0;
  for (auto& front : fronts) {
    for (auto index : front) {
      tmp_inputs[i] = inputs[index];
      tmp_values[i] = values[index];
      ++i;
    }
  }
  swap(inputs, tmp_inputs);
  swap(values, tmp_values);

  // vector<array<real, 3>> inputs{};
  // vector<array<real, 2>> values{};

  // {
  //   const auto start = chrono::high_resolution_clock::now();

  //   const size_t n = 300;
  //   for (size_t i = 0; i < n; ++i) {
  //     const float x = 10 * real(i) / (n - 1) - 5;
  //     for (size_t j = 0; j < n; ++j) {
  //       const float y = 10 * real(j) / (n - 1) - 5;
  //       for (size_t k = 0; k < n; ++k) {
  //         const float z = 10 * real(k) / (n - 1) - 5;
  //         const auto v = kursawe({x, y, z});
  //         inputs.push_back({x, y, z});
  //         values.push_back(v);
  //       }
  //     }
  //   }

  //   const auto end = chrono::high_resolution_clock::now();
  //   const auto time = chrono::duration<float>(end - start).count();
  //   cout << "generation " << time << " s\n";
  // }

  vector<size_t> buffer{};
  set<size_t> pareto_indices{0};

  {
    const auto start = chrono::high_resolution_clock::now();

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

    const auto end = chrono::high_resolution_clock::now();
    const auto time = chrono::duration<float>(end - start).count();
    cout << "pareto " << time << " s\n";
  }

  fstream population_file{"population.dat", ios::out};
  fstream pareto_file{"pareto.dat", ios::out};

  // for (size_t i = 0; i < n; ++i) {
  //   const auto& p = values[i];
  //   const auto& x = inputs[i];
  //   population_file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0]
  //                   << '\t' << p[1] << '\n';
  // }

  i = 0;
  for (auto& front : fronts) {
    sort(begin(values) + i, begin(values) + i + front.size());
    for (size_t j = i; j < i + front.size(); ++j) {
      const auto& p = values[j];
      const auto& x = inputs[j];
      population_file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0]
                      << '\t' << p[1] << '\n';
    }
    population_file << '\n';
    i += front.size();
  }

  population_file << flush;
  for (auto index : pareto_indices) {
    const auto& p = values[index];
    const auto& x = inputs[index];
    pareto_file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0] << '\t'
                << p[1] << '\n';
  }
  pareto_file << flush;

  gpp::pipe plot{};
  plot << "plot 'population.dat' u 4:5 w lp lt rgb'#999999', "
          "'pareto.dat' u 4:5 "
          "w p lt rgb '#ff3333'"
          "pt 13\n";
}