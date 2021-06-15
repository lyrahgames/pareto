#pragma once
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto {

template <generic::frontier frontier_type,
          generic::frontier_castable<frontier_type> optimizer_type>
auto frontier_cast(optimizer_type&& optimizer) {
  return optimizer.template frontier_cast<frontier_type>();
}

}  // namespace lyrahgames::pareto