#ifndef DFCOLUMN_HPP
#define DFCOLUMN_HPP

#include <string>
#include <memory>
#include <tuple>
#include "../core/dvector.hpp"
#include "../core/dunordered_map.hpp"
#include "hashtable.hpp"
#include "join.hpp"

#define DFNODESHIFT 48 // used to concatenate node id and local index

namespace frovedis {

class dfcolumn {
public:
  virtual ~dfcolumn(){}
  virtual size_t size() = 0;
  virtual std::vector<size_t> sizes() = 0;
  virtual void debug_print() = 0;
  virtual dvector<std::string> as_string() = 0;
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
                 node_local<std::vector<std::vector<size_t>>>&,
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
  virtual node_local<std::vector<size_t>>
  group_by(node_local<std::vector<size_t>>& global_idx) = 0;
  // for group by of multiple columns
  virtual node_local<std::vector<size_t>> calc_hash_base() = 0;
  virtual void
  calc_hash_base(node_local<std::vector<size_t>>& hash, int shift) = 0;
  virtual void
  multi_group_by_sort(node_local<std::vector<size_t>>& local_idx) = 0;
  virtual node_local<std::vector<size_t>>
  multi_group_by_split(node_local<std::vector<size_t>>& local_idx) = 0;
  // for grouped_dftable
  virtual std::shared_ptr<dfcolumn>
  sum(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx) = 0;
  virtual std::shared_ptr<dfcolumn>
  count(node_local<std::vector<size_t>>& grouped_idx,
        node_local<std::vector<size_t>>& idx_split,
        node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
        node_local<std::vector<std::vector<size_t>>>& exchanged_idx) = 0;
  virtual std::shared_ptr<dfcolumn>
  avg(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx) = 0;
  virtual std::shared_ptr<dfcolumn>
  max(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx) = 0;
  virtual std::shared_ptr<dfcolumn>
  min(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx) = 0;
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
  virtual bool is_string() {return false;}
  virtual std::string dtype() const = 0;
  virtual void save(const std::string& file) = 0;
  virtual void contain_nulls_check() = 0;
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
                 node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
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
  virtual node_local<std::vector<size_t>> calc_hash_base();
  void calc_hash_base(node_local<std::vector<size_t>>& hash_base, int shift);
  virtual node_local<std::vector<size_t>>
  group_by(node_local<std::vector<size_t>>& global_idx);
  virtual void
  multi_group_by_sort(node_local<std::vector<size_t>>& local_idx);
  virtual node_local<std::vector<size_t>>
  multi_group_by_split(node_local<std::vector<size_t>>& local_idx);
  virtual std::shared_ptr<dfcolumn>
  sum(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
  virtual std::shared_ptr<dfcolumn>
  count(node_local<std::vector<size_t>>& grouped_idx,
        node_local<std::vector<size_t>>& idx_split,
        node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
        node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
  virtual std::shared_ptr<dfcolumn>
  avg(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
  virtual std::shared_ptr<dfcolumn>
  max(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
  virtual std::shared_ptr<dfcolumn>
  min(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
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
  virtual void contain_nulls_check();
  node_local<std::vector<T>>& get_val(){return val;}
  node_local<std::vector<size_t>>& get_nulls(){return nulls;}
  node_local<std::vector<T>> val;
  node_local<std::vector<size_t>> nulls;
  bool contain_nulls;
};

template <>
class typed_dfcolumn<std::string> : public dfcolumn {
public:
  typed_dfcolumn() : contain_nulls(false) {}
  typed_dfcolumn(dvector<std::string>& dv) : contain_nulls(false)
    {init(dv);}
  typed_dfcolumn(dvector<std::string>&& dv) : contain_nulls(false)
    {init(dv);}
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
                 node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
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
  node_local<std::vector<size_t>> get_nulls();
  node_local<std::vector<std::string>> get_val();
  virtual dvector<std::string> as_string();
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
  virtual node_local<std::vector<size_t>> calc_hash_base();
  void calc_hash_base(node_local<std::vector<size_t>>& hash_base, int shift);
  virtual void
  multi_group_by_sort(node_local<std::vector<size_t>>& local_idx);
  virtual node_local<std::vector<size_t>>
  multi_group_by_split(node_local<std::vector<size_t>>& local_idx);
  virtual node_local<std::vector<size_t>>
  group_by(node_local<std::vector<size_t>>& global_idx);
  virtual std::shared_ptr<dfcolumn>
  sum(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
    throw std::runtime_error("sum of string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  count(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
  virtual std::shared_ptr<dfcolumn>
  avg(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
    throw std::runtime_error("avg of string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  max(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
    throw std::runtime_error("max of string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  min(node_local<std::vector<size_t>>& grouped_idx,
      node_local<std::vector<size_t>>& idx_split,
      node_local<std::vector<std::vector<size_t>>>& partitioned_idx,
      node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
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
  void init(dvector<std::string>& dv);
  typed_dfcolumn<size_t> sort_prepare();
  node_local<std::vector<size_t>> equal_prepare
  (std::shared_ptr<typed_dfcolumn<std::string>>&);
  virtual void contain_nulls_check();
  virtual std::shared_ptr<dfcolumn> head(size_t limit);
  // string -> idx; shared between columns
  std::shared_ptr<dunordered_map<std::string, size_t>> dic;
  // idx -> string; shared between columns
  std::shared_ptr<node_local<std::vector<std::string>>> dic_idx; 
  node_local<std::vector<size_t>> val;
  node_local<std::vector<size_t>> nulls;
  bool contain_nulls;
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
local_to_global_idx(node_local<std::vector<size_t>>& local_idx);

struct shift_local_index {
  size_t operator()(size_t i, size_t shift) {return i+shift;}
  SERIALIZE_NONE
};

struct shiftback_local_index {
  size_t operator()(size_t i, size_t shift) {return i-shift;}
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
    for(size_t i = 0; i < size; i++) local_idx[i] = i;
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
std::vector<T> flatten(std::vector<std::vector<T>>& v) {
  size_t total = 0;
  size_t vsize = v.size();
  for(size_t i = 0; i < vsize; i++) total += v[i].size();
  std::vector<T> ret(total);
  T* retp = &ret[0];
  size_t current = 0;
  for(size_t i = 0; i < vsize; i++) {
    T* vp = &v[i][0];
    size_t visize = v[i].size();
    for(size_t j = 0; j < visize; j++) {
      retp[current++] = vp[j];
    }
  }
  return ret;
}

}

#endif
