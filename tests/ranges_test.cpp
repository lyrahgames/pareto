#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iomanip>
#include <iostream>
#include <random>
#include <ranges>
#include <span>
#include <vector>

using namespace std;

template <ranges::random_access_range result_range,
          ranges::random_access_range input_range>
constexpr auto kursawe_compute(result_range&& y, const input_range& x) {
  using namespace std;
  using real = ranges::range_value_t<result_range>;

  assert(ranges::size(y) == 2);
  assert(ranges::size(x) == 3);

  // y[0] = -10 * (exp(real(-0.2) * sqrt(x[0] * x[0] + x[1] * x[1])) +
  //               exp(real(-0.2) * sqrt(x[1] * x[1] + x[2] * x[2])));
  // y[1] = pow(abs(x[0]), real(0.8)) + 5 * sin(x[0] * x[0] * x[0]) +
  //        pow(abs(x[1]), real(0.8)) + 5 * sin(x[1] * x[1] * x[1]) +
  //        pow(abs(x[2]), real(0.8)) + 5 * sin(x[2] * x[2] * x[2]);

  y[0] = x[0] + x[1] + x[2];
  y[1] = x[0] * x[1] * x[2];
}

template <ranges::range range>
ostream& operator<<(ostream& os, range r) {
  for (auto it = ranges::begin(r); it != ranges::end(r); ++it)
    os << setw(5) << *it;
  return os << '\n';
}

int main() {
  using real = float;
  const size_t n = 3;

  vector<real> inputs{1, 2, 3, 4, 5, 6, 7, 8, 9};
  vector<real> results(2 * 3);
  for (size_t i = 0; i < n; ++i)
    kursawe_compute(span(results).subspan(2 * i, 2),
                    span(inputs).subspan(3 * i, 3));
  cout << inputs << results;

  // vector<array<real, 3>> inputs{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  // vector<array<real, 2>> results(n);
  // for (size_t i = 0; i < n; ++i) kursawe_compute(results[i], inputs[i]);
  // cout << inputs << results;
}