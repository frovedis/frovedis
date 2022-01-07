#ifndef DFOPERATOR_HPP
#define DFOPERATOR_HPP

#include <regex>
#include "dftable.hpp"
#include "../core/set_operations.hpp"

namespace frovedis {

struct dfoperator;

// need to be here, because dfoperator inherits dffunction
struct dffunction {
  virtual bool is_id() const {return false;}
  virtual std::vector<std::string> used_col_names() const = 0;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    throw std::runtime_error
      ("execute on this operator is not implemented");
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("2 arg version of execute on this operator is not implemented");
  }
  virtual std::string get_as() {
    throw std::runtime_error("get_as on this operator is not implemented");
  }
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    throw std::runtime_error("as on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    throw std::runtime_error
      ("columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  virtual std::shared_ptr<dfoperator>
  modify_right(const std::string& rsuf = "_right") {
    throw std::runtime_error("modify_right on this operator is not implemented");
  }
};

// ----- dffunction_id -----
// placed here because id_col is needed in eq_im, etc.
struct dffunction_id : public dffunction {
  dffunction_id(const std::string& left): left(left), as_name(left) {}
  dffunction_id(const std::string& left, const std::string& as_name) :
    left(left), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_id>(*this);
  }
  virtual bool is_id() const {return true;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    std::string target = left;
    if (left == "*") { // to support case like count("*")
      auto cols = t.columns();
      require(cols.size() > 0, "execute (*): input table has no column!\n");
      target = cols[0];
    }
    return t.column(target);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("1. execute(t1, t2) is not available for id operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    std::string target = left;
    if (left == "*") { // to support case like count("*")
      auto cols = t.columns();
      require(cols.size() > 0, "columns_to_use (*): input table has no column!\n");
      target = cols[0];
    }
    return {t.raw_column(target)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {return {left};}
  std::string left;
  std::string as_name;
};

std::shared_ptr<dffunction> id_col(const std::string& left);
std::shared_ptr<dffunction> id_col_as(const std::string& left,
                                      const std::string& as);


struct dfoperator : public dffunction {
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
  virtual std::shared_ptr<dfoperator> 
  rectify_order(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("rectify_order on this operator is not implemented");
  }
};

// ----- dfoperator_eq -----
struct dfoperator_eq : public dfoperator {
  dfoperator_eq(const std::shared_ptr<dffunction>& left, 
                const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + "==" + right->get_as() + ")";
  }
  dfoperator_eq(const std::shared_ptr<dffunction>& left, 
                const std::shared_ptr<dffunction>& right, 
                const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_eq>(*this);
  }
  virtual std::shared_ptr<dfoperator> 
   modify_right(const std::string& rsuf) {
    if (left->get_as() == right->get_as()) {
      //if (!right.is_id()) throw std::runtime_error
      //("modify_right on this operator is not supported!\n");
      auto m_right = id_col(right->get_as() + rsuf);
      return std::make_shared<dfoperator_eq>(left, m_right);
    } 
    else return std::make_shared<dfoperator_eq>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    auto leftuse = left->columns_to_use(t);
    auto rightuse = right->columns_to_use(t);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    auto leftuse = left->columns_to_use(t1);
    auto rightuse = right->columns_to_use(t2);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = left->execute(t);
    auto right_column = right->execute(t);
    return left_column->filter_eq(right_column);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    hash_join(dftable_base& left_t, dftable_base& right_t,
              node_local<std::vector<size_t>>& left_idx,
              node_local<std::vector<size_t>>& right_idx) const {
    // slice to get raw_column from execute
    // TODO: execute is done on raw (not filtered) column
    // for better performance, join need to be modified to support filtered col
    dftable_base left_sliced = left_t;
    dftable_base right_sliced = right_t;
    auto left_column = left->execute(left_sliced);
    auto right_column = right->execute(right_sliced);
    return left_column->hash_join_eq(right_column, left_idx, right_idx);
  }
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
    outer_hash_join(dftable_base& left_t, dftable_base& right_t,
                    node_local<std::vector<size_t>>& left_idx,
                    node_local<std::vector<size_t>>& right_idx) const {
    dftable_base left_sliced = left_t;
    dftable_base right_sliced = right_t;
    auto left_column = left->execute(left_sliced);
    auto right_column = right->execute(right_sliced);
    return left_column->outer_hash_join_eq(right_column, left_idx, right_idx);
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join(dftable_base& left_t, dftable_base& right_t,
               node_local<std::vector<size_t>>& left_idx,
               node_local<std::vector<size_t>>& right_idx) const {
    dftable_base left_sliced = left_t;
    dftable_base right_sliced = right_t;
    auto left_column = left->execute(left_sliced);
    auto right_column = right->execute(right_sliced);
    return left_column->bcast_join_eq(right_column, left_idx, right_idx);
  }
  virtual std::tuple<node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>,
                     node_local<std::vector<size_t>>>
    outer_bcast_join(dftable_base& left_t, dftable_base& right_t,
                     node_local<std::vector<size_t>>& left_idx,
                     node_local<std::vector<size_t>>& right_idx) const {
    dftable_base left_sliced = left_t;
    dftable_base right_sliced = right_t;
    auto left_column = left->execute(left_sliced);
    auto right_column = right->execute(right_sliced);
    return left_column->outer_bcast_join_eq(right_column, left_idx, right_idx);
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    star_join(dftable_base& left_t, dftable_base& right_t,
              node_local<std::vector<size_t>>& left_idx,
              node_local<std::vector<size_t>>& right_idx) const {
    dftable_base left_sliced = left_t;
    dftable_base right_sliced = right_t;
    auto left_column = left->execute(left_sliced);
    auto right_column = right->execute(right_sliced);
    return left_column->star_join_eq(right_column, left_idx, right_idx);
  }
  virtual std::shared_ptr<dfoperator> exchange_lr() {
    return std::make_shared<dfoperator_eq>(dfoperator_eq(right, left));
  }
  virtual std::shared_ptr<dfoperator> 
  rectify_order(dftable_base& t1, dftable_base& t2) {
    std::shared_ptr<dfoperator> ret;
    try {
      columns_to_use(t1, t2);
      ret = std::make_shared<dfoperator_eq>(*this);
    } catch(std::exception& e) {
      std::string msg = e.what();
      if(msg.find("no such column") != std::string::npos ) ret = exchange_lr();
      else throw e;
    }
    return ret;
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dfoperator>
eq(const std::string& left, const std::string& right);

std::shared_ptr<dfoperator>
eq(const std::shared_ptr<dffunction>& left, const std::string& right);

std::shared_ptr<dfoperator>
eq(const std::string& left, const std::shared_ptr<dffunction>& right);

std::shared_ptr<dfoperator>
eq(const std::shared_ptr<dffunction>& left,
   const std::shared_ptr<dffunction>& right);

// ----- dfoperator_neq -----
struct dfoperator_neq : public dfoperator {
  dfoperator_neq(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + "!=" + right->get_as() + ")";
  }
  dfoperator_neq(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right, 
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_neq>(*this);
  }
  virtual std::shared_ptr<dfoperator> 
  modify_right(const std::string& rsuf) {
    if (left->get_as() == right->get_as()) {
      //if (!right.is_id()) throw std::runtime_error
      //("modify_right on this operator is not supported!\n");
      auto m_right = id_col(right->get_as() + rsuf);
      return std::make_shared<dfoperator_neq>(left, m_right);
    } 
    else return std::make_shared<dfoperator_neq>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    auto leftuse = left->columns_to_use(t);
    auto rightuse = right->columns_to_use(t);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    auto leftuse = left->columns_to_use(t1);
    auto rightuse = right->columns_to_use(t2);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = left->execute(t);
    auto right_column = right->execute(t);
    return left_column->filter_neq(right_column);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_eq(left, right).filter(t);
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

inline 
node_local<std::vector<size_t>>
dfoperator_eq::not_filter(dftable_base& t) const {
  return dfoperator_neq(left, right).filter(t);
}

std::shared_ptr<dfoperator>
neq(const std::string& left, const std::string& right);

std::shared_ptr<dfoperator>
neq(const std::shared_ptr<dffunction>& left, const std::string& right);

std::shared_ptr<dfoperator>
neq(const std::string& left, const std::shared_ptr<dffunction>& right);

std::shared_ptr<dfoperator>
neq(const std::shared_ptr<dffunction>& left,
    const std::shared_ptr<dffunction>& right);

// ----- dfoperator_eq_immed -----
template <class T>
struct dfoperator_eq_immed : public dfoperator {
  dfoperator_eq_immed(const std::shared_ptr<dffunction>& left,
                      const T& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + "==" + STR(right) + ")";
  }
  dfoperator_eq_immed(const std::shared_ptr<dffunction>& left, 
                      const T& right,
                      const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_eq_immed>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    auto left_column = left->execute(t);
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return left_column->eq_im(right_scalar);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return left->execute(t)->filter_eq_immed(right_scalar);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("2 arg version of execute on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfoperator>
eq_im(const std::shared_ptr<dffunction>& left, const T& right) {
  return std::make_shared<dfoperator_eq_immed<T>>(left,right);
}

template <class T>
std::shared_ptr<dfoperator>
eq_im(const std::string& left, const T& right) {
  return std::make_shared<dfoperator_eq_immed<T>>(id_col(left),right);
}

// ----- dfoperator_neq_immed -----
template <class T>
struct dfoperator_neq_immed : public dfoperator {
  dfoperator_neq_immed(const std::shared_ptr<dffunction>& left,
                       const T& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + "!=" + STR(right) + ")";
  }
  dfoperator_neq_immed(const std::shared_ptr<dffunction>& left, 
                       const T& right,
                       const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_neq_immed>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    auto left_column = left->execute(t);
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return left_column->neq_im(right_scalar);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return left->execute(t)->filter_neq_immed(right_scalar);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_eq_immed<T>(left, right).filter(t);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("2 arg version of execute on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  T right;
  std::string as_name;
};

template <class T>
node_local<std::vector<size_t>>
dfoperator_eq_immed<T>::not_filter(dftable_base& t) const {
  return dfoperator_neq_immed<T>(left, right).filter(t);
}

template <class T>
std::shared_ptr<dfoperator>
neq_im(const std::shared_ptr<dffunction>& left, const T& right) {
  return std::make_shared<dfoperator_neq_immed<T>>(left,right);
}

template <class T>
std::shared_ptr<dfoperator>
neq_im(const std::string& left, const T& right) {
  return std::make_shared<dfoperator_neq_immed<T>>(id_col(left),right);
}


// ----- dfoperator_lt -----
struct dfoperator_lt : public dfoperator {
  dfoperator_lt(const std::shared_ptr<dffunction>& left, 
                const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + "<" + right->get_as() + ")";
  }
  dfoperator_lt(const std::shared_ptr<dffunction>& left, 
                const std::shared_ptr<dffunction>& right, 
                const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_lt>(*this);
  }
  virtual std::shared_ptr<dfoperator> 
  modify_right(const std::string& rsuf) {
    if (left->get_as() == right->get_as()) {
      //if (!right.is_id()) throw std::runtime_error
      //("modify_right on this operator is not supported!\n");
      auto m_right = id_col(right->get_as() + rsuf);
      return std::make_shared<dfoperator_lt>(left, m_right);
    } 
    else return std::make_shared<dfoperator_lt>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    auto leftuse = left->columns_to_use(t);
    auto rightuse = right->columns_to_use(t);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    auto leftuse = left->columns_to_use(t1);
    auto rightuse = right->columns_to_use(t2);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = left->execute(t);
    auto right_column = right->execute(t);
    return left_column->filter_lt(right_column);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join(dftable_base& left_t, dftable_base& right_t,
               node_local<std::vector<size_t>>& left_idx,
               node_local<std::vector<size_t>>& right_idx) const {
    dftable_base left_sliced = left_t;
    dftable_base right_sliced = right_t;
    auto left_column = left->execute(left_sliced);
    auto right_column = right->execute(right_sliced);
    return left_column->bcast_join_lt(right_column, left_idx, right_idx);
  }
  virtual std::shared_ptr<dfoperator> exchange_lr();
  virtual std::shared_ptr<dfoperator> 
  rectify_order(dftable_base& t1, dftable_base& t2) {
    std::shared_ptr<dfoperator> ret;
    try {
      columns_to_use(t1, t2);
      ret = std::make_shared<dfoperator_lt>(*this);
    } catch(std::exception& e) {
      std::string msg = e.what();
      if(msg.find("no such column") != std::string::npos ) ret = exchange_lr();
      else throw e;
    }
    return ret;
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dfoperator>
lt(const std::string& left, const std::string& right);

std::shared_ptr<dfoperator>
lt(const std::shared_ptr<dffunction>& left, const std::string& right);

std::shared_ptr<dfoperator>
lt(const std::string& left, const std::shared_ptr<dffunction>& right);

std::shared_ptr<dfoperator>
lt(const std::shared_ptr<dffunction>& left,
   const std::shared_ptr<dffunction>& right);

// ----- dfoperator_ge -----
struct dfoperator_ge : public dfoperator {
  dfoperator_ge(const std::shared_ptr<dffunction>& left, 
                const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + ">=" + right->get_as() + ")";
  }
  dfoperator_ge(const std::shared_ptr<dffunction>& left, 
                const std::shared_ptr<dffunction>& right, 
                const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_ge>(*this);
  }
  virtual std::shared_ptr<dfoperator> 
  modify_right(const std::string& rsuf) {
    if (left->get_as() == right->get_as()) {
      //if (!right.is_id()) throw std::runtime_error
      //("modify_right on this operator is not supported!\n");
      auto m_right = id_col(right->get_as() + rsuf);
      return std::make_shared<dfoperator_ge>(left, m_right);
    } 
    else return std::make_shared<dfoperator_ge>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    auto leftuse = left->columns_to_use(t);
    auto rightuse = right->columns_to_use(t);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    auto leftuse = left->columns_to_use(t1);
    auto rightuse = right->columns_to_use(t2);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = left->execute(t);
    auto right_column = right->execute(t);
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
    dftable_base left_sliced = left_t;
    dftable_base right_sliced = right_t;
    auto left_column = left->execute(left_sliced);
    auto right_column = right->execute(right_sliced);
    return left_column->bcast_join_ge(right_column, left_idx, right_idx);
  }
  virtual std::shared_ptr<dfoperator> exchange_lr();
  virtual std::shared_ptr<dfoperator> 
  rectify_order(dftable_base& t1, dftable_base& t2) {
    std::shared_ptr<dfoperator> ret;
    try {
      columns_to_use(t1, t2);
      ret = std::make_shared<dfoperator_ge>(*this);
    } catch(std::exception& e) {
      std::string msg = e.what();
      if(msg.find("no such column") != std::string::npos ) ret = exchange_lr();
      else throw e;
    }
    return ret;
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

inline
node_local<std::vector<size_t>>
dfoperator_lt::not_filter(dftable_base& t) const {
  return dfoperator_ge(left, right).filter(t);
}

std::shared_ptr<dfoperator>
ge(const std::string& left, const std::string& right);

std::shared_ptr<dfoperator>
ge(const std::shared_ptr<dffunction>& left, const std::string& right);

std::shared_ptr<dfoperator>
ge(const std::string& left, const std::shared_ptr<dffunction>& right);

std::shared_ptr<dfoperator>
ge(const std::shared_ptr<dffunction>& left,
   const std::shared_ptr<dffunction>& right);

// ----- dfoperator_le -----
struct dfoperator_le : public dfoperator {
  dfoperator_le(const std::shared_ptr<dffunction>& left, 
                const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + "<=" + right->get_as() + ")";
  }
  dfoperator_le(const std::shared_ptr<dffunction>& left, 
                const std::shared_ptr<dffunction>& right, 
                const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_le>(*this);
  }
  virtual std::shared_ptr<dfoperator> 
  modify_right(const std::string& rsuf) {
    if (left->get_as() == right->get_as()) {
      //if (!right.is_id()) throw std::runtime_error
      //("modify_right on this operator is not supported!\n");
      auto m_right = id_col(right->get_as() + rsuf);
      return std::make_shared<dfoperator_le>(left, m_right);
    } 
    else return std::make_shared<dfoperator_le>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    auto leftuse = left->columns_to_use(t);
    auto rightuse = right->columns_to_use(t);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    auto leftuse = left->columns_to_use(t1);
    auto rightuse = right->columns_to_use(t2);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = left->execute(t);
    auto right_column = right->execute(t);
    return left_column->filter_le(right_column);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join(dftable_base& left_t, dftable_base& right_t,
               node_local<std::vector<size_t>>& left_idx,
               node_local<std::vector<size_t>>& right_idx) const {
    dftable_base left_sliced = left_t;
    dftable_base right_sliced = right_t;
    auto left_column = left->execute(left_sliced);
    auto right_column = right->execute(right_sliced);
    return left_column->bcast_join_le(right_column, left_idx, right_idx);
  }
  virtual std::shared_ptr<dfoperator> exchange_lr();
  virtual std::shared_ptr<dfoperator> 
  rectify_order(dftable_base& t1, dftable_base& t2) {
    std::shared_ptr<dfoperator> ret;
    try {
      columns_to_use(t1, t2);
      ret = std::make_shared<dfoperator_le>(*this);
    } catch(std::exception& e) {
      std::string msg = e.what();
      if(msg.find("no such column") != std::string::npos ) ret = exchange_lr();
      else throw e;
    }
    return ret;
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dfoperator>
le(const std::string& left, const std::string& right);

std::shared_ptr<dfoperator>
le(const std::shared_ptr<dffunction>& left, const std::string& right);

std::shared_ptr<dfoperator>
le(const std::string& left, const std::shared_ptr<dffunction>& right);

std::shared_ptr<dfoperator>
le(const std::shared_ptr<dffunction>& left,
   const std::shared_ptr<dffunction>& right);

// ----- dfoperator_gt -----
struct dfoperator_gt : public dfoperator {
  dfoperator_gt(const std::shared_ptr<dffunction>& left, 
                const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + ">" + right->get_as() + ")";
  }
  dfoperator_gt(const std::shared_ptr<dffunction>& left, 
                const std::shared_ptr<dffunction>& right, 
                const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_gt>(*this);
  }
  virtual std::shared_ptr<dfoperator> 
  modify_right(const std::string& rsuf) {
    if (left->get_as() == right->get_as()) {
      //if (!right.is_id()) throw std::runtime_error
      //("modify_right on this operator is not supported!\n");
      auto m_right = id_col(right->get_as() + rsuf);
      return std::make_shared<dfoperator_gt>(left, m_right);
    } 
    else return std::make_shared<dfoperator_gt>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    auto leftuse = left->columns_to_use(t);
    auto rightuse = right->columns_to_use(t);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    auto leftuse = left->columns_to_use(t1);
    auto rightuse = right->columns_to_use(t2);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left_column = left->execute(t);
    auto right_column = right->execute(t);
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
    dftable_base left_sliced = left_t;
    dftable_base right_sliced = right_t;
    auto left_column = left->execute(left_sliced);
    auto right_column = right->execute(right_sliced);
    return left_column->bcast_join_gt(right_column, left_idx, right_idx);
  }
  virtual std::shared_ptr<dfoperator> exchange_lr();
  virtual std::shared_ptr<dfoperator> 
  rectify_order(dftable_base& t1, dftable_base& t2) {
    std::shared_ptr<dfoperator> ret;
    try {
      columns_to_use(t1, t2);
      ret = std::make_shared<dfoperator_gt>(*this);
    } catch(std::exception& e) {
      std::string msg = e.what();
      if(msg.find("no such column") != std::string::npos ) ret = exchange_lr();
      else throw e;
    }
    return ret;
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

inline
node_local<std::vector<size_t>>
dfoperator_le::not_filter(dftable_base& t) const {
  return dfoperator_gt(left, right).filter(t);
}

std::shared_ptr<dfoperator>
gt(const std::string& left, const std::string& right);

std::shared_ptr<dfoperator>
gt(const std::shared_ptr<dffunction>& left, const std::string& right);

std::shared_ptr<dfoperator>
gt(const std::string& left, const std::shared_ptr<dffunction>& right);

std::shared_ptr<dfoperator>
gt(const std::shared_ptr<dffunction>& left,
   const std::shared_ptr<dffunction>& right);

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

// ----- dfoperator_lt_immed -----
template <class T>
struct dfoperator_lt_immed : public dfoperator {
  dfoperator_lt_immed(const std::shared_ptr<dffunction>& left,
                      const T& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + "<" + STR(right) + ")";
  }
  dfoperator_lt_immed(const std::shared_ptr<dffunction>& left, 
                      const T& right,
                      const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_lt_immed>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    auto left_column = left->execute(t);
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return left_column->lt_im(right_scalar);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return left->execute(t)->filter_lt_immed(right_scalar);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("2 arg version of execute on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfoperator>
lt_im(const std::shared_ptr<dffunction>& left, const T& right) {
  return std::make_shared<dfoperator_lt_immed<T>>(left,right);
}

template <class T>
std::shared_ptr<dfoperator>
lt_im(const std::string& left, const T& right) {
  return std::make_shared<dfoperator_lt_immed<T>>(id_col(left),right);
}


// ----- dfoperator_ge_immed -----
template <class T>
struct dfoperator_ge_immed : public dfoperator {
  dfoperator_ge_immed(const std::shared_ptr<dffunction>& left,
                      const T& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + ">=" + STR(right) + ")";
  }
  dfoperator_ge_immed(const std::shared_ptr<dffunction>& left, 
                      const T& right,
                      const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_ge_immed>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    auto left_column = left->execute(t);
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return left_column->ge_im(right_scalar);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return left->execute(t)->filter_ge_immed(right_scalar);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_lt_immed<T>(left, right).filter(t);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("2 arg version of execute on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfoperator>
ge_im(const std::shared_ptr<dffunction>& left, const T& right) {
  return std::make_shared<dfoperator_ge_immed<T>>(left,right);
}

template <class T>
std::shared_ptr<dfoperator>
ge_im(const std::string& left, const T& right) {
  return std::make_shared<dfoperator_ge_immed<T>>(id_col(left),right);
}

template <class T>
node_local<std::vector<size_t>> 
dfoperator_lt_immed<T>::not_filter(dftable_base& t) const {
  return dfoperator_ge_immed<T>(left, right).filter(t);
}

// ----- dfoperator_le_immed -----
template <class T>
struct dfoperator_le_immed : public dfoperator {
  dfoperator_le_immed(const std::shared_ptr<dffunction>& left,
                      const T& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + "<=" + STR(right) + ")";
  }
  dfoperator_le_immed(const std::shared_ptr<dffunction>& left, 
                      const T& right,
                      const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_le_immed>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    auto left_column = left->execute(t);
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return left_column->le_im(right_scalar);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return left->execute(t)->filter_le_immed(right_scalar);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("2 arg version of execute on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfoperator>
le_im(const std::shared_ptr<dffunction>& left, const T& right) {
  return std::make_shared<dfoperator_le_immed<T>>(left,right);
}

template <class T>
std::shared_ptr<dfoperator>
le_im(const std::string& left, const T& right) {
  return std::make_shared<dfoperator_le_immed<T>>(id_col(left),right);
}


// ----- dfoperator_gt_immed -----
template <class T>
struct dfoperator_gt_immed : public dfoperator {
  dfoperator_gt_immed(const std::shared_ptr<dffunction>& left,
                      const T& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + ">" + STR(right) + ")";
  }
  dfoperator_gt_immed(const std::shared_ptr<dffunction>& left, 
                      const T& right,
                      const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_gt_immed>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    auto left_column = left->execute(t);
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return left_column->gt_im(right_scalar);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    std::shared_ptr<dfscalar> right_scalar =
      std::make_shared<typed_dfscalar<T>>(right);
    return left->execute(t)->filter_gt_immed(right_scalar);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_le_immed<T>(left, right).filter(t);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("2 arg version of execute on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfoperator>
gt_im(const std::shared_ptr<dffunction>& left, const T& right) {
  return std::make_shared<dfoperator_gt_immed<T>>(left,right);
}

template <class T>
std::shared_ptr<dfoperator>
gt_im(const std::string& left, const T& right) {
  return std::make_shared<dfoperator_gt_immed<T>>(id_col(left),right);
}

template <class T>
node_local<std::vector<size_t>>
dfoperator_le_immed<T>::not_filter(dftable_base& t) const {
  return dfoperator_gt_immed<T>(left, right).filter(t);
}


// ----- dfoperator_is_null -----
struct dfoperator_is_null : public dfoperator {
  dfoperator_is_null(const std::shared_ptr<dffunction>& left) :
    left(left) {
    as_name = "is_null(" + left->get_as() + ")";
  }
  dfoperator_is_null(const std::shared_ptr<dffunction>& left, 
                     const std::string& as_name) :
    left(left), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_is_null>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    auto left_column = left->execute(t);
    return left_column->is_null();
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    return left->execute(t)->filter_is_null();
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("2 arg version of execute on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  std::string as_name;
};

std::shared_ptr<dfoperator>
is_null(const std::string& left);

std::shared_ptr<dfoperator>
is_null(const std::shared_ptr<dffunction>& left);


// ----- dfoperator_is_not_null -----
struct dfoperator_is_not_null : public dfoperator {
  dfoperator_is_not_null(const std::shared_ptr<dffunction>& left) :
    left(left) {
    as_name = "is_not_null(" + left->get_as() + ")";
  }
  dfoperator_is_not_null(const std::shared_ptr<dffunction>& left, 
                         const std::string& as_name) :
    left(left), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_is_not_null>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    auto left_column = left->execute(t);
    return left_column->is_not_null();
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    return left->execute(t)->filter_is_not_null();
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_is_null(left).filter(t);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("2 arg version of execute on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  std::string as_name;
};

inline 
node_local<std::vector<size_t>>
dfoperator_is_null::not_filter(dftable_base& t) const {
  return dfoperator_is_not_null(left).filter(t);
}

std::shared_ptr<dfoperator>
is_not_null(const std::string& left);

std::shared_ptr<dfoperator>
is_not_null(const std::shared_ptr<dffunction>& left);


// ----- dfoperator_regex -----
struct dfoperator_regex : public dfoperator {
  dfoperator_regex(const std::shared_ptr<dffunction>& left,
                   const std::string& pattern) :
    left(left), pattern(pattern) {
    as_name = "regex(" + left->get_as() + ")";
  }
  dfoperator_regex(const std::shared_ptr<dffunction>& left, 
                   const std::string& pattern,
                   const std::string& as_name) :
    left(left), pattern(pattern), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_regex>(*this);
  }
  // skip execute
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  node_local<std::vector<size_t>> 
  filter_impl(std::shared_ptr<dfcolumn>& tcol) const {
    auto left_column =
      std::dynamic_pointer_cast<typed_dfcolumn<std::string>>(tcol);
    if(!left_column)
      throw std::runtime_error("dfoperator_regex: column type is not string");
    return left_column->filter_regex(pattern);
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto tcol = left->execute(t);
    return filter_impl(tcol);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    auto tcol = left->execute(t);
    auto filter_idx = filter_impl(tcol);
    auto t_nulls  = tcol->get_nulls();
    return create_boolean_column(filter_idx, t_nulls, tcol->sizes());
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  std::string pattern;
  std::string as_name;
};

std::shared_ptr<dfoperator>
is_regex(const std::string& left, const std::string& pattern);

std::shared_ptr<dfoperator>
is_regex(const std::shared_ptr<dffunction>& left, const std::string& pattern);


// ----- dfoperator_not_regex -----
struct dfoperator_not_regex : public dfoperator {
  dfoperator_not_regex(const std::shared_ptr<dffunction>& left,
                       const std::string& pattern) :
    left(left), pattern(pattern) {
    as_name = "not_regex(" + left->get_as() + ")";
  }
  dfoperator_not_regex(const std::shared_ptr<dffunction>& left, 
                       const std::string& pattern,
                       const std::string& as_name) :
    left(left), pattern(pattern), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_not_regex>(*this);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  node_local<std::vector<size_t>> 
  filter_impl(std::shared_ptr<dfcolumn>& tcol) const {
    auto left_column =
      std::dynamic_pointer_cast<typed_dfcolumn<std::string>>(tcol);
    if(!left_column)
      throw std::runtime_error
        ("dfoperator_not_regex: column type is not string");
    return left_column->filter_not_regex(pattern);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_regex(left, pattern).filter(t);
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto tcol = left->execute(t);
    return filter_impl(tcol);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    auto tcol = left->execute(t);
    auto filter_idx = filter_impl(tcol);
    auto t_nulls  = tcol->get_nulls();
    return create_boolean_column(filter_idx, t_nulls, tcol->sizes());
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  std::string pattern;
  std::string as_name;
};

inline
node_local<std::vector<size_t>>
dfoperator_regex::not_filter(dftable_base& t) const {
  return dfoperator_not_regex(left, pattern).filter(t);
}

std::shared_ptr<dfoperator>
is_not_regex(const std::string& left, const std::string& pattern);

std::shared_ptr<dfoperator>
is_not_regex(const std::shared_ptr<dffunction>& left,
             const std::string& pattern);


// ----- dfoperator_like -----
struct dfoperator_like : public dfoperator {
  dfoperator_like(const std::shared_ptr<dffunction>& left,
                  const std::string& pattern) :
    left(left), pattern(pattern) {
    as_name = "like(" + left->get_as() + ")";
  }
  dfoperator_like(const std::shared_ptr<dffunction>& left, 
                  const std::string& pattern,
                  const std::string& as_name) :
    left(left), pattern(pattern), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_like>(*this);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  node_local<std::vector<size_t>> 
  filter_impl(std::shared_ptr<dfcolumn>& tcol) const {
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
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto tcol = left->execute(t);
    return filter_impl(tcol);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    auto tcol = left->execute(t);
    auto filter_idx = filter_impl(tcol);
    auto t_nulls  = tcol->get_nulls();
    return create_boolean_column(filter_idx, t_nulls, tcol->sizes());
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  std::string pattern;
  std::string as_name;
};

std::shared_ptr<dfoperator>
is_like(const std::string& left, const std::string& pattern);

std::shared_ptr<dfoperator>
is_like(const std::shared_ptr<dffunction>& left, const std::string& pattern);


// ----- dfoperator_not_like -----
// TODO: implemente execute
struct dfoperator_not_like : public dfoperator {
  dfoperator_not_like(const std::shared_ptr<dffunction>& left,
                      const std::string& pattern) :
    left(left), pattern(pattern) {
    as_name = "not_like(" + left->get_as() + ")";
  }
  dfoperator_not_like(const std::shared_ptr<dffunction>& left, 
                      const std::string& pattern,
                      const std::string& as_name) :
    left(left), pattern(pattern), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_not_like>(*this);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  node_local<std::vector<size_t>> 
  filter_impl(std::shared_ptr<dfcolumn>& tcol) const {
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
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto tcol = left->execute(t);
    return filter_impl(tcol);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    return dfoperator_like(left, pattern).filter(t);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    auto tcol = left->execute(t);
    auto filter_idx = filter_impl(tcol);
    auto t_nulls  = tcol->get_nulls();
    return create_boolean_column(filter_idx, t_nulls, tcol->sizes());
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  std::string pattern;
  std::string as_name;
};

std::shared_ptr<dfoperator>
is_not_like(const std::string& left, const std::string& pattern);

std::shared_ptr<dfoperator>
is_not_like(const std::shared_ptr<dffunction>& left,
            const std::string& pattern);

inline
node_local<std::vector<size_t>>
dfoperator_like::not_filter(dftable_base& t) const {
  return dfoperator_not_like(left, pattern).filter(t);
}

std::vector<size_t>
convert_filtered_idx(std::vector<size_t>& org_idx,
                     std::vector<size_t>& created_idx);

// ----- filtered_dftable -----
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
  virtual dftable fselect(const std::vector<std::shared_ptr<dffunction>>& cols);
  virtual filtered_dftable filter(const std::shared_ptr<dfoperator>& op);
  virtual sorted_dftable sort(const std::string& name);
  virtual sorted_dftable sort_desc(const std::string& name);
  virtual sorted_dftable fsort(const std::shared_ptr<dffunction>& col);
  virtual sorted_dftable fsort_desc(const std::shared_ptr<dffunction>& col);
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


// ----- dfoperator_and -----
struct dfoperator_and : public dfoperator {
  dfoperator_and(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + "&&" + right->get_as() + ")";
  }
  dfoperator_and(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right, 
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_and>(*this);
  }
  virtual std::shared_ptr<dfoperator> 
  modify_right(const std::string& rsuf) {
    auto m_left  = left->modify_right(rsuf);
    auto m_right = right->modify_right(rsuf);
    return std::make_shared<dfoperator_and>(m_left, m_right);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    auto leftuse = left->columns_to_use(t);
    auto rightuse = right->columns_to_use(t);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    auto leftuse = left->columns_to_use(t1, t2);
    auto rightuse = right->columns_to_use(t1, t2);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left2 = std::dynamic_pointer_cast<dfoperator>(left);
    auto right2 = std::dynamic_pointer_cast<dfoperator>(right);
    if(!left2 || !right2) throw std::runtime_error("filter by non operator");
    auto left_filtered_idx = left2->filter(t);
    auto right_filtered_idx = right2->filter(t);
    return left_filtered_idx.map(set_intersection<size_t>, right_filtered_idx);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    auto left2 = std::dynamic_pointer_cast<dfoperator>(left);
    auto right2 = std::dynamic_pointer_cast<dfoperator>(right);
    if(!left2 || !right2) throw std::runtime_error("filter by non operator");
    auto left_filtered_idx = left2->not_filter(t);
    auto right_filtered_idx = right2->not_filter(t);
    return left_filtered_idx.map(set_union<size_t>, right_filtered_idx);
  }
  virtual std::pair<node_local<std::vector<size_t>>,
                    node_local<std::vector<size_t>>>
    bcast_join(dftable_base& left, dftable_base& right,
               node_local<std::vector<size_t>>& left_idx,
               node_local<std::vector<size_t>>& right_idx) const;
  virtual std::shared_ptr<dfoperator> exchange_lr() {
    auto left2 = std::dynamic_pointer_cast<dfoperator>(left);
    auto right2 = std::dynamic_pointer_cast<dfoperator>(right);
    if(!left2 || !right2) throw std::runtime_error("join by non operator");
    return std::make_shared<dfoperator_and>
      (dfoperator_and(left2->exchange_lr(), right2->exchange_lr()));
  }
  virtual std::shared_ptr<dfoperator> 
  rectify_order(dftable_base& t1, dftable_base& t2) {
    auto left2 = std::dynamic_pointer_cast<dfoperator>(left);
    auto right2 = std::dynamic_pointer_cast<dfoperator>(right);
    if(!left2 || !right2) throw std::runtime_error("join by non operator");

    bool left_ok = true;
    try {
      left2->columns_to_use(t1, t2);
    } catch(std::exception& e) {
      std::string msg = e.what();
      if(msg.find("no such column") != std::string::npos) left_ok = false;
      else throw e;
    }

    bool right_ok = true;
    try {
      right2->columns_to_use(t1, t2);
    } catch(std::exception& e) {
      std::string msg = e.what();
      if(msg.find("no such column") != std::string::npos) right_ok = false;
      else throw e;
    }

    std::shared_ptr<dfoperator> ret;
    if (left_ok && right_ok) {
      ret = std::make_shared<dfoperator_and>(*this);
    } else if (left_ok && !right_ok) {
      auto m_right = right2->exchange_lr();
      ret = std::make_shared<dfoperator_and>(left2, m_right);
    } else if (!left_ok && right_ok) {
      auto m_left = left2->exchange_lr();
      ret = std::make_shared<dfoperator_and>(m_left, right2);
    } else {
      auto m_left = left2->exchange_lr();
      auto m_right = right2->exchange_lr();
      ret = std::make_shared<dfoperator_and>(m_left, m_right);
    }
    return ret;
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dfoperator>
and_op(const std::string& left, const std::string& right);

std::shared_ptr<dfoperator>
and_op(const std::shared_ptr<dffunction>& left, const std::string& right);

std::shared_ptr<dfoperator>
and_op(const std::string& left, const std::shared_ptr<dffunction>& right);

std::shared_ptr<dfoperator>
and_op(const std::shared_ptr<dffunction>& left,
       const std::shared_ptr<dffunction>& right);


// ----- dfoperator_or -----
struct dfoperator_or : public dfoperator {
  dfoperator_or(const std::shared_ptr<dffunction>& left, 
                const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + "||" + right->get_as() + ")";
  }
  dfoperator_or(const std::shared_ptr<dffunction>& left, 
                const std::shared_ptr<dffunction>& right, 
                const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_or>(*this);
  }
  virtual std::shared_ptr<dfoperator> 
  modify_right(const std::string& rsuf) {
    auto m_left  = left->modify_right(rsuf);
    auto m_right = right->modify_right(rsuf);
    return std::make_shared<dfoperator_or>(m_left, m_right);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    auto leftuse = left->columns_to_use(t);
    auto rightuse = right->columns_to_use(t);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    auto leftuse = left->columns_to_use(t1, t2);
    auto rightuse = right->columns_to_use(t1, t2);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    return leftuse;
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left2 = std::dynamic_pointer_cast<dfoperator>(left);
    auto right2 = std::dynamic_pointer_cast<dfoperator>(right);
    if(!left2 || !right2) throw std::runtime_error("filter by non operator");
    auto left_filtered_idx = left2->filter(t);
    auto right_filtered_idx = right2->filter(t);
    return left_filtered_idx.map(set_union<size_t>, right_filtered_idx);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    auto left2 = std::dynamic_pointer_cast<dfoperator>(left);
    auto right2 = std::dynamic_pointer_cast<dfoperator>(right);
    if(!left2 || !right2) throw std::runtime_error("filter by non operator");
    auto left_filtered_idx = left2->not_filter(t);
    auto right_filtered_idx = right2->not_filter(t);
    return left_filtered_idx.map(set_intersection<size_t>, right_filtered_idx);
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dfoperator>
or_op(const std::string& left, const std::string& right);

std::shared_ptr<dfoperator>
or_op(const std::shared_ptr<dffunction>& left, const std::string& right);

std::shared_ptr<dfoperator>
or_op(const std::string& left, const std::shared_ptr<dffunction>& right);

std::shared_ptr<dfoperator>
or_op(const std::shared_ptr<dffunction>& left,
      const std::shared_ptr<dffunction>& right);


// ----- dfoperator_not -----
// not shoud use not_filter, instead of set_difference from 0...N,
// considering the NULL and performance
struct dfoperator_not : public dfoperator {
  dfoperator_not(const std::shared_ptr<dffunction>& left) : 
    left(left) {
    as_name = "(!" + left->get_as() + ")";
  }
  dfoperator_not(const std::shared_ptr<dffunction>& left, 
                 const std::string& as_name) :
    left(left), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_not>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual node_local<std::vector<size_t>> filter(dftable_base& t) const {
    auto left2 = std::dynamic_pointer_cast<dfoperator>(left);
    if(!left2) throw std::runtime_error("filter by non operator");
    return left2->not_filter(t);
  }
  virtual node_local<std::vector<size_t>> not_filter(dftable_base& t) const {
    auto left2 = std::dynamic_pointer_cast<dfoperator>(left);
    if(!left2) throw std::runtime_error("filter by non operator");
    return left2->filter(t);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("2 arg version of execute on this operator is not implemented");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("2 arg version of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  std::string as_name;
};

std::shared_ptr<dfoperator>
not_op(const std::string& left);

std::shared_ptr<dfoperator>
not_op(const std::shared_ptr<dffunction>& left);


// ----- dfoperator_multi_eq -----
struct dfoperator_multi_eq : public dfoperator {
  dfoperator_multi_eq(const std::vector<std::shared_ptr<dffunction>>& leftv, 
                      const std::vector<std::shared_ptr<dffunction>>& rightv) :
    leftv(leftv), rightv(rightv) {as_name = "multieq";}
  dfoperator_multi_eq(const std::vector<std::shared_ptr<dffunction>>& leftv, 
                      const std::vector<std::shared_ptr<dffunction>>& rightv, 
                      const std::string& as_name) :
    leftv(leftv), rightv(rightv), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfoperator_multi_eq>(*this);
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
    for(size_t i = 0; i < leftv.size(); i++) {
      auto use = leftv[i]->columns_to_use(t1);
      ret.insert(ret.end(), use.begin(), use.end());
    }
    for(size_t i = 0; i < rightv.size(); i++) {
      auto use = rightv[i]->columns_to_use(t2);
      ret.insert(ret.end(), use.begin(), use.end());
    }
    return ret;
  }
  virtual std::shared_ptr<dfoperator> exchange_lr() {
    return std::make_shared<dfoperator_multi_eq>
      (dfoperator_multi_eq(rightv, leftv));
  }
  virtual std::vector<std::string> used_col_names() const {
    std::vector<std::string> ret;
    for(size_t i = 0; i < leftv.size(); i++) {
      auto use = leftv[i]->used_col_names();
      ret.insert(ret.end(), use.begin(), use.end());
    }
    for(size_t i = 0; i < rightv.size(); i++) {
      auto use = rightv[i]->used_col_names();
      ret.insert(ret.end(), use.begin(), use.end());
    }
    return ret;
  }

  std::vector<std::shared_ptr<dffunction>> leftv, rightv;
  std::string as_name;
};

void filter_idx(std::vector<size_t>& idx,
                const std::vector<size_t>& filter);

std::shared_ptr<dfoperator>
multi_eq(const std::vector<std::string>& left,
         const std::vector<std::string>& right);

// use different name since literal argument becomes ambiguous
std::shared_ptr<dfoperator>
fmulti_eq(const std::vector<std::shared_ptr<dffunction>>& left_func,
          const std::vector<std::shared_ptr<dffunction>>& right_func);


// ----- dfoperator_cross -----
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
  virtual std::vector<std::string> used_col_names() const {
    return std::vector<std::string>();
  }
};

std::shared_ptr<dfoperator> cross();


// ----- operators -----
std::shared_ptr<dfoperator> operator==(const std::shared_ptr<dffunction>& a,
                                       const std::shared_ptr<dffunction>& b);

std::shared_ptr<dfoperator> operator!=(const std::shared_ptr<dffunction>& a,
                                       const std::shared_ptr<dffunction>& b);

std::shared_ptr<dfoperator> operator<(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b);

std::shared_ptr<dfoperator> operator>=(const std::shared_ptr<dffunction>& a,
                                       const std::shared_ptr<dffunction>& b);

std::shared_ptr<dfoperator> operator<=(const std::shared_ptr<dffunction>& a,
                                       const std::shared_ptr<dffunction>& b);

std::shared_ptr<dfoperator> operator>(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b);

std::shared_ptr<dfoperator> operator&&(const std::shared_ptr<dfoperator>& a,
                                       const std::shared_ptr<dfoperator>& b);

std::shared_ptr<dfoperator> operator||(const std::shared_ptr<dfoperator>& a,
                                       const std::shared_ptr<dfoperator>& b);

std::shared_ptr<dfoperator> operator!(const std::shared_ptr<dfoperator>& a);

template <class T>
std::shared_ptr<dfoperator> operator==(const std::shared_ptr<dffunction>& a,
                                       const T& b) {
  return eq_im(a,b);
}

template <class T>
std::shared_ptr<dfoperator> operator==(const T& b,
                                       const std::shared_ptr<dffunction>& a) {
  return eq_im(a,b);
}

template <class T>
std::shared_ptr<dfoperator> operator!=(const std::shared_ptr<dffunction>& a,
                                       const T& b) {
  return neq_im(a,b);
}

template <class T>
std::shared_ptr<dfoperator> operator!=(const T& b,
                                       const std::shared_ptr<dffunction>& a) {
  return neq_im(a,b);
}

template <class T>
std::shared_ptr<dfoperator> operator<(const std::shared_ptr<dffunction>& a,
                                      const T& b) {
  return lt_im(a,b);
}

template <class T>
std::shared_ptr<dfoperator> operator<(const T& b,
                                      const std::shared_ptr<dffunction>& a) {
  return gt_im(a,b);
}

template <class T>
std::shared_ptr<dfoperator> operator>=(const std::shared_ptr<dffunction>& a,
                                       const T& b) {
  return ge_im(a,b);
}

template <class T>
std::shared_ptr<dfoperator> operator>=(const T& b,
                                       const std::shared_ptr<dffunction>& a) {
  return le_im(a,b);
}

template <class T>
std::shared_ptr<dfoperator> operator<=(const std::shared_ptr<dffunction>& a,
                                       const T& b) {
  return le_im(a,b);
}

template <class T>
std::shared_ptr<dfoperator> operator<=(const T& b,
                                       const std::shared_ptr<dffunction>& a) {
  return ge_im(a,b);
}

template <class T>
std::shared_ptr<dfoperator> operator>(const std::shared_ptr<dffunction>& a,
                                      const T& b) {
  return gt_im(a,b);
}

template <class T>
std::shared_ptr<dfoperator> operator>(const T& b,
                                      const std::shared_ptr<dffunction>& a) {
  return lt_im(a,b);
}

// ----- misc -----
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
