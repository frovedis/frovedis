#ifndef DFCOLUMN_HPP
#define DFCOLUMN_HPP

#include <string>
#include <memory>
#include <tuple>
#include "../core/dvector.hpp"
#include "../core/dunordered_map.hpp"
#include "../core/vector_operations.hpp"
#include "hashtable.hpp"
#include "join.hpp"
#include "../text/dict.hpp"
#include "../text/parsedatetime.hpp"

#define DFNODESHIFT 48 // used to concatenate node id and local index

namespace frovedis {

class dfcolumn {
public:
  virtual ~dfcolumn(){}
  virtual size_t size() = 0;
  virtual std::vector<size_t> sizes() = 0;
  virtual void debug_print() = 0;
  virtual dvector<std::string> as_string() = 0;
  virtual node_local<words> as_words(size_t precision = 6,
                                     const std::string& datetime_fmt = "%Y-%m-%d",
                                     bool quote_escape = false,
                                     const std::string& nullstr = "NULL") = 0;
  virtual node_local<std::vector<size_t>>
  filter_eq(std::shared_ptr<dfcolumn>& right) = 0;
  virtual node_local<std::vector<size_t>>
  filter_neq(std::shared_ptr<dfcolumn>& right) = 0;
  virtual node_local<std::vector<size_t>>
  filter_lt(std::shared_ptr<dfcolumn>& right) = 0;
  virtual node_local<std::vector<size_t>>
  filter_le(std::shared_ptr<dfcolumn>& right) = 0;
  virtual node_local<std::vector<size_t>>
  filter_gt(std::shared_ptr<dfcolumn>& right) = 0;
  virtual node_local<std::vector<size_t>>
  filter_ge(std::shared_ptr<dfcolumn>& right) = 0;
  virtual node_local<std::vector<size_t>>
  filter_is_null() = 0;
  virtual node_local<std::vector<size_t>>
  filter_is_not_null() = 0;
  virtual std::shared_ptr<dfcolumn>
  extract(node_local<std::vector<size_t>>& idx) = 0;
  virtual std::shared_ptr<dfcolumn>
  global_extract(node_local<std::vector<size_t>>&,
                 node_local<std::vector<size_t>>&,
                 node_local<std::vector<std::vector<size_t>>>&) = 0;
  virtual std::shared_ptr<dfcolumn>
  sort(node_local<std::vector<size_t>>&) = 0;
  virtual std::shared_ptr<dfcolumn>
  sort_desc(node_local<std::vector<size_t>>&) = 0;
  virtual std::shared_ptr<dfcolumn>
  sort_with_idx(node_local<std::vector<size_t>>&,
                node_local<std::vector<size_t>>&) = 0;
  virtual std::shared_ptr<dfcolumn>
  sort_with_idx_desc(node_local<std::vector<size_t>>&,
                     node_local<std::vector<size_t>>&) = 0;
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    hash_join_eq(std::shared_ptr<dfcolumn>& right,
                 node_local<std::vector<size_t>>& left_full_local_idx, 
                 node_local<std::vector<size_t>>& right_full_local_idx) = 0;
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
    outer_hash_join_eq(std::shared_ptr<dfcolumn>& right,
                       node_local<std::vector<size_t>>& left_full_local_idx, 
                       node_local<std::vector<size_t>>& right_full_local_idx)
    = 0;
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_eq(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx) = 0;
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_lt(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx) = 0;
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_le(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx) = 0;
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_gt(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx) = 0;
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_ge(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx) = 0;
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
    outer_bcast_join_eq(std::shared_ptr<dfcolumn>& right,
                        node_local<std::vector<size_t>>& left_full_local_idx, 
                        node_local<std::vector<size_t>>& right_full_local_idx)
    = 0;
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    star_join_eq(std::shared_ptr<dfcolumn>& right,
                 node_local<std::vector<size_t>>& left_full_local_idx, 
                 node_local<std::vector<size_t>>& right_full_local_idx) = 0;
  virtual node_local<std::vector<size_t>> get_local_index() = 0;
  // append nulls created by outer join
  virtual void append_nulls(node_local<std::vector<size_t>>& to_append) = 0;
  virtual std::shared_ptr<dfcolumn> group_by
  (node_local<std::vector<size_t>>& local_idx,
   node_local<std::vector<size_t>>& split_idx,
   node_local<std::vector<std::vector<size_t>>>& hash_divide,
   node_local<std::vector<std::vector<size_t>>>& merge_map) = 0;
  // for group by of multiple columns
  virtual void
  multi_group_by_sort(node_local<std::vector<size_t>>& local_idx) = 0;
  virtual node_local<std::vector<size_t>>
  multi_group_by_sort_split(node_local<std::vector<size_t>>& local_idx) = 0;
  virtual node_local<std::vector<size_t>>
  multi_group_by_split(node_local<std::vector<size_t>>& local_idx) = 0;
  virtual std::shared_ptr<dfcolumn> multi_group_by_extract
  (node_local<std::vector<size_t>>& local_idx,
   node_local<std::vector<size_t>>& split_idx,
   bool check_nulls) = 0;
  virtual node_local<std::vector<size_t>>
  calc_hash_base() = 0;
  virtual void
  calc_hash_base(node_local<std::vector<size_t>>& hash, int shift) = 0;
  virtual std::shared_ptr<dfcolumn> 
  multi_group_by_exchange(node_local<std::vector<std::vector<size_t>>>&
                          hash_divide) = 0;
  // for grouped_dftable
  virtual std::shared_ptr<dfcolumn>
  sum(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) = 0;
  virtual std::shared_ptr<dfcolumn>
  count(node_local<std::vector<size_t>>& local_grouped_idx,
        node_local<std::vector<size_t>>& local_idx_split,
        node_local<std::vector<std::vector<size_t>>>& hash_divide,
        node_local<std::vector<std::vector<size_t>>>& merge_map,
        node_local<size_t>& row_sizes) = 0;
  virtual std::shared_ptr<dfcolumn>
  avg(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) = 0;
  virtual std::shared_ptr<dfcolumn>
  max(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) = 0;
  virtual std::shared_ptr<dfcolumn>
  min(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) = 0;
  // for whole column
  virtual size_t count() = 0; // exclude null
  template <class T> T sum();
  virtual double avg() = 0;
  template <class T> T max();
  template <class T> T min();
  template <class T> dvector<T> as_dvector();
  // cast to float/double; throw exception when string 
  virtual dvector<float> as_dvector_float() = 0; 
  virtual dvector<double> as_dvector_double() = 0;
  virtual std::shared_ptr<dfcolumn> head(size_t limit) = 0;
  virtual std::shared_ptr<dfcolumn> tail(size_t limit) = 0;
  virtual bool is_string() {return false;}
  virtual std::string dtype() const = 0;
  virtual void save(const std::string& file) = 0;
  virtual void contain_nulls_check() = 0;
  virtual node_local<std::vector<size_t>> get_nulls() = 0;
};

template <class T>
class typed_dfcolumn : public dfcolumn {
public:
  typed_dfcolumn() : contain_nulls(false) {}
  typed_dfcolumn(const dvector<T>& dv) : contain_nulls(false) {
    auto dv2 = dv;
    val = dv2.moveto_node_local();
    nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  typed_dfcolumn(dvector<T>&& dv) : contain_nulls(false) {
    val = dv.moveto_node_local();
    nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  typed_dfcolumn(node_local<std::vector<T>>&& val, 
                 node_local<std::vector<size_t>>&& nulls) :
    val(std::move(val)), nulls(std::move(nulls)) {
    contain_nulls_check();
  }
  typed_dfcolumn(node_local<std::vector<T>>& val, 
                 node_local<std::vector<size_t>>& nulls) :
    val(val), nulls(nulls) {
    contain_nulls_check();
  }
  virtual size_t size();
  virtual std::vector<size_t> sizes();
  virtual dvector<std::string> as_string();
  virtual node_local<words> as_words(size_t precision = 6,
                                     const std::string& datetime_fmt = "%Y-%m-%d",
                                     bool escape = true,
                                     const std::string& nullstr = "NULL");
  virtual node_local<std::vector<size_t>>
  filter_eq(std::shared_ptr<dfcolumn>& right);
  node_local<std::vector<size_t>>
  filter_eq_immed(const T& right);
  virtual node_local<std::vector<size_t>>
  filter_neq(std::shared_ptr<dfcolumn>& right);
  node_local<std::vector<size_t>>
  filter_neq_immed(const T& right);
  virtual node_local<std::vector<size_t>>
  filter_lt(std::shared_ptr<dfcolumn>& right);
  node_local<std::vector<size_t>>
  filter_lt_immed(const T& right);
  virtual node_local<std::vector<size_t>>
  filter_le(std::shared_ptr<dfcolumn>& right);
  node_local<std::vector<size_t>>
  filter_le_immed(const T& right);
  virtual node_local<std::vector<size_t>>
  filter_gt(std::shared_ptr<dfcolumn>& right);
  node_local<std::vector<size_t>>
  filter_gt_immed(const T& right);
  virtual node_local<std::vector<size_t>>
  filter_ge(std::shared_ptr<dfcolumn>& right);
  node_local<std::vector<size_t>>
  filter_ge_immed(const T& right);
  virtual node_local<std::vector<size_t>>
  filter_is_null();
  virtual node_local<std::vector<size_t>>
  filter_is_not_null();
  virtual std::shared_ptr<dfcolumn>
  extract(node_local<std::vector<size_t>>& idx);
  virtual std::shared_ptr<dfcolumn>
  global_extract(node_local<std::vector<size_t>>& global_idx,
                 node_local<std::vector<size_t>>& to_store_idx,
                 node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
  std::shared_ptr<dfcolumn> sort(node_local<std::vector<size_t>>&);
  std::shared_ptr<dfcolumn> sort_desc(node_local<std::vector<size_t>>&);
  std::shared_ptr<dfcolumn>
  sort_with_idx(node_local<std::vector<size_t>>&,
                node_local<std::vector<size_t>>& );
  std::shared_ptr<dfcolumn>
  sort_with_idx_desc(node_local<std::vector<size_t>>&,
                     node_local<std::vector<size_t>>&);
  virtual node_local<std::vector<size_t>> get_local_index();
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    hash_join_eq(std::shared_ptr<dfcolumn>& right,
                 // might be filtered index
                 node_local<std::vector<size_t>>& left_full_local_idx, 
                 node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
    outer_hash_join_eq(std::shared_ptr<dfcolumn>& right,
                       // might be filtered index
                       node_local<std::vector<size_t>>& left_full_local_idx, 
                       node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_eq(std::shared_ptr<dfcolumn>& right,
                 // might be filtered index
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_lt(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_le(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_gt(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_ge(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
    outer_bcast_join_eq(std::shared_ptr<dfcolumn>& right,
                        // might be filtered index
                        node_local<std::vector<size_t>>& left_full_local_idx, 
                        node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    star_join_eq(std::shared_ptr<dfcolumn>& right,
                 // might be filtered index
                 node_local<std::vector<size_t>>& left_full_local_idx, 
                 node_local<std::vector<size_t>>& right_full_local_idx);
  virtual void append_nulls(node_local<std::vector<size_t>>& to_append);
  virtual std::shared_ptr<dfcolumn> group_by
  (node_local<std::vector<size_t>>& local_idx,
   node_local<std::vector<size_t>>& split_idx,
   node_local<std::vector<std::vector<size_t>>>& hash_divide,
   node_local<std::vector<std::vector<size_t>>>& merge_map);
  virtual void
  multi_group_by_sort(node_local<std::vector<size_t>>& local_idx);
  virtual node_local<std::vector<size_t>>
  multi_group_by_sort_split(node_local<std::vector<size_t>>& local_idx);
  virtual node_local<std::vector<size_t>>
  multi_group_by_split(node_local<std::vector<size_t>>& local_idx);
  virtual std::shared_ptr<dfcolumn> multi_group_by_extract
  (node_local<std::vector<size_t>>& local_idx,
   node_local<std::vector<size_t>>& split_idx,
   bool check_nulls);
  virtual node_local<std::vector<size_t>>
  calc_hash_base();
  virtual void
  calc_hash_base(node_local<std::vector<size_t>>& hash, int shift);
  virtual std::shared_ptr<dfcolumn> 
  multi_group_by_exchange(node_local<std::vector<std::vector<size_t>>>&
                          hash_divide);
  virtual std::shared_ptr<dfcolumn>
  sum(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn>
  count(node_local<std::vector<size_t>>& local_grouped_idx,
        node_local<std::vector<size_t>>& local_idx_split,
        node_local<std::vector<std::vector<size_t>>>& hash_divide,
        node_local<std::vector<std::vector<size_t>>>& merge_map,
        node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn>
  avg(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn>
  max(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn>
  min(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes);
  virtual size_t count();
  T sum();
  virtual double avg();
  T max();
  T min();
  virtual dvector<float> as_dvector_float(); 
  virtual dvector<double> as_dvector_double();
  virtual void debug_print();
  virtual std::string dtype() const;
  virtual void save(const std::string& file);
  virtual std::shared_ptr<dfcolumn> head(size_t limit);
  virtual std::shared_ptr<dfcolumn> tail(size_t limit);
  virtual void contain_nulls_check();
  node_local<std::vector<T>>& get_val(){return val;}
  virtual node_local<std::vector<size_t>> get_nulls(){return nulls;}
  node_local<std::vector<T>> val;
  node_local<std::vector<size_t>> nulls;
  bool contain_nulls;
};

template <class T>
void reset_null_val(const std::vector<T>& nulls,
                    std::vector<T>& val) {
  auto valp = val.data();
  auto nullp = nulls.data();
  auto tmax = std::numeric_limits<T>::max();
#pragma _NEC ivdep
  for(size_t i = 0; i < nulls.size(); ++i) valp[nullp[i]] = tmax;
}

template <>
class typed_dfcolumn<std::string> : public dfcolumn {
public:
  typed_dfcolumn() : contain_nulls(false) {}
  typed_dfcolumn(dvector<std::string>& dv) : contain_nulls(false)
    {auto nl = dv.viewas_node_local(); init(nl);}
  typed_dfcolumn(dvector<std::string>&& dv) : contain_nulls(false)
    {auto nl = dv.moveto_node_local(); init(nl);}
  typed_dfcolumn(node_local<std::vector<std::string>>& dv,
                 node_local<std::vector<size_t>>& nulls_) {
    init(dv); nulls = nulls_; contain_nulls_check();
    if(contain_nulls) nulls.mapv(reset_null_val<size_t>, val);
  }
  typed_dfcolumn(node_local<std::vector<std::string>>&& dv,
                 node_local<std::vector<size_t>>&& nulls_) {
    init(dv); nulls = std::move(nulls_); contain_nulls_check();
    if(contain_nulls) nulls.mapv(reset_null_val<size_t>, val);
  }
  virtual size_t size();
  virtual std::vector<size_t> sizes();
  virtual node_local<std::vector<size_t>>
  filter_eq(std::shared_ptr<dfcolumn>& right);
  node_local<std::vector<size_t>>
  filter_eq_immed(const std::string& right);
  virtual node_local<std::vector<size_t>>
  filter_neq(std::shared_ptr<dfcolumn>& right);
  node_local<std::vector<size_t>>
  filter_neq_immed(const std::string& right);
  virtual node_local<std::vector<size_t>>
  filter_lt(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with lt for string");
  }
  virtual node_local<std::vector<size_t>>
  filter_le(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with le for string");
  }
  virtual node_local<std::vector<size_t>>
  filter_gt(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with gt for string");
  }
  virtual node_local<std::vector<size_t>>
  filter_ge(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with ge for string");
  }
  virtual node_local<std::vector<size_t>>
  filter_is_null();
  virtual node_local<std::vector<size_t>>
  filter_is_not_null();
  node_local<std::vector<size_t>>
  filter_regex(const std::string& pattern);
  node_local<std::vector<size_t>>
  filter_not_regex(const std::string& pattern);
  virtual std::shared_ptr<dfcolumn>
  extract(node_local<std::vector<size_t>>& idx);
  virtual std::shared_ptr<dfcolumn>
  global_extract(node_local<std::vector<size_t>>& global_idx,
                 node_local<std::vector<size_t>>& to_store_idx,
                 node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
  virtual std::shared_ptr<dfcolumn>
  sort(node_local<std::vector<size_t>>& idx);
  virtual std::shared_ptr<dfcolumn>
  sort_desc(node_local<std::vector<size_t>>& idx);
  virtual std::shared_ptr<dfcolumn>
  sort_with_idx(node_local<std::vector<size_t>>& idx,
                node_local<std::vector<size_t>>& ret_idx);
  virtual std::shared_ptr<dfcolumn>
  sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                     node_local<std::vector<size_t>>& ret_idx);
  virtual void debug_print();
  virtual node_local<std::vector<size_t>> get_nulls();
  node_local<std::vector<std::string>> get_val();
  virtual dvector<std::string> as_string();
  virtual node_local<words> as_words(size_t precision = 6,
                                     const std::string& datetime_fmt = "%Y-%m-%d",
                                     bool quote_escape = false,
                                     const std::string& nullstr = "NULL");
  virtual node_local<std::vector<size_t>> get_local_index();
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  hash_join_eq(std::shared_ptr<dfcolumn>& right,
               node_local<std::vector<size_t>>& left_full_local_idx, 
               node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
  outer_hash_join_eq(std::shared_ptr<dfcolumn>& right,
                     node_local<std::vector<size_t>>& left_full_local_idx, 
                     node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  bcast_join_eq(std::shared_ptr<dfcolumn>& right,
                node_local<std::vector<size_t>>& left_full_local_idx, 
                node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_lt(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx){
    throw std::runtime_error("bcast_join with lt for string");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_le(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx){
    throw std::runtime_error("bcast_join with le for string");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_gt(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx){
    throw std::runtime_error("bcast_join with gt for string");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_ge(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx){
    throw std::runtime_error("bcast_join with ge for string");
  }
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
  outer_bcast_join_eq(std::shared_ptr<dfcolumn>& right,
                      node_local<std::vector<size_t>>& left_full_local_idx, 
                      node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  star_join_eq(std::shared_ptr<dfcolumn>& right,
               node_local<std::vector<size_t>>& left_full_local_idx, 
               node_local<std::vector<size_t>>& right_full_local_idx);
  virtual void append_nulls(node_local<std::vector<size_t>>& to_append);
  virtual std::shared_ptr<dfcolumn> group_by
  (node_local<std::vector<size_t>>& local_idx,
   node_local<std::vector<size_t>>& split_idx,
   node_local<std::vector<std::vector<size_t>>>& hash_divide,
   node_local<std::vector<std::vector<size_t>>>& merge_map);
  virtual void
  multi_group_by_sort(node_local<std::vector<size_t>>& local_idx);
  virtual node_local<std::vector<size_t>>
  multi_group_by_sort_split(node_local<std::vector<size_t>>& local_idx);
  virtual node_local<std::vector<size_t>>
  multi_group_by_split(node_local<std::vector<size_t>>& local_idx);
  virtual std::shared_ptr<dfcolumn> multi_group_by_extract
  (node_local<std::vector<size_t>>& local_idx,
   node_local<std::vector<size_t>>& split_idx,
   bool check_nulls);
  virtual node_local<std::vector<size_t>>
  calc_hash_base();
  virtual void
  calc_hash_base(node_local<std::vector<size_t>>& hash, int shift);
  virtual std::shared_ptr<dfcolumn> 
  multi_group_by_exchange(node_local<std::vector<std::vector<size_t>>>&
                          hash_divide);
  virtual std::shared_ptr<dfcolumn>
  sum(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("sum of string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  count(node_local<std::vector<size_t>>& local_grouped_idx,
        node_local<std::vector<size_t>>& local_idx_split,
        node_local<std::vector<std::vector<size_t>>>& hash_divide,
        node_local<std::vector<std::vector<size_t>>>& merge_map,
        node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn>
  avg(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("avg of string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  max(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("max of string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  min(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("min of string is not defined");
  }
  virtual size_t count();
  std::string sum() {
    throw std::runtime_error("sum of string is not defined");
  }
  virtual double avg() {
    throw std::runtime_error("avg of string is not defined");
  }
  std::string max() {
    throw std::runtime_error("max of string is not defined");
  }
  std::string min() {
    throw std::runtime_error("min of string is not defined");
  }
  virtual dvector<float> as_dvector_float() {
    throw std::runtime_error("as_dvector_float of string is not defined");
  } 
  virtual dvector<double> as_dvector_double() {
    throw std::runtime_error("as_dvector_double of string is not defined");
  }
  virtual std::string dtype() const {return std::string("string");}
  virtual void save(const std::string& file);
  virtual bool is_string() {return true;}
  void init(node_local<std::vector<std::string>>& nl);
  typed_dfcolumn<size_t> sort_prepare();
  node_local<std::vector<size_t>> equal_prepare
  (std::shared_ptr<typed_dfcolumn<std::string>>&);
  virtual void contain_nulls_check();
  virtual std::shared_ptr<dfcolumn> head(size_t limit);
  virtual std::shared_ptr<dfcolumn> tail(size_t limit);
  // string -> idx; shared between columns
  std::shared_ptr<dunordered_map<std::string, size_t>> dic;
  // idx -> string; shared between columns
  std::shared_ptr<node_local<std::vector<std::string>>> dic_idx; 
  node_local<std::vector<size_t>> val;
  node_local<std::vector<size_t>> nulls;
  bool contain_nulls;
};

struct dic_string {}; // for tag

template <>
class typed_dfcolumn<dic_string> : public dfcolumn {
public:
  typed_dfcolumn() : contain_nulls(false) {}
  typed_dfcolumn(node_local<words>& ws) : contain_nulls(false) {init(ws);}
  typed_dfcolumn(node_local<words>&& ws) : contain_nulls(false) {init(ws);}
  typed_dfcolumn(node_local<words>& ws,
                 node_local<std::vector<size_t>>& nulls) :
    nulls(nulls) {init(ws, false); contain_nulls_check();}
  typed_dfcolumn(node_local<words>&& ws,
                 node_local<std::vector<size_t>>&& nulls) :
    nulls(std::move(nulls)) {init(ws, false); contain_nulls_check();}
  typed_dfcolumn(node_local<compressed_words>& ws) : contain_nulls(false)
    {init_compressed(ws);}
  typed_dfcolumn(node_local<compressed_words>&& ws) : contain_nulls(false)
    {init_compressed(ws);}
  typed_dfcolumn(node_local<compressed_words>& ws,
                 node_local<std::vector<size_t>>& nulls) :
    nulls(nulls) {init_compressed(ws, false); contain_nulls_check();}
  typed_dfcolumn(node_local<compressed_words>&& ws,
                 node_local<std::vector<size_t>>&& nulls)
    : nulls(std::move(nulls))
    {init_compressed(ws, false); contain_nulls_check();}
  virtual size_t size(); 
  virtual std::vector<size_t> sizes(); 
  virtual node_local<std::vector<size_t>>
  filter_eq(std::shared_ptr<dfcolumn>& right);
  node_local<std::vector<size_t>>
  filter_eq_immed(const std::string& right);
  virtual node_local<std::vector<size_t>>
  filter_neq(std::shared_ptr<dfcolumn>& right);
  node_local<std::vector<size_t>>
  filter_neq_immed(const std::string& right);
  virtual node_local<std::vector<size_t>>
  filter_lt(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with lt for dic_string");
  }
  virtual node_local<std::vector<size_t>>
  filter_le(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with le for dic_string");
  }
  virtual node_local<std::vector<size_t>>
  filter_gt(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with gt for dic_string");
  }
  virtual node_local<std::vector<size_t>>
  filter_ge(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with ge for dic_string");
  }
  virtual node_local<std::vector<size_t>>
  filter_is_null();
  virtual node_local<std::vector<size_t>>
  filter_is_not_null();
  node_local<std::vector<size_t>>
  filter_like(const std::string& pattern, int wild_card = '%');
  node_local<std::vector<size_t>>
  filter_not_like(const std::string& pattern, int wild_card = '%');
  virtual std::shared_ptr<dfcolumn>
  extract(node_local<std::vector<size_t>>& idx);
  virtual std::shared_ptr<dfcolumn>
  global_extract(node_local<std::vector<size_t>>& global_idx,
                 node_local<std::vector<size_t>>& to_store_idx,
                 node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
  virtual std::shared_ptr<dfcolumn>
  sort(node_local<std::vector<size_t>>& idx);
  virtual std::shared_ptr<dfcolumn>
  sort_desc(node_local<std::vector<size_t>>& idx);
  virtual std::shared_ptr<dfcolumn>
  sort_with_idx(node_local<std::vector<size_t>>& idx,
                node_local<std::vector<size_t>>& ret_idx);
  virtual std::shared_ptr<dfcolumn>
  sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                     node_local<std::vector<size_t>>& ret_idx);
  virtual void debug_print();
  virtual node_local<std::vector<size_t>> get_nulls(){return nulls;}
  node_local<std::vector<dic_string>> get_val() {
    throw std::runtime_error("get_val is not defined for dic_string");
  }
  virtual dvector<std::string> as_string() {
    throw std::runtime_error("as_string is obsolete");
  }
  virtual node_local<words> as_words(size_t precision = 6,
                                     const std::string& datetime_fmt = "%Y-%m-%d",
                                     bool quote_escape = false,
                                     const std::string& nullstr = "NULL");
  virtual node_local<std::vector<size_t>> get_local_index();
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  hash_join_eq(std::shared_ptr<dfcolumn>& right,
               node_local<std::vector<size_t>>& left_full_local_idx, 
               node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
  outer_hash_join_eq(std::shared_ptr<dfcolumn>& right,
                     node_local<std::vector<size_t>>& left_full_local_idx, 
                     node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  bcast_join_eq(std::shared_ptr<dfcolumn>& right,
                node_local<std::vector<size_t>>& left_full_local_idx, 
                node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_lt(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx){
    throw std::runtime_error("bcast_join with lt for dic_string");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_le(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx){
    throw std::runtime_error("bcast_join with le for dic_string");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_gt(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx){
    throw std::runtime_error("bcast_join with gt for dic_string");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_ge(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx){
    throw std::runtime_error("bcast_join with ge for dic_string");
  }
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
  outer_bcast_join_eq(std::shared_ptr<dfcolumn>& right,
                      node_local<std::vector<size_t>>& left_full_local_idx, 
                      node_local<std::vector<size_t>>& right_full_local_idx);
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  star_join_eq(std::shared_ptr<dfcolumn>& right,
               node_local<std::vector<size_t>>& left_full_local_idx, 
               node_local<std::vector<size_t>>& right_full_local_idx);
  virtual void append_nulls(node_local<std::vector<size_t>>& to_append);
  virtual std::shared_ptr<dfcolumn> group_by
  (node_local<std::vector<size_t>>& local_idx,
   node_local<std::vector<size_t>>& split_idx,
   node_local<std::vector<std::vector<size_t>>>& hash_divide,
   node_local<std::vector<std::vector<size_t>>>& merge_map);
  virtual void
  multi_group_by_sort(node_local<std::vector<size_t>>& local_idx);
  virtual node_local<std::vector<size_t>>
  multi_group_by_sort_split(node_local<std::vector<size_t>>& local_idx);
  virtual node_local<std::vector<size_t>>
  multi_group_by_split(node_local<std::vector<size_t>>& local_idx);
  virtual std::shared_ptr<dfcolumn> multi_group_by_extract
  (node_local<std::vector<size_t>>& local_idx,
   node_local<std::vector<size_t>>& split_idx,
   bool check_nulls);
  virtual node_local<std::vector<size_t>>
  calc_hash_base();
  virtual void
  calc_hash_base(node_local<std::vector<size_t>>& hash, int shift);
  virtual std::shared_ptr<dfcolumn> 
  multi_group_by_exchange(node_local<std::vector<std::vector<size_t>>>&
                          hash_divide);
  virtual std::shared_ptr<dfcolumn>
  sum(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("sum of dic_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  count(node_local<std::vector<size_t>>& local_grouped_idx,
        node_local<std::vector<size_t>>& local_idx_split,
        node_local<std::vector<std::vector<size_t>>>& hash_divide,
        node_local<std::vector<std::vector<size_t>>>& merge_map,
        node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn>
  avg(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("avg of dic_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  max(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("max of dic_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  min(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("min of dic_string is not defined");
  }
  virtual size_t count();
  dic_string sum() {
    throw std::runtime_error("sum of dic_string is not defined");
  }
  virtual double avg() {
    throw std::runtime_error("avg of dic_string is not defined");
  }
  dic_string max() {
    throw std::runtime_error("max of dic_string is not defined");
  }
  dic_string min() {
    throw std::runtime_error("min of dic_string is not defined");
  }
  virtual dvector<float> as_dvector_float() {
    throw std::runtime_error("as_dvector_float of dic_string is not defined");
  } 
  virtual dvector<double> as_dvector_double() {
    throw std::runtime_error("as_dvector_double of dic_string is not defined");
  }
  virtual std::string dtype() const {return std::string("dic_string");}
  virtual void save(const std::string& file);
  virtual bool is_string() {return true;} // to check cachable in sort
  void init(node_local<words>& ws, bool allocate_nulls = true);
  void init_compressed(node_local<compressed_words>& ws,
                       bool allocate_nulls = true);
  typed_dfcolumn<size_t> sort_prepare();
  node_local<std::vector<size_t>> equal_prepare
  (std::shared_ptr<typed_dfcolumn<dic_string>>&);
  virtual void contain_nulls_check();
  virtual std::shared_ptr<dfcolumn> head(size_t limit);
  virtual std::shared_ptr<dfcolumn> tail(size_t limit);
  // dictionary is shared between columns; all node have the same dic
  std::shared_ptr<node_local<dict>> dic;
  node_local<std::vector<size_t>> val;
  node_local<std::vector<size_t>> nulls;
  bool contain_nulls;
};

struct raw_string {}; // for tag

template <>
class typed_dfcolumn<raw_string> : public dfcolumn {
public:
  typed_dfcolumn() : contain_nulls(false) {}
  typed_dfcolumn(node_local<words>& ws) : contain_nulls(false) {init(ws);}
  typed_dfcolumn(node_local<words>&& ws) : contain_nulls(false) {init(ws);}
  typed_dfcolumn(node_local<words>& ws,
                 node_local<std::vector<size_t>>& nulls) :
    nulls(nulls) {init(ws, false); contain_nulls_check();}
  typed_dfcolumn(node_local<words>&& ws,
                 node_local<std::vector<size_t>>&& nulls) :
    nulls(std::move(nulls)) {init(ws, false); contain_nulls_check();}
  typed_dfcolumn(node_local<compressed_words>& ws) : contain_nulls(false)
    {init_compressed(ws);}
  typed_dfcolumn(node_local<compressed_words>&& ws) : contain_nulls(false)
    {init_compressed(std::move(ws));}
  typed_dfcolumn(node_local<compressed_words>& ws,
                 node_local<std::vector<size_t>>& nulls) :
    nulls(nulls) {init_compressed(ws, false); contain_nulls_check();}
  typed_dfcolumn(node_local<compressed_words>&& ws,
                 node_local<std::vector<size_t>>&& nulls)
    : nulls(std::move(nulls))
    {init_compressed(std::move(ws), false); contain_nulls_check();}
  virtual size_t size(); 
  virtual std::vector<size_t> sizes(); 
  virtual node_local<std::vector<size_t>>
  filter_eq(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with eq for raw_string");
  }
  // TODO: efficient implementation of filter_[n]eq_immed
  node_local<std::vector<size_t>>
  filter_eq_immed(const std::string& right) {return filter_like(right,0);}
  virtual node_local<std::vector<size_t>>
  filter_neq(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with neq for raw_string");
  }
  node_local<std::vector<size_t>>
  filter_neq_immed(const std::string& right) {return filter_not_like(right,0);}
  virtual node_local<std::vector<size_t>>
  filter_lt(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with lt for raw_string");
  }
  virtual node_local<std::vector<size_t>>
  filter_le(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with le for raw_string");
  }
  virtual node_local<std::vector<size_t>>
  filter_gt(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with gt for raw_string");
  }
  virtual node_local<std::vector<size_t>>
  filter_ge(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with ge for raw_string");
  }
  virtual node_local<std::vector<size_t>>
  filter_is_null();
  virtual node_local<std::vector<size_t>>
  filter_is_not_null();
  node_local<std::vector<size_t>>
  filter_like(const std::string& pattern, int wild_card = '%');
  node_local<std::vector<size_t>>
  filter_not_like(const std::string& pattern, int wild_card = '%');
  virtual std::shared_ptr<dfcolumn>
  extract(node_local<std::vector<size_t>>& idx);
  virtual std::shared_ptr<dfcolumn>
  global_extract(node_local<std::vector<size_t>>& global_idx,
                 node_local<std::vector<size_t>>& to_store_idx,
                 node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
  virtual std::shared_ptr<dfcolumn>
  sort(node_local<std::vector<size_t>>& idx) {
    throw std::runtime_error("sort is not defined for raw_string");
  }
  virtual std::shared_ptr<dfcolumn>
  sort_desc(node_local<std::vector<size_t>>& idx) {
    throw std::runtime_error("sort_desc is not defined for raw_string");
  }
  virtual std::shared_ptr<dfcolumn>
  sort_with_idx(node_local<std::vector<size_t>>& idx,
                node_local<std::vector<size_t>>& ret_idx) {
    throw std::runtime_error("sort_with_idx is not defined for raw_string");
  }
  virtual std::shared_ptr<dfcolumn>
  sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                     node_local<std::vector<size_t>>& ret_idx) {
    throw std::runtime_error("sort_with_idx_desc is not defined for raw_string");
  }
  virtual void debug_print();
  virtual node_local<std::vector<size_t>> get_nulls(){return nulls;}
  node_local<std::vector<raw_string>> get_val() {
    throw std::runtime_error("get_val is not defined for raw_string");
  }
  virtual dvector<std::string> as_string() {
    throw std::runtime_error("as_string is obsolete");
  }
  virtual node_local<words> as_words(size_t precision = 6,
                                     const std::string& datetime_fmt = "%Y-%m-%d",
                                     bool quote_escape = false,
                                     const std::string& nullstr = "NULL");
  virtual node_local<std::vector<size_t>> get_local_index();
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  hash_join_eq(std::shared_ptr<dfcolumn>& right,
               node_local<std::vector<size_t>>& left_full_local_idx, 
               node_local<std::vector<size_t>>& right_full_local_idx) {
    throw std::runtime_error("join is not defined for raw_string");
  }
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
  outer_hash_join_eq(std::shared_ptr<dfcolumn>& right,
                     node_local<std::vector<size_t>>& left_full_local_idx, 
                     node_local<std::vector<size_t>>& right_full_local_idx) {
    throw std::runtime_error("join is not defined for raw_string");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  bcast_join_eq(std::shared_ptr<dfcolumn>& right,
                node_local<std::vector<size_t>>& left_full_local_idx, 
                node_local<std::vector<size_t>>& right_full_local_idx) {
    throw std::runtime_error("join is not defined for raw_string");
  }
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
  outer_bcast_join_eq(std::shared_ptr<dfcolumn>& right,
                      node_local<std::vector<size_t>>& left_full_local_idx, 
                      node_local<std::vector<size_t>>& right_full_local_idx) {
    throw std::runtime_error("join is not defined for raw_string");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_lt(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx){
    throw std::runtime_error("join is not defined for raw_string");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_le(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx){
    throw std::runtime_error("join is not defined for raw_string");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_gt(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx){
    throw std::runtime_error("join is not defined for raw_string");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join_ge(std::shared_ptr<dfcolumn>& right,
                  node_local<std::vector<size_t>>& left_full_local_idx, 
                  node_local<std::vector<size_t>>& right_full_local_idx){
    throw std::runtime_error("join is not defined for raw_string");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  star_join_eq(std::shared_ptr<dfcolumn>& right,
               node_local<std::vector<size_t>>& left_full_local_idx, 
               node_local<std::vector<size_t>>& right_full_local_idx) {
    throw std::runtime_error("join is not defined for raw_string");
  }
  virtual void append_nulls(node_local<std::vector<size_t>>& to_append);
  virtual std::shared_ptr<dfcolumn> group_by
  (node_local<std::vector<size_t>>& local_idx,
   node_local<std::vector<size_t>>& split_idx,
   node_local<std::vector<std::vector<size_t>>>& hash_divide,
   node_local<std::vector<std::vector<size_t>>>& merge_map) {
    throw std::runtime_error("group_by is not defined for raw_string");
  }
  virtual void
  multi_group_by_sort(node_local<std::vector<size_t>>& local_idx) {
    throw std::runtime_error("group_by is not defined for raw_string");
  }
  virtual node_local<std::vector<size_t>>
  multi_group_by_sort_split(node_local<std::vector<size_t>>& local_idx) {
    throw std::runtime_error("group_by is not defined for raw_string");
  }
  virtual node_local<std::vector<size_t>>
  multi_group_by_split(node_local<std::vector<size_t>>& local_idx) {
    throw std::runtime_error("group_by is not defined for raw_string");
  }
  virtual std::shared_ptr<dfcolumn> multi_group_by_extract
  (node_local<std::vector<size_t>>& local_idx,
   node_local<std::vector<size_t>>& split_idx,
   bool check_nulls) {
    throw std::runtime_error("group_by is not defined for raw_string");
  }
  virtual node_local<std::vector<size_t>>
  calc_hash_base() {
    throw std::runtime_error("group_by is not defined for raw_string");
  }
  virtual void
  calc_hash_base(node_local<std::vector<size_t>>& hash, int shift) {
    throw std::runtime_error("group_by is not defined for raw_string");
  }
  virtual std::shared_ptr<dfcolumn> 
  multi_group_by_exchange(node_local<std::vector<std::vector<size_t>>>&
                          hash_divide) {
    throw std::runtime_error("group_by is not defined for raw_string");
  }
  virtual std::shared_ptr<dfcolumn>
  sum(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("sum of raw_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  count(node_local<std::vector<size_t>>& local_grouped_idx,
        node_local<std::vector<size_t>>& local_idx_split,
        node_local<std::vector<std::vector<size_t>>>& hash_divide,
        node_local<std::vector<std::vector<size_t>>>& merge_map,
        node_local<size_t>& row_sizes) {
    throw std::runtime_error("count of raw_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  avg(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("avg of raw_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  max(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("max of raw_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  min(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("min of raw_string is not defined");
  }
  virtual size_t count();
  raw_string sum() {
    throw std::runtime_error("sum of raw_string is not defined");
  }
  virtual double avg() {
    throw std::runtime_error("avg of raw_string is not defined");
  }
  raw_string max() {
    throw std::runtime_error("max of raw_string is not defined");
  }
  raw_string min() {
    throw std::runtime_error("min of raw_string is not defined");
  }
  virtual dvector<float> as_dvector_float() {
    throw std::runtime_error("as_dvector_float of raw_string is not defined");
  } 
  virtual dvector<double> as_dvector_double() {
    throw std::runtime_error("as_dvector_double of raw_string is not defined");
  }
  virtual std::string dtype() const {return std::string("raw_string");}
  virtual void save(const std::string& file);
  virtual bool is_string() {return true;} // to check cachable in sort
  void init(node_local<words>& ws, bool allocate_nulls = true);
  void init_compressed(node_local<compressed_words>&& ws,
                       bool allocate_nulls = true);
  void init_compressed(node_local<compressed_words>& ws,
                       bool allocate_nulls = true);
  typed_dfcolumn<size_t> sort_prepare() {
    throw std::runtime_error("sort is not defined for raw_string");
  }
  node_local<std::vector<size_t>> equal_prepare
  (std::shared_ptr<typed_dfcolumn<raw_string>>&) {
    throw std::runtime_error("eq is not defined for raw_string");
  }
  virtual void contain_nulls_check();
  virtual std::shared_ptr<dfcolumn> head(size_t limit);
  virtual std::shared_ptr<dfcolumn> tail(size_t limit);
  void align_as(const std::vector<size_t>&);
  node_local<compressed_words> comp_words;
  node_local<std::vector<size_t>> nulls;
  bool contain_nulls;
};

struct datetime {}; // for tag

template <>
class typed_dfcolumn<datetime> : public typed_dfcolumn<datetime_t> {
public:
  typed_dfcolumn() : typed_dfcolumn<datetime_t>() {}
  typed_dfcolumn(const dvector<datetime_t>& dv) :
    typed_dfcolumn<datetime_t>(dv) {}
  typed_dfcolumn(dvector<datetime_t>&& dv) :
    typed_dfcolumn<datetime_t>(std::move(dv)) {}
  typed_dfcolumn(node_local<std::vector<datetime_t>>&& val, 
                 node_local<std::vector<size_t>>&& nulls) :
    typed_dfcolumn<datetime_t>(std::move(val), std::move(nulls)) {}
  typed_dfcolumn(node_local<std::vector<datetime_t>>& val, 
                 node_local<std::vector<size_t>>& nulls) :
    typed_dfcolumn<datetime_t>(val, nulls) {}
  virtual node_local<words> as_words(size_t precision = 6,
                                     const std::string& datetime_fmt="%Y-%m-%d",
                                     bool escape = true,
                                     const std::string& nullstr = "NULL");
  // need to define member functions that returns typed_dfcolumn<datetime>
  virtual std::shared_ptr<dfcolumn>
  extract(node_local<std::vector<size_t>>& idx);
  virtual std::shared_ptr<dfcolumn>
  global_extract(node_local<std::vector<size_t>>&,
                 node_local<std::vector<size_t>>&,
                 node_local<std::vector<std::vector<size_t>>>&);
  virtual std::shared_ptr<dfcolumn>
  sort(node_local<std::vector<size_t>>&);
  virtual std::shared_ptr<dfcolumn>
  sort_desc(node_local<std::vector<size_t>>&);
  virtual std::shared_ptr<dfcolumn>
  sort_with_idx(node_local<std::vector<size_t>>&,
                node_local<std::vector<size_t>>&);
  virtual std::shared_ptr<dfcolumn>
  sort_with_idx_desc(node_local<std::vector<size_t>>&,
                     node_local<std::vector<size_t>>&);
  virtual std::shared_ptr<dfcolumn> group_by
  (node_local<std::vector<size_t>>& local_idx,
   node_local<std::vector<size_t>>& split_idx,
   node_local<std::vector<std::vector<size_t>>>& hash_divide,
   node_local<std::vector<std::vector<size_t>>>& merge_map);
  virtual std::shared_ptr<dfcolumn> multi_group_by_extract
  (node_local<std::vector<size_t>>& local_idx,
   node_local<std::vector<size_t>>& split_idx,
   bool check_nulls);
  virtual std::shared_ptr<dfcolumn> 
  multi_group_by_exchange(node_local<std::vector<std::vector<size_t>>>&
                          hash_divide);
  virtual std::shared_ptr<dfcolumn>
  sum(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("sum of datetime is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  avg(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("avg of datetime is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  max(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn>
  min(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes);
  virtual void debug_print();
  virtual std::shared_ptr<dfcolumn> head(size_t limit);
  virtual std::shared_ptr<dfcolumn> tail(size_t limit);
  virtual std::string dtype() const {return std::string("datetime");}
};

template <class T>
dvector<T> dfcolumn::as_dvector() {
  try {
    auto& typed_col = dynamic_cast<typed_dfcolumn<T>&>(*this);
    return typed_col.get_val().template as_dvector<T>();
  } catch (std::bad_cast& e) {
    throw std::runtime_error("type mismatch of as_dvector<T>()");
  }
}

// added special case for string handling (std::string, dic_string, raw_string)
template <>
dvector<std::string> dfcolumn::as_dvector();

std::vector<std::string> 
words_to_string_vector(words& ws,
                       std::vector<size_t>& nulls,
                       const std::string& nullstr = "NULL");

template <class T>
T dfcolumn::sum() {
  try {
    return dynamic_cast<typed_dfcolumn<T>&>(*this).sum();
  } catch (std::bad_cast& e) {
    throw std::runtime_error("type mismatch of sum<T>()");
  }
}

template <class T>
T dfcolumn::max() {
  try {
    return dynamic_cast<typed_dfcolumn<T>&>(*this).max();
  } catch (std::bad_cast& e) {
    throw std::runtime_error("type mismatch of max<T>()");
  }
}

template <class T>
T dfcolumn::min() {
  try {
    return dynamic_cast<typed_dfcolumn<T>&>(*this).min();
  } catch (std::bad_cast& e) {
    throw std::runtime_error("type mismatch of min<T>()");
  }
}

// These functions are also used in dftable
std::vector<size_t> get_unique_idx(std::vector<size_t>& idx);

node_local<std::vector<std::vector<size_t>>> 
partition_global_index_bynode(node_local<std::vector<size_t>>& global_idx);

node_local<std::vector<std::vector<size_t>>> 
exchange_partitioned_index(node_local<std::vector<std::vector<size_t>>>&
                           partitioned_idx);

node_local<std::vector<size_t>> 
make_to_store_idx(node_local<std::vector<std::vector<size_t>>>&
                  partitioned_idx,
                  node_local<std::vector<size_t>>& global_idx);

node_local<std::vector<size_t>>
local_to_global_idx(node_local<std::vector<size_t>>& local_idx);

struct shift_local_index {
  size_t operator()(size_t i, size_t shift) {return i+shift;}
  SERIALIZE_NONE
};

std::vector<std::vector<size_t>> separate_to_bucket(std::vector<int>& key,
                                                    std::vector<size_t>& idx,
                                                    size_t num_bucket);

// separated to take size for using for local split to improve cache usage
template <class T>
void split_by_hash_with_size(std::vector<T>& val,
                             std::vector<std::vector<T>>& split_val,
                             std::vector<size_t>& global_idx,
                             std::vector<std::vector<size_t>>& split_idx,
                             size_t split_size) {
  if(split_size == 0) 
    throw std::runtime_error("split_by_hash_with_size: split_size is zero");
  else if(split_size == 1) { // skip hash
    T* valp = &val[0];
    size_t* global_idxp = &global_idx[0];
    split_val.resize(1);
    split_idx.resize(1);
    size_t sepsize = val.size();
    split_val[0].resize(sepsize);
    split_idx[0].resize(sepsize);
    T* split_valp = &split_val[0][0];
    size_t* split_idxp = &split_idx[0][0];
    for(size_t j = 0; j < sepsize; j++) {
      split_valp[j] = valp[j];
      split_idxp[j] = global_idxp[j];
    }
    
  } else {
    size_t size = val.size();
    std::vector<int> hash(size);
    int* hashp = &hash[0];
    T* valp = &val[0];
    size_t* global_idxp = &global_idx[0];
    for(size_t i = 0; i < size; i++) {
      hashp[i] = static_cast<int>(myhash(valp[i], split_size));
    }
    std::vector<size_t> local_idx(size);
    auto local_idxp = local_idx.data();
    for(size_t i = 0; i < size; i++) local_idxp[i] = i;
    auto sep = separate_to_bucket(hash, local_idx, split_size);
    split_val.resize(split_size);
    split_idx.resize(split_size);
    for(size_t i = 0; i < split_size; i++) {
      size_t sepsize = sep[i].size();
      split_val[i].resize(sepsize);
      split_idx[i].resize(sepsize);
      T* split_valp = &split_val[i][0];
      size_t* split_idxp = &split_idx[i][0];
      size_t* sepp = &sep[i][0];
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t j = 0; j < sepsize; j++) {
        split_valp[j] = valp[sepp[j]];
        split_idxp[j] = global_idxp[sepp[j]];
      }
    }
  }
}

template <class T>
void split_by_hash(std::vector<T>& val,
                   std::vector<std::vector<T>>& split_val,
                   std::vector<size_t>& global_idx,
                   std::vector<std::vector<size_t>>& split_idx) {
  split_by_hash_with_size<T>(val, split_val, global_idx, split_idx, 
                             static_cast<size_t>(get_nodesize()));
}

template <class T>
void split_by_hash_no_outval_with_size
(std::vector<T>& val,
 std::vector<size_t>& global_idx,
 std::vector<std::vector<size_t>>& split_idx,
 size_t split_size) {
  if(split_size == 0) 
    throw std::runtime_error("split_by_hash_with_size: split_size is zero");
  else if(split_size == 1) { // skip hash
    size_t* global_idxp = &global_idx[0];
    split_idx.resize(1);
    size_t sepsize = val.size();
    split_idx[0].resize(sepsize);
    size_t* split_idxp = &split_idx[0][0];
    for(size_t j = 0; j < sepsize; j++) {
      split_idxp[j] = global_idxp[j];
    }
    
  } else {
    size_t size = val.size();
    std::vector<int> hash(size);
    int* hashp = &hash[0];
    T* valp = &val[0];
    size_t* global_idxp = &global_idx[0];
    for(size_t i = 0; i < size; i++) {
      hashp[i] = static_cast<int>(myhash(valp[i], split_size));
    }
    std::vector<size_t> local_idx(size);
    for(size_t i = 0; i < size; i++) local_idx[i] = i;
    auto sep = separate_to_bucket(hash, local_idx, split_size);
    split_idx.resize(split_size);
    for(size_t i = 0; i < split_size; i++) {
      size_t sepsize = sep[i].size();
      split_idx[i].resize(sepsize);
      size_t* split_idxp = &split_idx[i][0];
      size_t* sepp = &sep[i][0];
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t j = 0; j < sepsize; j++) {
        split_idxp[j] = global_idxp[sepp[j]];
      }
    }
  }
}

template <class T>
void split_by_hash_no_outval(std::vector<T>& val,
                             std::vector<size_t>& global_idx,
                             std::vector<std::vector<size_t>>& split_idx) {
  split_by_hash_no_outval_with_size<T>(val, global_idx, split_idx, 
                                       static_cast<size_t>(get_nodesize()));
}

template <class T>
std::vector<T> flatten(const std::vector<std::vector<T>>& v) {
  size_t total = 0;
  size_t vsize = v.size();
  for(size_t i = 0; i < vsize; i++) total += v[i].size();
  std::vector<T> ret(total);
  T* retp = &ret[0];
  size_t current = 0;
  for(size_t i = 0; i < vsize; i++) {
    const T* vp = v[i].data();
    size_t visize = v[i].size();
    for(size_t j = 0; j < visize; j++) {
      retp[current++] = vp[j];
    }
  }
  return ret;
}

template <class T>
std::vector<size_t>
get_null_like_positions (std::vector<T>& col) {
  return vector_find_tmax(col);
}

template <>
std::vector<size_t>
get_null_like_positions (std::vector<std::string>& col);
}

#endif
