#ifndef DFFUNCTION_HPP
#define DFFUNCTION_HPP

#include "dfscalar.hpp"
#include "dftable.hpp"

namespace frovedis {

bool check_distribution(dftable_base& left, 
                        dftable_base& right,
                        std::vector<size_t>& left_sizes);

dftable_base& realign_df(dftable_base& left, 
                         dftable_base& right,
                         const std::vector<std::string>& right_cols);

bool verify_column_identicality(dftable_base& left,
                                const std::string& lcol,
                                dftable_base& right,
                                const std::string& rcol);

struct dffunction {
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const = 0;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const  = 0;
  virtual std::string as() = 0;
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
};


// ----- id -----
struct dffunction_id : public dffunction {
  dffunction_id(const std::string& left): left(left), as_name(left) {}
  dffunction_id(const std::string& left, const std::string& as_name) :
    left(left), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const {
    return t.column(left);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("execute(t1, t2) is not available for id operation!\n");
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
  std::string as_name;
};

std::shared_ptr<dffunction> id_col(const std::string& left);
std::shared_ptr<dffunction> id_col_as(const std::string& left,
                                      const std::string& as);


// ----- add -----
struct dffunction_add : public dffunction {
  dffunction_add(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->as() + "+" + right->as() + ")";
  }
  dffunction_add(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right, 
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
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
  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dffunction>
add_col(const std::string& left, const std::string& right); 
std::shared_ptr<dffunction>
add_col(const std::shared_ptr<dffunction>& left, const std::string& right); 
std::shared_ptr<dffunction>
add_col(const std::string& left, const std::shared_ptr<dffunction>& right); 
std::shared_ptr<dffunction>
add_col(const std::shared_ptr<dffunction>& left,
        const std::shared_ptr<dffunction>& right); 

std::shared_ptr<dffunction>
add_col_as(const std::string& left, const std::string& right,
           const std::string& as);
std::shared_ptr<dffunction>
add_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
           const std::string& as);
std::shared_ptr<dffunction>
add_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
           const std::string& as);
std::shared_ptr<dffunction>
add_col_as(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right,
           const std::string& as);

template <class T>
struct dffunction_add_im : public dffunction {
  dffunction_add_im(const std::shared_ptr<dffunction>& left, T right): 
    left(left), right(right) {
    as_name = "(" + left->as() + "+" + STR(right) + ")";
  }
  dffunction_add_im(const std::shared_ptr<dffunction>& left, T right,
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1 + t2: is not available for add_im operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }

  std::shared_ptr<dffunction> left;
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_add_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = left->execute(t);
  return left_column->add_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
add_im(const std::string& left, T right) {
  return std::make_shared<dffunction_add_im<T>>(id_col(left), right);
}

template <class T>
std::shared_ptr<dffunction>
add_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_add_im<T>>(id_col(left), right, as);
}

template <class T>
std::shared_ptr<dffunction>
add_im(const std::shared_ptr<dffunction>& left, T right) {
  return std::make_shared<dffunction_add_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
add_im_as(const std::shared_ptr<dffunction>& left, T right,
          const std::string& as) {
  return std::make_shared<dffunction_add_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
add_im(T left, const std::string& right) {
  return std::make_shared<dffunction_add_im<T>>(id_col(right), left);
}

template <class T>
std::shared_ptr<dffunction>
add_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_add_im<T>>(id_col(right), left, as);
}

template <class T>
std::shared_ptr<dffunction>
add_im(T left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_add_im<T>>(right, left);
}

template <class T>
std::shared_ptr<dffunction>
add_im_as(T left, const std::shared_ptr<dffunction>& right,
          const std::string& as) {
  return std::make_shared<dffunction_add_im<T>>(right, left, as);
}


// ----- sub -----
struct dffunction_sub : public dffunction {
  dffunction_sub(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->as() + "-" + right->as() + ")";
  }
  dffunction_sub(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right, 
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
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
  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dffunction>
sub_col(const std::string& left, const std::string& right); 
std::shared_ptr<dffunction>
sub_col(const std::shared_ptr<dffunction>& left, const std::string& right); 
std::shared_ptr<dffunction>
sub_col(const std::string& left, const std::shared_ptr<dffunction>& right); 
std::shared_ptr<dffunction>
sub_col(const std::shared_ptr<dffunction>& left,
        const std::shared_ptr<dffunction>& right); 

std::shared_ptr<dffunction>
sub_col_as(const std::string& left, const std::string& right,
           const std::string& as);
std::shared_ptr<dffunction>
sub_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
           const std::string& as);
std::shared_ptr<dffunction>
sub_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
           const std::string& as);
std::shared_ptr<dffunction>
sub_col_as(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right,
           const std::string& as);

template <class T>
struct dffunction_sub_im : public dffunction {
  dffunction_sub_im(const std::shared_ptr<dffunction>& left, T right,
                    bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed) {
    if(is_reversed) as_name = "(" + STR(right) + "-" + left->as() + ")";
    else            as_name = "(" + left->as() + "-" + STR(right) + ")";
  }
  dffunction_sub_im(const std::shared_ptr<dffunction>& left, T right,
                    const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1 - t2: is not available for sub_im operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }

  std::shared_ptr<dffunction> left;
  T right;
  bool is_reversed;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_sub_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = left->execute(t);
  return is_reversed ? left_column->rsub_im(right_scalar) 
    : left_column->sub_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
sub_im(const std::string& left, T right) {
  return std::make_shared<dffunction_sub_im<T>>(id_col(left), right);
}

template <class T>
std::shared_ptr<dffunction>
sub_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_sub_im<T>>(id_col(left), right, as);
}

template <class T>
std::shared_ptr<dffunction>
sub_im(const std::shared_ptr<dffunction>& left, T right) {
  return std::make_shared<dffunction_sub_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
sub_im_as(const std::shared_ptr<dffunction>& left, T right,
          const std::string& as) {
  return std::make_shared<dffunction_sub_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
sub_im(T left, const std::string& right) {
  return std::make_shared<dffunction_sub_im<T>>(id_col(right), left, true);
}

template <class T>
std::shared_ptr<dffunction>
sub_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_sub_im<T>>(id_col(right), left, as, true);
}

template <class T>
std::shared_ptr<dffunction>
sub_im(T left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_sub_im<T>>(right, left, true);
}

template <class T>
std::shared_ptr<dffunction>
sub_im_as(T left, const std::shared_ptr<dffunction>& right,
          const std::string& as) {
  return std::make_shared<dffunction_sub_im<T>>(right, left, as, true);
}


// ----- mul -----
struct dffunction_mul : public dffunction {
  dffunction_mul(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->as() + "*" + right->as() + ")";
  }
  dffunction_mul(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right, 
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
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
  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dffunction>
mul_col(const std::string& left, const std::string& right); 
std::shared_ptr<dffunction>
mul_col(const std::shared_ptr<dffunction>& left, const std::string& right); 
std::shared_ptr<dffunction>
mul_col(const std::string& left, const std::shared_ptr<dffunction>& right); 
std::shared_ptr<dffunction>
mul_col(const std::shared_ptr<dffunction>& left,
        const std::shared_ptr<dffunction>& right); 

std::shared_ptr<dffunction>
mul_col_as(const std::string& left, const std::string& right,
           const std::string& as);
std::shared_ptr<dffunction>
mul_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
           const std::string& as);
std::shared_ptr<dffunction>
mul_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
           const std::string& as);
std::shared_ptr<dffunction>
mul_col_as(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right,
           const std::string& as);

template <class T>
struct dffunction_mul_im : public dffunction {
  dffunction_mul_im(const std::shared_ptr<dffunction>& left, T right): 
    left(left), right(right) {
    as_name = "(" + left->as() + "*" + STR(right) + ")";
  }
  dffunction_mul_im(const std::shared_ptr<dffunction>& left, T right,
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1 * t2: is not available for mul_im operation!\n");  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }

  std::shared_ptr<dffunction> left;
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_mul_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = left->execute(t);
  return left_column->mul_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
mul_im(const std::string& left, T right) {
  return std::make_shared<dffunction_mul_im<T>>(id_col(left), right);
}

template <class T>
std::shared_ptr<dffunction>
mul_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_mul_im<T>>(id_col(left), right, as);
}

template <class T>
std::shared_ptr<dffunction>
mul_im(const std::shared_ptr<dffunction>& left, T right) {
  return std::make_shared<dffunction_mul_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
mul_im_as(const std::shared_ptr<dffunction>& left, T right,
          const std::string& as) {
  return std::make_shared<dffunction_mul_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
mul_im(T left, const std::string& right) {
  return std::make_shared<dffunction_mul_im<T>>(id_col(right), left);
}

template <class T>
std::shared_ptr<dffunction>
mul_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_mul_im<T>>(id_col(right), left, as);
}

template <class T>
std::shared_ptr<dffunction>
mul_im(T left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_mul_im<T>>(right, left);
}

template <class T>
std::shared_ptr<dffunction>
mul_im_as(T left, const std::shared_ptr<dffunction>& right,
          const std::string& as) {
  return std::make_shared<dffunction_mul_im<T>>(right, left, as);
}


// ----- fdiv -----
struct dffunction_fdiv : public dffunction {
  dffunction_fdiv(const std::shared_ptr<dffunction>& left, 
                  const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->as() + "/" + right->as() + ")";
  }
  dffunction_fdiv(const std::shared_ptr<dffunction>& left, 
                  const std::shared_ptr<dffunction>& right, 
                  const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
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
  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dffunction>
fdiv_col(const std::string& left, const std::string& right); 
std::shared_ptr<dffunction>
fdiv_col(const std::shared_ptr<dffunction>& left, const std::string& right); 
std::shared_ptr<dffunction>
fdiv_col(const std::string& left, const std::shared_ptr<dffunction>& right); 
std::shared_ptr<dffunction>
fdiv_col(const std::shared_ptr<dffunction>& left,
         const std::shared_ptr<dffunction>& right); 

std::shared_ptr<dffunction>
fdiv_col_as(const std::string& left, const std::string& right,
            const std::string& as);
std::shared_ptr<dffunction>
fdiv_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
            const std::string& as);
std::shared_ptr<dffunction>
fdiv_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
            const std::string& as);
std::shared_ptr<dffunction>
fdiv_col_as(const std::shared_ptr<dffunction>& left,
            const std::shared_ptr<dffunction>& right,
            const std::string& as);

template <class T>
struct dffunction_fdiv_im : public dffunction {
  dffunction_fdiv_im(const std::shared_ptr<dffunction>& left, T right,
                     bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed) {
    if(is_reversed) as_name = "(" + STR(right) + "/" + left->as() + ")";
    else            as_name = "(" + left->as() + "/" + STR(right) + ")";
  }
  dffunction_fdiv_im(const std::shared_ptr<dffunction>& left, T right,
                     const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1 / t2: is not available for fdiv_im operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }

  std::shared_ptr<dffunction> left;
  T right;
  bool is_reversed;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn>
dffunction_fdiv_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = left->execute(t);
  return is_reversed ? left_column->rfdiv_im(right_scalar) 
    : left_column->fdiv_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
fdiv_im(const std::string& left, T right) {
  return std::make_shared<dffunction_fdiv_im<T>>(id_col(left), right);
}

template <class T>
std::shared_ptr<dffunction>
fdiv_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_fdiv_im<T>>(id_col(left), right, as);
}

template <class T>
std::shared_ptr<dffunction>
fdiv_im(const std::shared_ptr<dffunction>& left, T right) {
  return std::make_shared<dffunction_fdiv_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
fdiv_im_as(const std::shared_ptr<dffunction>& left, T right,
           const std::string& as) {
  return std::make_shared<dffunction_fdiv_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
fdiv_im(T left, const std::string& right) {
  return std::make_shared<dffunction_fdiv_im<T>>(id_col(right), left, true);
}

template <class T>
std::shared_ptr<dffunction>
fdiv_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_fdiv_im<T>>(id_col(right), left, as, true);
}

template <class T>
std::shared_ptr<dffunction>
fdiv_im(T left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_fdiv_im<T>>(right, left, true);
}

template <class T>
std::shared_ptr<dffunction>
fdiv_im_as(T left, const std::shared_ptr<dffunction>& right,
           const std::string& as) {
  return std::make_shared<dffunction_fdiv_im<T>>(right, left, as, true);
}

// ----- idiv -----
struct dffunction_idiv : public dffunction {
  dffunction_idiv(const std::shared_ptr<dffunction>& left, 
                  const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->as() + " div " + right->as() + ")";
  }
  dffunction_idiv(const std::shared_ptr<dffunction>& left, 
                  const std::shared_ptr<dffunction>& right, 
                  const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
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
  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dffunction>
idiv_col(const std::string& left, const std::string& right); 
std::shared_ptr<dffunction>
idiv_col(const std::shared_ptr<dffunction>& left, const std::string& right); 
std::shared_ptr<dffunction>
idiv_col(const std::string& left, const std::shared_ptr<dffunction>& right); 
std::shared_ptr<dffunction>
idiv_col(const std::shared_ptr<dffunction>& left,
         const std::shared_ptr<dffunction>& right); 

std::shared_ptr<dffunction>
idiv_col_as(const std::string& left, const std::string& right,
            const std::string& as);
std::shared_ptr<dffunction>
idiv_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
            const std::string& as);
std::shared_ptr<dffunction>
idiv_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
            const std::string& as);
std::shared_ptr<dffunction>
idiv_col_as(const std::shared_ptr<dffunction>& left,
            const std::shared_ptr<dffunction>& right,
            const std::string& as);

template <class T>
struct dffunction_idiv_im : public dffunction {
  dffunction_idiv_im(const std::shared_ptr<dffunction>& left, T right,
                     bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed) {
    if(is_reversed) as_name = "(" + STR(right) + " div " + left->as() + ")";
    else            as_name = "(" + left->as() + " div " + STR(right) + ")";
  }
  dffunction_idiv_im(const std::shared_ptr<dffunction>& left, T right,
                     const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1 // t2: is not available for idiv_im operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }

  std::shared_ptr<dffunction> left;
  T right;
  bool is_reversed;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn>
dffunction_idiv_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = left->execute(t);
  return is_reversed ? left_column->ridiv_im(right_scalar) 
    : left_column->idiv_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
idiv_im(const std::string& left, T right) {
  return std::make_shared<dffunction_idiv_im<T>>(id_col(left), right);
}

template <class T>
std::shared_ptr<dffunction>
idiv_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_idiv_im<T>>(id_col(left), right, as);
}

template <class T>
std::shared_ptr<dffunction>
idiv_im(const std::shared_ptr<dffunction>& left, T right) {
  return std::make_shared<dffunction_idiv_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
idiv_im_as(const std::shared_ptr<dffunction>& left, T right,
           const std::string& as) {
  return std::make_shared<dffunction_idiv_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
idiv_im(T left, const std::string& right) {
  return std::make_shared<dffunction_idiv_im<T>>(id_col(right), left, true);
}

template <class T>
std::shared_ptr<dffunction>
idiv_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_idiv_im<T>>(id_col(right), left, as, true);
}

template <class T>
std::shared_ptr<dffunction>
idiv_im(T left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_idiv_im<T>>(right, left, true);
}

template <class T>
std::shared_ptr<dffunction>
idiv_im_as(T left, const std::shared_ptr<dffunction>& right,
           const std::string& as) {
  return std::make_shared<dffunction_idiv_im<T>>(right, left, as, true);
}


// ----- mod -----
struct dffunction_mod : public dffunction {
  dffunction_mod(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->as() + "%" + right->as() + ")";
  }
  dffunction_mod(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right, 
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
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
  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dffunction>
mod_col(const std::string& left, const std::string& right); 
std::shared_ptr<dffunction>
mod_col(const std::shared_ptr<dffunction>& left, const std::string& right); 
std::shared_ptr<dffunction>
mod_col(const std::string& left, const std::shared_ptr<dffunction>& right); 
std::shared_ptr<dffunction>
mod_col(const std::shared_ptr<dffunction>& left,
        const std::shared_ptr<dffunction>& right); 

std::shared_ptr<dffunction>
mod_col_as(const std::string& left, const std::string& right,
           const std::string& as);
std::shared_ptr<dffunction>
mod_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
           const std::string& as);
std::shared_ptr<dffunction>
mod_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
           const std::string& as);
std::shared_ptr<dffunction>
mod_col_as(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right,
           const std::string& as);

template <class T>
struct dffunction_mod_im : public dffunction {
  dffunction_mod_im(const std::shared_ptr<dffunction>& left, T right,
                    bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed) {
    if(is_reversed) as_name = "(" + STR(right) + "%" + left->as() + ")";
    else            as_name = "(" + left->as() + "%" + STR(right) + ")";
  }
  dffunction_mod_im(const std::shared_ptr<dffunction>& left, T right,
                    const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1 % t2: is not available for mod_im operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }

  std::shared_ptr<dffunction> left;
  T right;
  bool is_reversed;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn>
dffunction_mod_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = left->execute(t);
  return is_reversed ? left_column->rmod_im(right_scalar) 
    : left_column->mod_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
mod_im(const std::string& left, T right) {
  return std::make_shared<dffunction_mod_im<T>>(id_col(left), right);
}

template <class T>
std::shared_ptr<dffunction>
mod_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_mod_im<T>>(id_col(left), right, as);
}

template <class T>
std::shared_ptr<dffunction>
mod_im(const std::shared_ptr<dffunction>& left, T right) {
  return std::make_shared<dffunction_mod_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
mod_im_as(const std::shared_ptr<dffunction>& left, T right,
          const std::string& as) {
  return std::make_shared<dffunction_mod_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
mod_im(T left, const std::string& right) {
  return std::make_shared<dffunction_mod_im<T>>(id_col(right), left, true);
}

template <class T>
std::shared_ptr<dffunction>
mod_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_mod_im<T>>(id_col(right), left, as, true);
}

template <class T>
std::shared_ptr<dffunction>
mod_im(T left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_mod_im<T>>(right, left, true);
}

template <class T>
std::shared_ptr<dffunction>
mod_im_as(T left, const std::shared_ptr<dffunction>& right,
          const std::string& as) {
  return std::make_shared<dffunction_mod_im<T>>(right, left, as, true);
}


// ----- pow -----
struct dffunction_pow : public dffunction {
  dffunction_pow(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->as() + "**" + right->as() + ")";
  }
  dffunction_pow(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right, 
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
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
  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dffunction>
pow_col(const std::string& left, const std::string& right); 
std::shared_ptr<dffunction>
pow_col(const std::shared_ptr<dffunction>& left, const std::string& right); 
std::shared_ptr<dffunction>
pow_col(const std::string& left, const std::shared_ptr<dffunction>& right); 
std::shared_ptr<dffunction>
pow_col(const std::shared_ptr<dffunction>& left,
        const std::shared_ptr<dffunction>& right); 

std::shared_ptr<dffunction>
pow_col_as(const std::string& left, const std::string& right,
           const std::string& as);
std::shared_ptr<dffunction>
pow_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
           const std::string& as);
std::shared_ptr<dffunction>
pow_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
           const std::string& as);
std::shared_ptr<dffunction>
pow_col_as(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right,
           const std::string& as);

template <class T>
struct dffunction_pow_im : public dffunction {
  dffunction_pow_im(const std::shared_ptr<dffunction>& left, T right,
                    bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed) {
    if(is_reversed) as_name = "(" + STR(right) + "**" + left->as() + ")";
    else            as_name = "(" + left->as() + "**" + STR(right) + ")";
  }
  dffunction_pow_im(const std::shared_ptr<dffunction>& left, T right,
                    const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1 ** t2: is not available for pow_im operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }

  std::shared_ptr<dffunction> left;
  T right;
  bool is_reversed;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn>
dffunction_pow_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = left->execute(t);
  return is_reversed ? left_column->rpow_im(right_scalar) 
    : left_column->pow_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
pow_im(const std::string& left, T right) {
  return std::make_shared<dffunction_pow_im<T>>(id_col(left), right);
}

template <class T>
std::shared_ptr<dffunction>
pow_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_pow_im<T>>(id_col(left), right, as);
}

template <class T>
std::shared_ptr<dffunction>
pow_im(const std::shared_ptr<dffunction>& left, T right) {
  return std::make_shared<dffunction_pow_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
pow_im_as(const std::shared_ptr<dffunction>& left, T right,
          const std::string& as) {
  return std::make_shared<dffunction_pow_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
pow_im(T left, const std::string& right) {
  return std::make_shared<dffunction_pow_im<T>>(id_col(right), left, true);
}

template <class T>
std::shared_ptr<dffunction>
pow_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_pow_im<T>>(id_col(right), left, as, true);
}

template <class T>
std::shared_ptr<dffunction>
pow_im(T left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_pow_im<T>>(right, left, true);
}

template <class T>
std::shared_ptr<dffunction>
pow_im_as(T left, const std::shared_ptr<dffunction>& right,
          const std::string& as) {
  return std::make_shared<dffunction_pow_im<T>>(right, left, as, true);
}


// ----- abs -----
struct dffunction_abs : public dffunction {
  dffunction_abs(const std::shared_ptr<dffunction>& left) : left(left) {
    as_name = "abs(" + left->as() + ")";
  }
  dffunction_abs(const std::shared_ptr<dffunction>& left,
                 const std::string& as_name) :
    left(left), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, 
                                            dftable_base& t2) const {
    throw std::runtime_error("abs(): is not available for binary operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }

  std::shared_ptr<dffunction> left;
  std::string as_name;
};

std::shared_ptr<dffunction>
abs_col(const std::string& left);

std::shared_ptr<dffunction>
abs_col(const std::shared_ptr<dffunction>& left);

std::shared_ptr<dffunction>
abs_col_as(const std::string& left, const std::string& as);

std::shared_ptr<dffunction>
abs_col_as(const std::shared_ptr<dffunction>& left, const std::string& as);

}
#endif
