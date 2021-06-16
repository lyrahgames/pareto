#pragma once
#include <lyrahgames/pareto/meta.hpp>

namespace lyrahgames::pareto {

/// After an optimization, this function casts the respective optimizer to the
/// frontier structure given as template argument. The resulting Pareto frontier
/// can then be easily read from the frontier structure.
/// To enable this function, the optimizer has to provide a templatized member
/// function 'frontier_cast'.
template <generic::frontier frontier_type,
          generic::frontier_castable<frontier_type> optimizer_type>
auto frontier_cast(optimizer_type&& optimizer) {
  return optimizer.template frontier_cast<frontier_type>();
}

}  // namespace lyrahgames::pareto