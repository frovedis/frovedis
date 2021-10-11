#ifndef DFAGGREGATOR_HPP
#define DFAGGREGATOR_HPP

#include "dfcolumn.hpp"

namespace frovedis {

class dftable_base;

struct dfaggregator {
  dfaggregator(const std::string& col, const std::string& as) :
    has_as(true), col(col), as(as) {}
  dfaggregator(const std::string& col) : has_as(false), col(col) {}
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes) = 0;
  bool has_as;
  std::string col;
  std::string as;
};

struct dfaggregator_sum : public dfaggregator {
  dfaggregator_sum(const std::string& col, const std::string& as) :
    dfaggregator(col,as) {}
  dfaggregator_sum(const std::string& col) : dfaggregator(col) {}
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
};

struct dfaggregator_count : public dfaggregator {
  dfaggregator_count(const std::string& col, const std::string& as) :
    dfaggregator(col,as) {}
  dfaggregator_count(const std::string& col) : dfaggregator(col) {}
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
};

struct dfaggregator_size : public dfaggregator {
  dfaggregator_size(const std::string& col, const std::string& as) :
    dfaggregator(col,as) {}
  dfaggregator_size(const std::string& col) : dfaggregator(col) {}
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
};

struct dfaggregator_avg : public dfaggregator {
  dfaggregator_avg(const std::string& col, const std::string& as) :
    dfaggregator(col,as) {}
  dfaggregator_avg(const std::string& col) : dfaggregator(col) {}
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
};

struct dfaggregator_max : public dfaggregator {
  dfaggregator_max(const std::string& col, const std::string& as) :
    dfaggregator(col,as) {}
  dfaggregator_max(const std::string& col) : dfaggregator(col) {}
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
};

struct dfaggregator_min : public dfaggregator {
  dfaggregator_min(const std::string& col, const std::string& as) :
    dfaggregator(col,as) {}
  dfaggregator_min(const std::string& col) : dfaggregator(col) {}
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
};

std::shared_ptr<dfaggregator>
sum(const std::string& col);

std::shared_ptr<dfaggregator>
sum_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
count(const std::string& col);

std::shared_ptr<dfaggregator>
count_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
size(const std::string& col);

std::shared_ptr<dfaggregator>
size_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
avg(const std::string& col);

std::shared_ptr<dfaggregator>
avg_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
max(const std::string& col);

std::shared_ptr<dfaggregator>
max_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
min(const std::string& col);

std::shared_ptr<dfaggregator>
min_as(const std::string& col, const std::string& as);

}
#endif
