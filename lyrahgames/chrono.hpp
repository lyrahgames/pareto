#pragma once
#include <chrono>

namespace lyrahgames {

template <typename Functor>
inline auto time(Functor f) {
  const auto start = std::chrono::high_resolution_clock::now();
  f();
  const auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<float>(end - start).count();
}

}  // namespace lyrahgames