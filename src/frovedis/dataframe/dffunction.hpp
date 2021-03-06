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

struct dffunction_add : public dffunction {
  dffunction_add(const std::string& left, const std::string& right): 
    left(left), right(right) {
    as_name = "(" + left + "+" + right + ")";
  }
  dffunction_add(const std::string& left, const std::string& right,
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {} 
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }

  std::string left, right, as_name;
};

std::shared_ptr<dffunction>
add_col(const std::string& left, const std::string& right); 

std::shared_ptr<dffunction>
add_col_as(const std::string& left, const std::string& right,
           const std::string& as);

template <class T>
struct dffunction_add_im : public dffunction {
  dffunction_add_im(const std::string& left, T right): 
    left(left), right(right) {
    as_name = "(" + left + "+" + STR(right) + ")";
  }
  dffunction_add_im(const std::string& left, T right,
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error("t1 + t2: is not available for add_im operation!\n");
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
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_add_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = t.column(left);
  return left_column->add_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
add_im(const std::string& left, T right) {
  return std::make_shared<dffunction_add_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
add_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_add_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
add_im(T left, const std::string& right) {
  return std::make_shared<dffunction_add_im<T>>(right, left);
}

template <class T>
std::shared_ptr<dffunction>
add_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_add_im<T>>(right, left, as);
}

struct dffunction_sub : public dffunction {
  dffunction_sub(const std::string& left, const std::string& right):
    left(left), right(right) {
    as_name = "(" + left + "-" + right + ")";
  }
  dffunction_sub(const std::string& left, const std::string& right,
                 const std::string& as_name): 
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }

  std::string left, right, as_name; 
};

std::shared_ptr<dffunction>
sub_col(const std::string& left, const std::string& right);

std::shared_ptr<dffunction>
sub_col_as(const std::string& left, const std::string& right,
           const std::string& as);

template <class T>
struct dffunction_sub_im : public dffunction {
  dffunction_sub_im(const std::string& left, T right,
                    bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed) {
    if(is_reversed) as_name = "(" + STR(right) + "-" + left + ")";
    else            as_name = "(" + left + "-" + STR(right) + ")";
  }
  dffunction_sub_im(const std::string& left, T right,
                    const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), 
    is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error("t1 - t2: is not available for sub_im operation!\n");
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
  T right;
  bool is_reversed;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_sub_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = t.column(left);
  return is_reversed ? left_column->rsub_im(right_scalar) 
                     : left_column->sub_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
sub_im(const std::string& left, T right) {
  return std::make_shared<dffunction_sub_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
sub_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_sub_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
sub_im(T left, const std::string& right) {
  return std::make_shared<dffunction_sub_im<T>>(right, left, true);
}

template <class T>
std::shared_ptr<dffunction>
sub_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_sub_im<T>>(right, left, as, true);
}


struct dffunction_mul : public dffunction {
  dffunction_mul(const std::string& left, const std::string& right):
    left(left), right(right) {
    as_name = "(" + left + "*" + right + ")";
  }
  dffunction_mul(const std::string& left, const std::string& right,
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }

  std::string left, right, as_name;
};

std::shared_ptr<dffunction>
mul_col(const std::string& left, const std::string& right);

std::shared_ptr<dffunction>
mul_col_as(const std::string& left, const std::string& right,
           const std::string& as);

template <class T>
struct dffunction_mul_im : public dffunction {
  dffunction_mul_im(const std::string& left, T right):
    left(left), right(right) {
    as_name = "(" + left + "*" + STR(right) + ")";
  }
  dffunction_mul_im(const std::string& left, T right,
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error("t1 * t2: is not available for mul_im operation!\n");
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
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_mul_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = t.column(left);
  return left_column->mul_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
mul_im(const std::string& left, T right) {
  return std::make_shared<dffunction_mul_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
mul_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_mul_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
mul_im(T left, const std::string& right) {
  return std::make_shared<dffunction_mul_im<T>>(right, left);
}

template <class T>
std::shared_ptr<dffunction>
mul_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_mul_im<T>>(right, left, as);
}

struct dffunction_fdiv : public dffunction {
  dffunction_fdiv(const std::string& left, const std::string& right) :
    left(left), right(right) {
    as_name = "(" + left + "/" + right + ")";
  }
  dffunction_fdiv(const std::string& left, const std::string& right,
                  const std::string& as_name): 
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }

  std::string left, right, as_name; 
};

std::shared_ptr<dffunction>
fdiv_col(const std::string& left, const std::string& right); 

std::shared_ptr<dffunction>
fdiv_col_as(const std::string& left, const std::string& right,
            const std::string& as);

template <class T>
struct dffunction_fdiv_im : public dffunction {
  dffunction_fdiv_im(const std::string& left, T right, 
                     bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed) {
    if(is_reversed) as_name = "(" + STR(right) + "/" + left + ")";
    else            as_name = "(" + left + "/" + STR(right) + ")";
  }
  dffunction_fdiv_im(const std::string& left, T right,
                     const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error("t1 / t2: is not available for fdiv_im operation!\n");
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
  T right;
  bool is_reversed;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_fdiv_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = t.column(left);
  return is_reversed ? left_column->rfdiv_im(right_scalar) 
                     : left_column->fdiv_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
fdiv_im(const std::string& left, T right) {
  return std::make_shared<dffunction_fdiv_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
fdiv_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_fdiv_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
fdiv_im(T left, const std::string& right) {
  return std::make_shared<dffunction_fdiv_im<T>>(right, left, true);
}

template <class T>
std::shared_ptr<dffunction>
fdiv_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_fdiv_im<T>>(right, left, as, true);
}

struct dffunction_idiv : public dffunction {
  dffunction_idiv(const std::string& left, const std::string& right):
    left(left), right(right) {
    as_name = "(" + left + " div " + right + ")";
  }
  dffunction_idiv(const std::string& left, const std::string& right,
                  const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }

  std::string left, right, as_name; 
};

std::shared_ptr<dffunction>
idiv_col(const std::string& left, const std::string& right); 

std::shared_ptr<dffunction>
idiv_col_as(const std::string& left, const std::string& right,
            const std::string& as);

template <class T>
struct dffunction_idiv_im : public dffunction {
  dffunction_idiv_im(const std::string& left, T right,
                     bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed) {
    if(is_reversed) as_name = "(" + STR(right) + " div " + left + ")";
    else            as_name = "(" + left + " div " + STR(right) + ")";
  }
  dffunction_idiv_im(const std::string& left, T right,
                     const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error("t1 // t2: is not available for idiv_im operation!\n");
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
  T right;
  bool is_reversed;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_idiv_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = t.column(left);
  return is_reversed ? left_column->ridiv_im(right_scalar)
                     : left_column->idiv_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
idiv_im(const std::string& left, T right) {
  return std::make_shared<dffunction_idiv_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
idiv_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_idiv_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
idiv_im(T left, const std::string& right) {
  return std::make_shared<dffunction_idiv_im<T>>(right, left, true);
}

template <class T>
std::shared_ptr<dffunction>
idiv_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_idiv_im<T>>(right, left, as, true);
}

struct dffunction_mod : public dffunction {
  dffunction_mod(const std::string& left, const std::string& right):
    left(left), right(right) {
    as_name = "(" + left + "%" + right + ")";
  }
  dffunction_mod(const std::string& left, const std::string& right,
                 const std::string& as_name):
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }

  std::string left, right, as_name; 
};

std::shared_ptr<dffunction>
mod_col(const std::string& left, const std::string& right); 

std::shared_ptr<dffunction>
mod_col_as(const std::string& left, const std::string& right,
           const std::string& as);

template <class T>
struct dffunction_mod_im : public dffunction {
  dffunction_mod_im(const std::string& left, T right,
                    bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed) {
    if(is_reversed) as_name = "(" + STR(right) + "%" + left + ")";
    else            as_name = "(" + left + "%" + STR(right) + ")";
  }
  dffunction_mod_im(const std::string& left, T right,
                    const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed),as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error("t1 % t2: is not available for mod_im operation!\n");
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
  T right;
  bool is_reversed;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_mod_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = t.column(left);
  return is_reversed ? left_column->rmod_im(right_scalar)
                     : left_column->mod_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
mod_im(const std::string& left, T right) {
  return std::make_shared<dffunction_mod_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
mod_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_mod_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
mod_im(T left, const std::string& right) {
  return std::make_shared<dffunction_mod_im<T>>(right, left, true);
}

template <class T>
std::shared_ptr<dffunction>
mod_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_mod_im<T>>(right, left, as, true);
}

struct dffunction_pow : public dffunction {
  dffunction_pow(const std::string& left, const std::string& right):
    left(left), right(right) {
    as_name = "(" + left + " ** " + right + ")";
  }
  dffunction_pow(const std::string& left, const std::string& right,
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return {t.raw_column(left), t.raw_column(right)};
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    return {t1.raw_column(left), t2.raw_column(right)};
  }

  std::string left, right, as_name; 
};

std::shared_ptr<dffunction>
pow_col(const std::string& left, const std::string& right); 

std::shared_ptr<dffunction>
pow_col_as(const std::string& left, const std::string& right,
           const std::string& as);

template <class T>
struct dffunction_pow_im : public dffunction {
  dffunction_pow_im(const std::string& left, T right,
                    bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed) {
    if(is_reversed) as_name = "(" + STR(right) + " ** " + left + ")";
    else            as_name = "(" + left + " ** " + STR(right) + ")";
  }
  dffunction_pow_im(const std::string& left, T right,
                    const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error("t1 ** t2: is not available for pow_im operation!\n");
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
  T right;
  bool is_reversed;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_pow_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = t.column(left);
  return is_reversed ? left_column->rpow_im(right_scalar)
                     : left_column->pow_im(right_scalar);
}

template <class T>
std::shared_ptr<dffunction>
pow_im(const std::string& left, T right) {
  return std::make_shared<dffunction_pow_im<T>>(left, right);
}

template <class T>
std::shared_ptr<dffunction>
pow_im_as(const std::string& left, T right, const std::string& as) {
  return std::make_shared<dffunction_pow_im<T>>(left, right, as);
}

template <class T>
std::shared_ptr<dffunction>
pow_im(T left, const std::string& right) {
  return std::make_shared<dffunction_pow_im<T>>(right, left, true);
}

template <class T>
std::shared_ptr<dffunction>
pow_im_as(T left, const std::string& right, const std::string& as) {
  return std::make_shared<dffunction_pow_im<T>>(right, left, as, true);
}

struct dffunction_abs : public dffunction {
  dffunction_abs(const std::string& left) : left(left) {
    as_name = "abs(" + left + ")";
  }
  dffunction_abs(const std::string& left, const std::string& as_name) :
    left(left), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, 
                                            dftable_base& t2) const {
    throw std::runtime_error("abs(): is not available for binary operation!\n");
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

  std::string left, as_name;
};

std::shared_ptr<dffunction>
abs_col(const std::string& left);

std::shared_ptr<dffunction>
abs_col_as(const std::string& left, const std::string& as);

}
#endif
