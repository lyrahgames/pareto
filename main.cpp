#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <thread>
#include <vector>
//
#include <gpp.hpp>

using namespace std;
using namespace chrono_literals;
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

struct nsga2 {
  void make_new_population() {
    // crossover
    size_t i = 0;
    uniform_int_distribution<size_t> population_dist{0, m - 1};
    for (; i < m / 2; i += 2) {
      auto x = inputs[population_dist(rng)];
      auto y = inputs[population_dist(rng)];
      // const auto crossings = uniform_int_distribution<size_t>{1, 3 - 1}(rng);
      const size_t crossings = 1;  // odd number ensures results are different
      for (size_t j = 0; j < crossings; ++j) {
        const auto index = uniform_int_distribution<size_t>{0, 3 - 1}(rng);
        swap(x[index], y[index]);
      }
      auto x_value = kursawe(x);
      auto y_value = kursawe(y);
      inputs[m + i + 0] = x;
      inputs[m + i + 1] = y;
      values[m + i + 0] = x_value;
      values[m + i + 1] = y_value;
    }
    // mutation
    for (; i < m; ++i) {
      auto x = inputs[population_dist(rng)];
      constexpr real stepsize = 0.1 * 5;
      for (size_t k = 0; k < x.size(); ++k) {
        const auto step = uniform_real_distribution<real>{-1, 1}(rng);
        x[k] += step * stepsize;
      }
      inputs[m + i] = x;
      values[m + i] = kursawe(x);
    }
  }

  void initialize() {
    uniform_real_distribution<real> dist{-5, 5};
    for (size_t i = 0; i < m; ++i) {
      const array<real, 3> x{dist(rng), dist(rng), dist(rng)};
      const auto v = kursawe(x);
      inputs[i] = x;
      values[i] = v;
    }
    make_new_population();
  }

  void non_dominated_sort() {
    fronts.resize(1);
    fronts.front().resize(0);

    vector<size_t> ranks(n, 0);
    vector<size_t> dominations(n, 0);
    vector<vector<size_t>> dominated_sets(n);

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
  }

  void assign_crowding_distances() {
    for (size_t i = 0; i < n; ++i) crowding_distances[i] = 0;
    for (auto& front : fronts) {
      if (front.empty()) break;
      for (size_t v = 0; v < 2; ++v) {
        sort(begin(front), end(front),
             [&](auto x, auto y) { return values[x][v] < values[y][v]; });
        crowding_distances[front.front()] = INFINITY;
        crowding_distances[front.back()] = INFINITY;
        for (size_t i = 1; i < front.size() - 1; ++i)
          crowding_distances[front[i]] +=
              (values[front[i + 1]][v] - values[front[i - 1]][v]) /
              (values[front.back()][v] - values[front.front()][v]);
      }
    }
  }

  void advance() {
    non_dominated_sort();
    assign_crowding_distances();
    size_t last_front_index = 0;
    size_t s = fronts.front().size();
    while (s < m) {
      ++last_front_index;
      s += fronts[last_front_index].size();
    }
    if (s != m) {
      sort(begin(fronts[last_front_index]), end(fronts[last_front_index]),
           [&](auto x, auto y) {
             return crowding_distances[x] > crowding_distances[y];
           });
    }

    vector<array<real, 3>> tmp_inputs(n);
    vector<array<real, 2>> tmp_values(n);
    size_t index = 0;
    for (size_t i = 0; i < last_front_index; ++i) {
      for (auto j : fronts[i]) {
        tmp_inputs[index] = inputs[j];
        tmp_values[index] = values[j];
        ++index;
      }
    }
    for (size_t j = 0; index < m; ++index, ++j) {
      tmp_inputs[index] = inputs[fronts[last_front_index][j]];
      tmp_values[index] = values[fronts[last_front_index][j]];
    }
    swap(inputs, tmp_inputs);
    swap(values, tmp_values);

    make_new_population();
  }

  nsga2() : inputs(n), values(n), crowding_distances(n) { initialize(); }

  mt19937 rng{random_device{}()};
  size_t m = 100 * 4;
  size_t n = 2 * m;
  vector<array<real, 3>> inputs;
  vector<array<real, 2>> values;
  vector<vector<size_t>> fronts;
  vector<real> crowding_distances;
};

int main() {
  // mt19937 rng{random_device{}()};
  // uniform_real_distribution<real> dist{-5, 5};
  // const auto random = [&] { return dist(rng); };

  // const int n = 5 * 4;
  // vector<array<real, 3>> inputs(n);
  // vector<array<real, 2>> values(n);
  // for (size_t i = 0; i < n; ++i) {
  //   const array<real, 3> x{random(), random(), random()};
  //   const auto v = kursawe(x);
  //   inputs[i] = x;
  //   values[i] = v;
  // }

  // vector<size_t> ranks(n, 0);
  // vector<size_t> dominations(n, 0);
  // vector<vector<size_t>> dominated_sets(n);
  // vector<vector<size_t>> fronts(1);
  // for (size_t i = 0; i < n; ++i) {
  //   const auto& p = values[i];
  //   for (size_t j = 0; j < n; ++j) {
  //     const auto& q = values[j];
  //     if (domination(p, q))
  //       dominated_sets[i].push_back(j);
  //     else if (domination(q, p))
  //       ++dominations[i];
  //   }
  //   if (dominations[i] == 0) {
  //     ranks[i] = 1;
  //     fronts[0].push_back(i);
  //   }
  // }
  // size_t i = 0;
  // while (!fronts[i].empty()) {
  //   vector<size_t> buffer{};
  //   for (auto pid : fronts[i]) {
  //     for (auto qid : dominated_sets[pid]) {
  //       --dominations[qid];
  //       if (dominations[qid] == 0) {
  //         ranks[qid] = i + 2;
  //         buffer.push_back(qid);
  //       }
  //     }
  //   }
  //   ++i;
  //   fronts.push_back({});
  //   fronts[i] = buffer;
  // }

  // vector<real> crowding_distances(n, 0);
  // for (auto& front : fronts) {
  //   if (front.empty()) break;
  //   for (size_t v = 0; v < 2; ++v) {
  //     sort(begin(front), end(front),
  //          [&](auto x, auto y) { return values[x][v] < values[y][v]; });
  //     crowding_distances[front.front()] = INFINITY;
  //     crowding_distances[front.back()] = INFINITY;
  //     for (size_t i = 1; i < front.size() - 1; ++i)
  //       crowding_distances[front[i]] +=
  //           (values[front[i + 1]][v] - values[front[i - 1]][v]) /
  //           (values[front.back()][v] - values[front.front()][v]);
  //   }
  // }

  // vector<array<real, 3>> offspring_inputs(n);
  // vector<array<real, 2>> offspring_values(n);
  // // crossover
  // i = 0;
  // uniform_int_distribution<size_t> population_dist{0, n - 1};
  // for (; i < n / 2; i += 2) {
  //   auto x = inputs[population_dist(rng)];
  //   auto y = inputs[population_dist(rng)];
  //   // const auto crossings = uniform_int_distribution<size_t>{1, 3 -
  //   1}(rng); const size_t crossings = 1;  // odd number ensures results are
  //   different for (size_t j = 0; j < crossings; ++j) {
  //     const auto index = uniform_int_distribution<size_t>{0, 3 - 1}(rng);
  //     swap(x[index], y[index]);
  //   }
  //   auto x_value = kursawe(x);
  //   auto y_value = kursawe(y);
  //   offspring_inputs[i + 0] = x;
  //   offspring_inputs[i + 1] = y;
  //   offspring_values[i + 0] = x_value;
  //   offspring_values[i + 1] = y_value;
  // }
  // // mutation
  // for (; i < n; ++i) {
  //   auto x = inputs[population_dist(rng)];
  //   constexpr real stepsize = 0.1 * 5;
  //   for (size_t k = 0; k < x.size(); ++k) {
  //     const auto step = uniform_real_distribution<real>{-1, 1}(rng);
  //     x[k] += step * stepsize;
  //   }
  //   offspring_inputs[i] = x;
  //   offspring_values[i] = kursawe(x);
  // }

  // // combine
  // vector<array<real, 3>> combined_inputs(2 * n);
  // vector<array<real, 2>> combined_values(2 * n);
  // for (size_t i = 0; i < n; ++i) {
  //   combined_inputs[i] = inputs[i];
  //   combined_inputs[i + n] = offspring_inputs[i];
  //   combined_values[i] = values[i];
  //   combined_values[i + n] = offspring_values[i];
  // }
  // non-dominated sort

  // vector<array<real, 3>> tmp_inputs(n);
  // vector<array<real, 2>> tmp_values(n);
  // i = 0;
  // for (auto& front : fronts) {
  //   for (auto index : front) {
  //     tmp_inputs[i] = inputs[index];
  //     tmp_values[i] = values[index];
  //     ++i;
  //   }
  // }
  // swap(inputs, tmp_inputs);
  // swap(values, tmp_values);

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

  // vector<size_t> buffer{};
  // set<size_t> pareto_indices{0};

  // {
  //   const auto start = chrono::high_resolution_clock::now();

  //   for (size_t i = 1; i < values.size(); ++i) {
  //     const auto& v = values[i];
  //     bool to_add = true;
  //     buffer.resize(0);
  //     for (auto index : pareto_indices) {
  //       const auto& p = values[index];
  //       if (domination(p, v)) {
  //         to_add = false;
  //         break;
  //       }
  //       if (domination(v, p)) buffer.push_back(index);
  //     }
  //     for (auto index : buffer) pareto_indices.erase(index);
  //     if (to_add) pareto_indices.insert(i);
  //   }

  //   const auto end = chrono::high_resolution_clock::now();
  //   const auto time = chrono::duration<float>(end - start).count();
  //   cout << "pareto " << time << " s\n";
  // }

  // fstream population_file{"population.dat", ios::out};
  // fstream offspring_file{"offspring.dat", ios::out};
  // fstream pareto_file{"pareto.dat", ios::out};

  // // i = 0;
  // for (auto& front : fronts) {
  //   // sort(begin(values) + i, begin(values) + i + front.size());
  //   // for (size_t j = i; j < i + front.size(); ++j) {
  //   for (auto j : front) {
  //     const auto& p = values[j];
  //     const auto& x = inputs[j];
  //     const auto& d = crowding_distances[j];
  //     population_file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0]
  //                     << '\t' << p[1] << '\t' << d << '\n';
  //   }
  //   population_file << '\n';
  //   // i += front.size();
  // }
  // population_file << flush;

  // for (size_t i = 0; i < n; ++i) {
  //   const auto& p = offspring_values[i];
  //   const auto& x = offspring_inputs[i];
  //   offspring_file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0]
  //                  << '\t' << p[1] << '\n';
  // }
  // offspring_file << flush;

  // for (auto index : pareto_indices) {
  //   const auto& p = values[index];
  //   const auto& x = inputs[index];
  //   pareto_file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0] <<
  //   '\t'
  //               << p[1] << '\n';
  // }
  // pareto_file << flush;

  // gpp::pipe plot{};
  // plot << "plot 'population.dat' u 4:5 w l lt rgb '#999999', "  //
  //         "'' u 4:5:6 w yerrorbars lt rgb '#999999', "          //
  //         "'offspring.dat' u 4:5 w p lt rgb '#9999ff', "        //
  //         "'pareto.dat' u 4:5 w p lt rgb '#ff3333' pt 13\n";    //

  nsga2 moo{};
  gpp::pipe plot{};
  gpp::pipe plot2{};
  while (true) {
    moo.advance();
    moo.non_dominated_sort();
    moo.assign_crowding_distances();
    cout << "pareto front size = " << moo.fronts.front().size() << '\n';

    fstream population_file{"population.dat", ios::out};
    for (auto& front : moo.fronts) {
      for (auto j : front) {
        const auto& p = moo.values[j];
        const auto& x = moo.inputs[j];
        const auto& d = moo.crowding_distances[j];
        population_file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0]
                        << '\t' << p[1] << '\t' << d << '\n';
      }
      population_file << '\n';
    }
    population_file << flush;

    fstream pareto_file{"pareto.dat", ios::out};
    for (auto index : moo.fronts.front()) {
      const auto& p = moo.values[index];
      const auto& x = moo.inputs[index];
      pareto_file << x[0] << '\t' << x[1] << '\t' << x[2] << '\t' << p[0]
                  << '\t' << p[1] << '\n';
    }
    pareto_file << flush;

    plot << "set xr [-21:-14]\n"
         << "set yr [-12:2]\n"
         << "plot 'population.dat' u 4:5 w l lt rgb '#999999', "  //
            "'' u 4:5:6 w yerrorbars lt rgb '#999999', "          //
            "'pareto.dat' u 4:5 w p lt rgb '#ff3333' pt 13\n";
    plot2 << "set xr [-5:5]\n"
          << "set yr [-5:5]\n"
          << "set zr [-5:5]\n"
          << "splot 'pareto.dat' u 1:2:3 w lines\n";
    this_thread::sleep_for(50ms);
  }
}