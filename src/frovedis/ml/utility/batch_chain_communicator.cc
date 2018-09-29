#include "batch_chain_communicator.hpp"

namespace frovedis {

std::vector<size_t> fix_managed_parameter_range(std::vector<size_t>& count)
{
  auto* ptr_count = count.data();
  std::vector<size_t> count_prefix_sum(count.size());
  auto* ptr_count_prefix_sum = count_prefix_sum.data();

  ptr_count_prefix_sum[0] = ptr_count[0];
  for (size_t i = 1; i < count.size(); i++) {
    ptr_count_prefix_sum[i] = ptr_count_prefix_sum[i-1] + ptr_count[i];
  }
  size_t nodesize = static_cast<size_t>(get_nodesize());
  auto sum_per_node = ceil_div(ptr_count_prefix_sum[count.size() - 1], nodesize);

  std::vector<size_t> managed_borders(nodesize + 1);
  managed_borders[0] = 0;
  for (size_t i_node = 1; i_node < nodesize; i_node++) {
    auto value = sum_per_node * i_node;
    auto it = std::lower_bound(count_prefix_sum.begin(), count_prefix_sum.end(), value);
    managed_borders[i_node] = it - count_prefix_sum.begin();
  }
  managed_borders[nodesize] = count.size();

  return managed_borders;
}

}
