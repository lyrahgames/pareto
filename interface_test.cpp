#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <concepts>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <ranges>
#include <set>
#include <span>
#include <tuple>
#include <type_traits>
#include <vector>
//
#include <lyrahgames/gpp.hpp>
#include <lyrahgames/meta.hpp>

using namespace std;

template <floating_point real>
struct kursawe_problem {
  static constexpr size_t configurations = 3;
  static constexpr size_t objectives = 2;
  // static constexpr array<real, 2> box[] = {{-5, 5}, {-5, 5}, {-5, 5}};
  static constexpr auto box(size_t i) { return array<real, 2>{-5, 5}; }
  void operator()(const real* x, real* y) {
    using namespace std;
    y[0] = -10 * (exp(real(-0.2) * sqrt(x[0] * x[0] + x[1] * x[1])) +
                  exp(real(-0.2) * sqrt(x[1] * x[1] + x[2] * x[2])));
    y[1] = pow(abs(x[0]), real(0.8)) + 5 * sin(x[0] * x[0] * x[0]) +
           pow(abs(x[1]), real(0.8)) + 5 * sin(x[1] * x[1] * x[1]) +
           pow(abs(x[2]), real(0.8)) + 5 * sin(x[2] * x[2] * x[2]);
  }
};
template <floating_point real>
kursawe_problem<real> kursawe{};

template <floating_point real>
struct poloni2_problem {
  static constexpr size_t configurations = 2;
  static constexpr size_t objectives = 2;
  // static constexpr array<real, 2> box[] = {{-M_PI, M_PI}, {-M_PI, M_PI}};
  auto box(size_t i) { return pair<real, real>{-M_PI, M_PI}; }
  void operator()(const real* x, real* y) {
    using namespace std;
    const auto square = [](auto x) { return x * x; };
    constexpr real a1 = 0.5 * sin(1) - 2 * cos(1) + sin(2) - 1.5 * cos(2);
    constexpr real a2 = 1.5 * sin(1) - cos(1) + 2 * sin(2) - 0.5 * cos(2);
    const auto b1 = real{0.5} * sin(x[0]) - real{2.0} * cos(x[0]) + sin(x[1]) -
                    real{1.5} * cos(x[1]);
    const auto b2 = real{1.5} * sin(x[0]) - cos(x[0]) + real{2.0} * sin(x[1]) -
                    real{0.5} * cos(x[1]);
    y[0] = real{1} + square(a1 - b1) + square(a2 - b2);
    y[1] = square(x[0] + 3) + square(x[1] + 1);
  }
};
template <floating_point real>
poloni2_problem<real> poloni2{};

template <floating_point real>
struct zdt3_problem {
  static constexpr size_t configurations = 30;
  static constexpr size_t objectives = 2;
  auto box(size_t i) { return pair<real, real>{0, 1}; }
  void operator()(const real* x, real* y) {
    using namespace std;

    real g = 0;
    for (size_t i = 1; i < configurations; ++i) g += x[i];
    g = 1 + 9 * g / 29;

    real h = x[0] / g;
    h = 1 - sqrt(h) - h * sin(10 * M_PI * x[0]);

    y[0] = x[0];
    y[1] = h * g;
  }
};
template <floating_point real>
zdt3_problem<real> zdt3{};

template <typename vector>
constexpr auto domination(const vector& x, const vector& y) {
  assert(size(x) == size(y));
  for (size_t i = 0; i < size(x); ++i)
    if (x[i] > y[i]) return false;
  for (size_t i = 0; i < size(x); ++i)
    if (x[i] < y[i]) return true;
  return false;
}

template <std::floating_point real>
inline auto domination(size_t n, real* x, real* y) {
  for (size_t i = 0; i < n; ++i)
    if (x[i] > y[i]) return false;
  for (size_t i = 0; i < n; ++i)
    if (x[i] < y[i]) return true;
  return false;
}

template <typename pareto_problem, typename RNG>
inline auto pareto_optimization(pareto_problem problem, size_t n, RNG&& rng) {
  using namespace std;

  using real = remove_pointer_t<lyrahgames::meta::argument<pareto_problem, 1>>;
  using configurations_type = vector<real>;
  using objectives_type = vector<real>;

  uniform_real_distribution<real> distribution{0, 1};
  set<objectives_type> pareto_set{};

  configurations_type x(problem.configurations);
  objectives_type v(problem.objectives);

  // Use naive Monte-Carlo approach to estimate Pareto front.
  for (size_t i = 0; i < n; ++i) {
    // Get random argument vector in AABB.
    for (size_t k = 0; k < problem.configurations; ++k) {
      const auto random = distribution(rng);
      const auto [a, b] = problem.box(k);
      x[k] = random * a + (1 - random) * b;
    }

    // Check if it is a non-dominated point.
    problem(x.data(), v.data());

    bool to_add = true;
    for (auto it = begin(pareto_set); it != end(pareto_set);) {
      const auto& p = *it;
      if (domination(p, v)) {
        to_add = false;
        break;
      }
      if (domination(v, p))
        it = pareto_set.erase(it);
      else
        ++it;
    }
    if (to_add) pareto_set.insert(v);
  }
  // Copy results into a vector and return it.
  vector<objectives_type> result(pareto_set.size());
  size_t i = 0;
  for (auto& p : pareto_set) {
    result[i] = p;
    ++i;
  }
  return result;
}

namespace nsga2 {
// struct optimizer {
//   size_t initial_size = 100;
//   size_t population_size = 2 * initial_size;
//   std::vector<real> configurations;
//   std::vector<real> objectives;
//   std::vector<real> buffer;
//   std::vector<size_t> permutation;

//   nsga2(){}

//   void create_initial_population(){}

// void populate() {
//   size_t i = 0;
//   for (; i < crossover_count; i += 2) {
//     const auto index1 = random();
//     const auto index2 = random();
//     crossover(permutation[index1], permutation[index2],
//               permutation[initial_size + i],
//               permutation[initial_size + i + 1]);
//   }
//   for (; i < mutation_count; ++i) {
//     const auto index = random();
//     mutate(permutation[index], permutatiton[initial_size + i]);
//   }
// }
// void crossover(size_t index1, size_t index2, size_t store1, size_t store2);
// void mutate(size_t index, size_t store);
// void non_dominated_sort();
// void assign_crowding_distances();
// void advance(size_t iterations) {
//   for (size_t it = 0; it < iterations; ++it) {
//     populate();
//     non_dominated_sort();
//     assign_crowding_distances();
//     sort();           // we sort only the permutation array
//     copy_and_swap();  // copy not needed with permutatiton array
//   }
// }
// };

template <typename pareto_problem, typename RNG,
          typename real =
              remove_pointer_t<lyrahgames::meta::argument<pareto_problem, 1>>>
auto initialize_population(pareto_problem&& problem, RNG&& rng, size_t n,
                           real* configurations, real* objectives) {
  using namespace std;

  uniform_real_distribution<real> distribution{0, 1};
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < problem.configurations; ++j) {
      const auto random = distribution(rng);
      const auto [a, b] = problem.box(j);
      configurations[problem.configurations * i + j] =
          random * a + (1 - random) * b;
    }
    problem(&configurations[problem.configurations * i],
            &objectives[problem.objectives * i]);
  }
}
template <typename pareto_problem, typename RNG>
auto optimization(pareto_problem&& problem, RNG&& rng, size_t n) {
  using namespace std;

  using real = remove_pointer_t<lyrahgames::meta::argument<pareto_problem, 1>>;

  vector<real> configurations(n * problem.configurations);
  vector<real> objectives(n * problem.objectives);

  initialize_population(problem, rng, n, configurations.data(),
                        objectives.data());

  vector<size_t> permutation(n);
  iota(begin(permutation), end(permutation), 0);
  vector<size_t> buffer{};
  buffer.reserve(n);
  vector<size_t> fronts{0};

  // vector<size_t> ranks(n, 0);
  // vector<size_t> dominations(n, 0);
  // vector<vector<size_t>> dominated_sets(n);
  // for (size_t i = 0; i < n; ++i) {
  //   auto p = &objectives[problem.objectives * permutation[i]];
  //   for (size_t j = 0; j < n; ++j) {
  //     auto q = &objectives[problem.objectives * permutation[j]];
  //     if (domination(problem.objectives, p, q))
  //       dominated_sets[i].push_back(j);
  //     else if (domination(problem.objectives, q, p))
  //       ++dominations[i];
  //   }
  //   if (dominations[i] == 0) {
  //     ranks[i] = 1;
  //     buffer.push_back(i);
  //   }
  // }
  // fronts.push_back(buffer.size());
  // size_t i = 0;
  // while (fronts[i + 1] < n) {
  //   for (size_t k = fronts[i]; k < fronts[i + 1]; ++k) {
  //     const auto pid = buffer[k];

  //     for (auto qid : dominated_sets[pid]) {
  //       --dominations[qid];
  //       if (dominations[qid] == 0) {
  //         ranks[qid] = i + 2;
  //         buffer.push_back(qid);
  //       }
  //     }
  //   }
  //   ++i;
  //   fronts.push_back(buffer.size());
  // }
  // for (size_t i = 0; i < buffer.size(); ++i)  //
  //   buffer[i] = permutation[buffer[i]];

  size_t back = n - 1;
  set<size_t> pareto_indices{0};
  for (size_t i = 1; i < n; ++i) {
    auto p = &objectives[problem.objectives * i];
    bool to_add = true;
    auto it = begin(pareto_indices);
    for (; it != end(pareto_indices); ++it) {
      const auto j = *it;
      auto q = &objectives[problem.objectives * j];
      if (domination(problem.objectives, q, p)) {
        to_add = false;
        permutation[back] = i;
        --back;
        break;
      }
      if (domination(problem.objectives, p, q)) {
        it = pareto_indices.erase(it);
        permutation[back] = j;
        --back;
        break;
      }
    }
    if (to_add) {
      for (; it != end(pareto_indices);) {
        const auto j = *it;
        auto q = &objectives[problem.objectives * j];
        if (domination(problem.objectives, p, q)) {
          it = pareto_indices.erase(it);
          permutation[back] = j;
          --back;
        } else
          ++it;
      }
      pareto_indices.insert(i);
    }
  }
  size_t i = 0;
  for (auto j : pareto_indices) {
    permutation[i] = j;
    ++i;
  }
  fronts.push_back(pareto_indices.size());

  return tuple{configurations, objectives, permutation, fronts};
}

}  // namespace nsga2

int main() {
  using namespace std;
  using namespace lyrahgames;

  using real = float;

  mt19937 rng{random_device{}()};

  // auto pareto_front = pareto_optimization(kursawe<float>, 10'000'000, rng);
  // auto pareto_front = pareto_optimization(poloni2<float>, 100'000, rng);
  // auto pareto_front = pareto_optimization(zdt3<float>, 10'000'000, rng);
  // fstream pareto_file{"pareto.dat", ios::out};
  // for (auto& p : pareto_front) pareto_file << p[0] << '\t' << p[1] << '\n';
  // pareto_file << flush;

  // gpp plot{};
  // plot << "plot 'pareto.dat' u 1:2 w p lt rgb '#ff3333' pt 13\n";

  size_t n = 10000;
  auto& problem = kursawe<float>;

  const auto start = chrono::high_resolution_clock::now();

  const auto [pareto_configs, pareto_front, buffer, fronts] =
      nsga2::optimization(problem, rng, n);

  const auto end = chrono::high_resolution_clock::now();
  const auto time = chrono::duration<float>(end - start).count();

  cout << "time = " << time << " s\n";

  fstream pareto_file{"pareto.dat", ios::out};
  for (size_t k = 0; k < fronts.size() - 1; ++k) {
    for (size_t i = fronts[k]; i < fronts[k + 1]; ++i) {
      const auto index = problem.objectives * buffer[i];
      // cout << fronts[0][i] << '\n';
      pareto_file << pareto_front[index + 0] << '\t' << pareto_front[index + 1]
                  << '\n';
    }
    pareto_file << '\n';
  }
  pareto_file << flush;

  gpp plot{};
  plot << "plot 'pareto.dat' u 1:2 w lp lt rgb '#ff3333' pt 13\n";
}