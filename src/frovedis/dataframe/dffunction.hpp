#ifndef DFFUNCTION_HPP
#define DFFUNCTION_HPP

#include "dfscalar.hpp"
#include "dftable.hpp"
#include "../text/datetime_to_words.hpp"
#include "dfoperator.hpp" // for struct dffunction definition

namespace frovedis {

bool check_distribution(dftable_base& left, 
                        dftable_base& right,
                        std::vector<size_t>& left_sizes);

std::shared_ptr<dfcolumn> realign_df(dftable_base& left, 
                                     dftable_base& right,
                                     std::shared_ptr<dfcolumn>& rightcol);

bool verify_column_identicality(dftable_base& left,
                                const std::string& lcol,
                                dftable_base& right,
                                const std::string& rcol);

/*
  naming convention of free functions:
  - add "_col" for functions, since function name like "add" might be
    already used (e.g. core/utility.hpp)
  - add "_im" for immediate arguments.
    do not use the same function name since over loading does not work here
    because immediate is template type
*/

// ----- cast -----
struct dffunction_cast : public dffunction {
  dffunction_cast(const std::shared_ptr<dffunction>& left,
                  const std::string& to): left(left), to_type(to) {
    as_name = "CAST(" + left->get_as() + " AS " + to + ")";
  }
  dffunction_cast(const std::shared_ptr<dffunction>& left,
                 const std::string& to,
                 const std::string& as_name) :
    left(left), to_type(to), as_name(as_name) {}

  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_cast>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) { return left->columns_to_use(t); }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  std::string to_type, as_name;
};

std::shared_ptr<dffunction>
cast_col(const std::string& left, const std::string& to);
 
std::shared_ptr<dffunction>
cast_col(const std::shared_ptr<dffunction>& left, const std::string& to); 

std::shared_ptr<dffunction>
cast_col_as(const std::string& left, const std::string& to,
            const std::string& as);

std::shared_ptr<dffunction>
cast_col_as(const std::shared_ptr<dffunction>& left, const std::string& to,
            const std::string& as);

// ----- add -----
struct dffunction_add : public dffunction {
  dffunction_add(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "(" + left->get_as() + "+" + right->get_as() + ")";
  }
  dffunction_add(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right, 
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_add>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
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
    as_name = "(" + left->get_as() + "+" + STR(right) + ")";
  }
  dffunction_add_im(const std::shared_ptr<dffunction>& left, T right,
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_add_im>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
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
    as_name = "(" + left->get_as() + "-" + right->get_as() + ")";
  }
  dffunction_sub(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right, 
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_sub>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
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
    if(is_reversed) as_name = "(" + STR(right) + "-" + left->get_as() + ")";
    else            as_name = "(" + left->get_as() + "-" + STR(right) + ")";
  }
  dffunction_sub_im(const std::shared_ptr<dffunction>& left, T right,
                    const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_sub_im>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
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
    as_name = "(" + left->get_as() + "*" + right->get_as() + ")";
  }
  dffunction_mul(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right, 
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_mul>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
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
    as_name = "(" + left->get_as() + "*" + STR(right) + ")";
  }
  dffunction_mul_im(const std::shared_ptr<dffunction>& left, T right,
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_mul_im>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
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
    as_name = "(" + left->get_as() + "/" + right->get_as() + ")";
  }
  dffunction_fdiv(const std::shared_ptr<dffunction>& left, 
                  const std::shared_ptr<dffunction>& right, 
                  const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_fdiv>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
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
    if(is_reversed) as_name = "(" + STR(right) + "/" + left->get_as() + ")";
    else            as_name = "(" + left->get_as() + "/" + STR(right) + ")";
  }
  dffunction_fdiv_im(const std::shared_ptr<dffunction>& left, T right,
                     const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_fdiv_im>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
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
    as_name = "(" + left->get_as() + " div " + right->get_as() + ")";
  }
  dffunction_idiv(const std::shared_ptr<dffunction>& left, 
                  const std::shared_ptr<dffunction>& right, 
                  const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_idiv>(*this);
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
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
    if(is_reversed) as_name = "(" + STR(right) + " div " + left->get_as() + ")";
    else            as_name = "(" + left->get_as() + " div " + STR(right) + ")";
  }
  dffunction_idiv_im(const std::shared_ptr<dffunction>& left, T right,
                     const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_idiv_im>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
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
    as_name = "(" + left->get_as() + "%" + right->get_as() + ")";
  }
  dffunction_mod(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right, 
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_mod>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
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
    if(is_reversed) as_name = "(" + STR(right) + "%" + left->get_as() + ")";
    else            as_name = "(" + left->get_as() + "%" + STR(right) + ")";
  }
  dffunction_mod_im(const std::shared_ptr<dffunction>& left, T right,
                    const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_mod_im>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
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
    as_name = "(" + left->get_as() + "**" + right->get_as() + ")";
  }
  dffunction_pow(const std::shared_ptr<dffunction>& left, 
                 const std::shared_ptr<dffunction>& right, 
                 const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_pow>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
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
    if(is_reversed) as_name = "(" + STR(right) + "**" + left->get_as() + ")";
    else            as_name = "(" + left->get_as() + "**" + STR(right) + ")";
  }
  dffunction_pow_im(const std::shared_ptr<dffunction>& left, T right,
                    const std::string& as_name, bool is_reversed = false) :
    left(left), right(right), is_reversed(is_reversed), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_pow_im>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
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
    as_name = "abs(" + left->get_as() + ")";
  }
  dffunction_abs(const std::shared_ptr<dffunction>& left,
                 const std::string& as_name) :
    left(left), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_abs>(*this);
  }
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
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


// ----- datetime_extract -----
struct dffunction_datetime_extract : public dffunction {
  dffunction_datetime_extract(const std::shared_ptr<dffunction>& left,
                              const datetime_type type)
    : left(left), type(type) {
    auto typestr = datetime_type_to_string(type);
    as_name = typestr + "(" + left->get_as() + ")";
  }
  dffunction_datetime_extract(const std::shared_ptr<dffunction>& left,
                              const datetime_type type,
                              const std::string& as_name) :
    left(left), type(type), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_datetime_extract>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, 
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("datetime_extract(): is not available for binary operation!\n");
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  datetime_type type;
  std::string as_name;
};

std::shared_ptr<dffunction>
datetime_extract_col(const std::string& left, datetime_type type);

std::shared_ptr<dffunction>
datetime_extract_col(const std::shared_ptr<dffunction>& left,
                     datetime_type type);

std::shared_ptr<dffunction>
datetime_extract_col_as(const std::string& left, datetime_type type,
                        const std::string& as);

std::shared_ptr<dffunction>
datetime_extract_col_as(const std::shared_ptr<dffunction>& left,
                        datetime_type type, const std::string& as);


// ----- datetime_diff -----
struct dffunction_datetime_diff : public dffunction {
  dffunction_datetime_diff(const std::shared_ptr<dffunction>& left,
                           const std::shared_ptr<dffunction>& right,
                           const datetime_type type)
    : left(left), right(right), type(type) {
    auto typestr = datetime_type_to_string(type);
    as_name = "diff_" + typestr + "(" + left->get_as() + "," + right->get_as() + ")";
  }
  dffunction_datetime_diff(const std::shared_ptr<dffunction>& left,
                           const std::shared_ptr<dffunction>& right,
                           const datetime_type type,
                           const std::string& as_name) :
    left(left), right(right), type(type), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_datetime_diff>(*this);
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  datetime_type type;
  std::string as_name;
};

std::shared_ptr<dffunction>
datetime_diff_col(const std::string& left, const std::string& right,
                  datetime_type type);

std::shared_ptr<dffunction>
datetime_diff_col(const std::shared_ptr<dffunction>& left,
                  const std::string& right,
                  datetime_type type);

std::shared_ptr<dffunction>
datetime_diff_col(const std::string& left,
                  const std::shared_ptr<dffunction>& right,
                  datetime_type type);

std::shared_ptr<dffunction>
datetime_diff_col(const std::shared_ptr<dffunction>&left,
                  const std::shared_ptr<dffunction>& right,
                  datetime_type type);

std::shared_ptr<dffunction>
datetime_diff_col_as(const std::string& left, const std::string& right,
                     datetime_type type, const std::string& as);

std::shared_ptr<dffunction>
datetime_diff_col_as(const std::shared_ptr<dffunction>& left,
                     const std::string& right,
                     datetime_type type, const std::string& as);

std::shared_ptr<dffunction>
datetime_diff_col_as(const std::string& left,
                     const std::shared_ptr<dffunction>& right,
                     datetime_type type, const std::string& as);

std::shared_ptr<dffunction>
datetime_diff_col_as(const std::shared_ptr<dffunction>&left,
                     const std::shared_ptr<dffunction>& right,
                     datetime_type type, const std::string& as);


// ----- datetime_diff_im -----
struct dffunction_datetime_diff_im : public dffunction {
  dffunction_datetime_diff_im(const std::shared_ptr<dffunction>& left,
                              datetime_t right,
                              const datetime_type type,
                              bool is_reversed = false) :
    left(left), right(right), type(type), is_reversed(is_reversed) {
    auto vs = words_to_vector_string(datetime_to_words({right}, "%Y-%m-%d"));
    if(is_reversed) as_name = "datetime_diff(" + vs[0] + "," + left->get_as() + ")";
    else as_name = "datetime_diff(" + left->get_as() + "," + vs[0] + ")";
  }
  dffunction_datetime_diff_im(const std::shared_ptr<dffunction>& left,
                              datetime_t right,
                              const datetime_type type,
                              const std::string& as_name,
                              bool is_reversed = false) :
    left(left), right(right), type(type), is_reversed(is_reversed),
    as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_datetime_diff_im>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1, t2 is not available for datetime_diff_im operation!\n");
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  datetime_t right;
  datetime_type type;
  bool is_reversed;
  std::string as_name;
};

std::shared_ptr<dffunction>
datetime_diff_im(const std::string& left, datetime_t right,
                 datetime_type type);

std::shared_ptr<dffunction>
datetime_diff_im_as(const std::string& left, datetime_t right,
                    datetime_type type, const std::string& as);

std::shared_ptr<dffunction>
datetime_diff_im(const std::shared_ptr<dffunction>& left, datetime_t right,
                 datetime_type type);

std::shared_ptr<dffunction>
datetime_diff_im_as(const std::shared_ptr<dffunction>& left, datetime_t right,
                    datetime_type type, const std::string& as);

std::shared_ptr<dffunction>
datetime_diff_im
(datetime_t left, const std::string& right, datetime_type type);

std::shared_ptr<dffunction>
datetime_diff_im_as(datetime_t left, const std::string& right,
                    datetime_type type, const std::string& as);

std::shared_ptr<dffunction>
datetime_diff_im(datetime_t left, const std::shared_ptr<dffunction>& right,
                 datetime_type type);

std::shared_ptr<dffunction>
datetime_diff_im_as(datetime_t left, const std::shared_ptr<dffunction>& right,
                    datetime_type type, const std::string& as);

// ----- datetime_add -----
struct dffunction_datetime_add : public dffunction {
  dffunction_datetime_add(const std::shared_ptr<dffunction>& left,
                          const std::shared_ptr<dffunction>& right,
                          const datetime_type type)
    : left(left), right(right), type(type) {
    auto typestr = datetime_type_to_string(type);
    as_name = "(" + left->get_as() + "+" + right->get_as() + typestr + ")";
  }
  dffunction_datetime_add(const std::shared_ptr<dffunction>& left,
                          const std::shared_ptr<dffunction>& right,
                          const datetime_type type,
                          const std::string& as_name) :
    left(left), right(right), type(type), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_datetime_add>(*this);
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  datetime_type type;
  std::string as_name;
};

std::shared_ptr<dffunction>
datetime_add_col(const std::string& left, const std::string& right,
                 datetime_type type);

std::shared_ptr<dffunction>
datetime_add_col(const std::shared_ptr<dffunction>& left,
                 const std::string& right,
                 datetime_type type);

std::shared_ptr<dffunction>
datetime_add_col(const std::string& left,
                 const std::shared_ptr<dffunction>& right,
                 datetime_type type);

std::shared_ptr<dffunction>
datetime_add_col(const std::shared_ptr<dffunction>&left,
                 const std::shared_ptr<dffunction>& right,
                 datetime_type type);

std::shared_ptr<dffunction>
datetime_add_col_as(const std::string& left, const std::string& right,
                    datetime_type type, const std::string& as);

std::shared_ptr<dffunction>
datetime_add_col_as(const std::shared_ptr<dffunction>& left,
                    const std::string& right,
                    datetime_type type, const std::string& as);

std::shared_ptr<dffunction>
datetime_add_col_as(const std::string& left,
                    const std::shared_ptr<dffunction>& right,
                    datetime_type type, const std::string& as);

std::shared_ptr<dffunction>
datetime_add_col_as(const std::shared_ptr<dffunction>&left,
                    const std::shared_ptr<dffunction>& right,
                    datetime_type type, const std::string& as);


// ----- datetime_add_im -----
struct dffunction_datetime_add_im : public dffunction {
  dffunction_datetime_add_im(const std::shared_ptr<dffunction>& left, int right,
                             datetime_type type): 
    left(left), right(right), type(type)  {
    auto typestr = datetime_type_to_string(type);
    as_name = "(" + left->get_as() + "+" + STR(right) + typestr + ")";
  }
  dffunction_datetime_add_im(const std::shared_ptr<dffunction>& left, int right,
                             datetime_type type, const std::string& as_name) :
    left(left), right(right), type(type), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_datetime_add_im>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1, t2: is not available for datetime_add_im operation!\n");
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  int right;
  datetime_type type;
  std::string as_name;
};

std::shared_ptr<dffunction>
datetime_add_im(const std::string& left, int right, datetime_type type);

std::shared_ptr<dffunction>
datetime_add_im_as(const std::string& left, int right, datetime_type type,
                   const std::string& as);

std::shared_ptr<dffunction>
datetime_add_im(const std::shared_ptr<dffunction>& left, int right,
                datetime_type type);

std::shared_ptr<dffunction>
datetime_add_im_as(const std::shared_ptr<dffunction>& left, int right,
                   datetime_type type, const std::string& as);


// ----- datetime_sub -----
struct dffunction_datetime_sub : public dffunction {
  dffunction_datetime_sub(const std::shared_ptr<dffunction>& left,
                          const std::shared_ptr<dffunction>& right,
                          const datetime_type type)
    : left(left), right(right), type(type) {
    auto typestr = datetime_type_to_string(type);
    as_name = "(" + left->get_as() + "-" + right->get_as() + typestr + ")";
  }
  dffunction_datetime_sub(const std::shared_ptr<dffunction>& left,
                          const std::shared_ptr<dffunction>& right,
                          const datetime_type type,
                          const std::string& as_name) :
    left(left), right(right), type(type), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_datetime_sub>(*this);
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  datetime_type type;
  std::string as_name;
};

std::shared_ptr<dffunction>
datetime_sub_col(const std::string& left, const std::string& right,
                 datetime_type type);

std::shared_ptr<dffunction>
datetime_sub_col(const std::shared_ptr<dffunction>& left,
                 const std::string& right,
                 datetime_type type);

std::shared_ptr<dffunction>
datetime_sub_col(const std::string& left,
                 const std::shared_ptr<dffunction>& right,
                 datetime_type type);

std::shared_ptr<dffunction>
datetime_sub_col(const std::shared_ptr<dffunction>&left,
                 const std::shared_ptr<dffunction>& right,
                 datetime_type type);

std::shared_ptr<dffunction>
datetime_sub_col_as(const std::string& left, const std::string& right,
                    datetime_type type, const std::string& as);

std::shared_ptr<dffunction>
datetime_sub_col_as(const std::shared_ptr<dffunction>& left,
                    const std::string& right,
                    datetime_type type, const std::string& as);

std::shared_ptr<dffunction>
datetime_sub_col_as(const std::string& left,
                    const std::shared_ptr<dffunction>& right,
                    datetime_type type, const std::string& as);

std::shared_ptr<dffunction>
datetime_sub_col_as(const std::shared_ptr<dffunction>&left,
                    const std::shared_ptr<dffunction>& right,
                    datetime_type type, const std::string& as);


// ----- datetime_sub_im -----
struct dffunction_datetime_sub_im : public dffunction {
  dffunction_datetime_sub_im(const std::shared_ptr<dffunction>& left, int right,
                             datetime_type type): 
    left(left), right(right), type(type)  {
    auto typestr = datetime_type_to_string(type);
    as_name = "(" + left->get_as() + "-" + STR(right) + typestr + ")";
  }
  dffunction_datetime_sub_im(const std::shared_ptr<dffunction>& left, int right,
                             datetime_type type, const std::string& as_name) :
    left(left), right(right), type(type), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_datetime_sub_im>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1, t2: is not available for datetime_sub_im operation!\n");
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  int right;
  datetime_type type;
  std::string as_name;
};

std::shared_ptr<dffunction>
datetime_sub_im(const std::string& left, int right, datetime_type type);

std::shared_ptr<dffunction>
datetime_sub_im_as(const std::string& left, int right, datetime_type type,
                   const std::string& as);

std::shared_ptr<dffunction>
datetime_sub_im(const std::shared_ptr<dffunction>& left, int right,
                datetime_type type);

std::shared_ptr<dffunction>
datetime_sub_im_as(const std::shared_ptr<dffunction>& left, int right,
                   datetime_type type, const std::string& as);


// ----- datetime_truncate -----
struct dffunction_datetime_truncate : public dffunction {
  dffunction_datetime_truncate(const std::shared_ptr<dffunction>& left,
                               const datetime_type type)
    : left(left), type(type) {
    auto typestr = datetime_type_to_string(type);
    as_name = typestr + "(" + left->get_as() + ")";
  }
  dffunction_datetime_truncate(const std::shared_ptr<dffunction>& left,
                               const datetime_type type,
                               const std::string& as_name) :
    left(left), type(type), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_datetime_truncate>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1, 
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("datetime_truncate(): is not available for binary operation!\n");
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  datetime_type type;
  std::string as_name;
};

std::shared_ptr<dffunction>
datetime_truncate_col(const std::string& left, datetime_type type);

std::shared_ptr<dffunction>
datetime_truncate_col(const std::shared_ptr<dffunction>& left,
                      datetime_type type);

std::shared_ptr<dffunction>
datetime_truncate_col_as(const std::string& left, datetime_type type,
                         const std::string& as);

std::shared_ptr<dffunction>
datetime_truncate_col_as(const std::shared_ptr<dffunction>& left,
                         datetime_type type, const std::string& as);


// ----- datetime_months_between -----
struct dffunction_datetime_months_between : public dffunction {
  dffunction_datetime_months_between(const std::shared_ptr<dffunction>& left,
                                     const std::shared_ptr<dffunction>& right)
    : left(left), right(right){
    as_name =
      "month_between(" + left->get_as() + "," + right->get_as() + ")";
  }
  dffunction_datetime_months_between(const std::shared_ptr<dffunction>& left,
                                     const std::shared_ptr<dffunction>& right,
                                     const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_datetime_months_between>(*this);
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dffunction>
datetime_months_between_col(const std::string& left, const std::string& right);

std::shared_ptr<dffunction>
datetime_months_between_col(const std::shared_ptr<dffunction>& left,
                            const std::string& right);

std::shared_ptr<dffunction>
datetime_months_between_col(const std::string& left,
                            const std::shared_ptr<dffunction>& right);

std::shared_ptr<dffunction>
datetime_months_between_col(const std::shared_ptr<dffunction>&left,
                            const std::shared_ptr<dffunction>& right);

std::shared_ptr<dffunction>
datetime_months_between_col_as(const std::string& left, const std::string& right,
                               const std::string& as);

std::shared_ptr<dffunction>
datetime_months_between_col_as(const std::shared_ptr<dffunction>& left,
                               const std::string& right, const std::string& as);

std::shared_ptr<dffunction>
datetime_months_between_col_as(const std::string& left,
                               const std::shared_ptr<dffunction>& right,
                               const std::string& as);

std::shared_ptr<dffunction>
datetime_months_between_col_as(const std::shared_ptr<dffunction>&left,
                               const std::shared_ptr<dffunction>& right,
                               const std::string& as);


// ----- datetime_next_day -----
struct dffunction_datetime_next_day : public dffunction {
  dffunction_datetime_next_day(const std::shared_ptr<dffunction>& left,
                               const std::shared_ptr<dffunction>& right)
    : left(left), right(right) {
    as_name = "next_day(" + left->get_as() + "," + right->get_as() + ")";
  }
  dffunction_datetime_next_day(const std::shared_ptr<dffunction>& left,
                               const std::shared_ptr<dffunction>& right,
                               const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_datetime_next_day>(*this);
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dffunction>
datetime_next_day_col(const std::string& left, const std::string& right);

std::shared_ptr<dffunction>
datetime_next_day_col(const std::shared_ptr<dffunction>& left,
                      const std::string& right);

std::shared_ptr<dffunction>
datetime_next_day_col(const std::string& left,
                      const std::shared_ptr<dffunction>& right);

std::shared_ptr<dffunction>
datetime_next_day_col(const std::shared_ptr<dffunction>&left,
                      const std::shared_ptr<dffunction>& right);

std::shared_ptr<dffunction>
datetime_next_day_col_as(const std::string& left, const std::string& right,
                         const std::string& as);

std::shared_ptr<dffunction>
datetime_next_day_col_as(const std::shared_ptr<dffunction>& left,
                         const std::string& right, const std::string& as);

std::shared_ptr<dffunction>
datetime_next_day_col_as(const std::string& left,
                         const std::shared_ptr<dffunction>& right,
                         const std::string& as);

std::shared_ptr<dffunction>
datetime_next_day_col_as(const std::shared_ptr<dffunction>&left,
                         const std::shared_ptr<dffunction>& right,
                         const std::string& as);


// ----- datetime_next_day_im -----
struct dffunction_datetime_next_day_im : public dffunction {
  dffunction_datetime_next_day_im(const std::shared_ptr<dffunction>& left,
                                  int right) :
    left(left), right(right) {
    as_name = "next_day(" + left->get_as() + "," + STR(right) + ")";
  }
  dffunction_datetime_next_day_im(const std::shared_ptr<dffunction>& left,
                                  int right, const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_datetime_next_day_im>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1, t2: is not available for datetime_next_day_im operation!\n");
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  int right;
  std::string as_name;
};

std::shared_ptr<dffunction>
datetime_next_day_im(const std::string& left, int right);

std::shared_ptr<dffunction>
datetime_next_day_im_as(const std::string& left, int right,
                        const std::string& as);

std::shared_ptr<dffunction>
datetime_next_day_im(const std::shared_ptr<dffunction>& left, int right);

std::shared_ptr<dffunction>
datetime_next_day_im_as(const std::shared_ptr<dffunction>& left, int right,
                        const std::string& as);

// ----- when -----
struct dffunction_when : public dffunction {
  dffunction_when(const std::vector<std::shared_ptr<dfoperator>>& cond,
                  const std::vector<std::shared_ptr<dffunction>>& func):
    cond(cond), func(func) {
    as_name = "when";
  }
  dffunction_when(const std::vector<std::shared_ptr<dfoperator>>& cond,
                  const std::vector<std::shared_ptr<dffunction>>& func,
                  const std::string& as_name) :
    cond(cond), func(func) , as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_when>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1, t2: is not available for dffunction_when operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    std::vector<std::shared_ptr<dfcolumn>> touse;
    for(size_t i = 0; i < cond.size(); i++) {
      auto crnt_touse = cond[i]->columns_to_use(t);
      touse.insert(touse.end(), crnt_touse.begin(), crnt_touse.end());
    }
    for(size_t i = 0; i < func.size(); i++) {
      auto crnt_touse = func[i]->columns_to_use(t);
      touse.insert(touse.end(), crnt_touse.begin(), crnt_touse.end());
    }
    return touse;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string>
  used_col_names() const {
    std::vector<std::string> touse;
    for(size_t i = 0; i < cond.size(); i++) {
      auto crnt_touse = cond[i]->used_col_names();
      touse.insert(touse.end(), crnt_touse.begin(), crnt_touse.end());
    }
    for(size_t i = 0; i < func.size(); i++) {
      auto crnt_touse = func[i]->used_col_names();
      touse.insert(touse.end(), crnt_touse.begin(), crnt_touse.end());
    }
    return touse;
  }
  
  std::vector<std::shared_ptr<dfoperator>> cond;
  std::vector<std::shared_ptr<dffunction>> func;
  std::string as_name;
};

std::shared_ptr<dffunction> 
when(const std::vector<std::shared_ptr<dfoperator>>& cond,
     const std::vector<std::shared_ptr<dffunction>>& func);

std::shared_ptr<dffunction> 
when(const std::vector<std::pair<std::shared_ptr<dfoperator>, 
     std::shared_ptr<dffunction>>>& cond_func_pairs);

std::shared_ptr<dffunction> 
when(const std::vector<std::pair<std::shared_ptr<dfoperator>, 
     std::shared_ptr<dffunction>>>& cond_func_pairs,
     const std::shared_ptr<dffunction>& else_func);


// ----- im -----
template <class T>
struct dffunction_im : public dffunction {
  dffunction_im(T value) : value(value) {
    as_name = "im(" + STR(value) + ")";
  }
  dffunction_im(T value, const std::string& as_name) :
    value(value), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_im<T>>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1, t2: is not available for dffunction_im operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return std::vector<std::shared_ptr<dfcolumn>>();
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    return std::vector<std::string>();
  }
  
  T value;
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn> dffunction_im<T>::execute(dftable_base& t) const {
  auto num_rows = make_node_local_scatter(t.num_rows());
  auto nlval = num_rows.map(+[](size_t num_row, T value) {
      return std::vector<T>(num_row, value);
    }, broadcast(value));
  auto dvval = nlval.template moveto_dvector<T>();
  return std::make_shared<typed_dfcolumn<T>>(std::move(dvval));
}

template <class T>
std::shared_ptr<dffunction>
im(T value) {
  return std::make_shared<dffunction_im<T>>(value);
}

template <class T>
std::shared_ptr<dffunction>
im_as(T value, const std::string& as) {
  return std::make_shared<dffunction_im<T>>(value, as);
}

// ----- datetime_im -----
struct dffunction_datetime_im : public dffunction {
  dffunction_datetime_im(datetime_t value) : value(value) {
    as_name = "datetime_im(" + STR(value) + ")";
  }
  dffunction_datetime_im(datetime_t value, const std::string& as_name) :
    value(value), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_datetime_im>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1, t2: is not available for dffunction_datetime_im operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return std::vector<std::shared_ptr<dfcolumn>>();
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    return std::vector<std::string>();
  }
  
  datetime_t value;
  std::string as_name;
};

std::shared_ptr<dffunction>
datetime_im(datetime_t value);

std::shared_ptr<dffunction>
datetime_im_as(datetime_t value, const std::string& as);


// ----- dic_string_im -----
struct dffunction_dic_string_im : public dffunction {
  dffunction_dic_string_im(const std::string& value) : value(value) {
    as_name = "dic_string_im(" + STR(value) + ")";
  }
  dffunction_dic_string_im(const std::string& value,
                           const std::string& as_name) :
    value(value), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_dic_string_im>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1, t2: is not available for dffunction_datetime_im operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return std::vector<std::shared_ptr<dfcolumn>>();
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    return std::vector<std::string>();
  }
  
  std::string value;
  std::string as_name;
};

std::shared_ptr<dffunction>
dic_string_im(const std::string& value);

std::shared_ptr<dffunction>
dic_string_im_as(const std::string& value, const std::string& as);


// ----- null_column -----
template <class T>
struct dffunction_null_column : public dffunction {
  dffunction_null_column() { 
    as_name = "null_column";
  }
  dffunction_null_column(const std::string& as_name) :
    as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_null_column<T>>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1, t2: is not available for dffunction_null_column operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return std::vector<std::shared_ptr<dfcolumn>>();
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    return std::vector<std::string>();
  }
  
  std::string as_name;
};

template <class T>
std::shared_ptr<dfcolumn>
dffunction_null_column<T>::execute(dftable_base& t) const {
  return create_null_column<T>(t.num_rows());
}

template <class T>
std::shared_ptr<dffunction>
null_column() {
  return std::make_shared<dffunction_null_column<T>>();
}

template <class T>
std::shared_ptr<dffunction>
null_column_as(const std::string& as) {
  return std::make_shared<dffunction_null_column<T>>(as);
}


// ----- null_string_column -----
struct dffunction_null_string_column : public dffunction {
  dffunction_null_string_column() { 
    as_name = "null_string_column";
  }
  dffunction_null_string_column(const std::string& as_name) :
    as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_null_string_column>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1, t2: is not available for dffunction_null_string_column operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return std::vector<std::shared_ptr<dfcolumn>>();
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    return std::vector<std::string>();
  }
  
  std::string as_name;
};

std::shared_ptr<dffunction>
null_string_column();

std::shared_ptr<dffunction>
null_string_column_as(const std::string& as);

// ----- null_dic_string_column -----
struct dffunction_null_dic_string_column : public dffunction {
  dffunction_null_dic_string_column() { 
    as_name = "null_dic_string_column";
  }
  dffunction_null_dic_string_column(const std::string& as_name) :
    as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_null_dic_string_column>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1, t2: is not available for dffunction_null_dic_string_column operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return std::vector<std::shared_ptr<dfcolumn>>();
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    return std::vector<std::string>();
  }
  
  std::string as_name;
};

std::shared_ptr<dffunction>
null_dic_string_column();

std::shared_ptr<dffunction>
null_dic_string_column_as(const std::string& as);


// ----- null_datetime_column -----
struct dffunction_null_datetime_column : public dffunction {
  dffunction_null_datetime_column() { 
    as_name = "null_datetime_column";
  }
  dffunction_null_datetime_column(const std::string& as_name) :
    as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_null_datetime_column>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1, t2: is not available for dffunction_null_datetime_column operation!\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return std::vector<std::shared_ptr<dfcolumn>>();
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("two args of columns_to_use on this operator is not implemented");
  }
  virtual std::vector<std::string> used_col_names() const {
    return std::vector<std::string>();
  }
  
  std::string as_name;
};

std::shared_ptr<dffunction>
null_datetime_column();

std::shared_ptr<dffunction>
null_datetime_column_as(const std::string& as);


// ----- substr -----
// right: pos
struct dffunction_substr : public dffunction {
  dffunction_substr(const std::shared_ptr<dffunction>& left, 
                    const std::shared_ptr<dffunction>& right) :
    left(left), right(right) {
    as_name = "substr(" + left->get_as() + "," + right->get_as() + ")";
  }
  dffunction_substr(const std::shared_ptr<dffunction>& left, 
                    const std::shared_ptr<dffunction>& right, 
                    const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}

  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_substr>(*this);
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  std::string as_name;
};

std::shared_ptr<dffunction>
substr_col(const std::string& left, const std::string& right); 
std::shared_ptr<dffunction>
substr_col(const std::shared_ptr<dffunction>& left, const std::string& right); 
std::shared_ptr<dffunction>
substr_col(const std::string& left, const std::shared_ptr<dffunction>& right); 
std::shared_ptr<dffunction>
substr_col(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right); 

std::shared_ptr<dffunction>
substr_col_as(const std::string& left, const std::string& right,
              const std::string& as);
std::shared_ptr<dffunction>
substr_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
              const std::string& as);
std::shared_ptr<dffunction>
substr_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
              const std::string& as);
std::shared_ptr<dffunction>
substr_col_as(const std::shared_ptr<dffunction>& left,
              const std::shared_ptr<dffunction>& right,
              const std::string& as);

struct dffunction_substr_im : public dffunction {
  dffunction_substr_im(const std::shared_ptr<dffunction>& left, int right): 
    left(left), right(right) {
    as_name = "substr(" + left->get_as() + "," + STR(right) + ")";
  }
  dffunction_substr_im(const std::shared_ptr<dffunction>& left, int right,
                       const std::string& as_name) :
    left(left), right(right), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_substr_im>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("t1 + t2: is not available for substr_im operation!\n");
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  int right;
  std::string as_name;
};

std::shared_ptr<dffunction>
substr_im(const std::string& left, int right);

std::shared_ptr<dffunction>
substr_im_as(const std::string& left, int right, const std::string& as);

std::shared_ptr<dffunction>
substr_im(const std::shared_ptr<dffunction>& left, int right);

std::shared_ptr<dffunction>
substr_im_as(const std::shared_ptr<dffunction>& left, int right,
             const std::string& as);

struct dffunction_substr_num : public dffunction {
  dffunction_substr_num(const std::shared_ptr<dffunction>& left, 
                        const std::shared_ptr<dffunction>& right,
                        const std::shared_ptr<dffunction>& num) :
    left(left), right(right), num(num) {
    as_name = "substr_num(" + left->get_as() + "," + right->get_as() +
      "," + num->get_as() + ")";
  }
  dffunction_substr_num(const std::shared_ptr<dffunction>& left, 
                        const std::shared_ptr<dffunction>& right, 
                        const std::shared_ptr<dffunction>& num,
                        const std::string& as_name) :
    left(left), right(right), num(num), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_substr_num>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("execute of substr_num is not defined for two argument\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    auto leftuse = left->columns_to_use(t);
    auto rightuse = right->columns_to_use(t);
    auto numuse = num->columns_to_use(t);
    leftuse.insert(leftuse.end(), rightuse.begin(), rightuse.end());
    leftuse.insert(leftuse.end(), numuse.begin(), numuse.end());
    return leftuse;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("execute of substr_num is not defined for two argument\n");
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    auto numnames = num->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    leftnames.insert(leftnames.end(), numnames.begin(), numnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right, num;
  std::string as_name;
};

std::shared_ptr<dffunction>
substr_poscol_numcol(const std::string& left,
                     const std::string& right,
                     const std::string& num); 
std::shared_ptr<dffunction>
substr_poscol_numcol(const std::shared_ptr<dffunction>& left,
                     const std::shared_ptr<dffunction>& right,
                     const std::shared_ptr<dffunction>& num); 

std::shared_ptr<dffunction>
substr_poscol_numcol_as(const std::string& left,
                        const std::string& right,
                        const std::string& num,
                        const std::string& as);
std::shared_ptr<dffunction>
substr_poscol_numcol_as(const std::shared_ptr<dffunction>& left,
                        const std::shared_ptr<dffunction>& right,
                        const std::shared_ptr<dffunction>& num,
                        const std::string& as);

struct dffunction_substr_posim_num : public dffunction {
  dffunction_substr_posim_num(const std::shared_ptr<dffunction>& left,
                              int right, 
                              const std::shared_ptr<dffunction>& num): 
    left(left), right(right), num(num) {
    as_name = "substr_posim_num(" + left->get_as() + "," + STR(right) +
      "," + num->get_as() + ")";
  }
  dffunction_substr_posim_num(const std::shared_ptr<dffunction>& left,
                              int right,
                              const std::shared_ptr<dffunction>& num,
                              const std::string& as_name) :
    left(left), right(right), num(num), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_substr_posim_num>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const;
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    auto leftuse = left->columns_to_use(t);
    auto numuse = num->columns_to_use(t);
    leftuse.insert(leftuse.end(), numuse.begin(), numuse.end());
    return leftuse;
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    auto leftuse = left->columns_to_use(t1);
    auto numuse = num->columns_to_use(t2);
    leftuse.insert(leftuse.end(), numuse.begin(), numuse.end());
    return leftuse;
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto numnames = num->used_col_names();
    leftnames.insert(leftnames.end(), numnames.begin(), numnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  int right;
  std::shared_ptr<dffunction> num;
  std::string as_name;
};

std::shared_ptr<dffunction>
substr_posim_numcol(const std::string& left, int right, const std::string& num);

std::shared_ptr<dffunction>
substr_posim_numcol_as(const std::string& left, int right,
                       const std::string& num, const std::string& as);

std::shared_ptr<dffunction>
substr_posim_numcol(const std::shared_ptr<dffunction>& left, int right,
                    const std::shared_ptr<dffunction>& num);

std::shared_ptr<dffunction>
substr_posim_numcol_as(const std::shared_ptr<dffunction>& left, int right,
                       const std::shared_ptr<dffunction>& num,
                       const std::string& as);

struct dffunction_substr_numim : public dffunction {
  dffunction_substr_numim(const std::shared_ptr<dffunction>& left,
                          const std::shared_ptr<dffunction>& right,
                          int num) :
    left(left), right(right), num(num) {
    as_name = "substr_numim(" + left->get_as() + "," + right->get_as() +
      "," + STR(num) + ")";
  }
  dffunction_substr_numim(const std::shared_ptr<dffunction>& left,
                          const std::shared_ptr<dffunction>& right,
                          int num,
                          const std::string& as_name) :
    left(left), right(right), num(num), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_substr_numim>(*this);
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
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    auto rightnames = right->used_col_names();
    leftnames.insert(leftnames.end(), rightnames.begin(), rightnames.end());
    return leftnames;
  }

  std::shared_ptr<dffunction> left, right;
  int num;
  std::string as_name;
};

std::shared_ptr<dffunction>
substr_poscol_numim(const std::string& left, const std::string& right,
                    int num);

std::shared_ptr<dffunction>
substr_poscol_numim_as(const std::string& left, const std::string& right,
                       int num, const std::string& as);

std::shared_ptr<dffunction>
substr_poscol_numim(const std::shared_ptr<dffunction>& left, 
                    const std::shared_ptr<dffunction>& right,
                    int num);


std::shared_ptr<dffunction>
substr_poscol_numim_as(const std::shared_ptr<dffunction>& left, 
                       const std::shared_ptr<dffunction>& right,
                       int num,
                       const std::string& as);

struct dffunction_substr_posim_numim : public dffunction {
  dffunction_substr_posim_numim(const std::shared_ptr<dffunction>& left,
                                int right, int num) :
    left(left), right(right), num(num) {
    as_name = "substr_posim_numim(" + left->get_as() + "," + STR(right) +
      "," + STR(num) + ")";
  }
  dffunction_substr_posim_numim(const std::shared_ptr<dffunction>& left,
                                int right, int num, 
                                const std::string& as_name) :
    left(left), right(right), num(num), as_name(as_name) {}
  virtual std::string get_as() {return as_name;}
  virtual std::shared_ptr<dffunction> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dffunction_substr_posim_numim>(*this);
  }
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t) const;
  virtual std::shared_ptr<dfcolumn> execute(dftable_base& t1,
                                            dftable_base& t2) const {
    throw std::runtime_error
      ("execute of substr_posim_numim is not defined for two argument\n");
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t) {
    return left->columns_to_use(t);
  }
  virtual std::vector<std::shared_ptr<dfcolumn>>
  columns_to_use(dftable_base& t1, dftable_base& t2) {
    throw std::runtime_error
      ("execute of substr_posim_numim is not defined for two argument\n");
  }
  virtual std::vector<std::string> used_col_names() const {
    auto leftnames = left->used_col_names();
    return leftnames;
  }

  std::shared_ptr<dffunction> left;
  int right, num;
  std::string as_name;
};

std::shared_ptr<dffunction>
substr_posim_numim(const std::string& left, int right, int num);

std::shared_ptr<dffunction>
substr_posim_numim_as(const std::string& left, int right,
                      int num, const std::string& as);

std::shared_ptr<dffunction>
substr_posim_numim(const std::shared_ptr<dffunction>& left,
                   int right, int num);

std::shared_ptr<dffunction>
substr_posim_numim_as(const std::shared_ptr<dffunction>& left,
                      int right, int num, const std::string& as);

// ----- utility functions for user's direct use -----

// alias of id_col
std::shared_ptr<dffunction> col(const std::string& col);

// create id_col using operator~
std::shared_ptr<dffunction> operator~(const std::string& col);

std::shared_ptr<dffunction> operator+(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b);

template <class T>
std::shared_ptr<dffunction> operator+(const std::shared_ptr<dffunction>& a,
                                      T b) {
  return add_im(a,b);
}

template <class T>
std::shared_ptr<dffunction> operator+(T b,
                                      const std::shared_ptr<dffunction>& a) {
                                      
  return add_im(a,b);
}

std::shared_ptr<dffunction> operator-(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b);

template <class T>
std::shared_ptr<dffunction> operator-(const std::shared_ptr<dffunction>& a,
                                      T b) {
  return sub_im(a,b);
}

template <class T>
std::shared_ptr<dffunction> operator-(T b,
                                      const std::shared_ptr<dffunction>& a) {
                                      
  return sub_im(b,a);
}

std::shared_ptr<dffunction> operator*(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b);

template <class T>
std::shared_ptr<dffunction> operator*(const std::shared_ptr<dffunction>& a,
                                      T b) {
  return mul_im(a,b);
}

template <class T>
std::shared_ptr<dffunction> operator*(T b,
                                      const std::shared_ptr<dffunction>& a) {
                                      
  return mul_im(a,b);
}

std::shared_ptr<dffunction> operator/(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b);

template <class T>
std::shared_ptr<dffunction> operator/(const std::shared_ptr<dffunction>& a,
                                      T b) {
  return fdiv_im(a,b);
}

template <class T>
std::shared_ptr<dffunction> operator/(T b,
                                      const std::shared_ptr<dffunction>& a) {
                                      
  return fdiv_im(b,a);
}

std::shared_ptr<dffunction> operator%(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b);

template <class T>
std::shared_ptr<dffunction> operator%(const std::shared_ptr<dffunction>& a,
                                      T b) {
  return mod_im(a,b);
}

template <class T>
std::shared_ptr<dffunction> operator%(T b,
                                      const std::shared_ptr<dffunction>& a) {
                                      
  return mod_im(b,a);
}

std::pair<std::shared_ptr<dfoperator>, std::shared_ptr<dffunction>>
operator>>(const std::shared_ptr<dfoperator>& cond,
           const std::shared_ptr<dffunction>& func);

}
#endif
