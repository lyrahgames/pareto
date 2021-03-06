#include <algorithm>
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
#include <unordered_set>
#include <vector>
//
#include <lyrahgames/gnuplot_pipe.hpp>
#include <lyrahgames/pareto/meta.hpp>

using namespace std;
using namespace lyrahgames::pareto;

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
    for (size_t i = 1; i < configurations; ++i)
      g += x[i];
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

  using real = remove_pointer_t<meta::argument<pareto_problem, 1>>;
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

template <typename pareto_problem,
          typename RNG,
          typename real = remove_pointer_t<meta::argument<pareto_problem, 1>>>
auto initialize_population(pareto_problem&& problem,
                           RNG&& rng,
                           size_t n,
                           real* configurations,
                           real* objectives) {
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

template <typename real>
inline auto hash_based_non_domination_sort(size_t n,
                                           size_t m,
                                           real* objectives,
                                           size_t* permutation,
                                           size_t select) {
  iota(permutation, permutation + n, 0);
  vector<size_t> fronts{0};
  unordered_set<size_t> pareto_indices{};
  pareto_indices.reserve(n);

  while (fronts.back() < select) {
    pareto_indices.clear();
    pareto_indices.insert(permutation[0]);
    size_t front = 0;

    for (size_t i = 1; i < n - fronts.back(); ++i) {
      const auto index = permutation[i];
      auto p = &objectives[m * index];
      bool to_add = true;
      auto it = begin(pareto_indices);
      for (; it != end(pareto_indices);) {
        const auto j = *it;
        auto q = &objectives[m * j];
        if (domination(m, q, p)) {
          to_add = false;
          permutation[front] = index;
          ++front;
          break;
        }
        if (domination(m, p, q)) {
          it = pareto_indices.erase(it);
          permutation[front] = j;
          ++front;
        } else
          ++it;
      }
      if (to_add) pareto_indices.insert(index);
    }

    // Order points in reverse order. Heuristics to fasten up things.
    for (size_t i = 0; i < front / 2; ++i)
      swap(permutation[i], permutation[front - 1 - i]);

    for (auto j : pareto_indices) {
      permutation[front] = j;
      ++front;
    }
    fronts.push_back(fronts.back() + pareto_indices.size());
  }
  return fronts;
}

template <typename real>
inline auto non_domination_sort(size_t n,
                                size_t m,
                                real* objectives,
                                size_t* permutation) {
  vector<size_t> fronts{0};
  vector<size_t> ranks(n, 0);
  vector<size_t> dominations(n, 0);
  vector<vector<size_t>> dominated_sets(n);
  size_t front = 0;

  for (size_t i = 0; i < n; ++i) {
    auto p = &objectives[m * i];
    for (size_t j = 0; j < n; ++j) {
      auto q = &objectives[m * j];
      if (domination(m, p, q))
        dominated_sets[i].push_back(j);
      else if (domination(m, q, p))
        ++dominations[i];
    }
    if (dominations[i] == 0) {
      ranks[i] = 1;
      permutation[front] = i;
      ++front;
    }
  }
  fronts.push_back(front);
  size_t i = 0;
  while (front < n) {
    for (size_t k = fronts[i]; k < fronts[i + 1]; ++k) {
      const auto pid = permutation[k];

      for (auto qid : dominated_sets[pid]) {
        --dominations[qid];
        if (dominations[qid] == 0) {
          ranks[qid] = i + 2;
          permutation[front] = qid;
          ++front;
        }
      }
    }
    ++i;
    fronts.push_back(front);
  }

  return fronts;
}

template <typename real>
inline auto crowding_distance_sort(size_t n,
                                   size_t m,
                                   real* objectives,
                                   size_t* permutation,
                                   size_t offset,
                                   size_t count,
                                   real* crowding_distances) {
  for (size_t i = 0; i < n; ++i)
    crowding_distances[i] = 0;
  for (size_t v = 0; v < m; ++v) {
    sort(permutation + offset, permutation + offset + count,
         [&](auto x, auto y) {
           return objectives[m * x + v] < objectives[m * y + v];
         });
    crowding_distances[permutation[offset]] = INFINITY;
    crowding_distances[permutation[offset + count - 1]] = INFINITY;
    const auto scale =
        1 / (objectives[m * permutation[offset + count - 1] + v] -
             objectives[m * permutation[offset] + v]);
    for (size_t i = offset + 1; i < offset + count - 1; ++i)
      crowding_distances[permutation[i]] +=
          scale * (objectives[m * permutation[i + 1] + v] -
                   objectives[m * permutation[i - 1] + v]);
  }
  sort(permutation + offset, permutation + offset + count, [&](auto x, auto y) {
    return crowding_distances[x] < crowding_distances[y];
  });
}

template <typename pareto_problem,
          typename RNG,
          typename real = remove_pointer_t<meta::argument<pareto_problem, 1>>>
void simulated_binary_crossover(pareto_problem&& problem,
                                RNG&& rng,
                                real* configurations,
                                real* objectives,
                                size_t parent1,
                                size_t parent2,
                                size_t offspring1,
                                size_t offspring2) {
  using namespace std;

  constexpr real distribution_index = 2;
  constexpr auto distribution_index_coeff = 1 / (distribution_index + 1);

  uniform_real_distribution<real> distribution{0, 1};

  for (size_t i = 0; i < problem.configurations; ++i) {
    const auto random = distribution(rng);
    const auto beta =
        (random <= real(0.5))
            ? (pow(2 * random, distribution_index_coeff))
            : (pow(1 / (2 * (1 - random)), distribution_index_coeff));
    const auto tmp1 =
        real(0.5) *
        ((1 + beta) * configurations[problem.configurations * parent1 + i] +
         (1 - beta) * configurations[problem.configurations * parent2 + i]);
    const auto tmp2 =
        real(0.5) *
        ((1 - beta) * configurations[problem.configurations * parent1 + i] +
         (1 + beta) * configurations[problem.configurations * parent2 + i]);
    const auto [a, b] = problem.box(i);
    configurations[problem.configurations * offspring1 + i] = clamp(tmp1, a, b);
    configurations[problem.configurations * offspring2 + i] = clamp(tmp2, a, b);
  }
  problem(&configurations[problem.configurations * offspring1],
          &objectives[problem.objectives * offspring1]);
  problem(&configurations[problem.configurations * offspring2],
          &objectives[problem.objectives * offspring2]);
}

template <typename pareto_problem,
          typename RNG,
          typename real = remove_pointer_t<meta::argument<pareto_problem, 1>>>
void alternate_random_mutation(pareto_problem&& problem,
                               RNG&& rng,
                               real* configurations,
                               real* objectives,
                               size_t parent,
                               size_t offspring) {
  constexpr real stepsize = 0.1;

  uniform_real_distribution<real> distribution{-1, 1};

  for (size_t k = 0; k < problem.configurations; ++k) {
    const auto random = distribution(rng);
    const auto [a, b] = problem.box(k);
    const auto tmp = configurations[problem.configurations * parent + k] +
                     random * stepsize * (b - a);
    configurations[problem.configurations * offspring + k] = clamp(tmp, a, b);
  }
  problem(&configurations[problem.configurations * offspring],
          &objectives[problem.objectives * offspring]);
}

template <typename pareto_problem,
          typename RNG,
          typename real = remove_pointer_t<meta::argument<pareto_problem, 1>>>
void populate(pareto_problem&& problem,
              RNG&& rng,
              size_t n,
              real* configurations,
              real* objectives,
              size_t* permutation,
              size_t select) {
  using namespace std;

  constexpr real probability_crossover = 0.3;
  const size_t count = n - select;
  const size_t crossovers = 2 * size_t(probability_crossover * (count / 2));

  uniform_int_distribution<size_t> distribution{0, select - 1};

  size_t i = 0;
  for (; i < crossovers; i += 2) {
    const auto parent1 = permutation[n - 1 - distribution(rng)];
    const auto parent2 = permutation[n - 1 - distribution(rng)];
    simulated_binary_crossover(
        problem, rng, configurations, objectives, parent1, parent2,
        permutation[n - 1 - select - i], permutation[n - 1 - select - i - 1]);
  }
  for (; i < count; ++i) {
    const auto parent = permutation[n - 1 - distribution(rng)];
    alternate_random_mutation(problem, rng, configurations, objectives, parent,
                              permutation[n - 1 - select - i]);
  }
}

template <typename pareto_problem, typename RNG>
auto optimization(pareto_problem&& problem, RNG&& rng, size_t n) {
  using namespace std;

  using real = remove_pointer_t<meta::argument<pareto_problem, 1>>;

  vector<real> configurations(n * problem.configurations);
  vector<real> objectives(n * problem.objectives);
  vector<size_t> permutation(n);
  vector<real> crowding_distances(n, 0);
  vector<size_t> fronts;

  initialize_population(problem, rng, n, configurations.data(),
                        objectives.data());

  for (size_t it = 0; it < 1000; ++it) {
    fronts = hash_based_non_domination_sort(
        n, problem.objectives, objectives.data(), permutation.data(), n / 2);

    // auto fronts = non_domination_sort(n, problem.objectives,
    // objectives.data(),
    //                                   permutation.data());

    // for (auto i : permutation) cout << i << '\t';
    // cout << '\n';

    if (fronts.back() > n / 2) {
      const auto offset = n - fronts[fronts.size() - 1];
      const auto count = fronts[fronts.size() - 1] - fronts[fronts.size() - 2];
      crowding_distance_sort(n, problem.objectives, objectives.data(),
                             permutation.data(), offset, count,
                             crowding_distances.data());
    }

    populate(problem, rng, n, configurations.data(), objectives.data(),
             permutation.data(), n / 2);
  }

  fronts = hash_based_non_domination_sort(
      n, problem.objectives, objectives.data(), permutation.data(), n / 2);
  if (fronts.back() > n / 2) {
    const auto offset = n - fronts[fronts.size() - 1];
    const auto count = fronts[fronts.size() - 1] - fronts[fronts.size() - 2];
    crowding_distance_sort(n, problem.objectives, objectives.data(),
                           permutation.data(), offset, count,
                           crowding_distances.data());
  }

  // fronts.back() = n / 2;
  // fronts.push_back(n);
  for (auto i : fronts)
    cout << i << '\t';
  cout << '\n';

  return tuple{configurations, objectives, permutation, fronts,
               crowding_distances};
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

  size_t n = 1000;
  auto& problem = zdt3<float>;

  const auto start = chrono::high_resolution_clock::now();

  auto [pareto_configs, pareto_front, permutation, fronts, distances] =
      nsga2::optimization(problem, rng, n);

  const auto end = chrono::high_resolution_clock::now();
  const auto time = chrono::duration<float>(end - start).count();

  cout << "time = " << time << " s\n" << flush;

  fstream pareto_file{"pareto.dat", ios::out};
  for (size_t k = 0; k < fronts.size() - 1; ++k) {
    // sort(begin(permutation) + fronts[k], begin(permutation) + fronts[k + 1],
    //      [&](auto x, auto y) {
    //        return pareto_front[problem.objectives * x] <=
    //               pareto_front[problem.objectives * y];
    //      });
    sort(begin(permutation) + (n - fronts[k + 1]),
         begin(permutation) + (n - fronts[k]), [&](auto x, auto y) {
           return pareto_front[problem.objectives * x] <=
                  pareto_front[problem.objectives * y];
         });
    for (size_t i = fronts[k]; i < fronts[k + 1]; ++i) {
      // const auto index = problem.objectives * permutation[i];
      const auto index = problem.objectives * permutation[n - 1 - i];
      pareto_file << pareto_front[index + 0] << '\t' << pareto_front[index + 1]
                  << '\t' << distances[permutation[n - 1 - i]] << '\n';
    }
    pareto_file << '\n';
  }
  pareto_file << flush;

  gnuplot_pipe plot{};
  plot << "plot 'pareto.dat' u 1:2 w lp lt rgb '#ff3333' pt 13\n";
  // "'' u 1:2:3 w yerrorbars lt rgb '#999999'\n";
}
