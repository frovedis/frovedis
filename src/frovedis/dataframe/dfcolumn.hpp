#ifndef DFCOLUMN_HPP
#define DFCOLUMN_HPP

#include <string>
#include <memory>
#include <tuple>
#include <list>
#include <unordered_map>
#include "../core/dvector.hpp"
#include "../core/dunordered_map.hpp"
#include "../core/vector_operations.hpp"
#include "hashtable.hpp"
#include "join.hpp"
#include "../text/dict.hpp"
#include "../text/parsedatetime.hpp"
#include "../text/datetime_utility.hpp"
#include "dfscalar.hpp"

#define DFNODESHIFT 48 // used to concatenate node id and local index

namespace frovedis {

struct dfcolumn_spillable_t {
  dfcolumn_spillable_t();
  bool is_spillable;
  void set(bool val) {is_spillable = val;}
  bool get(){return is_spillable;}
};
extern dfcolumn_spillable_t dfcolumn_spillable;
enum spill_state_type {
  restored,
  spilled
};

enum trim_type {
  leading,
  trailing,
  both
};

class dfcolumn {
public:
  dfcolumn() : spillable(dfcolumn_spillable.get()), spill_initialized(false),
               already_spilled_to_disk(false), cleared(false),
               spill_state(spill_state_type::restored),
               spill_size_cache(-1) {}
  dfcolumn(const dfcolumn& c);
  dfcolumn(dfcolumn&& c);
  dfcolumn& operator=(const dfcolumn& c);
  dfcolumn& operator=(dfcolumn&& c);
  virtual ~dfcolumn();
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
  filter_eq_immed(std::shared_ptr<dfscalar>& right) = 0;
  virtual node_local<std::vector<size_t>>
  filter_neq(std::shared_ptr<dfcolumn>& right) = 0;
  virtual node_local<std::vector<size_t>>
  filter_neq_immed(std::shared_ptr<dfscalar>& right) = 0;
  virtual node_local<std::vector<size_t>>
  filter_lt(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filter_lt is not supported for this type");
  }
  virtual node_local<std::vector<size_t>>
  filter_lt_immed(std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("filter_lt_immed is not supported for this type");
  }
  virtual node_local<std::vector<size_t>>
  filter_le(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filter_le is not supported for this type");
  }
  virtual node_local<std::vector<size_t>>
  filter_le_immed(std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("filter_le_immed is not supported for this type");
  }
  virtual node_local<std::vector<size_t>>
  filter_gt(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filter_gt is not supported for this type");
  }
  virtual node_local<std::vector<size_t>>
  filter_gt_immed(std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("filter_gt_immed is not supported for this type");
  }
  virtual node_local<std::vector<size_t>>
  filter_ge(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filter_ge is not supported for this type");
  }
  virtual node_local<std::vector<size_t>>
  filter_ge_immed(std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("filter_ge_immed is not supported for this type");
  }
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
  virtual node_local<std::vector<size_t>>
  calc_hash_base_multi_join(std::shared_ptr<dfcolumn>& left) = 0;
  virtual void
  calc_hash_base_multi_join(node_local<std::vector<size_t>>& hash, int shift,
                            std::shared_ptr<dfcolumn>& left) = 0;
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
  size(node_local<std::vector<size_t>>& local_grouped_idx,
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
  var(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0) = 0;
  virtual std::shared_ptr<dfcolumn>
  sem(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0) = 0;
  virtual std::shared_ptr<dfcolumn>
  std(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0) = 0;
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
  virtual std::shared_ptr<dfcolumn>
  mad(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) = 0;
  virtual std::shared_ptr<dfcolumn>
  count_distinct(node_local<std::vector<size_t>>& local_grouped_idx,
                 node_local<std::vector<size_t>>& local_idx_split,
                 node_local<std::vector<std::vector<size_t>>>& hash_divide,
                 node_local<std::vector<std::vector<size_t>>>& merge_map,
                 node_local<size_t>& row_sizes) = 0;
  virtual std::shared_ptr<dfcolumn>
  sum_distinct(node_local<std::vector<size_t>>& local_grouped_idx,
                 node_local<std::vector<size_t>>& local_idx_split,
                 node_local<std::vector<std::vector<size_t>>>& hash_divide,
                 node_local<std::vector<std::vector<size_t>>>& merge_map,
                 node_local<size_t>& row_sizes) = 0;
  virtual std::shared_ptr<dfcolumn>
  first(node_local<std::vector<size_t>>& local_grouped_idx,
        node_local<std::vector<size_t>>& local_idx_split,
        node_local<std::vector<std::vector<size_t>>>& hash_divide,
        node_local<std::vector<std::vector<size_t>>>& merge_map,
        node_local<size_t>& row_sizes,
        bool ignore_nulls) = 0;
  virtual std::shared_ptr<dfcolumn>
  last(node_local<std::vector<size_t>>& local_grouped_idx,
       node_local<std::vector<size_t>>& local_idx_split,
       node_local<std::vector<std::vector<size_t>>>& hash_divide,
       node_local<std::vector<std::vector<size_t>>>& merge_map,
       node_local<size_t>& row_sizes,
       bool ignore_nulls) = 0;
  // for whole column
  virtual size_t count() = 0; // exclude null
  template <class T> T sum();
  virtual double avg() = 0;
  virtual double std(double ddof = 1.0) = 0;
  virtual double sem(double ddof = 1.0) = 0;
  virtual double var(double ddof = 1.0) = 0;
  virtual double mad() = 0;
  template <class T> T max();
  template <class T> T min();
  template <class T> T first(bool ignore_nulls = false);
  template <class T> T last(bool ignore_nulls = false);
  template <class T> T at(size_t i);
  template <class T> dvector<T> as_dvector();
  // cast to float/double; throw exception when string 
  virtual dvector<float> as_dvector_float() = 0; 
  virtual dvector<double> as_dvector_double() = 0;
  virtual std::shared_ptr<dfcolumn> type_cast(const std::string& to_type,
                                              bool check_bool_like = false) {
    throw std::runtime_error("type_cast is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  add(const std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("add is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  add_im(const std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("add_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  sub(const std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("sub is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  sub_im(const std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("sub_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  rsub_im(const std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("rsub_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  mul(const std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("mul is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  mul_im(const std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("mul_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  fdiv(const std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("fdiv is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  fdiv_im(const std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("fdiv_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  rfdiv_im(const std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("rfdiv_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  idiv(const std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("idiv is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  idiv_im(const std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("idiv_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  ridiv_im(const std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("ridiv_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  mod(const std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("mod is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  mod_im(const std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("mod_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  rmod_im(const std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("rmod_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  pow(const std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("pow is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  pow_im(const std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("pow_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  rpow_im(const std::shared_ptr<dfscalar>& right) {
    throw std::runtime_error("rpow_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  abs() {throw std::runtime_error("abs is not supported for this type");}
  virtual std::shared_ptr<dfcolumn>
  eq(const std::shared_ptr<dfcolumn>& right); 
  virtual std::shared_ptr<dfcolumn>
  eq_im(const std::shared_ptr<dfscalar>& right); 
  virtual std::shared_ptr<dfcolumn>
  neq(const std::shared_ptr<dfcolumn>& right);
  virtual std::shared_ptr<dfcolumn>
  neq_im(const std::shared_ptr<dfscalar>& right);
  virtual std::shared_ptr<dfcolumn>
  lt(const std::shared_ptr<dfcolumn>& right);
  virtual std::shared_ptr<dfcolumn>
  lt_im(const std::shared_ptr<dfscalar>& right);
  virtual std::shared_ptr<dfcolumn>
  le(const std::shared_ptr<dfcolumn>& right);
  virtual std::shared_ptr<dfcolumn>
  le_im(const std::shared_ptr<dfscalar>& right);
  virtual std::shared_ptr<dfcolumn>
  gt(const std::shared_ptr<dfcolumn>& right);
  virtual std::shared_ptr<dfcolumn>
  gt_im(const std::shared_ptr<dfscalar>& right);
  virtual std::shared_ptr<dfcolumn>
  ge(const std::shared_ptr<dfcolumn>& right);
  virtual std::shared_ptr<dfcolumn>
  ge_im(const std::shared_ptr<dfscalar>& right);
  virtual std::shared_ptr<dfcolumn> is_null();
  virtual std::shared_ptr<dfcolumn> is_not_null();
  virtual std::shared_ptr<dfcolumn>
  and_op(const std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("and_op is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  or_op(const std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("or_op is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  not_op() {
    throw std::runtime_error("not_op is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  datetime_extract(datetime_type kind){ 
    throw std::runtime_error("datetime_extract is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  datetime_diff(const std::shared_ptr<dfcolumn>& right, datetime_type kind){ 
    throw std::runtime_error("datetime_diff is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  datetime_diff_im(datetime_t right, datetime_type kind){ 
    throw std::runtime_error("datetime_diff_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  rdatetime_diff_im(datetime_t right, datetime_type kind){ 
    throw std::runtime_error
      ("rdatetime_diff_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  datetime_add(const std::shared_ptr<dfcolumn>& right, datetime_type kind) {
    throw std::runtime_error
      ("datetime_add is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  datetime_add_im(int right, datetime_type kind){ 
    throw std::runtime_error("datetime_add_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  datetime_sub(const std::shared_ptr<dfcolumn>& right, datetime_type kind) {
    throw std::runtime_error
      ("datetime_sub is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  datetime_sub_im(int right, datetime_type kind){ 
    throw std::runtime_error("datetime_sub_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  datetime_truncate(datetime_type kind){ 
    throw std::runtime_error
      ("datetime_truncate is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  datetime_months_between(const std::shared_ptr<dfcolumn>& right){ 
    throw std::runtime_error
      ("datetime_months_between is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  datetime_next_day(const std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error
      ("datetime_next_day is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn>
  datetime_next_day_im(int right) {
    throw std::runtime_error
      ("datetime_next_day_im is not supported for this type");
  }
  virtual std::shared_ptr<dfcolumn> substr(int pos, int num);
  virtual std::shared_ptr<dfcolumn>
  substr(const std::shared_ptr<dfcolumn>& pos, int num);
  virtual std::shared_ptr<dfcolumn>
  substr(int pos, const std::shared_ptr<dfcolumn>& num);
  virtual std::shared_ptr<dfcolumn>
  substr(const std::shared_ptr<dfcolumn>& pos,
         const std::shared_ptr<dfcolumn>& num);
  virtual std::shared_ptr<dfcolumn> substr(int pos);
  virtual std::shared_ptr<dfcolumn>
  substr(const std::shared_ptr<dfcolumn>& pos);
  virtual std::shared_ptr<dfcolumn> length();
  virtual std::shared_ptr<dfcolumn> locate(const std::string& str, int pos);
  virtual std::shared_ptr<dfcolumn>
  trim(trim_type kind, const std::string& to_trim);
  virtual std::shared_ptr<dfcolumn>
  replace(const std::string& from, const std::string& to);
  virtual std::shared_ptr<dfcolumn> reverse();
  virtual std::shared_ptr<dfcolumn>
  substring_index(const std::string& delim, int count);
  virtual std::shared_ptr<dfcolumn>
  union_columns(const std::vector<std::shared_ptr<dfcolumn>>& cols) = 0;
  virtual std::shared_ptr<dfcolumn> head(size_t limit) = 0;
  virtual std::shared_ptr<dfcolumn> tail(size_t limit) = 0;
  virtual bool is_string() {return false;}
  virtual std::string dtype() const = 0;
  virtual void save(const std::string& file) = 0;
  virtual void contain_nulls_check() = 0;
  virtual node_local<std::vector<size_t>> get_nulls() = 0;
  virtual bool if_contain_nulls() = 0;
  virtual bool is_unique() = 0;
  virtual bool is_all_null() = 0;

  // for spill-restore
  void spill();
  void restore();
  virtual void spill_to_disk() = 0;
  virtual void restore_from_disk() = 0;
  virtual size_t calc_spill_size() = 0;
  size_t spill_size();
  void init_spill();
  bool spillable;
  bool spill_initialized; // also used to switch put_new/put
  bool already_spilled_to_disk;
  bool cleared;
  spill_state_type spill_state;
  ssize_t spill_size_cache;
  node_local<std::string> spill_path;
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
  typed_dfcolumn(node_local<std::vector<T>>&& val, 
                 node_local<std::vector<size_t>>& nulls) :
    val(std::move(val)), nulls(nulls) {
    contain_nulls_check();
  }
  typed_dfcolumn(node_local<std::vector<T>>& val, 
                 node_local<std::vector<size_t>>& nulls) :
    val(val), nulls(nulls) {
    contain_nulls_check();
  }
  typed_dfcolumn(const typed_dfcolumn<T>& c) :
    dfcolumn(c), val(c.val), nulls(c.nulls) {}
  typed_dfcolumn(typed_dfcolumn<T>&& c) :
    dfcolumn(std::move(c)), val(std::move(c.val)), nulls(std::move(c.nulls)) {}
  typed_dfcolumn& operator=(const typed_dfcolumn<T>& c) {
    dfcolumn::operator=(c); val = c.val; nulls = c.nulls;
    return *this;
  }
  typed_dfcolumn& operator=(typed_dfcolumn<T>&& c) {
    dfcolumn::operator=(std::move(c));
    val = std::move(c.val); nulls = std::move(c.nulls);
    return *this;
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
  virtual node_local<std::vector<size_t>>
  filter_eq_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_neq(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_neq_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_lt(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_lt_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_le(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_le_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_gt(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_gt_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_ge(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_ge_immed(std::shared_ptr<dfscalar>& right);
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
  virtual node_local<std::vector<size_t>>
  calc_hash_base_multi_join(std::shared_ptr<dfcolumn>& left);
  virtual void
  calc_hash_base_multi_join(node_local<std::vector<size_t>>& hash, int shift,
                            std::shared_ptr<dfcolumn>& left);
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
  size(node_local<std::vector<size_t>>& local_grouped_idx,
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
  var(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0);
  virtual std::shared_ptr<dfcolumn>
  sem(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0);
  virtual std::shared_ptr<dfcolumn>
  std(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0);
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
  virtual std::shared_ptr<dfcolumn>
  mad(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn>
  count_distinct(node_local<std::vector<size_t>>& local_grouped_idx,
                 node_local<std::vector<size_t>>& local_idx_split,
                 node_local<std::vector<std::vector<size_t>>>& hash_divide,
                 node_local<std::vector<std::vector<size_t>>>& merge_map,
                 node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn>
  sum_distinct(node_local<std::vector<size_t>>& local_grouped_idx,
               node_local<std::vector<size_t>>& local_idx_split,
               node_local<std::vector<std::vector<size_t>>>& hash_divide,
               node_local<std::vector<std::vector<size_t>>>& merge_map,
               node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn>
  first(node_local<std::vector<size_t>>& local_grouped_idx,
        node_local<std::vector<size_t>>& local_idx_split,
        node_local<std::vector<std::vector<size_t>>>& hash_divide,
        node_local<std::vector<std::vector<size_t>>>& merge_map,
        node_local<size_t>& row_sizes,
        bool ignore_nulls);
  virtual std::shared_ptr<dfcolumn>
  last(node_local<std::vector<size_t>>& local_grouped_idx,
       node_local<std::vector<size_t>>& local_idx_split,
       node_local<std::vector<std::vector<size_t>>>& hash_divide,
       node_local<std::vector<std::vector<size_t>>>& merge_map,
       node_local<size_t>& row_sizes,
       bool ignore_nulls);
  virtual size_t count();
  T sum();
  virtual double avg();
  virtual double std(double ddof = 1.0);
  virtual double sem(double ddof = 1.0);
  virtual double var(double ddof = 1.0);
  virtual double mad();
  T max();
  T min();
  T first(bool ignore_nulls = false);
  T last(bool ignore_nulls = false);
  T at(size_t i);
  virtual dvector<float> as_dvector_float(); 
  virtual dvector<double> as_dvector_double();
  virtual std::shared_ptr<dfcolumn> type_cast(const std::string& to_type,
                                              bool check_bool_like=false);

  virtual std::shared_ptr<dfcolumn> add(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_add(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  add_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_add_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn> sub(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_sub(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  sub_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_sub_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  rsub_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_rsub_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn> mul(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_mul(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  mul_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_mul_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  fdiv(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_fdiv(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  fdiv_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_fdiv_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  rfdiv_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_rfdiv_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn> 
  idiv(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_idiv(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  idiv_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_idiv_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  ridiv_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_ridiv_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn> mod(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_mod(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  mod_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_mod_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  rmod_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_rmod_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn> pow(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_pow(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  pow_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_pow_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  rpow_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_rpow_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn> abs();
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_eq(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  eq(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_eq_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  eq_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_neq(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  neq(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_neq_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  neq_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_lt(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  lt(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_lt_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  lt_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_le(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  le(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_le_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  le_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_gt(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  gt(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_gt_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  gt_im(const std::shared_ptr<dfscalar>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_ge(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  ge(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_ge_im(const std::shared_ptr<typed_dfscalar<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  ge_im(const std::shared_ptr<dfscalar>& right);
/*
  virtual std::shared_ptr<dfcolumn>
  is_null();
  virtual std::shared_ptr<dfcolumn>
  is_not_null();
*/
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_and_op(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  and_op(const std::shared_ptr<dfcolumn>& right);
  template <class U>
  std::shared_ptr<dfcolumn>
  typed_or_op(const std::shared_ptr<typed_dfcolumn<U>>& right);
  virtual std::shared_ptr<dfcolumn>
  or_op(const std::shared_ptr<dfcolumn>& right);
  virtual std::shared_ptr<dfcolumn>
  not_op();
  virtual void debug_print();
  virtual std::string dtype() const;
  virtual void save(const std::string& file);
  virtual std::shared_ptr<dfcolumn> head(size_t limit);
  virtual std::shared_ptr<dfcolumn> tail(size_t limit);
  virtual std::shared_ptr<dfcolumn> 
  union_columns(const std::vector<std::shared_ptr<dfcolumn>>& cols);
  virtual void contain_nulls_check();
  node_local<std::vector<T>>& get_val(){return val;}
  virtual node_local<std::vector<size_t>> get_nulls(){return nulls;}
  node_local<std::vector<T>> val;
  node_local<std::vector<size_t>> nulls;
  bool contain_nulls;
  virtual bool if_contain_nulls(){return contain_nulls;}
  virtual bool is_unique();
  virtual bool is_all_null();

  // for spill-restore
  virtual void spill_to_disk();
  virtual void restore_from_disk();
  virtual size_t calc_spill_size();
};

template <class T>
void reset_null_val(const std::vector<T>& nulls,
                    std::vector<T>& val) {
  auto valp = val.data();
  auto nullp = nulls.data();
  auto tmax = std::numeric_limits<T>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nulls.size(); ++i) valp[nullp[i]] = tmax;
}

// defined in typed_dfcolumn_string.cc 
words dfcolumn_string_as_words_helper(const std::vector<std::string>& str,
                                      const std::vector<size_t>& nulls,
                                      const std::string& nullstr);

// defined in dfcolumn.cc: used for conversion of 
// words (dictionary) (having true/false like strings) to int (1/0)
std::vector<int> words_to_bool(words& w);

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
  typed_dfcolumn(node_local<std::vector<std::size_t>>&& val_,
                 node_local<std::vector<size_t>>&& nulls_,
                 std::shared_ptr<dunordered_map<std::string, size_t>>&& dic_,
                 std::shared_ptr<node_local<std::vector<std::string>>>&&
                 dic_idx_) {
    val = std::move(val_); nulls = std::move(nulls_);
    dic = std::move(dic_); dic_idx = std::move(dic_idx_);
    contain_nulls_check();
    if(contain_nulls) nulls.mapv(reset_null_val<size_t>, val);
  }
  typed_dfcolumn(node_local<std::vector<std::size_t>>& val_,
                 node_local<std::vector<size_t>>& nulls_,
                 std::shared_ptr<dunordered_map<std::string, size_t>>& dic_,
                 std::shared_ptr<node_local<std::vector<std::string>>>&
                 dic_idx_) {
    val = val_; nulls = nulls_;
    dic = dic_; dic_idx = dic_idx_;
    contain_nulls_check();
    if(contain_nulls) nulls.mapv(reset_null_val<size_t>, val);
  }
  typed_dfcolumn(const typed_dfcolumn<std::string>& c) :
    dfcolumn(c), dic(c.dic), dic_idx(c.dic_idx), val(c.val), nulls(c.nulls) {}
  typed_dfcolumn(typed_dfcolumn<std::string>&& c) :
    dfcolumn(std::move(c)), dic(std::move(c.dic)),
    dic_idx(std::move(c.dic_idx)),
    val(std::move(c.val)), nulls(std::move(c.nulls)) {}
  typed_dfcolumn& operator=(const typed_dfcolumn<std::string>& c) {
    dfcolumn::operator=(c);
    dic = c.dic; dic_idx = c.dic_idx; val = c.val; nulls = c.nulls;
    return *this;
  }
  typed_dfcolumn& operator=(typed_dfcolumn<std::string>&& c) {
    dfcolumn::operator=(std::move(c));
    dic = std::move(c.dic); dic_idx = std::move(c.dic_idx);
    val = std::move(c.val); nulls = std::move(c.nulls);
    return *this;
  }

  virtual size_t size();
  virtual std::vector<size_t> sizes();
  virtual node_local<std::vector<size_t>>
  filter_eq(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_eq_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_neq(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_neq_immed(std::shared_ptr<dfscalar>& right);
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
  virtual node_local<std::vector<size_t>>
  calc_hash_base_multi_join(std::shared_ptr<dfcolumn>& left);
  virtual void
  calc_hash_base_multi_join(node_local<std::vector<size_t>>& hash, int shift,
                            std::shared_ptr<dfcolumn>& left);
  virtual std::shared_ptr<dfcolumn> 
  multi_group_by_exchange(node_local<std::vector<std::vector<size_t>>>&
                          hash_divide);
  virtual std::shared_ptr<dfcolumn> type_cast(const std::string& to_type,
                                              bool check_bool_like = false);
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
  size(node_local<std::vector<size_t>>& local_grouped_idx,
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
  var(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0){
    throw std::runtime_error("var of string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  sem(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0){
    throw std::runtime_error("sem of string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  std(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0){
    throw std::runtime_error("std of string is not defined");
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
  virtual std::shared_ptr<dfcolumn>
  mad(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("mad of string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  count_distinct(node_local<std::vector<size_t>>& local_grouped_idx,
                 node_local<std::vector<size_t>>& local_idx_split,
                 node_local<std::vector<std::vector<size_t>>>& hash_divide,
                 node_local<std::vector<std::vector<size_t>>>& merge_map,
                 node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn>
  sum_distinct(node_local<std::vector<size_t>>& local_grouped_idx,
               node_local<std::vector<size_t>>& local_idx_split,
               node_local<std::vector<std::vector<size_t>>>& hash_divide,
               node_local<std::vector<std::vector<size_t>>>& merge_map,
               node_local<size_t>& row_sizes) {
    throw std::runtime_error("sum_distinct of string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  first(node_local<std::vector<size_t>>& local_grouped_idx,
        node_local<std::vector<size_t>>& local_idx_split,
        node_local<std::vector<std::vector<size_t>>>& hash_divide,
        node_local<std::vector<std::vector<size_t>>>& merge_map,
        node_local<size_t>& row_sizes,
        bool ignore_nulls);
  virtual std::shared_ptr<dfcolumn>
  last(node_local<std::vector<size_t>>& local_grouped_idx,
       node_local<std::vector<size_t>>& local_idx_split,
       node_local<std::vector<std::vector<size_t>>>& hash_divide,
       node_local<std::vector<std::vector<size_t>>>& merge_map,
       node_local<size_t>& row_sizes,
       bool ignore_nulls);
  virtual size_t count();
  std::string sum() {
    throw std::runtime_error("sum of string is not defined");
  }
  virtual double avg() {
    throw std::runtime_error("avg of string is not defined");
  }
  virtual double std(double ddof = 1.0) {
    throw std::runtime_error("std of string is not defined");
  }
  virtual double sem(double ddof = 1.0) {
    throw std::runtime_error("sem of string is not defined");
  }
  virtual double var(double ddof = 1.0) {
    throw std::runtime_error("var of string is not defined");
  }
  virtual double mad() {
    throw std::runtime_error("mad of string is not defined");
  }
  std::string max() {
    throw std::runtime_error("max of string is not defined");
  }
  std::string min() {
    throw std::runtime_error("min of string is not defined");
  }
  std::string first(bool ignore_nulls = false);
  std::string last(bool ignore_nulls = false);
  std::string at(size_t i) {
    throw std::runtime_error("at of string is not defined");
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
  node_local<std::vector<size_t>>
  equal_prepare_multi_join(typed_dfcolumn<std::string>& right);
  virtual void contain_nulls_check();
  virtual std::shared_ptr<dfcolumn> head(size_t limit);
  virtual std::shared_ptr<dfcolumn> tail(size_t limit);
  virtual std::shared_ptr<dfcolumn> 
  union_columns(const std::vector<std::shared_ptr<dfcolumn>>& cols);
  // string -> idx; shared between columns
  std::shared_ptr<dunordered_map<std::string, size_t>> dic;
  // idx -> string; shared between columns
  std::shared_ptr<node_local<std::vector<std::string>>> dic_idx; 
  node_local<std::vector<size_t>> val;
  node_local<std::vector<size_t>> nulls;
  bool contain_nulls;
  virtual bool if_contain_nulls(){return contain_nulls;}
  virtual bool is_unique();
  virtual bool is_all_null();

  // for spill-restore
  virtual void spill_to_disk();
  virtual void restore_from_disk();
  virtual size_t calc_spill_size();
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
    nulls(nulls) {init(ws, false); contain_nulls_check();
    if(contain_nulls) nulls.mapv(reset_null_val<size_t>, val);}
  typed_dfcolumn(node_local<words>&& ws,
                 node_local<std::vector<size_t>>&& nulls) :
    nulls(std::move(nulls)) {init(ws, false); contain_nulls_check();
    if(contain_nulls) this->nulls.mapv(reset_null_val<size_t>, val);}
  typed_dfcolumn(node_local<compressed_words>& ws) : contain_nulls(false)
    {init_compressed(ws);}
  typed_dfcolumn(node_local<compressed_words>&& ws) : contain_nulls(false)
    {init_compressed(ws);}
  typed_dfcolumn(node_local<compressed_words>& ws,
                 node_local<std::vector<size_t>>& nulls) :
    nulls(nulls) {init_compressed(ws, false); contain_nulls_check();
    if(contain_nulls) nulls.mapv(reset_null_val<size_t>, val);}
  typed_dfcolumn(node_local<compressed_words>&& ws,
                 node_local<std::vector<size_t>>&& nulls)
    : nulls(std::move(nulls))
    {init_compressed(ws, false); contain_nulls_check();
    if(contain_nulls) this->nulls.mapv(reset_null_val<size_t>, val);}
  typed_dfcolumn(std::shared_ptr<dict>& dic,
                 node_local<std::vector<size_t>>& val,
                 node_local<std::vector<size_t>>& nulls)
    : dic(dic), val(val), nulls(nulls) {contain_nulls_check();
    if(contain_nulls) nulls.mapv(reset_null_val<size_t>, this->val);}
  typed_dfcolumn(std::shared_ptr<dict>&& dic,
                 node_local<std::vector<size_t>>&& val,
                 node_local<std::vector<size_t>>&& nulls)
    : dic(std::move(dic)), val(std::move(val)), nulls(std::move(nulls))
    {contain_nulls_check();
    if(contain_nulls) this->nulls.mapv(reset_null_val<size_t>, this->val);}
  typed_dfcolumn(const typed_dfcolumn<dic_string>& c) :
    dfcolumn(c), dic(c.dic), val(c.val), nulls(c.nulls) {}
  typed_dfcolumn(typed_dfcolumn<dic_string>&& c) :
    dfcolumn(std::move(c)), dic(std::move(c.dic)), val(std::move(c.val)),
    nulls(std::move(c.nulls)) {}
    
  typed_dfcolumn& operator=(const typed_dfcolumn<dic_string>& c) {
    dfcolumn::operator=(c);
    dic = c.dic; val = c.val; nulls = c.nulls; 
    return *this;
  }
  typed_dfcolumn& operator=(typed_dfcolumn<dic_string>&& c) {
    dfcolumn::operator=(std::move(c));
    dic = std::move(c.dic); val = std::move(c.val); nulls = std::move(c.nulls);
    return *this;
  }

  virtual size_t size(); 
  virtual std::vector<size_t> sizes(); 
  virtual node_local<std::vector<size_t>>
  filter_eq(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_eq_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_neq(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_neq_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_lt(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_lt_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_le(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_le_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_gt(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_gt_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_ge(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_ge_immed(std::shared_ptr<dfscalar>& right);
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
  virtual node_local<std::vector<size_t>>
  calc_hash_base_multi_join(std::shared_ptr<dfcolumn>& left);
  virtual void
  calc_hash_base_multi_join(node_local<std::vector<size_t>>& hash, int shift,
                            std::shared_ptr<dfcolumn>& left);
  virtual std::shared_ptr<dfcolumn> 
  multi_group_by_exchange(node_local<std::vector<std::vector<size_t>>>&
                          hash_divide);
  virtual std::shared_ptr<dfcolumn> type_cast(const std::string& to_type,
                                              bool check_bool_like = false);
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
  size(node_local<std::vector<size_t>>& local_grouped_idx,
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
  var(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0){
    throw std::runtime_error("var of dic_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  sem(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0){
    throw std::runtime_error("sem of dic_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  std(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0){
    throw std::runtime_error("std of dic_string is not defined");
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
  virtual std::shared_ptr<dfcolumn>
  mad(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("mad of dic_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  count_distinct(node_local<std::vector<size_t>>& local_grouped_idx,
                 node_local<std::vector<size_t>>& local_idx_split,
                 node_local<std::vector<std::vector<size_t>>>& hash_divide,
                 node_local<std::vector<std::vector<size_t>>>& merge_map,
                 node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn>
  sum_distinct(node_local<std::vector<size_t>>& local_grouped_idx,
               node_local<std::vector<size_t>>& local_idx_split,
               node_local<std::vector<std::vector<size_t>>>& hash_divide,
               node_local<std::vector<std::vector<size_t>>>& merge_map,
               node_local<size_t>& row_sizes) {
    throw std::runtime_error("sum_distinct of dic_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  first(node_local<std::vector<size_t>>& local_grouped_idx,
        node_local<std::vector<size_t>>& local_idx_split,
        node_local<std::vector<std::vector<size_t>>>& hash_divide,
        node_local<std::vector<std::vector<size_t>>>& merge_map,
        node_local<size_t>& row_sizes,
        bool ignore_nulls);
  virtual std::shared_ptr<dfcolumn>
  last(node_local<std::vector<size_t>>& local_grouped_idx,
       node_local<std::vector<size_t>>& local_idx_split,
       node_local<std::vector<std::vector<size_t>>>& hash_divide,
       node_local<std::vector<std::vector<size_t>>>& merge_map,
       node_local<size_t>& row_sizes,
       bool ignore_nulls);
  virtual size_t count();
  dic_string sum() {
    throw std::runtime_error("sum of dic_string is not defined");
  }
  virtual double avg() {
    throw std::runtime_error("avg of dic_string is not defined");
  }
  virtual double std(double ddof = 1.0) {
    throw std::runtime_error("std of dic_string is not defined");
  }
  virtual double sem(double ddof = 1.0) {
    throw std::runtime_error("sem of dic_string is not defined");
  }
  virtual double var(double ddof = 1.0) {
    throw std::runtime_error("var of dic_string is not defined");
  }
  virtual double mad() {
    throw std::runtime_error("mad of dic_string is not defined");
  }
  dic_string max() {
    throw std::runtime_error("max of dic_string is not defined");
  }
  dic_string min() {
    throw std::runtime_error("min of dic_string is not defined");
  }
  std::string first(bool ignore_nulls = false);
  std::string last(bool ignore_nulls = false);
  dic_string at(size_t i) {
    throw std::runtime_error("at of dic_string is not defined");
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
  node_local<std::vector<size_t>>
  equal_prepare_multi_join(typed_dfcolumn<dic_string>& right);
  void compare_prepare(std::shared_ptr<typed_dfcolumn<dic_string>>&,
                       node_local<std::vector<size_t>>&,
                       node_local<std::vector<size_t>>&);
  void compare_prepare_immed(const std::string&,
                             node_local<std::vector<size_t>>&,
                             size_t&);
  virtual void contain_nulls_check();
  virtual std::shared_ptr<dfcolumn> head(size_t limit);
  virtual std::shared_ptr<dfcolumn> tail(size_t limit);
  virtual std::shared_ptr<dfcolumn> 
  union_columns(const std::vector<std::shared_ptr<dfcolumn>>& cols);
  // Only rank 0 has the dictionary to save memory
  std::shared_ptr<dict> dic;
  node_local<std::vector<size_t>> val;
  node_local<std::vector<size_t>> nulls;
  bool contain_nulls;
  virtual bool if_contain_nulls(){return contain_nulls;}
  virtual bool is_unique();
  virtual bool is_all_null();
  // for spill-restore
  virtual void spill_to_disk();
  virtual void restore_from_disk();
  virtual size_t calc_spill_size();
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
  typed_dfcolumn(const typed_dfcolumn<raw_string>& c) :
    dfcolumn(c), comp_words(c.comp_words), nulls(c.nulls) {}
  typed_dfcolumn(typed_dfcolumn<raw_string>&& c) :
    dfcolumn(std::move(c)), comp_words(std::move(c.comp_words)),
    nulls(std::move(c.nulls)) {}
  typed_dfcolumn& operator=(const typed_dfcolumn<raw_string>& c) {
    dfcolumn::operator=(c);
    comp_words = c.comp_words; nulls = c.nulls;
    return *this;
  }
  typed_dfcolumn& operator=(typed_dfcolumn<raw_string>&& c) {
    dfcolumn::operator=(std::move(c));
    comp_words = std::move(c.comp_words); nulls = std::move(c.nulls);
    return *this;
  }
  virtual size_t size(); 
  virtual std::vector<size_t> sizes(); 
  virtual node_local<std::vector<size_t>>
  filter_eq(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with eq for raw_string");
  }
  virtual node_local<std::vector<size_t>>
  filter_eq_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_neq(std::shared_ptr<dfcolumn>& right) {
    throw std::runtime_error("filtering with neq for raw_string");
  }
  virtual node_local<std::vector<size_t>>
  filter_neq_immed(std::shared_ptr<dfscalar>& right);
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
  virtual node_local<std::vector<size_t>>
  calc_hash_base_multi_join(std::shared_ptr<dfcolumn>& left) {
    throw std::runtime_error("join is not defined for raw_string");
  }
  virtual void
  calc_hash_base_multi_join(node_local<std::vector<size_t>>& hash, int shift,
                            std::shared_ptr<dfcolumn>& left) {
    throw std::runtime_error("join is not defined for raw_string");
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
  size(node_local<std::vector<size_t>>& local_grouped_idx,
       node_local<std::vector<size_t>>& local_idx_split,
       node_local<std::vector<std::vector<size_t>>>& hash_divide,
       node_local<std::vector<std::vector<size_t>>>& merge_map,
       node_local<size_t>& row_sizes) {
    throw std::runtime_error("size of raw_string is not defined");
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
  var(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0){
   throw std::runtime_error("var of raw_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  sem(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0){
   throw std::runtime_error("sem of raw_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  std(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0){
    throw std::runtime_error("std of raw_string is not defined");
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
  virtual std::shared_ptr<dfcolumn>
  mad(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("mad of raw_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  count_distinct(node_local<std::vector<size_t>>& local_grouped_idx,
                 node_local<std::vector<size_t>>& local_idx_split,
                 node_local<std::vector<std::vector<size_t>>>& hash_divide,
                 node_local<std::vector<std::vector<size_t>>>& merge_map,
                 node_local<size_t>& row_sizes) {
    throw std::runtime_error("count_distinct of raw_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  sum_distinct(node_local<std::vector<size_t>>& local_grouped_idx,
               node_local<std::vector<size_t>>& local_idx_split,
               node_local<std::vector<std::vector<size_t>>>& hash_divide,
               node_local<std::vector<std::vector<size_t>>>& merge_map,
               node_local<size_t>& row_sizes) {
    throw std::runtime_error("sum_distinct of raw_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  first(node_local<std::vector<size_t>>& local_grouped_idx,
        node_local<std::vector<size_t>>& local_idx_split,
        node_local<std::vector<std::vector<size_t>>>& hash_divide,
        node_local<std::vector<std::vector<size_t>>>& merge_map,
        node_local<size_t>& row_sizes,
        bool ignore_nulls) {
    throw std::runtime_error("first of raw_string is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  last(node_local<std::vector<size_t>>& local_grouped_idx,
       node_local<std::vector<size_t>>& local_idx_split,
       node_local<std::vector<std::vector<size_t>>>& hash_divide,
       node_local<std::vector<std::vector<size_t>>>& merge_map,
       node_local<size_t>& row_sizes,
       bool ignore_nulls) {
    throw std::runtime_error("last of raw_string is not defined");
  }
  virtual size_t count();
  raw_string sum() {
    throw std::runtime_error("sum of raw_string is not defined");
  }
  virtual double avg() {
    throw std::runtime_error("avg of raw_string is not defined");
  }
  virtual double std(double ddof = 1.0) {
    throw std::runtime_error("std of raw_string is not defined");
  }
  virtual double sem(double ddof = 1.0) {
    throw std::runtime_error("sem of raw_string is not defined");
  }
  virtual double var(double ddof = 1.0) {
    throw std::runtime_error("var of raw_string is not defined");
  }
  virtual double mad() {
    throw std::runtime_error("mad of raw_string is not defined");
  }
  raw_string max() {
    throw std::runtime_error("max of raw_string is not defined");
  }
  raw_string min() {
    throw std::runtime_error("min of raw_string is not defined");
  }
  raw_string at(size_t i) {
    throw std::runtime_error("at of raw_string is not defined");
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
  virtual std::shared_ptr<dfcolumn>
  union_columns(const std::vector<std::shared_ptr<dfcolumn>>& cols);
  void align_as(const std::vector<size_t>&);
  node_local<compressed_words> comp_words;
  node_local<std::vector<size_t>> nulls;
  bool contain_nulls;
  virtual bool if_contain_nulls(){return contain_nulls;}
  virtual bool is_unique() {
    throw std::runtime_error("is_unique is not defined for raw_string");
  }
  virtual bool is_all_null();
  // for spill-restore
  virtual void spill_to_disk();
  virtual void restore_from_disk();
  virtual size_t calc_spill_size();
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
  virtual node_local<std::vector<size_t>>
  filter_eq(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_eq_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_neq(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_neq_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_lt(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_lt_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_le(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_le_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_gt(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_gt_immed(std::shared_ptr<dfscalar>& right);
  virtual node_local<std::vector<size_t>>
  filter_ge(std::shared_ptr<dfcolumn>& right);
  virtual node_local<std::vector<size_t>>
  filter_ge_immed(std::shared_ptr<dfscalar>& right);
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
  var(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0){
    throw std::runtime_error("var of datetime is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  sem(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0){
    throw std::runtime_error("sem of datetime is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  std(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes,
      double ddof = 1.0){
    throw std::runtime_error("std of datetime is not defined");
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
  virtual std::shared_ptr<dfcolumn>
  mad(node_local<std::vector<size_t>>& local_grouped_idx,
      node_local<std::vector<size_t>>& local_idx_split,
      node_local<std::vector<std::vector<size_t>>>& hash_divide,
      node_local<std::vector<std::vector<size_t>>>& merge_map,
      node_local<size_t>& row_sizes) {
    throw std::runtime_error("mad of datetime is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  sum_distinct(node_local<std::vector<size_t>>& local_grouped_idx,
               node_local<std::vector<size_t>>& local_idx_split,
               node_local<std::vector<std::vector<size_t>>>& hash_divide,
               node_local<std::vector<std::vector<size_t>>>& merge_map,
               node_local<size_t>& row_sizes) {
    throw std::runtime_error("sum_distinct of datetime is not defined");
  }
  virtual std::shared_ptr<dfcolumn>
  first(node_local<std::vector<size_t>>& local_grouped_idx,
        node_local<std::vector<size_t>>& local_idx_split,
        node_local<std::vector<std::vector<size_t>>>& hash_divide,
        node_local<std::vector<std::vector<size_t>>>& merge_map,
        node_local<size_t>& row_sizes,
        bool ignore_nulls);
  virtual std::shared_ptr<dfcolumn>
  last(node_local<std::vector<size_t>>& local_grouped_idx,
       node_local<std::vector<size_t>>& local_idx_split,
       node_local<std::vector<std::vector<size_t>>>& hash_divide,
       node_local<std::vector<std::vector<size_t>>>& merge_map,
       node_local<size_t>& row_sizes,
       bool ignore_nulls);
  virtual double std(double ddof = 1.0) {
    throw std::runtime_error("std of datetime is not defined");
  }
  virtual double sem(double ddof = 1.0) {
    throw std::runtime_error("sem of datetime is not defined");
  }
  virtual double var(double ddof = 1.0) {
    throw std::runtime_error("var of datetime is not defined");
  }
  virtual double mad() {
    throw std::runtime_error("mad of datetime is not defined");
  }
  virtual double avg() {
    throw std::runtime_error("avg of datetime is not defined");
  }
  datetime_t first(bool ignore_nulls = false);
  datetime_t last(bool ignore_nulls = false);
  virtual std::shared_ptr<dfcolumn>
  datetime_extract(datetime_type kind);
  virtual std::shared_ptr<dfcolumn>
  datetime_diff(const std::shared_ptr<dfcolumn>& right, datetime_type kind);
  virtual std::shared_ptr<dfcolumn>
  datetime_diff_im(datetime_t right, datetime_type kind);
  virtual std::shared_ptr<dfcolumn>
  rdatetime_diff_im(datetime_t right, datetime_type kind);
  virtual std::shared_ptr<dfcolumn>
  datetime_add(const std::shared_ptr<dfcolumn>& right, datetime_type kind);
  virtual std::shared_ptr<dfcolumn>
  datetime_add_im(int right, datetime_type kind);
  virtual std::shared_ptr<dfcolumn>
  datetime_sub(const std::shared_ptr<dfcolumn>& right, datetime_type kind);
  virtual std::shared_ptr<dfcolumn>
  datetime_sub_im(int right, datetime_type kind);
  virtual std::shared_ptr<dfcolumn>
  datetime_truncate(datetime_type kind);
  virtual std::shared_ptr<dfcolumn>
  datetime_months_between(const std::shared_ptr<dfcolumn>& right);
  virtual std::shared_ptr<dfcolumn>
  datetime_next_day(const std::shared_ptr<dfcolumn>& right);
  virtual std::shared_ptr<dfcolumn>
  datetime_next_day_im(int right);
  virtual void debug_print();
  virtual std::shared_ptr<dfcolumn> head(size_t limit);
  virtual std::shared_ptr<dfcolumn> tail(size_t limit);
  virtual std::shared_ptr<dfcolumn>
  union_columns(const std::vector<std::shared_ptr<dfcolumn>>& cols);
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

template <class T>
T dfcolumn::first(bool ignore_nulls) {
  try {
    return dynamic_cast<typed_dfcolumn<T>&>(*this).first(ignore_nulls);
  } catch (std::bad_cast& e) {
    throw std::runtime_error("type mismatch of first<T>()");
  }
}

template <>
std::string dfcolumn::first(bool ignore_nulls);

template <class T>
T dfcolumn::last(bool ignore_nulls) {
  try {
    return dynamic_cast<typed_dfcolumn<T>&>(*this).last(ignore_nulls);
  } catch (std::bad_cast& e) {
    throw std::runtime_error("type mismatch of last<T>()");
  }
}

template <>
std::string dfcolumn::last(bool ignore_nulls);

template <class T>
T dfcolumn::at(size_t i) {
  try {
    return dynamic_cast<typed_dfcolumn<T>&>(*this).at(i);
  } catch (std::bad_cast& e) {
    throw std::runtime_error("type mismatch of at<T>()");
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
#pragma _NEC vovertake
#pragma _NEC vob
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
#pragma _NEC vovertake
#pragma _NEC vob
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


template <class T>
std::shared_ptr<dfcolumn>
create_null_column(const std::vector<size_t>& sizes) {
  auto nlsizes = make_node_local_scatter(sizes);
  auto val = make_node_local_allocate<std::vector<T>>();
  auto nulls = val.map(+[](std::vector<T>& val, size_t size) {
      val.resize(size);
      auto valp = val.data();
      auto max = std::numeric_limits<T>::max();
      std::vector<size_t> nulls(size);
      auto nullsp = nulls.data();
      for(size_t i = 0; i < size; i++) {
        valp[i] = max;
        nullsp[i] = i;
      }
      return nulls;
    }, nlsizes);
  return std::make_shared<typed_dfcolumn<T>>(std::move(val), std::move(nulls));
}

template <>
std::shared_ptr<dfcolumn>
create_null_column<std::string>(const std::vector<size_t>& sizes);

template <>
std::shared_ptr<dfcolumn>
create_null_column<dic_string>(const std::vector<size_t>& sizes);

template <>
std::shared_ptr<dfcolumn>
create_null_column<raw_string>(const std::vector<size_t>& sizes);

template <>
std::shared_ptr<dfcolumn>
create_null_column<datetime>(const std::vector<size_t>& sizes);


// for spill-restore

// for doing spill-restore in RAII manner, which is exception safe
// can be called nested way:
// call spill in dtor only when state is changed in ctor
struct use_dfcolumn {
  void init() {
    prev_state.resize(dfcolumn_to_use.size());
    for(size_t i = 0; i < dfcolumn_to_use.size(); i++) {
      prev_state[i] = dfcolumn_to_use[i]->spill_state;
      if(prev_state[i] == spill_state_type::spilled) {
        dfcolumn_to_use[i]->restore();
      }
    }
  }
  template <class T>
  use_dfcolumn(const std::vector<std::string>& colname,
               T& tbl) {
    auto size = colname.size();
    dfcolumn_to_use.resize(size);
    for(size_t i = 0; i < size; i++) {
      dfcolumn_to_use[i] = tbl.raw_column(colname[i]);
    }
    init();
  }
  use_dfcolumn(const std::shared_ptr<dfcolumn>& c) : dfcolumn_to_use({c}) {
    init();
  }
  use_dfcolumn(const std::vector<std::shared_ptr<dfcolumn>>& cs) :
    dfcolumn_to_use(cs) {
    init();
  }
  ~use_dfcolumn() {
    for(size_t i = 0; i < dfcolumn_to_use.size(); i++) {
      if(prev_state[i] == spill_state_type::spilled) {
        dfcolumn_to_use[i]->spill();
      }
    }
  }
  use_dfcolumn(const use_dfcolumn&) = delete;
  use_dfcolumn & operator=(const use_dfcolumn&) = delete;

  std::vector<std::shared_ptr<dfcolumn>> dfcolumn_to_use;
  std::vector<spill_state_type> prev_state;
};

// LRU: https://stackoverflow.com/questions/2504178/lru-cache-design
struct dfcolumn_spill_queue_t {
  dfcolumn_spill_queue_t() : capacity_initialized(false),
                             number_of_put(0), number_of_get(0),
                             number_of_restore_from_disk(0),
                             number_of_spill_to_disk(0),
                             spill_time(0),
                             restore_time(0) {}
  // put column first time; kind of register the column
  void put_new(dfcolumn*);
  // put column and return used memory; increase queue_capacy
  void put(dfcolumn*);
  void get(dfcolumn*);
  void remove(dfcolumn*);
  // if some columns are used by get, queue_capacity will be increased
  // if the columns is put again
  void set_queue_capacity(size_t s)
    {queue_capacity = s; capacity_initialized = true;}
  void init_queue_capacity();
  void spill_until_capacity();
  void spill_all();
  void spill_one();
  void debug_print() {
    std::cout << "queue_capacity: " << queue_capacity
              << ", current_usage: " << current_usage << std::endl;
    print_stat();
  }
  void print_stat() {
    std::cout << "number of put: " << number_of_put << "\n"
              << "number of get: " << number_of_get << "\n"
              << "number of spill to disk: " << number_of_spill_to_disk
              << "\n"
              << "number of restore from disk: "
              << number_of_restore_from_disk << "\n"
              << "spill time: " << spill_time << "\n"
              << "restore time: " << restore_time << std::endl;
  }
  void reset_stat() {
    number_of_put = 0;
    number_of_get = 0;
    number_of_restore_from_disk = 0;
    number_of_spill_to_disk = 0;
    spill_time = 0;
    restore_time = 0;
  }
  std::list<dfcolumn*> item_list;
  std::unordered_map<dfcolumn*, decltype(item_list.begin())> item_map;
  ssize_t queue_capacity; // can be negative if large column is get
  ssize_t current_usage; // used memory; positive or 0
  bool capacity_initialized;

  size_t number_of_put;
  size_t number_of_get;
  size_t number_of_restore_from_disk;
  size_t number_of_spill_to_disk;
  double spill_time;
  double restore_time;
};

extern dfcolumn_spill_queue_t dfcolumn_spill_queue;

std::vector<int> 
get_bool_mask_helper(std::vector<size_t>& local_idx, size_t size);

std::shared_ptr<dfcolumn>
create_boolean_column(node_local<std::vector<size_t>>& filter_idx,
                      node_local<std::vector<size_t>>& ret_nulls,
                      const std::vector<size_t>& sizes);

}
#endif
