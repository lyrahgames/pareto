#pragma once
#include <lyrahgames/meta.hpp>

namespace lyrahgames::pareto {

template <generic::vector vector>
struct aabb {
  vector min;
  vector max;
};

}  // namespace lyrahgames::pareto