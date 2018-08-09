#include "dftable.hpp"
#include "dfaggregator.hpp"

namespace frovedis {

std::shared_ptr<dfcolumn> 
dfaggregator_sum::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& grouped_idx,
          node_local<std::vector<size_t>>& idx_split,
          node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
          node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto colp = table.column(col);
  return colp->sum(grouped_idx, idx_split, partitioned_idx, exchanged_idx);
}

std::shared_ptr<dfcolumn> 
dfaggregator_count::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& grouped_idx,
          node_local<std::vector<size_t>>& idx_split,
          node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
          node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto colp = table.column(col);
  return colp->count(grouped_idx, idx_split, partitioned_idx, exchanged_idx);
}

std::shared_ptr<dfcolumn> 
dfaggregator_avg::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& grouped_idx,
          node_local<std::vector<size_t>>& idx_split,
          node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
          node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto colp = table.column(col);
  return colp->avg(grouped_idx, idx_split, partitioned_idx, exchanged_idx);
}

std::shared_ptr<dfcolumn> 
dfaggregator_max::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& grouped_idx,
          node_local<std::vector<size_t>>& idx_split,
          node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
          node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto colp = table.column(col);
  return colp->max(grouped_idx, idx_split, partitioned_idx, exchanged_idx);
}

std::shared_ptr<dfcolumn> 
dfaggregator_min::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& grouped_idx,
          node_local<std::vector<size_t>>& idx_split,
          node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
          node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto colp = table.column(col);
  return colp->min(grouped_idx, idx_split, partitioned_idx, exchanged_idx);
}

std::shared_ptr<dfaggregator> sum(const std::string& col) {
  return std::make_shared<dfaggregator_sum>(col);
}

std::shared_ptr<dfaggregator> sum_as(const std::string& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_sum>(col,as);
}

std::shared_ptr<dfaggregator> count(const std::string& col) {
  return std::make_shared<dfaggregator_count>(col);
}

std::shared_ptr<dfaggregator> count_as(const std::string& col,
                                       const std::string& as) {
  return std::make_shared<dfaggregator_count>(col,as);
}

std::shared_ptr<dfaggregator> avg(const std::string& col) {
  return std::make_shared<dfaggregator_avg>(col);
}

std::shared_ptr<dfaggregator> avg_as(const std::string& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_avg>(col,as);
}

std::shared_ptr<dfaggregator> max(const std::string& col) {
  return std::make_shared<dfaggregator_max>(col);
}

std::shared_ptr<dfaggregator> max_as(const std::string& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_max>(col,as);
}

std::shared_ptr<dfaggregator> min(const std::string& col) {
  return std::make_shared<dfaggregator_min>(col);
}

std::shared_ptr<dfaggregator> min_as(const std::string& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_min>(col,as);
}

}
