#ifndef DFOPERATOR_HPP
#define DFOPERATOR_HPP

#include <regex>
#include "dftable.hpp"
#include "set_operations.hpp"

namespace frovedis {

struct dfoperator {
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const = 0;
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  hash_join(dftable_base& left, dftable_base& right,
            node_local<std::vector<size_t>>& left_idx,
            node_local<std::vector<size_t>>& right_idx) const {
    throw std::runtime_error("hash_join on this operator is not implemented");
  }
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
  outer_hash_join(dftable_base& left, dftable_base& right,
            node_local<std::vector<size_t>>& left_idx,
            node_local<std::vector<size_t>>& right_idx) const {
    throw std::runtime_error
      ("outer_hash_join on this operator is not implemented");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  bcast_join(dftable_base& left, dftable_base& right,
             node_local<std::vector<size_t>>& left_idx,
             node_local<std::vector<size_t>>& right_idx) const {
    throw std::runtime_error("bcast_join on this operator is not implemented");
  }
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
  outer_bcast_join(dftable_base& left, dftable_base& right,
                   node_local<std::vector<size_t>>& left_idx,
                   node_local<std::vector<size_t>>& right_idx) const {
    throw std::runtime_error
      ("outer_bcast_join on this operator is not implemented");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  star_join(dftable_base& left, dftable_base& right,
            node_local<std::vector<size_t>>& left_idx,
            node_local<std::vector<size_t>>& right_idx) const {
    throw std::runtime_error("bcast_join on this operator is not implemented");
  }
};

struct dfoperator_eq : public dfoperator {
  dfoperator_eq(const std::string& left, const std::string& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = t.column(left);
    auto right_column = t.column(right);
    return left_column->filter_eq(right_column);
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    hash_join(dftable_base& left_t, dftable_base& right_t,
              node_local<std::vector<size_t>>& left_idx,
              node_local<std::vector<size_t>>& right_idx) const {
    auto left_column = left_t.raw_column(left);
    auto right_column = right_t.raw_column(right);
    return left_column->hash_join_eq(right_column, left_idx, right_idx);
  }
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
    outer_hash_join(dftable_base& left_t, dftable_base& right_t,
                    node_local<std::vector<size_t>>& left_idx,
                    node_local<std::vector<size_t>>& right_idx) const {
    auto left_column = left_t.raw_column(left);
    auto right_column = right_t.raw_column(right);
    return left_column->outer_hash_join_eq(right_column, left_idx, right_idx);
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join(dftable_base& left_t, dftable_base& right_t,
               node_local<std::vector<size_t>>& left_idx,
               node_local<std::vector<size_t>>& right_idx) const {
    auto left_column = left_t.raw_column(left);
    auto right_column = right_t.raw_column(right);
    return left_column->bcast_join_eq(right_column, left_idx, right_idx);
  }
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
    outer_bcast_join(dftable_base& left_t, dftable_base& right_t,
                     node_local<std::vector<size_t>>& left_idx,
                     node_local<std::vector<size_t>>& right_idx) const {
    auto left_column = left_t.raw_column(left);
    auto right_column = right_t.raw_column(right);
    return left_column->outer_bcast_join_eq(right_column, left_idx, right_idx);
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    star_join(dftable_base& left_t, dftable_base& right_t,
              node_local<std::vector<size_t>>& left_idx,
              node_local<std::vector<size_t>>& right_idx) const {
    auto left_column = left_t.raw_column(left);
    auto right_column = right_t.raw_column(right);
    return left_column->star_join_eq(right_column, left_idx, right_idx);
  }

  std::string left, right;
};

template <class T>
struct dfoperator_eq_immed : public dfoperator {
  dfoperator_eq_immed(const std::string& left, const T& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column =
      std::dynamic_pointer_cast<typed_dfcolumn<T>>(t.column(left));
    if(!left_column)
      throw std::runtime_error("dfoperator_eq_immed: column type is different");
    return left_column->filter_eq_immed(right);
  }
  std::string left;
  T right;
};

struct dfoperator_neq : public dfoperator {
  dfoperator_neq(const std::string& left, const std::string& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = t.column(left);
    auto right_column = t.column(right);
    return left_column->filter_neq(right_column);
  }
  std::string left, right;
};

template <class T>
struct dfoperator_neq_immed : public dfoperator {
  dfoperator_neq_immed(const std::string& left, const T& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column =
      std::dynamic_pointer_cast<typed_dfcolumn<T>>(t.column(left));
    if(!left_column)
      throw std::runtime_error
        ("dfoperator_neq_immed: column type is different");
    return left_column->filter_neq_immed(right);
  }
  std::string left;
  T right;
};

struct dfoperator_lt : public dfoperator {
  dfoperator_lt(const std::string& left, const std::string& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = t.column(left);
    auto right_column = t.column(right);
    return left_column->filter_lt(right_column);
  }
  std::string left, right;
};

template <class T>
struct dfoperator_lt_immed : public dfoperator {
  dfoperator_lt_immed(const std::string& left, const T& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column =
      std::dynamic_pointer_cast<typed_dfcolumn<T>>(t.column(left));
    if(!left_column)
      throw std::runtime_error("dfoperator_lt_immed: column type is different");
    return left_column->filter_lt_immed(right);
  }
  std::string left;
  T right;
};

struct dfoperator_le : public dfoperator {
  dfoperator_le(const std::string& left, const std::string& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = t.column(left);
    auto right_column = t.column(right);
    return left_column->filter_le(right_column);
  }
  std::string left, right;
};

template <class T>
struct dfoperator_le_immed : public dfoperator {
  dfoperator_le_immed(const std::string& left, const T& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column =
      std::dynamic_pointer_cast<typed_dfcolumn<T>>(t.column(left));
    if(!left_column)
      std::runtime_error("dfoperator_le_immed: column type is different");
    return left_column->filter_le_immed(right);
  }
  std::string left;
  T right;
};

struct dfoperator_gt : public dfoperator {
  dfoperator_gt(const std::string& left, const std::string& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = t.column(left);
    auto right_column = t.column(right);
    return left_column->filter_gt(right_column);
  }
  std::string left, right;
};

template <class T>
struct dfoperator_gt_immed : public dfoperator {
  dfoperator_gt_immed(const std::string& left, const T& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column =
      std::dynamic_pointer_cast<typed_dfcolumn<T>>(t.column(left));
    if(!left_column)
      throw std::runtime_error("dfoperator_gt_immed: column type is different");
    return left_column->filter_gt_immed(right);
  }
  std::string left;
  T right;
};

struct dfoperator_ge : public dfoperator {
  dfoperator_ge(const std::string& left, const std::string& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = t.column(left);
    auto right_column = t.column(right);
    return left_column->filter_ge(right_column);
  }
  std::string left, right;
};

template <class T>
struct dfoperator_ge_immed : public dfoperator {
  dfoperator_ge_immed(const std::string& left, const T& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column =
      std::dynamic_pointer_cast<typed_dfcolumn<T>>(t.column(left));
    if(!left_column)
      throw std::runtime_error("dfoperator_ge_immed: column type is different");
    return left_column->filter_ge_immed(right);
  }
  std::string left;
  T right;
};

struct dfoperator_is_null : public dfoperator {
  dfoperator_is_null(const std::string& col) : col(col) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    return t.column(col)->filter_is_null();
  }
  std::string col;
};

struct dfoperator_is_not_null : public dfoperator {
  dfoperator_is_not_null(const std::string& col) : col(col) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    return t.column(col)->filter_is_not_null();
  }
  std::string col;
};

template <class T>
std::shared_ptr<dfoperator>
eq_im(const std::string& left, const T& right) {
  return std::make_shared<dfoperator_eq_immed<T>>(left,right);
}

std::shared_ptr<dfoperator>
eq(const std::string& left, const std::string& right);

template <class T>
std::shared_ptr<dfoperator>
neq_im(const std::string& left, const T& right) {
  return std::make_shared<dfoperator_neq_immed<T>>(left,right);
}

std::shared_ptr<dfoperator>
neq(const std::string& left, const std::string& right);

std::shared_ptr<dfoperator>
lt(const std::string& left, const std::string& right);

std::shared_ptr<dfoperator>
le(const std::string& left, const std::string& right);

std::shared_ptr<dfoperator>
gt(const std::string& left, const std::string& right);

std::shared_ptr<dfoperator>
ge(const std::string& left, const std::string& right);

template <class T>
std::shared_ptr<dfoperator>
le_im(const std::string& left, const T& right) {
  return std::make_shared<dfoperator_le_immed<T>>(left,right);
}

template <class T>
std::shared_ptr<dfoperator>
lt_im(const std::string& left, const T& right) {
  return std::make_shared<dfoperator_lt_immed<T>>(left,right);
}

template <class T>
std::shared_ptr<dfoperator>
gt_im(const std::string& left, const T& right) {
  return std::make_shared<dfoperator_gt_immed<T>>(left,right);
}

template <class T>
std::shared_ptr<dfoperator>
ge_im(const std::string& left, const T& right) {
  return std::make_shared<dfoperator_ge_immed<T>>(left,right);
}

std::shared_ptr<dfoperator>
is_null(const std::string& col);

std::shared_ptr<dfoperator>
is_not_null(const std::string& col);

struct dfoperator_regex : public dfoperator {
  dfoperator_regex(const std::string& left, const std::string& pattern) :
    left(left), pattern(pattern) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column =
      std::dynamic_pointer_cast<typed_dfcolumn<std::string>>(t.column(left));
    if(!left_column)
      throw std::runtime_error("dfoperator_regex: column type is not string");
    return left_column->filter_regex(pattern);
  }
  std::string left;
  std::string pattern;
};

struct dfoperator_not_regex : public dfoperator {
  dfoperator_not_regex(const std::string& left, const std::string& pattern) :
    left(left), pattern(pattern) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column =
      std::dynamic_pointer_cast<typed_dfcolumn<std::string>>(t.column(left));
    if(!left_column)
      throw std::runtime_error("dfoperator_regex: column type is not string");
    return left_column->filter_not_regex(pattern);
  }
  std::string left;
  std::string pattern;
};

std::shared_ptr<dfoperator>
is_regex(const std::string& col, const std::string& pattern);

std::shared_ptr<dfoperator>
is_not_regex(const std::string& col, const std::string& pattern);

std::vector<size_t>
convert_filtered_idx(std::vector<size_t>& org_idx,
                     std::vector<size_t>& created_idx);

// need to be here because filtered_dftable depends on operators, 
// and dfoperator_and depends on the definition of filtered_dftable
class filtered_dftable : public dftable_base {
public:
  filtered_dftable(dftable_base& table,
                   node_local<std::vector<size_t>>&& filtered_idx) :
    dftable_base(table), filtered_idx(std::move(filtered_idx)) {}
  filtered_dftable(dftable_base& table,
                   const node_local<std::vector<size_t>>& filtered_idx) :
    dftable_base(table), filtered_idx(filtered_idx) {}
  virtual size_t num_row();
  virtual dftable select(const std::vector<std::string>& cols);
  virtual filtered_dftable filter(const std::shared_ptr<dfoperator>& op);
  virtual sorted_dftable sort(const std::string& name);
  virtual sorted_dftable sort_desc(const std::string& name);
  // it is OK to call dftable_base's join and group_by, 
  // which calls get_local_index
  virtual node_local<std::vector<size_t>> get_local_index() {
    return filtered_idx;
  }
  virtual std::shared_ptr<dfcolumn> column(const std::string& name);
  virtual void debug_print();
private:
  node_local<std::vector<size_t>> filtered_idx;
};

struct dfoperator_and : public dfoperator {
  dfoperator_and(const std::shared_ptr<dfoperator>& left,
                 const std::shared_ptr<dfoperator>& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_filtered_idx = left->filter(t);
    auto right_filtered_idx = right->filter(t);
    return left_filtered_idx.map(set_intersection<size_t>, right_filtered_idx);
  }
  std::shared_ptr<dfoperator> left;
  std::shared_ptr<dfoperator> right;
};

std::shared_ptr<dfoperator>
and_op(const std::shared_ptr<dfoperator>& left,
       const std::shared_ptr<dfoperator>& right);

struct dfoperator_or : public dfoperator {
  dfoperator_or(const std::shared_ptr<dfoperator>& left,
                const std::shared_ptr<dfoperator>& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_filtered_idx = left->filter(t);
    auto right_filtered_idx = right->filter(t);
    return left_filtered_idx.map(set_union<size_t>, right_filtered_idx);
  }
  std::shared_ptr<dfoperator> left;
  std::shared_ptr<dfoperator> right;
};

std::shared_ptr<dfoperator>
or_op(const std::shared_ptr<dfoperator>& left,
      const std::shared_ptr<dfoperator>& right);

}
#endif
