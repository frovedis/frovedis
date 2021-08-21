#ifndef DFOPERATOR_HPP
#define DFOPERATOR_HPP

#include <regex>
#include "dftable.hpp"
#include "../core/set_operations.hpp"

namespace frovedis {

struct dfoperator {
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const = 0;
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const = 0;
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
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    throw std::runtime_error
      ("columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("columns_to_use on this operator is not implemented");
  }
  virtual std::shared_ptr<dfoperator> exchange_lr() {
    throw std::runtime_error
      ("exchange_lr (used for bcast_join) on this operator is not implemented");
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
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
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
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    // use raw_column to avoid useless extract in the case of filtered_dftable
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }
  virtual std::shared_ptr<dfoperator> exchange_lr() {
    return std::make_shared<dfoperator_eq>(dfoperator_eq(right, left));
  }
  std::string left, right;
};

struct dfoperator_neq : public dfoperator {
  dfoperator_neq(const std::string& left, const std::string& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = t.column(left);
    auto right_column = t.column(right);
    return left_column->filter_neq(right_column);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_eq(left, right).filter(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }
  std::string left, right;
};

inline 
node_local<std::vector<size_t>>
dfoperator_eq::not_filter(dftable_base& t) const {
  return dfoperator_neq(left, right).filter(t);
}

template <class T>
struct dfoperator_eq_immed : public dfoperator {
  dfoperator_eq_immed(const std::string& left, const T& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return t.column(left)->filter_eq_immed(right_scalar);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("columns_to_use on this operator is not implemented");
  }
  std::string left;
  T right;
};

template <class T>
struct dfoperator_neq_immed : public dfoperator {
  dfoperator_neq_immed(const std::string& left, const T& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return t.column(left)->filter_neq_immed(right_scalar);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_eq_immed<T>(left, right).filter(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("columns_to_use on this operator is not implemented");
  }
  std::string left;
  T right;
};

template <class T>
node_local<std::vector<size_t>>
dfoperator_eq_immed<T>::not_filter(dftable_base& t) const {
  return dfoperator_neq_immed<T>(left, right).filter(t);
}

struct dfoperator_lt : public dfoperator {
  dfoperator_lt(const std::string& left, const std::string& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = t.column(left);
    auto right_column = t.column(right);
    return left_column->filter_lt(right_column);
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join(dftable_base& left_t, dftable_base& right_t,
               node_local<std::vector<size_t>>& left_idx,
               node_local<std::vector<size_t>>& right_idx) const {
    auto left_column = left_t.raw_column(left);
    auto right_column = right_t.raw_column(right);
    return left_column->bcast_join_lt(right_column, left_idx, right_idx);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }
  virtual std::shared_ptr<dfoperator> exchange_lr();
  std::string left, right;
};

struct dfoperator_ge : public dfoperator {
  dfoperator_ge(const std::string& left, const std::string& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = t.column(left);
    auto right_column = t.column(right);
    return left_column->filter_ge(right_column);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_lt(left, right).filter(t);
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join(dftable_base& left_t, dftable_base& right_t,
               node_local<std::vector<size_t>>& left_idx,
               node_local<std::vector<size_t>>& right_idx) const {
    auto left_column = left_t.raw_column(left);
    auto right_column = right_t.raw_column(right);
    return left_column->bcast_join_ge(right_column, left_idx, right_idx);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }
  virtual std::shared_ptr<dfoperator> exchange_lr();
  std::string left, right;
};

inline
node_local<std::vector<size_t>>
dfoperator_lt::not_filter(dftable_base& t) const {
  return dfoperator_ge(left, right).filter(t);
}

struct dfoperator_le : public dfoperator {
  dfoperator_le(const std::string& left, const std::string& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = t.column(left);
    auto right_column = t.column(right);
    return left_column->filter_le(right_column);
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join(dftable_base& left_t, dftable_base& right_t,
               node_local<std::vector<size_t>>& left_idx,
               node_local<std::vector<size_t>>& right_idx) const {
    auto left_column = left_t.raw_column(left);
    auto right_column = right_t.raw_column(right);
    return left_column->bcast_join_le(right_column, left_idx, right_idx);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }
  virtual std::shared_ptr<dfoperator> exchange_lr();
  std::string left, right;
};

struct dfoperator_gt : public dfoperator {
  dfoperator_gt(const std::string& left, const std::string& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = t.column(left);
    auto right_column = t.column(right);
    return left_column->filter_gt(right_column);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_le(left, right).filter(t);
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join(dftable_base& left_t, dftable_base& right_t,
               node_local<std::vector<size_t>>& left_idx,
               node_local<std::vector<size_t>>& right_idx) const {
    auto left_column = left_t.raw_column(left);
    auto right_column = right_t.raw_column(right);
    return left_column->bcast_join_gt(right_column, left_idx, right_idx);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }
  virtual std::shared_ptr<dfoperator> exchange_lr();
  std::string left, right;
};

inline std::shared_ptr<dfoperator> dfoperator_lt::exchange_lr() {
  return std::make_shared<dfoperator_gt>(dfoperator_gt(right, left));
}

inline std::shared_ptr<dfoperator> dfoperator_ge::exchange_lr() {
  return std::make_shared<dfoperator_le>(dfoperator_le(right, left));
}

inline std::shared_ptr<dfoperator> dfoperator_le::exchange_lr() {
  return std::make_shared<dfoperator_ge>(dfoperator_ge(right, left));
}

inline std::shared_ptr<dfoperator> dfoperator_gt::exchange_lr() {
  return std::make_shared<dfoperator_lt>(dfoperator_lt(right, left));
}


inline
node_local<std::vector<size_t>>
dfoperator_le::not_filter(dftable_base& t) const {
  return dfoperator_gt(left, right).filter(t);
}

template <class T>
struct dfoperator_lt_immed : public dfoperator {
  dfoperator_lt_immed(const std::string& left, const T& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return t.column(left)->filter_lt_immed(right_scalar);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("columns_to_use on this operator is not implemented");
  }
  std::string left;
  T right;
};

template <class T>
struct dfoperator_ge_immed : public dfoperator {
  dfoperator_ge_immed(const std::string& left, const T& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return t.column(left)->filter_ge_immed(right_scalar);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_lt_immed<T>(left, right).filter(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("columns_to_use on this operator is not implemented");
  }
  std::string left;
  T right;
};

template <class T>
node_local<std::vector<size_t>> 
dfoperator_lt_immed<T>::not_filter(dftable_base& t) const {
  return dfoperator_ge_immed<T>(left, right).filter(t);
}

template <class T>
struct dfoperator_le_immed : public dfoperator {
  dfoperator_le_immed(const std::string& left, const T& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return t.column(left)->filter_le_immed(right_scalar);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("columns_to_use on this operator is not implemented");
  }
  std::string left;
  T right;
};

template <class T>
struct dfoperator_gt_immed : public dfoperator {
  dfoperator_gt_immed(const std::string& left, const T& right) :
    left(left), right(right) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return t.column(left)->filter_gt_immed(right_scalar);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_le_immed<T>(left, right).filter(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("columns_to_use on this operator is not implemented");
  }
  std::string left;
  T right;
};

template <class T>
node_local<std::vector<size_t>>
dfoperator_le_immed<T>::not_filter(dftable_base& t) const {
  return dfoperator_gt_immed<T>(left, right).filter(t);
}

struct dfoperator_is_null : public dfoperator {
  dfoperator_is_null(const std::string& col) : col(col) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    return t.column(col)->filter_is_null();
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(col)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  std::string col;
};

struct dfoperator_is_not_null : public dfoperator {
  dfoperator_is_not_null(const std::string& col) : col(col) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    return t.column(col)->filter_is_not_null();
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_is_null(col).filter(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(col)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  std::string col;
};

inline 
node_local<std::vector<size_t>>
dfoperator_is_null::not_filter(dftable_base& t) const {
  return dfoperator_is_not_null(col).filter(t);
}

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
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
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
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_regex(left, pattern).filter(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  std::string left;
  std::string pattern;
};

inline
node_local<std::vector<size_t>>
dfoperator_regex::not_filter(dftable_base& t) const {
  return dfoperator_not_regex(left, pattern).filter(t);
}

std::shared_ptr<dfoperator>
is_regex(const std::string& col, const std::string& pattern);

std::shared_ptr<dfoperator>
is_not_regex(const std::string& col, const std::string& pattern);

struct dfoperator_like : public dfoperator {
  dfoperator_like(const std::string& left, const std::string& pattern) :
    left(left), pattern(pattern) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto tcol = t.column(left);
    if(tcol->dtype() == "dic_string") {
      auto left_column_dic =
        std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(tcol);
      return left_column_dic->filter_like(pattern);
    }
    else if(tcol->dtype() == "raw_string") {
      auto left_column_raw =
        std::dynamic_pointer_cast<typed_dfcolumn<raw_string>>(tcol);
      return left_column_raw->filter_like(pattern);
    }
    else {
      auto words = tcol->as_words();
      auto nulls = tcol->get_nulls();
      auto tmp = std::make_shared<typed_dfcolumn<dic_string>>(
                     std::move(words), std::move(nulls));
      return tmp->filter_like(pattern);
    }
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  std::string left;
  std::string pattern;
};

struct dfoperator_not_like : public dfoperator {
  dfoperator_not_like(const std::string& left, const std::string& pattern) :
    left(left), pattern(pattern) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto tcol = t.column(left);
    if(tcol->dtype() == "dic_string") {
      auto left_column_dic =
        std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(tcol);
      return left_column_dic->filter_not_like(pattern);
    } 
    else if(tcol->dtype() == "raw_string") {
      auto left_column_raw =
        std::dynamic_pointer_cast<typed_dfcolumn<raw_string>>(tcol);
      return left_column_raw->filter_not_like(pattern);
    } 
    else {
      auto words = tcol->as_words();
      auto nulls = tcol->get_nulls();
      auto tmp = std::make_shared<typed_dfcolumn<dic_string>>(
                     std::move(words), std::move(nulls));
      return tmp->filter_not_like(pattern);
    }
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    //return dfoperator_not_regex(left, pattern).filter(t);
    return dfoperator_like(left, pattern).filter(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  std::string left;
  std::string pattern;
};

inline
node_local<std::vector<size_t>>
dfoperator_like::not_filter(dftable_base& t) const {
  //return dfoperator_not_regex(left, pattern).filter(t);
  return dfoperator_not_like(left, pattern).filter(t);
}

std::shared_ptr<dfoperator>
is_like(const std::string& col, const std::string& pattern);

std::shared_ptr<dfoperator>
is_not_like(const std::string& col, const std::string& pattern);

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
  virtual std::vector<size_t> num_rows();
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
  virtual dftable_base* clone();
  virtual dftable_base* drop_cols(const std::vector<std::string>& cols);
  virtual dftable_base* rename_cols(const std::string& name,
                                    const std::string& name2);
  filtered_dftable& drop(const std::string& name);
  filtered_dftable& rename(const std::string& name, const std::string& name2);
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
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    auto left_filtered_idx = left->not_filter(t);
    auto right_filtered_idx = right->not_filter(t);
    return left_filtered_idx.map(set_union<size_t>, right_filtered_idx);
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  bcast_join(dftable_base& left, dftable_base& right,
             node_local<std::vector<size_t>>& left_idx,
             node_local<std::vector<size_t>>& right_idx) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    auto left_use = left->columns_to_use(t);
    auto right_use = right->columns_to_use(t);
    left_use.insert(left_use.end(), right_use.begin(), right_use.end());
    return left_use;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    auto left_use = left->columns_to_use(t1, t2);
    auto right_use = right->columns_to_use(t1, t2);
    left_use.insert(left_use.end(), right_use.begin(), right_use.end());
    return left_use;
  }
  virtual std::shared_ptr<dfoperator> exchange_lr() {
    return std::make_shared<dfoperator_and>
      (dfoperator_and(left->exchange_lr(), right->exchange_lr()));
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
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    auto left_filtered_idx = left->not_filter(t);
    auto right_filtered_idx = right->not_filter(t);
    return left_filtered_idx.map(set_intersection<size_t>, right_filtered_idx);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    auto left_use = left->columns_to_use(t);
    auto right_use = right->columns_to_use(t);
    left_use.insert(left_use.end(), right_use.begin(), right_use.end());
    return left_use;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    auto left_use = left->columns_to_use(t1, t2);
    auto right_use = right->columns_to_use(t1, t2);
    left_use.insert(left_use.end(), right_use.begin(), right_use.end());
    return left_use;
  }
  std::shared_ptr<dfoperator> left;
  std::shared_ptr<dfoperator> right;
};

std::shared_ptr<dfoperator>
or_op(const std::shared_ptr<dfoperator>& left,
      const std::shared_ptr<dfoperator>& right);

// not shoud use not_filter, instead of set_difference from 0...N,
// considering the NULL and performance
struct dfoperator_not : public dfoperator {
  dfoperator_not(const std::shared_ptr<dfoperator>& op) : op(op) {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    return op->not_filter(t);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return op->filter(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return op->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return op->columns_to_use(t1, t2);
  }
  std::shared_ptr<dfoperator> op;
};

std::shared_ptr<dfoperator>
not_op(const std::shared_ptr<dfoperator>& op);

struct dfoperator_multi_eq : public dfoperator {
  dfoperator_multi_eq(const std::vector<std::string>& leftv,
                      const std::vector<std::string>& rightv) :
    leftv(leftv), rightv(rightv) {
    if(leftv.size() != rightv.size())
      throw std::runtime_error("number of columns is different");
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    throw std::runtime_error("filter on this operator is not implemented");
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    throw std::runtime_error("not_filter on this operator is not implemented");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    hash_join(dftable_base& left_t, dftable_base& right_t,
              node_local<std::vector<size_t>>& left_idx,
              node_local<std::vector<size_t>>& right_idx) const;
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join(dftable_base& left, dftable_base& right,
               node_local<std::vector<size_t>>& left_idx,
               node_local<std::vector<size_t>>& right_idx) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    throw std::runtime_error
      ("one arg of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    std::vector<std::shared_ptr<dfcolumn>> ret;
    for(auto& c: leftv) ret.push_back(t1.raw_column(c));
    for(auto& c: rightv) ret.push_back(t2.raw_column(c));
    return ret;
  }
  virtual std::shared_ptr<dfoperator> exchange_lr() {
    return std::make_shared<dfoperator_multi_eq>
      (dfoperator_multi_eq(rightv, leftv));
  }
  std::shared_ptr<dfoperator> op;
  std::vector<std::string> leftv, rightv;
};

void filter_idx(std::vector<size_t>& idx,
                const std::vector<size_t>& filter);

std::shared_ptr<dfoperator>
multi_eq(const std::vector<std::string>& left,
         const std::vector<std::string>& right);

struct dfoperator_cross : public dfoperator {
  dfoperator_cross() {}
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    throw std::runtime_error("filter on this operator is not implemented");
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    throw std::runtime_error("not_filter on this operator is not implemented");
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
  bcast_join(dftable_base& left, dftable_base& right,
             node_local<std::vector<size_t>>& left_idx,
             node_local<std::vector<size_t>>& right_idx) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    throw std::runtime_error
      ("one arg of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return std::vector<std::shared_ptr<dfcolumn>>();
  }
  virtual std::shared_ptr<dfoperator> exchange_lr() {
    return std::make_shared<dfoperator_cross>(*this);
  }
};

std::shared_ptr<dfoperator> cross();

template <class T> 
std::vector<size_t> 
dftable_base::get_loc(const std::string& col, const T& val) {
  auto fdf = filter(eq_im(col, val));
  require(fdf.num_row() > 0, 
  "get_loc: given value doesn't found in column: " + col + "!\n");
  auto lidx = fdf.get_local_index();
  auto sizes = num_rows();
  auto nproc = sizes.size();
  std::vector<size_t> myst(nproc); myst[0] = 0;
  for(size_t i = 1; i < nproc; ++i) myst[i] = myst[i - 1] + sizes[i - 1];
  auto lmyst = make_node_local_scatter(myst);
  auto gidx = lidx.map(+[](const std::vector<size_t>& vec, 
                           size_t myst) { return vec + myst; }
                       , lmyst);
  return gidx.template moveto_dvector<size_t>().gather();
}

template <class T>
dftable dftable_base::is_in_im(const std::string& target_col,
                               const std::vector<T>& target_values) {
  dftable right_t;
  std::string right_col = "__r_key__";
  right_t.append_column(right_col, make_dvector_scatter(target_values));
  return isin_impl(*this, target_col, right_t, right_col, false);
}

template <class T>
dftable dftable_base::select_rows(const std::string& target_col,
                                  const std::vector<T>& target_values) {
  dftable right_t;
  std::string right_col = "__r_key__";
  right_t.append_column(right_col, make_dvector_scatter(target_values));
  return isin_impl(*this, target_col, right_t, right_col, true);
}

template <class T>
dftable dftable_base::is_not_in_im(const std::string& target_col,
                                   const std::vector<T>& target_values) {
  dftable right_t;
  std::string right_col = "__r_key__";
  right_t.append_column(right_col, make_dvector_scatter(target_values));
  return isnotin_impl(*this, target_col, right_t, right_col, false);
}

template <class T>
dftable dftable_base::drop_rows(const std::string& target_col,
                                const std::vector<T>& target_values) {
  dftable right_t;
  std::string right_col = "__r_key__";
  right_t.append_column(right_col, make_dvector_scatter(target_values));
  return isnotin_impl(*this, target_col, right_t, right_col, true);
}

}
#endif
