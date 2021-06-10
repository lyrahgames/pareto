#pragma once

namespace lyrahgames::pareto {

namespace generic {

template <typename T, typename U>
concept frontier_castable = requires(T& t) {
  { t.template frontier_cast<U>() } -> identical<U>;
};

}  // namespace generic

template <generic::frontier frontier_type,
          generic::frontier_castable<frontier_type> optimizer_type>
auto frontier_cast(optimizer_type&& optimizer) {
  return optimizer.template frontier_cast<frontier_type>();
}

}  // namespace lyrahgames::pareto