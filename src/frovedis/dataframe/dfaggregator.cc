#include "dftable.hpp"
#include "dfaggregator.hpp"

namespace frovedis {

std::shared_ptr<dfcolumn> 
dfaggregator_sum::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes) {
  auto colp = table.raw_column(col);
  return colp->sum(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_count::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes) {
  auto colp = table.raw_column(col);
  return colp->count(local_grouped_idx, local_idx_split, hash_divide,
                     merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_size::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes) {
  auto colp = table.raw_column(col);
  return colp->size(local_grouped_idx, local_idx_split, hash_divide,
                     merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_avg::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes) {
  auto colp = table.raw_column(col);
  return colp->avg(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_var::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes) {
  auto colp = table.raw_column(col);
  return colp->var(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes, ddof);
}

std::shared_ptr<dfcolumn> 
dfaggregator_sem::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes) {
  auto colp = table.raw_column(col);
  return colp->sem(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes, ddof);
}

std::shared_ptr<dfcolumn> 
dfaggregator_max::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes) {
  auto colp = table.raw_column(col);
  return colp->max(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_min::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes) {
  auto colp = table.raw_column(col);
  return colp->min(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes);
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

std::shared_ptr<dfaggregator> size(const std::string& col) {
  return std::make_shared<dfaggregator_size>(col);
}

std::shared_ptr<dfaggregator> size_as(const std::string& col,
                                       const std::string& as) {
  return std::make_shared<dfaggregator_size>(col,as);
}

std::shared_ptr<dfaggregator> avg(const std::string& col) {
  return std::make_shared<dfaggregator_avg>(col);
}

std::shared_ptr<dfaggregator> avg_as(const std::string& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_avg>(col,as);
}

std::shared_ptr<dfaggregator> var(const std::string& col, const double& ddof) {
  return std::make_shared<dfaggregator_var>(col, ddof);
}

std::shared_ptr<dfaggregator> var_as(const std::string& col,
                                     const std::string& as,
                                     const double& ddof) {
  return std::make_shared<dfaggregator_var>(col, as, ddof);
}

std::shared_ptr<dfaggregator> sem(const std::string& col, const double& ddof) {
  return std::make_shared<dfaggregator_sem>(col, ddof);
}

std::shared_ptr<dfaggregator> sem_as(const std::string& col,
                                     const std::string& as,
                                     const double& ddof) {
  return std::make_shared<dfaggregator_sem>(col, as, ddof);
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
