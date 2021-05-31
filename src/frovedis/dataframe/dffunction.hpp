#ifndef DFFUNCTION_HPP
#define DFFUNCTION_HPP

#include "dfscalar.hpp"
#include "dftable.hpp"

namespace frovedis {

struct dffunction {
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const = 0;
  virtual std::string as() = 0;
};

struct dffunction_add : public dffunction {
  dffunction_add(const std::string& left, const std::string& right) :
    left(left), right(right) {
    as_name = "(" + left + "+" + right + ")";
  }
  dffunction_add(const std::string& left, const std::string& right,
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  std::shared_ptr<dfcolumn> execute(dftable_base& t1, dftable_base& t2) const;

  std::string left, right, as_name;
};

std::shared_ptr<dffunction>
add_col(const std::string& left, const std::string& right);

std::shared_ptr<dffunction>
add_col_as(const std::string& left, const std::string& right,
           const std::string& as);

template <class T>
struct dffunction_add_im : public dffunction {
  dffunction_add_im(const std::string& left, T right) :
    left(left), right(right) {
    as_name = "(" + left + "+" + std::to_string(right) + ")";
  }
  dffunction_add_im(const std::string& left, T right,
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;

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


struct dffunction_sub : public dffunction {
  dffunction_sub(const std::string& left, const std::string& right) :
    left(left), right(right) {
    as_name = "(" + left + "-" + right + ")";
  }
  dffunction_sub(const std::string& left, const std::string& right,
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;

  std::string left, right, as_name;
};

std::shared_ptr<dffunction>
sub_col(const std::string& left, const std::string& right);

std::shared_ptr<dffunction>
sub_col_as(const std::string& left, const std::string& right,
           const std::string& as);

template <class T>
struct dffunction_sub_im : public dffunction {
  dffunction_sub_im(const std::string& left, T right) :
    left(left), right(right) {
    as_name = "(" + left + "-" + std::to_string(right) + ")";
  }
  dffunction_sub_im(const std::string& left, T right,
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;

  std::string left;
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_sub_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = t.column(left);
  return left_column->sub_im(right_scalar);
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


struct dffunction_mul : public dffunction {
  dffunction_mul(const std::string& left, const std::string& right) :
    left(left), right(right) {
    as_name = "(" + left + "*" + right + ")";
  }
  dffunction_mul(const std::string& left, const std::string& right,
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;

  std::string left, right, as_name;
};

std::shared_ptr<dffunction>
mul_col(const std::string& left, const std::string& right);

std::shared_ptr<dffunction>
mul_col_as(const std::string& left, const std::string& right,
           const std::string& as);

template <class T>
struct dffunction_mul_im : public dffunction {
  dffunction_mul_im(const std::string& left, T right) :
    left(left), right(right) {
    as_name = "(" + left + "*" + std::to_string(right) + ")";
  }
  dffunction_mul_im(const std::string& left, T right,
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;

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


struct dffunction_fdiv : public dffunction {
  dffunction_fdiv(const std::string& left, const std::string& right) :
    left(left), right(right) {
    as_name = "(" + left + "/" + right + ")";
  }
  dffunction_fdiv(const std::string& left, const std::string& right,
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;

  std::string left, right, as_name;
};

std::shared_ptr<dffunction>
fdiv_col(const std::string& left, const std::string& right);

std::shared_ptr<dffunction>
fdiv_col_as(const std::string& left, const std::string& right,
           const std::string& as);

template <class T>
struct dffunction_fdiv_im : public dffunction {
  dffunction_fdiv_im(const std::string& left, T right) :
    left(left), right(right) {
    as_name = "(" + left + "/" + std::to_string(right) + ")";
  }
  dffunction_fdiv_im(const std::string& left, T right,
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;

  std::string left;
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_fdiv_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = t.column(left);
  return left_column->fdiv_im(right_scalar);
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


struct dffunction_idiv : public dffunction {
  dffunction_idiv(const std::string& left, const std::string& right) :
    left(left), right(right) {
    as_name = "(" + left + " div " + right + ")";
  }
  dffunction_idiv(const std::string& left, const std::string& right,
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;

  std::string left, right, as_name;
};

std::shared_ptr<dffunction>
idiv_col(const std::string& left, const std::string& right);

std::shared_ptr<dffunction>
idiv_col_as(const std::string& left, const std::string& right,
           const std::string& as);

template <class T>
struct dffunction_idiv_im : public dffunction {
  dffunction_idiv_im(const std::string& left, T right) :
    left(left), right(right) {
    as_name = "(" + left + " div " + std::to_string(right) + ")";
  }
  dffunction_idiv_im(const std::string& left, T right,
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;

  std::string left;
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_idiv_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = t.column(left);
  return left_column->idiv_im(right_scalar);
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


struct dffunction_mod : public dffunction {
  dffunction_mod(const std::string& left, const std::string& right) :
    left(left), right(right) {
    as_name = "(" + left + "%" + right + ")";
  }
  dffunction_mod(const std::string& left, const std::string& right,
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;

  std::string left, right, as_name;
};

std::shared_ptr<dffunction>
mod_col(const std::string& left, const std::string& right);

std::shared_ptr<dffunction>
mod_col_as(const std::string& left, const std::string& right,
           const std::string& as);

template <class T>
struct dffunction_mod_im : public dffunction {
  dffunction_mod_im(const std::string& left, T right) :
    left(left), right(right) {
    as_name = "(" + left + "%" + std::to_string(right) + ")";
  }
  dffunction_mod_im(const std::string& left, T right,
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;

  std::string left;
  T right;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_mod_im<T>::execute(dftable_base& t) const {
  std::shared_ptr<dfscalar> right_scalar =
    std::make_shared<typed_dfscalar<T>>(right);
  auto left_column = t.column(left);
  return left_column->mod_im(right_scalar);
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


struct dffunction_abs : public dffunction {
  dffunction_abs(const std::string& left) : left(left) {
    as_name = "abs(" + left + ")";
  }
  dffunction_abs(const std::string& left, const std::string& as_name) :
    left(left), as_name(as_name) {}
  virtual std::string as() {return as_name;}
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;

  std::string left, as_name;
};

std::shared_ptr<dffunction>
abs_col(const std::string& left);

std::shared_ptr<dffunction>
abs_col_as(const std::string& left, const std::string& as);

}
#endif
