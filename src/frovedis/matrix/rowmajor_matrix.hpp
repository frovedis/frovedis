#ifndef ROWMAJOR_MATRIX_HPP
#define ROWMAJOR_MATRIX_HPP

#include <fstream>
#include <dirent.h>
#include <cmath>
#include <sys/stat.h>
#include <sys/types.h>

#include "../core/dvector.hpp"
#include "../core/mpihelper.hpp"
#include "../core/partition_sort.hpp"
#include "../core/vector_operations.hpp"
#include "diag_matrix.hpp"
#ifdef __ve__
#include "../text/load_csv.hpp"
#include "../text/parseint.hpp"
#include "../text/parsefloat.hpp"
#include "../text/int_to_words.hpp"
#include "../text/float_to_words.hpp"
#include "../core/utility.hpp"
#endif

#define MAT_VLEN 256

namespace frovedis {

template <class T, class I, class O>
struct crs_matrix_local;

template <class T, class I, class O>
struct crs_matrix;

template <class T>
struct rowmajor_matrix_local {
  rowmajor_matrix_local(){}
  rowmajor_matrix_local(size_t r, size_t c) :
    local_num_row(r), local_num_col(c) {
    val.resize(r*c);
  }
  rowmajor_matrix_local(size_t r, size_t c,
                        T* raw_data) :
    local_num_row(r), local_num_col(c) {
    val.resize(r*c);
    T* valp = &val[0];
    for(size_t i=0; i<val.size(); ++i) valp[i] = raw_data[i];
  }
  // move operations are defined because SX C++ compiler's STL
  // does not know move currently
  rowmajor_matrix_local(rowmajor_matrix_local<T>&& m) {
    val.swap(m.val);
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
  }
  rowmajor_matrix_local<T>& operator=(rowmajor_matrix_local<T>&& m) {
    val.swap(m.val);
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
    return *this;
  }
  // copy ctor is explicitly defined because SX C++ compiler
  // produce very inefficient code otherwise...
  rowmajor_matrix_local(const rowmajor_matrix_local<T>& m) {
    val = m.val;
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
  }
  rowmajor_matrix_local<T>&
  operator=(const rowmajor_matrix_local<T>& m) {
    val = m.val;
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
    return *this;
  }
  // implicit conversion: (lvalue) vector -> rowmajor_matrix_local
  rowmajor_matrix_local(const std::vector<T>& vec) {
    set_local_num(vec.size(),1);
    val = vec; // copying since (lvalue)
  }
  // implicit conversion: (rvalue) vector -> rowmajor_matrix_local
  rowmajor_matrix_local(std::vector<T>&& vec) {
    set_local_num(vec.size(),1); // this setting must be done before the below swap
    val.swap(vec); // swapping since (rvalue)
  }
  void set_local_num(size_t r, size_t c) {
    local_num_row = r; local_num_col = c;
  }
  template <class I = size_t, class O = size_t>
  crs_matrix_local<T,I,O> to_crs();
  void debug_print(size_t limit = 0);
  void debug_pretty_print(size_t limit = 0);
  std::vector<T> get_row(size_t r) const;
  void save(const std::string& file);
  void clear() {
    std::vector<T> tmpval; tmpval.swap(val);
    local_num_row = 0;
    local_num_col = 0;
  }
  void savebinary(const std::string&);
  template <class U>
  rowmajor_matrix_local<U> astype();    
  rowmajor_matrix_local<T> transpose() const;
  rowmajor_matrix_local<T> pow_val(T exponent) const;
  node_local<rowmajor_matrix_local<T>> broadcast(); // for performance
  std::vector<T> val;
  size_t local_num_row;
  size_t local_num_col;

  SERIALIZE(val, local_num_row, local_num_col)
  typedef T value_type;
};

template <class T>
template <class U>
rowmajor_matrix_local<U> rowmajor_matrix_local<T>::astype() {
  rowmajor_matrix_local<U> ret;
  ret.val = vector_astype<U>(val);
  ret.set_local_num(local_num_row, local_num_col);  
  return ret;    
}    

template <class T>
void get_row_impl(const T* inp_buf,
                  size_t buf_size,
                  T* out_buf,
                  size_t out_stride) {
  for(size_t i = 0; i < buf_size; ++i) out_buf[i * out_stride] = inp_buf[i];
}

template <class T>
std::vector<T>
rowmajor_matrix_local<T>::get_row(size_t k) const {
  if (k >= local_num_row) throw std::runtime_error("get_row: invalid position");
  auto inp_buf = val.data() + local_num_col * k;
  std::vector<T> r(local_num_col); auto out_buf = r.data();
  get_row_impl(inp_buf, local_num_col, out_buf, 1);
  return r;
}

template <class T>
rowmajor_matrix_local<T>
extract_rows(rowmajor_matrix_local<T>& mat,
             const std::vector<size_t>& rowids,
             bool need_transpose = false) {
  auto n = rowids.size();
  auto rowidp = rowids.data();
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  size_t count_invalid = 0;
  for (size_t i = 0; i < n; ++i) count_invalid += (rowidp[i] >= nrow) ;
  if (count_invalid) throw std::runtime_error("extract_rows: invalid position");

  std::vector<T> out(n * ncol);
  auto rvalp = out.data();
  auto mvalp = mat.val.data();
  rowmajor_matrix_local<T> ret;

  if (!need_transpose) {
    for (size_t i = 0; i < n; ++i) {
      auto inp_buf = mvalp + rowidp[i] * ncol;
      auto out_buf = rvalp + i * ncol;
      get_row_impl(inp_buf, ncol, out_buf, 1);
    }
    ret.val.swap(out);
    ret.set_local_num(n, ncol);
  }
  else {
    for (size_t i = 0; i < n; ++i) {
      auto inp_buf = mvalp + rowidp[i] * ncol;
      auto out_buf = rvalp + i;
      get_row_impl(inp_buf, ncol, out_buf, n);
    }
    ret.val.swap(out);
    ret.set_local_num(ncol, n);
  }
  return ret;
}

template <class T>
struct rowmajor_matrix_broadcast_helper {
  rowmajor_matrix_broadcast_helper() {}
  rowmajor_matrix_broadcast_helper(size_t local_num_row,
                                   size_t local_num_col) :
    local_num_row(local_num_row), local_num_col(local_num_col) {}
  rowmajor_matrix_local<T> operator()(std::vector<T>& v) {
    rowmajor_matrix_local<T> ret;
    ret.val.swap(v);
    ret.local_num_row = local_num_row;
    ret.local_num_col = local_num_col;
    return ret;
  }
  size_t local_num_row;
  size_t local_num_col;

  SERIALIZE(local_num_row, local_num_col)
};

/*
  skip serialization by broadcasting vector directly
 */
template <class T>
node_local<rowmajor_matrix_local<T>> rowmajor_matrix_local<T>::broadcast() {
  auto bval = make_node_local_broadcast(val);
  return bval.template map<rowmajor_matrix_local<T>>
    (rowmajor_matrix_broadcast_helper<T>(local_num_row, local_num_col));
}

#ifdef __ve__
template <class T>
void rowmajor_matrix_local<T>::save(const std::string& file) {
  std::vector<size_t> new_starts;
  auto intvec = concat_words(number_to_words(val), " ", new_starts);
  auto intvecp = intvec.data();
  auto new_startsp = new_starts.data();
#pragma _NEC ivdep
  for(size_t i = 1; i < local_num_row; i++) {
    intvecp[new_startsp[i*local_num_col]-1] = '\n';
  }
  if(intvec.size() > 0) intvecp[intvec.size()-1] = '\n';
  auto str = int_to_char(intvec);
  savebinary_local(str.data(), str.size(), file);
}
#else
template <class T>
void rowmajor_matrix_local<T>::save(const std::string& file) {
  std::ofstream str(file.c_str());
  str << *this;
}
#endif

template <class T>
std::ostream& operator<<(std::ostream& str,
                         const rowmajor_matrix_local<T>& mat) {
  for(size_t r = 0; r < mat.local_num_row; r++) {
    size_t c;
    for(c = 0; c < mat.local_num_col - 1; c++) {
      str << mat.val[mat.local_num_col * r + c] << " ";
    }
    str << mat.val[mat.local_num_col * r + c] << "\n";
  }
  return str;
}

template <class T>
void rowmajor_matrix_local<T>::debug_print(size_t limit) {
  std::cout << "node = " << get_selfid()
            << ", local_num_row = " << local_num_row
            << ", local_num_col = " << local_num_col
            << ", val = ";
  debug_print_vector(val, limit);
}

template <class T>
void rowmajor_matrix_local<T>::debug_pretty_print(size_t limit) {
  auto nrow = local_num_row;
  auto ncol = local_num_col;
  if(limit == 0 || nrow <= 2 * limit) {
    for(size_t i = 0; i < nrow; ++i) {
      for(size_t j = 0; j < ncol; ++j) {
        std::cout << val[i * ncol + j] << " ";
      }
      std::cout << std::endl;
    }
  }
  else {
    for(size_t i = 0; i < limit; ++i) {
      for(size_t j = 0; j < ncol; ++j) {
        std::cout << val[i * ncol + j] << " ";
      }
      std::cout << std::endl;
    }
    std::cout << ":\n:\n";
    for(size_t i = nrow - limit; i < nrow; ++i) {
      for(size_t j = 0; j < ncol; ++j) {
        std::cout << val[i * ncol + j] << " ";
      }
      std::cout << std::endl;
    }
  }
}

template <class T>
void make_rowmajor_matrix_local_parseline(std::string&, std::vector<T>&,
                                          size_t&);
template <>
void make_rowmajor_matrix_local_parseline(std::string&, std::vector<double>&,
                                          size_t&);
template <>
void make_rowmajor_matrix_local_parseline(std::string&, std::vector<float>&,
                                          size_t&);
template <>
void make_rowmajor_matrix_local_parseline(std::string&, std::vector<int>&,
                                          size_t&);

template <class T>
rowmajor_matrix_local<T>
make_rowmajor_matrix_local_readstream(std::istream& str) {
  rowmajor_matrix_local<T> ret;
  std::string line;
  size_t width = 0;
  size_t lines = 0;
  while(std::getline(str,line)) {
    // width should be initializes as 0; otherwise not changed
    make_rowmajor_matrix_local_parseline<T>(line, ret.val, width);
    lines++;
  }
  ret.local_num_row = lines;
  ret.local_num_col = width;
  return ret;
}

template <class T>
std::istream& operator>>(std::istream& str,
                         rowmajor_matrix_local<T>& mat) {
  mat = make_rowmajor_matrix_local_readstream<T>(str);
  return str;
}

#ifdef __ve__
template <class T>
rowmajor_matrix_local<T>
make_rowmajor_matrix_local_load(const std::string& file) {
  std::vector<size_t> line_starts_byword;
  auto ws = load_simple_csv_local(file, line_starts_byword, false, false, ' ');
  rowmajor_matrix_local<T> ret;
  ret.val = parsenumber<T>(ws);
  if(line_starts_byword.size() > 1) { // 1st item is always 0
    ret.local_num_col = line_starts_byword[1];
  } else {
    ret.local_num_col = ret.val.size();
  }
  ret.local_num_row = line_starts_byword.size();
  return ret;
}
#else
template <class T>
rowmajor_matrix_local<T>
make_rowmajor_matrix_local_load(const std::string& file) {
  std::ifstream str(file.c_str());
  return make_rowmajor_matrix_local_readstream<T>(str);
}
#endif

// used for making (distributed) rowmajor_matrix
template <class T>
rowmajor_matrix_local<T>
make_rowmajor_matrix_local_vectorstring(std::vector<std::string>& vs) {
  rowmajor_matrix_local<T> ret;
  size_t width = 0;
  for(size_t i = 0; i < vs.size(); i++) {
    make_rowmajor_matrix_local_parseline(vs[i], ret.val, width);
  }
  ret.local_num_row = vs.size();
  ret.local_num_col = width;
  return ret;
}

/*
template <class T>
rowmajor_matrix_local<T> operator*(const rowmajor_matrix_local<T>& a,
                                   const rowmajor_matrix_local<T>& b) {
  if(a.local_num_col != b.local_num_row)
    throw std::runtime_error("invalid size for matrix multiplication");
  size_t imax = a.local_num_row;
  size_t jmax = b.local_num_col;
  size_t kmax = a.local_num_col; // == b.local_num_row
  rowmajor_matrix_local<T> c(imax, jmax);
  const T* ap = &a.val[0];
  const T* bp = &b.val[0];
  T* cp = &c.val[0];
  // let the SX compiler detect matmul
  for(size_t i = 0; i < imax; i++) {
    for(size_t j = 0; j < jmax; j++) {
      for(size_t k = 0; k < kmax; k++) {
        //cp[i][j] += ap[i][k] * bp[k][j];
        cp[i * jmax + j] += ap[i * kmax + k] * bp[k * jmax + j];
      }
    }
  }
  return c;
}
*/

template <class T>
rowmajor_matrix_local<T> operator*(const rowmajor_matrix_local<T>& a,
                                   const diag_matrix_local<T>& b) {
  if(a.local_num_col != b.local_num())
    throw std::runtime_error("invalid size for matrix multiplication");
  size_t imax = a.local_num_row;
  size_t jmax = a.local_num_col; // == b.local_num_row
  rowmajor_matrix_local<T> c(imax, jmax);
  const T* ap = &a.val[0];
  const T* bp = &b.val[0];
  T* cp = &c.val[0];
#if defined(_SX) || defined(__ve__)
  if(imax > jmax) {
    for(size_t i = 0; i<imax; i += MAT_VLEN) {
      auto range = (i + MAT_VLEN <= imax) ? (i + MAT_VLEN) : imax;
      for(size_t j = 0; j<jmax; ++j) {
        for (size_t k = i; k<range; ++k) {
          cp[k * jmax + j] = ap[k * jmax + j] * bp[j];
        }
      }
    }
  }
  else {
    for (size_t i = 0; i<imax; ++i) {
      for(size_t j = 0; j<jmax; ++j) {
        cp[i * jmax + j] = ap[i * jmax + j] * bp[j];
      }
    }
  }
#else
  for (size_t i = 0; i<imax; ++i) {
    for(size_t j = 0; j<jmax; ++j) {
      cp[i * jmax + j] = ap[i * jmax + j] * bp[j];
    }
  }
#endif
  return c;
}

template <class T>
rowmajor_matrix_local<T> operator*(const diag_matrix_local<T>& a,
                                   const rowmajor_matrix_local<T>& b) {
  if(a.local_num() != b.local_num_row)
    throw std::runtime_error("invalid size for matrix multiplication");
  size_t imax = b.local_num_row;
  size_t jmax = b.local_num_col; // == b.local_num_row
  rowmajor_matrix_local<T> c(imax, jmax);
  const T* ap = a.val.data();
  const T* bp = b.val.data();
  T* cp = c.val.data();
#if defined(_SX) || defined(__ve__)
  if (imax > jmax) {
    for(size_t i = 0; i<imax; i += MAT_VLEN) {
      auto range = (i + MAT_VLEN <= imax) ? (i + MAT_VLEN) : imax;
      for(size_t j = 0; j<jmax; ++j) {
        for (size_t k = i; k<range; ++k) {
          cp[k * jmax + j] = ap[k] * bp[k * jmax + j];
        }
      }
    }
  }
  else {
    for (size_t i = 0; i<imax; ++i) {
      for(size_t j = 0; j<jmax; ++j) {
        cp[i * jmax + j] = ap[i] * bp[i * jmax + j];
      }
    }
  }
#else
  for (size_t i = 0; i<imax; ++i) {
    for(size_t j = 0; j<jmax; ++j) {
      cp[i * jmax + j] = ap[i] * bp[i * jmax + j];
    }
  }
#endif
  return c;
}

template <class T>
std::vector<T> operator*(const rowmajor_matrix_local<T>& a,
                         const std::vector<T>& b) {
  if(a.local_num_col != b.size())
    throw std::runtime_error("invalid size for matrix vector multiplication");
  size_t imax = a.local_num_row;
  size_t jmax = a.local_num_col; // == b.local_num_row
  std::vector<T> c(imax, 0);
  const T* ap = a.val.data();
  const T* bp = b.data();
  T* cp = c.data();
  for(size_t j = 0; j < jmax; j++) {
    for(size_t i = 0; i < imax; i++) {
        cp[i] += ap[i * jmax + j] * bp[j];
    }
  }
  return c;
}

template <class T>
rowmajor_matrix_local<T> 
binarize(rowmajor_matrix_local<T>& mat,
         const T& threshold = 0) {
  auto v = vector_binarize(mat.val, threshold);
  rowmajor_matrix_local<T> ret;
  ret.val.swap(v);
  ret.local_num_row = mat.local_num_row;
  ret.local_num_col = mat.local_num_col;
  return ret;
}

// if you need to invoke binarize() for local rowmajor matrix from map(),
// invoke this function. Otherwise function pointer resolution issue might occur.
template <class T>
rowmajor_matrix_local<T> 
rmm_binarize(rowmajor_matrix_local<T>& mat, 
             const T& threshold = 0) {
  return binarize(mat, threshold);
}

template <class T>
T sum_of_elements(rowmajor_matrix_local<T>& mat) {
  return vector_sum(mat.val);
}

// if you need to invoke sum_of_elements() for local rowmajor matrix from map(),
// invoke this function. Otherwise function pointer resolution issue might occur.
template <class T>
T rmm_sum_of_elements(rowmajor_matrix_local<T>& mat) {
  return sum_of_elements(mat);
}

template <class T>
T squared_sum_of_elements(rowmajor_matrix_local<T>& mat) {
  return vector_squared_sum(mat.val);
}

// if you need to invoke squared_sum_of_elements() for local rowmajor matrix from map(),
// invoke this function. Otherwise function pointer resolution issue might occur.
template <class T>
T rmm_squared_sum_of_elements(rowmajor_matrix_local<T>& mat) {
  return squared_sum_of_elements(mat);
}

template <class T>
std::vector<T> sum_of_rows(const rowmajor_matrix_local<T>& m) {
  auto nrow = m.local_num_row;
  auto ncol = m.local_num_col;
  std::vector<T> ret(ncol,0);
  T* retp = &ret[0];
  const T* matp = &m.val[0];
  for (size_t i = 0; i < nrow; ++i) {
    for(size_t j = 0; j < ncol; ++j) retp[j] += matp[i * ncol + j];
  }
  return ret;
}

// if you need to invoke sum_of_rows() for local rowmajor matrix from map(),
// invoke this function. Otherwise function pointer resolution issue might occur.
template <class T>
std::vector<T> 
rmm_sum_of_rows(const rowmajor_matrix_local<T>& m) {
  return sum_of_rows(m);
}

template <class T>
std::vector<T> 
squared_sum_of_rows(const rowmajor_matrix_local<T>& m) {
  auto nrow = m.local_num_row;
  auto ncol = m.local_num_col;
  std::vector<T> ret(ncol,0);
  T* retp = &ret[0];
  const T* matp = &m.val[0];
  for (size_t i = 0; i < nrow; ++i) {
    for(size_t j = 0; j < ncol; ++j) {
      retp[j] += (matp[i * ncol + j] * matp[i * ncol + j]);
    }
  }
  return ret;
}

// if you need to invoke squared_sum_of_rows() for local rowmajor matrix from map(),
// invoke this function. Otherwise function pointer resolution issue might occur.
template <class T>
std::vector<T> 
rmm_squared_sum_of_rows(const rowmajor_matrix_local<T>& m) {
  return squared_sum_of_rows(m);
}

template <class T>
std::vector<T> 
sum_of_cols(const rowmajor_matrix_local<T>& m) {
  auto nrow = m.local_num_row;
  auto ncol = m.local_num_col;
  std::vector<T> ret(nrow,0);
  T* retp = &ret[0];
  const T* matp = &m.val[0];
#if defined(_SX) || defined(__ve__)
  if (nrow > ncol) {
    for(size_t i = 0; i<nrow; i += MAT_VLEN) {
      auto range = (i + MAT_VLEN <= nrow) ? (i + MAT_VLEN) : nrow;
      for(size_t j = 0; j<ncol; ++j) {
        for (size_t k=i; k<range; ++k) {
          retp[k] += matp[k * ncol + j];
        }
      }
    }
  }
  else {
    for (size_t i=0; i<nrow; ++i) {
      for(size_t j = 0; j<ncol; ++j) {
        retp[i] += matp[i * ncol + j];
      }
    }
  }
#else
  for (size_t i=0; i<nrow; ++i) {
    for(size_t j = 0; j<ncol; ++j) {
      retp[i] += matp[i * ncol + j];
    }
  }
#endif
  return ret;
}

// if you need to invoke sum_of_cols() for local rowmajor matrix from map(),
// invoke this function. Otherwise function pointer resolution issue might occur.
template <class T>
std::vector<T> 
rmm_sum_of_cols(const rowmajor_matrix_local<T>& m) {
  return sum_of_cols(m);
}

template <class T>
std::vector<T> 
squared_sum_of_cols(const rowmajor_matrix_local<T>& m) {
  auto nrow = m.local_num_row;
  auto ncol = m.local_num_col;
  std::vector<T> ret(nrow,0);
  T* retp = &ret[0];
  const T* matp = &m.val[0];
#if defined(_SX) || defined(__ve__)
  if (nrow > ncol) {
    for(size_t i = 0; i<nrow; i += MAT_VLEN) {
      auto range = (i + MAT_VLEN <= nrow) ? (i + MAT_VLEN) : nrow;
      for(size_t j = 0; j<ncol; ++j) {
        for (size_t k=i; k<range; ++k) {
          retp[k] += (matp[k * ncol + j] * matp[k * ncol + j]);
        }
      }
    }
  }
  else {
    for (size_t i=0; i<nrow; ++i) {
      for(size_t j = 0; j<ncol; ++j) {
        retp[i] += (matp[i * ncol + j] * matp[i * ncol + j]);
      }
    }
  }
#else
  for (size_t i=0; i<nrow; ++i) {
    for(size_t j = 0; j<ncol; ++j) {
      retp[i] += (matp[i * ncol + j] * matp[i * ncol + j]);
    }
  }
#endif
  return ret;
}

// if you need to invoke squared_sum_of_cols() for local rowmajor matrix from map(),
// invoke this function. Otherwise function pointer resolution issue might occur.
template <class T>
std::vector<T> 
rmm_squared_sum_of_cols(const rowmajor_matrix_local<T>& m) {
  return squared_sum_of_cols(m);
}

template <class T>
rowmajor_matrix_local<T> operator+(const rowmajor_matrix_local<T>& a,
                                   const rowmajor_matrix_local<T>& b) {
  if(a.local_num_row != b.local_num_row || a.local_num_col != b.local_num_col)
    throw std::runtime_error("invalid size for matrix addition");
    
  size_t imax = a.local_num_row;
  size_t jmax = a.local_num_col;
  rowmajor_matrix_local<T> c(imax, jmax);
  auto* ap = a.val.data();
  auto* bp = b.val.data();
  auto* cp = c.val.data();
  for(size_t i = 0; i < imax; i++) {
    for(size_t j = 0; j < jmax; j++) {
      cp[i * jmax + j] = ap[i * jmax + j] + bp[i * jmax + j];
    }
  } 
  return c; 
}

template <class T>
rowmajor_matrix_local<T> operator-(const rowmajor_matrix_local<T>& a,
                                   const rowmajor_matrix_local<T>& b) {
  if(a.local_num_row != b.local_num_row || a.local_num_col != b.local_num_col)
    throw std::runtime_error("invalid size for matrix addition");
    
  size_t imax = a.local_num_row;
  size_t jmax = a.local_num_col;
  rowmajor_matrix_local<T> c(imax, jmax);
  auto* ap = a.val.data();
  auto* bp = b.val.data();
  auto* cp = c.val.data();
  for(size_t i = 0; i < imax; i++) {
    for(size_t j = 0; j < jmax; j++) {
      cp[i * jmax + j] = ap[i * jmax + j] - bp[i * jmax + j];
    }
  } 
  return c; 
}

template <class T>
rowmajor_matrix_local<T> rowmajor_matrix_local<T>::transpose() const {
  rowmajor_matrix_local<T> ret(local_num_col, local_num_row);
  T* retp = &ret.val[0];
  const T* valp = &val[0];
  for(size_t i = 0; i < local_num_row; i++) {
    for(size_t j = 0; j < local_num_col; j++) {
      retp[j * local_num_row + i] = valp[i * local_num_col + j];
    }
  }
  return ret;
}

template <class T>
rowmajor_matrix_local<T> rowmajor_matrix_local<T>::pow_val(T exponent) const {
  rowmajor_matrix_local<T> ret(*this);
  auto* valp = ret.val.data();
  auto valsize = ret.val.size();
  for (size_t i = 0; i < valsize; i++) {
    valp[i] = std::pow(valp[i], exponent);
  }
  return ret;
}

/*
  The directory should contain following files:
  - val: big endian binary data file that contains values of the matrix 
         in row major order
  - nums: text file that contains num_row in the first line and
          num_col in the second line
 */
template <class T>
rowmajor_matrix_local<T>
make_rowmajor_matrix_local_loadbinary(const std::string& input) {
  std::string valfile = input + "/val";
  std::string numsfile = input + "/nums";
  std::string typefile = input + "/type";
  std::ifstream numstr;
  numstr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  numstr.open(numsfile.c_str());
  size_t num_row, num_col;
  numstr >> num_row >> num_col;
  struct stat sb;
  if(stat(typefile.c_str(), &sb) == 0) { // no file/directory
    std::ifstream typestr;
    typestr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    typestr.open(typefile.c_str());
    std::string valtype; 
    typestr >> valtype;
    confirm_given_type_against_expected<T>(valtype, __func__, "val");
  }
  else RLOG(INFO) << "no type file is present: skipping the typecheck for binary data!\n";
  rowmajor_matrix_local<T> ret;
  ret.set_local_num(num_row, num_col);
  auto vec = make_dvector_loadbinary<T>(valfile);
  auto tmp = vec.gather();
  ret.val.swap(tmp);
  return ret;
}

template <class T>
void rowmajor_matrix_local<T>::savebinary(const std::string& dir) {
  struct stat sb;
  if(stat(dir.c_str(), &sb) != 0) { // no file/directory
    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO; // man 2 stat
    if(mkdir(dir.c_str(), mode) != 0) {
      perror("mkdir failed:");
      throw std::runtime_error("mkdir failed");
    }
  } else if(!S_ISDIR(sb.st_mode)) {
    throw std::runtime_error(dir + " is not a directory");
  }
  std::string valfile = dir + "/val";
  std::string numsfile = dir + "/nums";
  std::string typefile = dir + "/type";
  std::ofstream numstr;
  numstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  numstr.open(numsfile.c_str());
  numstr << local_num_row << "\n" << local_num_col << std::endl;
  std::ofstream typestr;
  typestr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  typestr.open(typefile.c_str());
  typestr << get_type_name<T>() << std::endl;
  auto tmp = make_dvector_scatter(val);
  tmp.savebinary(valfile);
}

template <class T, class I>
void arg_partition(rowmajor_matrix_local<T>& key,
                   rowmajor_matrix_local<I>& value,
                   size_t k,
                   bool allow_multiple = false) {
  auto nrow = key.local_num_row;
  auto ncol = key.local_num_col;
  if (nrow != value.local_num_row && ncol != value.local_num_col)
    throw std::runtime_error("partition_sort: different dimension for key and value is encountered!\n");
  partition_sort(key.val.data(), value.val.data(), nrow, ncol, k, allow_multiple);
}


template <class T, class I = int>
rowmajor_matrix_local<I>
arg_partition(rowmajor_matrix_local<T>& key,
              size_t k,
              bool allow_multiple = false) {
  auto nrow = key.local_num_row;
  auto ncol = key.local_num_col;
  rowmajor_matrix_local<I> ret(nrow, ncol);
  auto iptr = ret.val.data();
  for(size_t i = 0; i < nrow; ++i) {
    for(size_t j = 0; j < ncol; ++j) {
      iptr[i * ncol + j] = j; // storing col-index
    }
  }
  partition_sort(key.val.data(), iptr, nrow, ncol, k, allow_multiple);
  return ret;
}

/*
  distributed version: rowmajor_matrix
 */

template <class T>
void rowmajor_clear_helper(rowmajor_matrix_local<T>& mat) {mat.clear();}

template <class T>
struct rowmajor_matrix {
  rowmajor_matrix(){}
  rowmajor_matrix(frovedis::node_local<rowmajor_matrix_local<T>>&& d) :
    data(std::move(d)) {}
  void set_num(size_t r, size_t c) {
    num_row = r; num_col = c;
  }
  rowmajor_matrix<T> transpose() const;
  rowmajor_matrix_local<T> gather();
  rowmajor_matrix<T>& align_as(const std::vector<size_t>&);  
  template <class U>
  rowmajor_matrix<T>& align_to(rowmajor_matrix<U>&);
  rowmajor_matrix<T>& align_block();
  template <class I = size_t, class O = size_t>
  crs_matrix<T,I,O> to_crs();
  std::vector<T> get_row(size_t r);
  std::vector<size_t> get_local_num_rows(); 
  void debug_print(size_t n = 0) {
    std::cout << "num_row = " << num_row
              << ", num_col = " << num_col << std::endl;
    auto g = data.gather();
    for(size_t i = 0; i < g.size(); i++) {
      std::cout << "node " << i << std::endl;
      g[i].debug_print(n);
    }
  }
  void save(const std::string& file);
  void clear() {
    data.mapv(rowmajor_clear_helper<T>);
    num_row = 0;
    num_col = 0;
  }
  void savebinary(const std::string&);
  frovedis::node_local<rowmajor_matrix_local<T>> data;
  size_t num_row;
  size_t num_col;
  typedef T value_type;
  typedef rowmajor_matrix_local<T> local_mat_type;
};

template <class T>
size_t rowmajor_get_local_num_row(const rowmajor_matrix_local<T>& mat) {
  return mat.local_num_row;
}

template <class T>
size_t rowmajor_get_local_num_col(const rowmajor_matrix_local<T>& mat) {
  return mat.local_num_col;
}

template <class T>
std::vector<size_t> rowmajor_matrix<T>::get_local_num_rows() {
  return data.map(rowmajor_get_local_num_row<T>).gather();
}

template <class T>
void rowmajor_set_local_num_col(rowmajor_matrix_local<T>& mat,
                                size_t num_col) {
  mat.local_num_col = num_col;
}

template <class T>
void rowmajor_get_row_helper(size_t& i, DVID<rowmajor_matrix_local<T>>& dvid,
                             size_t& pos, intptr_t& retp) {
  if(i == get_selfid()) {
    auto v = dvid.get_selfdata()->get_row(pos);
    send_data_helper(0, v);
  } else if(get_selfid() == 0) {
    std::vector<T>* ret = reinterpret_cast<std::vector<T>*>(retp);
    receive_data_helper(i, *ret);
  }
}

template <class T>
std::vector<T> rowmajor_matrix<T>::get_row(size_t pos) {
  auto sizes = data.map(rowmajor_get_local_num_row<T>).gather();
  if(pos < sizes[0]) return data.get_dvid().get_selfdata()->get_row(pos);
  else {
    pos -= sizes[0];
    for(size_t i = 1; i < sizes.size(); i++) {
      if(pos < sizes[i]) {
        std::vector<T> ret;
        intptr_t retp = reinterpret_cast<intptr_t>(&ret);
        bcast_rpc_oneway(rowmajor_get_row_helper<T>, i, data.get_dvid(),
                         pos, retp);
        return ret;
      } else pos -= sizes[i];
    }
  }
  throw std::runtime_error("get_row: invalid position");  
}

#ifdef __ve__
template <class T>
rowmajor_matrix<T> make_rowmajor_matrix_load(const std::string& input) {
  auto line_starts_byword = make_node_local_allocate<std::vector<size_t>>(); 
  auto ws = load_simple_csv(input, line_starts_byword, false, false, ' ');
  rowmajor_matrix<T>
    ret(ws.map(+[](const words& ws, const std::vector<size_t>& ls) {
          rowmajor_matrix_local<T> ret;
          ret.val = parsenumber<T>(ws);
          if(ls.size() > 1) { // 1st item is always 0
            ret.local_num_col = ls[1];
          } else {
            ret.local_num_col = ret.val.size();
          }
          ret.local_num_row = ls.size();
          return ret;
        }, line_starts_byword));
  ret.num_row = ret.data.map(rowmajor_get_local_num_row<T>).reduce(add<size_t>);
  ret.num_col = ret.data.map(rowmajor_get_local_num_col<T>).gather()[0];
  // if number of row is zero, num_col is not set properly
  ret.data.mapv(rowmajor_set_local_num_col<T>, broadcast(ret.num_col));
  ret.align_block();
  return ret;
}
#else
template <class T>
rowmajor_matrix<T> make_rowmajor_matrix_load(const std::string& input) {
  auto dvec = make_dvector_loadline(input);
  dvec.align_block();
  rowmajor_matrix<T>
    ret(dvec.moveto_node_local().map(make_rowmajor_matrix_local_vectorstring<T>));
  ret.num_row = ret.data.map(rowmajor_get_local_num_row<T>).reduce(add<size_t>);
  ret.num_col = ret.data.map(rowmajor_get_local_num_col<T>).gather()[0];
  // if number of row is zero, num_col is not set properly
  ret.data.mapv(rowmajor_set_local_num_col<T>, broadcast(ret.num_col));
  return ret;
}
#endif

template <class T>
struct moveto_rowmajor_matrix_local {
  moveto_rowmajor_matrix_local(){}
  moveto_rowmajor_matrix_local(std::vector<size_t> num_rows, size_t num_col)
    : num_rows(num_rows), num_col(num_col) {}
  rowmajor_matrix_local<T> operator()(std::vector<T>& v) {
    rowmajor_matrix_local<T> ret;
    ret.val.swap(v);
    ret.set_local_num(num_rows[get_selfid()], num_col);
    return ret;
  }
  std::vector<size_t> num_rows;
  size_t num_col;
  SERIALIZE(num_rows, num_col)
};

/*
  The directory should contain following files:
  - val: big endian binary data file that contains values of the matrix 
         in row major order
  - nums: text file that contains num_row in the first line and
          num_col in the second line
 */
template <class T>
rowmajor_matrix<T> make_rowmajor_matrix_loadbinary(const std::string& input) {
  std::string valfile = input + "/val";
  std::string numsfile = input + "/nums";
  std::string typefile = input + "/type";
  std::ifstream numstr;
  numstr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  numstr.open(numsfile.c_str());
  size_t num_row, num_col;
  numstr >> num_row >> num_col;
  struct stat sb;
  if(stat(typefile.c_str(), &sb) == 0) { // no file/directory
    std::ifstream typestr;
    typestr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    typestr.open(typefile.c_str());
    std::string valtype;
    typestr >> valtype;
    confirm_given_type_against_expected<T>(valtype, __func__, "val");
  }
  else RLOG(INFO) << "no type file is present: skipping the typecheck for binary data!\n";
  size_t each_num_row = ceil_div(num_row, static_cast<size_t>(get_nodesize()));
  std::vector<size_t> rows(get_nodesize());
  size_t left = num_row;
  for(size_t i = 0; i < rows.size(); i++) {
    if(left < each_num_row) {
      rows[i] = left;
      break;
    } else {
      rows[i] = each_num_row;
      left -= each_num_row;
    }
  }
  std::vector<size_t> sizes(get_nodesize());
  auto sizesp = sizes.data();
  auto rowsp = rows.data();
  for(size_t i = 0; i < sizes.size(); i++) {
    sizesp[i] = rowsp[i] * num_col;
  }
  auto vec = make_dvector_loadbinary<T>(valfile, sizes);
  rowmajor_matrix<T> ret(vec.moveto_node_local().
                         template map<rowmajor_matrix_local<T>>
                         (moveto_rowmajor_matrix_local<T>(rows, num_col)));
  ret.set_num(num_row, num_col);
  return ret;
}

template <class T>
std::vector<T>
rowmajor_matrix_local_extractval(rowmajor_matrix_local<T>& mat) {
  return mat.val;
}

template <class T>
void rowmajor_matrix<T>::savebinary(const std::string& dir) {
  struct stat sb;
  if(stat(dir.c_str(), &sb) != 0) { // no file/directory
    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO; // man 2 stat
    if(mkdir(dir.c_str(), mode) != 0) {
      perror("mkdir failed:");
      throw std::runtime_error("mkdir failed");
    }
  } else if(!S_ISDIR(sb.st_mode)) {
    throw std::runtime_error(dir + " is not a directory");
  }
  std::string valfile = dir + "/val";
  std::string numsfile = dir + "/nums";
  std::string typefile = dir + "/type";
  std::ofstream numstr;
  numstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  numstr.open(numsfile.c_str());
  numstr << num_row << "\n" << num_col << std::endl;
  std::ofstream typestr;
  typestr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  typestr.open(typefile.c_str());
  typestr << get_type_name<T>() << std::endl;
  auto tmp = data.map(rowmajor_matrix_local_extractval<T>).
    template moveto_dvector<T>();
  tmp.savebinary(valfile);
}

template <class T>
std::ostream& operator<<(std::ostream& str, const rowmajor_matrix<T>& mat) {
  // node_local<T>::gather() is not const...
  auto& mat2 = const_cast<rowmajor_matrix<T>&>(mat);
  auto gmat = mat2.data.gather();
  for(auto& l: gmat) str << l;
  return str;
}

#ifdef __ve__
template <class T>
void rowmajor_matrix<T>::save(const std::string& file) {
  auto to_save = data.map(+[](rowmajor_matrix_local<T>& m) {
      std::vector<size_t> new_starts;
      auto intvec = concat_words(number_to_words(m.val), " ", new_starts);
      auto intvecp = intvec.data();
      auto intvec_size = intvec.size();
      auto new_startsp = new_starts.data();
      auto local_num_row = m.local_num_row;
      auto local_num_col = m.local_num_col;
      for(size_t i = 1; i < local_num_row; i++) {
        intvecp[new_startsp[i*local_num_col]-1] = '\n';
      }
      if(intvec_size > 0) intvecp[intvec.size()-1] = '\n';
      std::vector<char> ret(intvec_size);
      int_to_char(intvecp, intvec_size, ret.data());
      return ret;
    });
  to_save.template moveto_dvector<char>().savebinary(file);
}
#else
template <class T>
void rowmajor_matrix<T>::save(const std::string& file) {
  std::ofstream str(file.c_str());
  str << *this;
}
#endif

std::vector<size_t> get_block_sizes(size_t num_row);
std::vector<size_t> get_block_sizes(size_t num_row, size_t wsize);

template <class T>
rowmajor_matrix<T> make_rowmajor_matrix_scatter(rowmajor_matrix_local<T>& m,
                                                std::vector<size_t>& rows) {
  std::vector<size_t> sizes(get_nodesize());
  auto sizesp = sizes.data();
  auto rowsp = rows.data();
  for(size_t i = 0; i < sizes.size(); i++) {
    sizesp[i] = rowsp[i] * m.local_num_col;
  }
  auto vec = make_dvector_scatter(m.val, sizes);
  rowmajor_matrix<T> ret(vec.moveto_node_local().
                         template map<rowmajor_matrix_local<T>>
                         (moveto_rowmajor_matrix_local<T>(rows,
                                                          m.local_num_col)));
  ret.set_num(m.local_num_row, m.local_num_col);
  return ret;
}

template <class T>
rowmajor_matrix<T> make_rowmajor_matrix_scatter(rowmajor_matrix_local<T>& m) {
  auto rows = get_block_sizes(m.local_num_row);
  return make_rowmajor_matrix_scatter(m, rows);
}

// scattering a vector into a number of requested pieces
template <class T>
std::vector<std::vector<T>>
get_scattered_vectors(const std::vector<T>& vec, 
                      size_t nrow, size_t ncol, size_t wsize) {
  auto rows = get_block_sizes(nrow, wsize);
  std::vector<size_t> sizevec(wsize);
  auto sizevecp = sizevec.data();
  auto rowsp = rows.data();
  for(size_t i = 0; i < wsize; i++) {
    sizevecp[i] = rowsp[i] * ncol;
  }
  std::vector<std::vector<T>> src2(wsize);
  const T* srcp = &vec[0];
  for(size_t i = 0; i < wsize; i++) {
    src2[i].resize(sizevecp[i]);
    for(size_t j = 0; j < sizevecp[i]; j++) {
      src2[i][j] = srcp[j];
    }
    srcp += sizevecp[i];
  }
  return src2;
}

template <class T>
std::vector<std::vector<T>>
get_scattered_vectors(const std::vector<T>& vec, 
                      size_t nrow, size_t ncol) {
  return get_scattered_vectors(vec,nrow,ncol,get_nodesize());
}

// scattering the local matrix into given number of chunks
template <class T>
std::vector<rowmajor_matrix_local<T>>
get_scattered_rowmajor_matrices(rowmajor_matrix_local<T>& m,
                                size_t wsize) {
  auto nrow = m.local_num_row;
  auto ncol = m.local_num_col;
  auto rows = get_block_sizes(nrow, wsize);
  auto src2 = get_scattered_vectors(m.val,nrow,ncol,wsize);
  std::vector<rowmajor_matrix_local<T>> ret(wsize);
  for(size_t i=0; i < wsize; ++i) {
    rowmajor_matrix_local<T> tmp;
    tmp.val.swap(src2[i]);
    tmp.set_local_num(rows[i],ncol);
    ret[i] = tmp;
  }
  return ret;
}

template <class T>
std::vector<rowmajor_matrix_local<T>>
get_scattered_rowmajor_matrices(rowmajor_matrix_local<T>& m) {
  return get_scattered_rowmajor_matrices(m, get_nodesize());
}

template <class T>
rowmajor_matrix_local<T>
merge(const std::vector<rowmajor_matrix_local<T>>& vec) {
  auto nmat = vec.size();
  if(nmat == 0) return rowmajor_matrix_local<T>();
  size_t nrow = 0;
  size_t ncol = vec[0].local_num_col; // all matrix should have same ncol
  for(size_t i = 0; i < nmat; ++i) nrow += vec[i].local_num_row;
  rowmajor_matrix_local<T> ret(nrow, ncol);
  auto rdata = ret.val.data();
  size_t k = 0;
  for(size_t i = 0; i < nmat; ++i) {
    auto vdata = vec[i].val.data();
    auto vsize = vec[i].val.size();
    for(size_t j = 0; j < vsize; ++j) rdata[k + j] = vdata[j];
    k += vsize;
  }
  return ret;
}

template <class T>
rowmajor_matrix_local<T> rowmajor_matrix<T>::gather() {
  auto locals = data.gather();
  return merge(locals);
}

template <class T>
rowmajor_matrix_local<T>
rowmajor_matrix_call_transpose(const rowmajor_matrix_local<T>& m) {
  return m.transpose();
}

template <class T>
struct rowmajor_matrix_divide_and_exchange {
  rowmajor_matrix_divide_and_exchange(){}
  rowmajor_matrix_divide_and_exchange(size_t r, size_t c) :
    new_num_row(r), new_num_col(c) {}
  rowmajor_matrix_local<T> operator()(rowmajor_matrix_local<T>& m,
                                      std::vector<size_t>& new_local_cols) {
    size_t node_size = frovedis::get_nodesize();
    /*
    size_t each_num_row =
      ceil_div(new_num_row, static_cast<size_t>(node_size));
    std::vector<size_t> rows(node_size);
    size_t left = new_num_row;
    for(size_t i = 0; i < rows.size(); i++) {
      if(left < each_num_row) {
        rows[i] = left;
        break;
      } else {
        rows[i] = each_num_row;
        left -= each_num_row;
      }
    }
    */
    auto rows = get_block_sizes(new_num_row);
    rowmajor_matrix_local<T> ret(rows[get_selfid()], new_num_col);
    std::vector<size_t> send_size(node_size);
    auto send_sizep = send_size.data();
    auto rowsp = rows.data();
    for(size_t i = 0; i < node_size; i++) {
      send_sizep[i] = rowsp[i] * m.local_num_col;
    }
    std::vector<size_t> recv_size(node_size);
    MPI_Alltoall(&send_size[0], sizeof(size_t), MPI_CHAR,
                 &recv_size[0], sizeof(size_t), MPI_CHAR, frovedis_comm_rpc);
    size_t total_size = 0;
    auto recv_sizep = recv_size.data();
    for(size_t i = 0; i < node_size; i++) {
      total_size += recv_sizep[i];
    }
    std::vector<size_t> send_displ(node_size);
    std::vector<size_t> recv_displ(node_size);
    auto send_displp = send_displ.data();
    auto recv_displp = recv_displ.data();
    send_displp[0] = 0; recv_displp[0] = 0;
    for(size_t i = 1; i < node_size; i++) {
      send_displp[i] = send_displp[i-1] + send_sizep[i-1];
      recv_displp[i] = recv_displp[i-1] + recv_sizep[i-1];
    }
    std::vector<T> tmpval(ret.val.size());
    large_alltoallv(sizeof(T),
                    reinterpret_cast<char*>(&m.val[0]), send_size, send_displ, 
                    reinterpret_cast<char*>(&tmpval[0]), recv_size, recv_displ, 
                    frovedis_comm_rpc);
    T* retvalp = &ret.val[0];
    T* off = &tmpval[0];
    size_t global_c_off = 0;
    size_t* new_local_colsp = &new_local_cols[0];
    for(size_t n = 0; n < node_size; n++) {
      for(size_t r = 0; r < ret.local_num_row; r++) {
        for(size_t local_c = 0; local_c < new_local_colsp[n]; local_c++) {
          retvalp[new_num_col * r + global_c_off + local_c] =
            off[new_local_colsp[n] * r + local_c];
        }
      }
      off += new_local_colsp[n] * ret.local_num_row;
      global_c_off += new_local_colsp[n];
    }
    return ret;
  }
  size_t new_num_row;
  size_t new_num_col;
  SERIALIZE(new_num_row, new_num_col)
};

template <class T>
rowmajor_matrix<T> rowmajor_matrix<T>::transpose() const {
  // transpose is semantically const, but map cannot be made const
  auto* x = const_cast<rowmajor_matrix<T>*>(this);
  auto local_num_rows =
    make_node_local_broadcast(x->data.map(rowmajor_get_local_num_row<T>).gather());
  auto shuffled = x->data.map(rowmajor_matrix_call_transpose<T>).template 
    map<rowmajor_matrix_local<T>>(rowmajor_matrix_divide_and_exchange<T>
                                  (num_col, num_row), local_num_rows);
  rowmajor_matrix<T> ret(std::move(shuffled));
  ret.set_num(num_col, num_row);
  return ret;
}

template <class T>
std::vector<T> get_vector(rowmajor_matrix_local<T>& m) {
  return m.val;
}

template <class T>
void set_vector(rowmajor_matrix_local<T>& m, std::vector<T>& v) {
  m.val = v;
}

template <class T>
void align_as_set_local_num(rowmajor_matrix_local<T>& m, size_t r, size_t c) {
  m.set_local_num(r, c);
}

// mostly the same as dvector
template <class T>
rowmajor_matrix<T>&
rowmajor_matrix<T>::align_as(const std::vector<size_t>& dst) {
  if(dst.size() != get_nodesize()) 
    throw std::runtime_error
      ("align_as: size of dst is not equal to node size");
  auto mysizes = data.map(rowmajor_get_local_num_row<T>).gather();
  size_t dsttotal = 0;
  size_t selftotal = 0;
  auto dstp = dst.data();
  auto mysizesp = mysizes.data();
  for(size_t i = 0; i < dst.size(); i++) dsttotal += dstp[i];
  for(size_t i = 0; i < mysizes.size(); i++) selftotal += mysizesp[i];
  if(dsttotal != selftotal)
    throw std::runtime_error
      ("align_as: total size of src and dst does not match");
  bool is_same = true;
  for(size_t i = 0; i < dst.size(); i++) {
    if(dstp[i] != mysizesp[i]) {
      is_same = false;
      break;
    }
  }
  if(is_same) return *this;
  auto tmp = make_node_local_allocate<std::vector<T>>();
  // in dvector.hpp
  auto alltoall_sizes = bcast_rpc(align_as_calc_alltoall_sizes, mysizes,
                                  const_cast<std::vector<size_t>&>(dst));
  for(size_t i = 0; i < alltoall_sizes.size(); i++) {
    for(size_t j = 0; j < alltoall_sizes[i].size(); j++) {
      alltoall_sizes[i][j] *= num_col;
    }
  }
  auto bcast_alltoall_sizes = make_node_local_broadcast(alltoall_sizes);
  // TODO: avoid copying
  auto local_vector = data.map(get_vector<T>);
  local_vector.mapv(align_as_align<T>, tmp, bcast_alltoall_sizes);
  data.mapv(set_vector<T>, tmp);
  auto num_rows =
    make_node_local_scatter(const_cast<std::vector<size_t>&>(dst));
  auto num_cols = make_node_local_broadcast(num_col);
  data.mapv(align_as_set_local_num<T>, num_rows, num_cols);
  return *this;
}

template <class T>
template <class U>
rowmajor_matrix<T>&
rowmajor_matrix<T>::align_to(rowmajor_matrix<U>& m) {
  auto sizes = m.data.map(rowmajor_get_local_num_row<U>).gather();
  return align_as(sizes);
}

template <class T>
rowmajor_matrix<T>& rowmajor_matrix<T>::align_block() {
  /*
  size_t sz = num_row;
  size_t nodesize = get_nodesize();
  std::vector<size_t> block_size(nodesize);
  size_t each = ceil_div(sz, nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    if(sz > each) {
      block_size[i] = each;
      sz -= each;
    } else {
      block_size[i] = sz;
      break;
    }
  }
  */
  auto block_size = get_block_sizes(num_row);
  return align_as(block_size);
}

template <class T>
void sub_vector_row(rowmajor_matrix_local<T>& m, std::vector<T>& v) {
  size_t num_col = m.local_num_col;
  size_t num_row = m.local_num_row;
  if(num_col != v.size())
    throw std::runtime_error("sub_vector_row: size mismatch");
  T* valp = m.val.data();
  T* vp = v.data();
  for(size_t i = 0; i < num_row; i++) {
    for(size_t j = 0; j < num_col; j++) valp[num_col * i + j] -= vp[j];
  }
}

template <class T>
void add_vector_row(rowmajor_matrix_local<T>& m, std::vector<T>& v) {
  size_t num_col = m.local_num_col;
  size_t num_row = m.local_num_row;
  if(num_col != v.size())
    throw std::runtime_error("add_vector_row: size mismatch");
  T* valp = m.val.data();
  T* vp = v.data();
  for(size_t i = 0; i < num_row; i++) {
    for(size_t j = 0; j < num_col; j++) valp[num_col * i + j] += vp[j];
  }
}

template <class T>
void mul_vector_row(rowmajor_matrix_local<T>& m, std::vector<T>& v) {
  size_t num_col = m.local_num_col;
  size_t num_row = m.local_num_row;
  if(num_col != v.size())
    throw std::runtime_error("mul_vector_row: size mismatch");
  T* valp = m.val.data();
  T* vp = v.data();
  for(size_t i = 0; i < num_row; i++) {
    for(size_t j = 0; j < num_col; j++) valp[num_col * i + j] *= vp[j];
  }
}

template <class T>
void scale_matrix(rowmajor_matrix<T>& mat, std::vector<T>& vec) {
  mat.data.mapv(mul_vector_row<T>, broadcast(vec));
} 

template<class T>
rowmajor_matrix_local<T> scale_rmm_matrix_impl(rowmajor_matrix_local<T>& mat,
                                int axis, const std::vector<T>& vect) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  rowmajor_matrix_local<T> ret(nrow, ncol); 
  auto vp = vect.data();
  auto retp = ret.val.data();
  auto matp = mat.val.data();
  if(axis == 1) {  
   require(vect.size() == ncol,
    "vector size does not match with number of cols in matrix");
   for(size_t j = 0; j < ncol; ++j) {
      for(size_t i = 0; i < nrow; ++i) {
        retp[i * ncol + j] = matp[i * ncol + j] * vp[j];
      }
    }
  }
  else{
    require(vect.size() == nrow,
     "vector size does not match with number of rows in matrix");
    for(size_t j = 0; j < ncol; ++j) {
      for(size_t i = 0; i < nrow; ++i) {
        retp[i * ncol + j] = matp[i * ncol + j] * vp[i];
      }
    }
  }
  return ret;
}

template<class T>
rowmajor_matrix<T> scale_rmm_matrix(rowmajor_matrix<T>& mat,
                                 int axis, const std::vector<T>& vect) {
  rowmajor_matrix_local<T> (*f)(rowmajor_matrix_local<T>&,
                            int, const std::vector<T>&) = scale_rmm_matrix_impl;
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  rowmajor_matrix<T> ret;
  if(axis == 1) {
    ret = rowmajor_matrix<T>(mat.data.map(f, broadcast(axis), 
                              broadcast(vect)));
  }
  else { 
    auto nvect = make_dvector_scatter(vect, mat.get_local_num_rows()).
                                      moveto_node_local();
    ret = rowmajor_matrix<T>(mat.data.map(f, 
                              broadcast(axis),  nvect));
  }
  ret.set_num(nrow, ncol);
  return ret;
}

template <class T>
std::vector<T>
compute_mean(rowmajor_matrix_local<T>& mat, int axis = -1,
             const std::vector<T>& sample_weight = std::vector<T>()) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  if(nrow == 0)
    throw std::runtime_error("matrix with ZERO rows for mean computation!");
  bool sw_present  = (!sample_weight.empty() && !(vector_is_uniform(sample_weight) &&
                       sample_weight[0] == 1));
  if(sw_present) { 
    auto sw_mat = scale_rmm_matrix_impl(mat, axis, sample_weight);
    auto sw_sum  = vector_sum(sample_weight);
    if (axis != 0 && axis != 1) {
      return std::vector<T>(1, vector_sum(sw_mat.val) / sw_sum);
    }
    auto ret = (axis == 0) ? rmm_sum_of_rows(sw_mat) : rmm_sum_of_cols(sw_mat);
    return (axis == 0) ? vector_divide(ret, (T)sw_sum) : 
                         vector_divide(ret, (T)sw_sum);
  }
  if (axis != 0 && axis != 1)
    return std::vector<T>(1, vector_mean(mat.val));
  auto ret = (axis == 0) ? rmm_sum_of_rows(mat) : rmm_sum_of_cols(mat);
  return (axis == 0) ? vector_divide(ret, (T)nrow) : 
                       vector_divide(ret, (T)ncol);
}

template <class T>
std::vector<T>
compute_mean(rowmajor_matrix<T>& mat, int axis = -1,
             const std::vector<T>& sample_weight = std::vector<T>()) {
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  if(nrow == 0)
    throw std::runtime_error("matrix with ZERO rows for mean computation!");
  bool sw_present  = (!sample_weight.empty() && !(vector_is_uniform(sample_weight) &&
                       sample_weight[0] == 1));
  if(sw_present) { 
    auto sw_mat = scale_rmm_matrix<T>(mat, axis, sample_weight);
    auto sw_sum  = vector_sum(sample_weight);
    if (axis != 0 && axis != 1)
      return std::vector<T>(1, sum_of_elements(sw_mat) / (sw_sum));
    auto ret = (axis == 0) ? sum_of_rows(sw_mat) : sum_of_cols(sw_mat);
    return (axis == 0) ? vector_divide(ret, sw_sum) : vector_divide(ret, sw_sum);
  }
  if (axis != 0 && axis != 1)
      return std::vector<T>(1, sum_of_elements(mat) / (nrow * ncol));
  auto ret = (axis == 0) ? sum_of_rows(mat) : sum_of_cols(mat);
  return (axis == 0) ? vector_divide(ret, (T)nrow) : vector_divide(ret, (T)ncol);
}

template <class T>
void centerize(rowmajor_matrix<T>& mat, std::vector<T>& mean) {
  if(mat.num_row < 2)
    throw std::runtime_error("cannot centerize if number of row is 0 or 1");
  mat.data.mapv(sub_vector_row<T>, broadcast(mean));
}

// destructively centerize in column direction; used in pca.hpp
template <class T>
void centerize(rowmajor_matrix<T>& mat) {
  auto mean = compute_mean(mat, 0); // column-wise mean
  centerize(mat, mean);
}

template <class T>
void decenterize(rowmajor_matrix<T>& mat, std::vector<T>& mean) {
  if(mat.num_row < 2)
    throw std::runtime_error("cannot decenterize if number of row is 0 or 1");
  mat.data.mapv(add_vector_row<T>, broadcast(mean));
}

template <class T>
std::vector<T>
compute_stddev(rowmajor_matrix<T>& mat, 
               std::vector<T>& mean, 
               bool sample_stddev = true) {
  if(mat.num_row < 2)
    throw std::runtime_error("cannot compute stddev if number of row is 0 or 1");
  centerize(mat, mean); // m = m - mean
  auto ret = mat.data.map(+[](rowmajor_matrix_local<T>& m)
                         {return squared_sum_of_rows(m);}).vector_sum();
  auto retp = ret.data();
  T to_div;
  if(sample_stddev) to_div = static_cast<T>(mat.num_row - 1);
  else to_div = static_cast<T>(mat.num_row);
  for(size_t i = 0; i < mat.num_col; ++i) {
    if(retp[i] == 0) retp[i] = 1.0;
    retp[i] = sqrt(retp[i] / to_div);
  }
  return ret;
}

template <class T>
std::vector<T>
compute_stddev(rowmajor_matrix<T>& mat, bool sample_stddev = true) {
  auto mean = compute_mean(mat, 0); // column-wise mean
  return compute_stddev(mat, mean, sample_stddev);
}

template <class T>
void standardize(rowmajor_matrix<T>& mat, 
                 std::vector<T>& mean, 
                 bool sample_stddev = true) {
  if(mat.num_row < 2)
    throw std::runtime_error("cannot standardize if number of row is 0 or 1");
  mat.data.mapv(sub_vector_row<T>, broadcast(mean));
  auto tmp = mat.data.map(+[](rowmajor_matrix_local<T>& m)
                         {return squared_sum_of_rows(m);}).vector_sum();
  auto tmpp = tmp.data();
  T to_div;
  if(sample_stddev) to_div = static_cast<T>(mat.num_row - 1);
  else to_div = static_cast<T>(mat.num_row);
  for(size_t i = 0; i < mat.num_col; ++i) {
    if(tmpp[i] == 0) tmpp[i] = 1.0; // data is zero so can be anything
    tmpp[i] = sqrt(to_div / tmpp[i]);
  }
  mat.data.mapv(mul_vector_row<T>, broadcast(tmp));
}

// destructively standardize in column direction; used in pca.hpp
template <class T>
void standardize(rowmajor_matrix<T>& mat, bool sample_stddev = true) {
  if(mat.num_row < 2)
    throw std::runtime_error("cannot standardize if number of row is 0 or 1");
  auto mean = compute_mean(mat, 0); // column-wise mean
  standardize(mat, mean, sample_stddev);
}

template <class T>
std::vector<T> variance_of_cols_helper(const rowmajor_matrix_local<T>& m,
                                       const std::vector<T>& avg) {
  auto nrow = m.local_num_row;
  auto ncol = m.local_num_col;
  std::vector<T> ret(ncol,0);
  T* retp = &ret[0];
  const T* matp = &m.val[0];
  const T* avgp = avg.data();
  for (size_t i = 0; i < nrow; ++i) {
    for(size_t j = 0; j < ncol; ++j) {
      auto tmp = matp[i * ncol + j] - avgp[j];
      retp[j] += tmp * tmp;
    }
  }
  return ret;
}

template <class T>
std::vector<T> variance(rowmajor_matrix<T>& mat, 
                        bool sample_variance = true) {
  if(mat.num_row < 2)
    throw std::runtime_error
      ("cannot call variance if number of row is 0 or 1");
  auto tmp = mat.data.map(+[](rowmajor_matrix_local<T>& m)
                          {return sum_of_rows(m);}).vector_sum();
  T* tmpp = tmp.data();
  T to_mul = static_cast<T>(1)/static_cast<T>(mat.num_row);
  size_t num_col = mat.num_col;
  for(size_t i = 0; i < num_col; i++) {
    tmpp[i] *= to_mul; // average
  }
  auto sq = mat.data.
    map(variance_of_cols_helper<T>, broadcast(tmp)).vector_sum();
  T* sqp = sq.data();
  T to_mul2;
  if(sample_variance)
    to_mul2 = static_cast<T>(1)/static_cast<T>(mat.num_row - 1);
  else
    to_mul2 = static_cast<T>(1)/static_cast<T>(mat.num_row);
  size_t size = sq.size();
  for(size_t i = 0; i < size; i++) sqp[i] *= to_mul2;
  return sq;
}

template <class T>
rowmajor_matrix<T> 
binarize(rowmajor_matrix<T>& mat, const T& threshold = 0) {
  rowmajor_matrix<T> ret(mat.data.map(rmm_binarize<T>, broadcast(threshold)));
  ret.num_row = mat.num_row;
  ret.num_col = mat.num_col;
  return ret;
}

template <class T>
T sum_of_elements(rowmajor_matrix<T>& mat) {
  return mat.data.map(rmm_sum_of_elements<T>).reduce(add<T>);
}

template <class T>
T squared_sum_of_elements(rowmajor_matrix<T>& mat) {
  return mat.data.map(rmm_squared_sum_of_elements<T>).reduce(add<T>);
}

template <class T>
std::vector<T>
sum_of_rows(rowmajor_matrix<T>& m) {
  return m.data.map(rmm_sum_of_rows<T>).vector_sum();
}

template <class T>
std::vector<T>
squared_sum_of_rows(rowmajor_matrix<T>& m) {
  return m.data.map(rmm_squared_sum_of_rows<T>).vector_sum();
}

template <class T>
std::vector<T>
sum_of_cols(rowmajor_matrix<T>& m) {
  return m.data.map(rmm_sum_of_cols<T>)
               .template moveto_dvector<T>().gather();
}

template <class T>
std::vector<T>
squared_sum_of_cols(rowmajor_matrix<T>& m) {
  return m.data.map(rmm_squared_sum_of_cols<T>)
               .template moveto_dvector<T>().gather();
}

template <class T>
std::vector <std::pair<T, size_t>>
min_of_elements(rowmajor_matrix_local<T>& mat,
                size_t start = 0) {
  auto global_min_index = vector_argmin(mat.val);
  auto global_min = mat.val[global_min_index];
  std::vector<std::pair<T, size_t>> ret(1);
  ret[0] = std::make_pair(global_min, global_min_index + start);
  return ret;
}

template <class T>
std::vector<std::pair<T, size_t>>
max_of_elements(rowmajor_matrix_local<T>& mat,
                size_t start = 0) {
  auto global_max_index = vector_argmax(mat.val);
  auto global_max = mat.val[global_max_index];
  std::vector<std::pair<T, size_t>> ret(1);
  ret[0] = std::make_pair(global_max, global_max_index + start);
  return ret;
}

template <class T>
std::vector<std::pair<T, size_t>>
min_of_rows(rowmajor_matrix_local<T>& mat,
            size_t start_index = 0) {
  auto mat_ptr = mat.val.data();
  int nrow = mat.local_num_row;
  int ncol = mat.local_num_col;
  T min = std::numeric_limits<T>::max();
  std::pair<T, size_t> p(min, 0);
  std::vector<std::pair<T, size_t>> res(ncol);
  auto res_ptr = res.data();

  for(size_t j = 0; j < ncol; ++j) res_ptr[j] = p;

#if defined(_SX) || defined(__ve__)
  if(nrow > ncol)  { //Iterate top-down
    for(size_t j = 0; j < ncol; ++j) {
      for(size_t i = 0; i < nrow; ++i) {
        auto index = ncol * i + j;
        if(mat_ptr[index] < res_ptr[j].first) {
          res_ptr[j].first = mat_ptr[index];
          res_ptr[j].second = i + start_index;
        }
      }
    }
  }
  else { //Iterate left-right
    for(size_t i = 0; i < nrow; ++i) {
      for(size_t j = 0; j < ncol; ++j) {
        auto index = ncol * i + j;
        if(mat_ptr[index] < res_ptr[j].first) {
          res_ptr[j].first = mat_ptr[index];
          res_ptr[j].second = i + start_index;
        }
      }
    }
  }
#else //Iterate left-right
  for(size_t i = 0; i < nrow; ++i) {
    for(size_t j = 0; j < ncol; ++j) {
      auto index = ncol * i + j;
      if(mat_ptr[index] < res_ptr[j].first) {
        res_ptr[j].first = mat_ptr[index];
        res_ptr[j].second = i + start_index;
      }
    }
  }
#endif
  return res;
}

template <class T>
std::vector<std::pair<T, size_t>>
max_of_rows(rowmajor_matrix_local<T>& mat,
            size_t start_index = 0) {
  auto mat_ptr = mat.val.data();
  int nrow = mat.local_num_row;
  int ncol = mat.local_num_col;
  T max = std::numeric_limits<T>::lowest();
  std::pair<T, size_t> p(max, 0);
  std::vector<std::pair<T, size_t>> res(ncol);
  auto res_ptr = res.data();

  for(size_t j = 0; j < ncol; ++j) res_ptr[j] = p;

#if defined(_SX) || defined(__ve__)
  if(nrow > ncol)  { //Iterate top-down
    for(size_t j = 0; j < ncol; ++j) {
      for(size_t i = 0; i < nrow; ++i) {
        auto index = ncol * i + j;
        if(mat_ptr[index] > res_ptr[j].first) {
          res_ptr[j].first = mat_ptr[index];
          res_ptr[j].second = i + start_index;
        }
      }
    }
  }
  else { //Iterate left-right
    for(size_t i = 0; i < nrow; ++i) {
      for(size_t j = 0; j < ncol; ++j) {
        auto index = ncol * i + j;
        if(mat_ptr[index] > res_ptr[j].first) {
          res_ptr[j].first = mat_ptr[index];
          res_ptr[j].second = i + start_index;
        }
      }
    }
  }
#else //Iterate left-right
  for(size_t i = 0; i < nrow; ++i) {
    for(size_t j = 0; j < ncol; ++j) {
      auto index = ncol * i + j;
        if(mat_ptr[index] > res_ptr[j].first) {
          res_ptr[j].first = mat_ptr[index];
          res_ptr[j].second = i + start_index;
        }
    }
  }
#endif
  return res;
}

template <class T>
std::vector<T>
min_of_cols(rowmajor_matrix_local<T>& mat,
            std::vector<size_t>& pos) {
  auto mat_ptr = mat.val.data();
  int nrow = mat.local_num_row;
  int ncol = mat.local_num_col;
  T min = std::numeric_limits<T>::max();
  std::vector<T> res(nrow);
  auto res_ptr = res.data();
  pos.resize(nrow);
  auto pos_ptr = pos.data();

  for(size_t i = 0; i < nrow; ++i) res_ptr[i] = min;

#if defined(_SX) || defined(__ve__)
  if(nrow > ncol) { //Iterate top-down
    for(size_t j = 0; j < ncol; ++j) {
      for(size_t i = 0; i < nrow; ++i) {
        auto index = ncol * i + j;
          if(mat_ptr[index] < res_ptr[i]) {
            res_ptr[i] = mat_ptr[index];
            pos_ptr[i] = j;
          }
        }
      }
  }
  else { //Iterate left-right
    for(size_t i = 0; i < nrow; ++i) {
      for(size_t j = 0; j < ncol; ++j) {
        auto index = ncol * i + j;
          if(mat_ptr[index] < res_ptr[i]) {
            res_ptr[i] = mat_ptr[index];
            pos_ptr[i] = j;
          }
        }
    }
  }
#else // Iterate left right
  for(size_t i = 0; i < nrow; ++i) {
    for(size_t j = 0; j < ncol; ++j) {
      auto index = ncol * i + j;
        if(mat_ptr[index] < res_ptr[i]) {
          res_ptr[i] = mat_ptr[index];
          pos_ptr[i] = j;
        }
    }
  }
#endif
  return res;
}

template <class T>
std::vector<T>
max_of_cols(rowmajor_matrix_local<T>& mat,
            std::vector<size_t>& pos) {
  auto mat_ptr = mat.val.data();
  int nrow = mat.local_num_row;
  int ncol = mat.local_num_col;
  T max = std::numeric_limits<T>::lowest();
  std::vector<T> res(nrow);
  auto res_ptr = res.data();
  pos.resize(nrow);
  auto pos_ptr = pos.data();

  for(size_t i = 0; i < nrow; ++i) res_ptr[i] = max;

#if defined(_SX) || defined(__ve__)
  if(nrow > ncol) { //Iterate top-down
    for(size_t j = 0; j < ncol; ++j) {
      for(size_t i = 0; i < nrow; ++i) {
        auto index = ncol * i + j;
          if(mat_ptr[index] > res_ptr[i]) {
            res_ptr[i] = mat_ptr[index];
            pos_ptr[i] = j;
          }
        }
      }
  }
  else { //Iterate left-right
    for(size_t i = 0; i < nrow; ++i) {
      for(size_t j = 0; j < ncol; ++j) {
        auto index = ncol * i + j;
          if(mat_ptr[index] > res_ptr[i]) {
            res_ptr[i] = mat_ptr[index];
            pos_ptr[i] = j;
          }
        }
    }
  }
#else // Iterate left right
  for(size_t i = 0; i < nrow; ++i) {
    for(size_t j = 0; j < ncol; ++j) {
      auto index = ncol * i + j;
        if(mat_ptr[index] > res_ptr[i]) {
          res_ptr[i] = mat_ptr[index];
          pos_ptr[i] = j;
        }
    }
  }
#endif
  return res;
}

// when calling from map, use these functions to avoid function pointer issue
template <class T>
std::vector<std::pair<T, size_t>>
rmm_max_of_elements(rowmajor_matrix_local<T>& mat,
                    size_t start = 0) {
  return max_of_elements(mat, start);

}

template <class T>
std::vector<std::pair<T, size_t>>
rmm_min_of_elements(rowmajor_matrix_local<T>& mat,
                    size_t start = 0) {
  return min_of_elements(mat, start);

}

template <class T>
std::vector<std::pair<T, size_t>>
rmm_max_of_rows(rowmajor_matrix_local<T>& mat,
                size_t start_index = 0) {
  return max_of_rows(mat, start_index);
}

template <class T>
std::vector<std::pair<T, size_t>>
rmm_min_of_rows(rowmajor_matrix_local<T>& mat,
                size_t start_index = 0) {
  return min_of_rows(mat, start_index);
}

template <class T>
std::vector<T>
rmm_max_of_cols(rowmajor_matrix_local<T>& mat,
                std::vector<size_t>& pos) {
  return max_of_cols(mat, pos);
}

template <class T>
std::vector<T>
rmm_min_of_cols(rowmajor_matrix_local<T>& mat,
                std::vector<size_t>& pos) {
  return min_of_cols(mat, pos);
}

// get number of elements of local matrix
template <class LMATRIX>
size_t get_num_elements(LMATRIX& mat) { return mat.val.size(); }

// must be invoked with distributed MATRIX (rowmajor, colmajor or crs)
template <class MATRIX>
node_local<size_t>
get_start_indices(MATRIX& mat) {
  auto nrows = mat.get_local_num_rows();
  std::vector<size_t> sidx(nrows.size()); sidx[0] = 0;
  for(size_t i = 1; i < nrows.size(); ++i)
    sidx[i] = sidx[i - 1] + nrows[i - 1];
  return make_node_local_scatter(sidx);
}

// must be invoked with distributed MATRIX (rowmajor or crs) // not applicable for colmajor
template <class MATRIX>
node_local<size_t>
get_global_indices(MATRIX& mat) {
  auto nelem = mat.data.map(get_num_elements<typename MATRIX::local_mat_type>)
                  .gather();
  std::vector<size_t> sidx(nelem.size()); sidx[0] = 0;
  for(size_t i = 1; i < nelem.size(); ++i)
    sidx[i] = sidx[i - 1] + nelem[i-1];
  return make_node_local_scatter(sidx);
}

template <class T>
std::vector<std::pair<T, size_t>>
argmin_pair(rowmajor_matrix<T>& mat, 
            int axis = -1) {
  std::vector<std::pair<T, size_t>> ret;
  if(axis == 0) {
    auto start = get_start_indices(mat);
    auto mv = mat.data.map(rmm_min_of_rows<T>, start);
    ret = mv.reduce(vector_min_pair<T, size_t>);
  }
  else if(axis == 1) {
    auto p_local = make_node_local_allocate<std::vector<size_t>>();
    auto v = mat.data.map(rmm_min_of_cols<T>, p_local);
    auto pos = p_local.template moveto_dvector<size_t>().gather();
    auto val = v.template moveto_dvector<T>().gather();
    ret = make_key_value_pair(val, pos);
  }
  else {
    auto start = get_global_indices(mat);
    auto mv = mat.data.map(rmm_min_of_elements<T>, start);
    ret = mv.reduce(vector_min_pair<T, size_t>);
  }
  return ret;
}

template <class T>
std::vector<std::pair<T, size_t>>
argmax_pair(rowmajor_matrix<T>& mat, 
            int axis = -1) {
  std::vector<std::pair<T, size_t>> ret;
  if(axis == 0) {
    auto start = get_start_indices(mat);
    auto mv = mat.data.map(rmm_max_of_rows<T>, start);
    ret = mv.reduce(vector_max_pair<T, size_t>);
  }
  else if(axis == 1) {
    auto p_local = make_node_local_allocate<std::vector<size_t>>();
    auto v = mat.data.map(rmm_max_of_cols<T>, p_local);
    auto val = v.template moveto_dvector<T>().gather();
    auto pos = p_local.template moveto_dvector<size_t>().gather();
    ret = make_key_value_pair(val, pos);
  }
  else {
    auto start = get_global_indices(mat);
    auto mv = mat.data.map(rmm_max_of_elements<T>, start);
    ret = mv.reduce(vector_max_pair<T, size_t>);
  }
  return ret;
}

template <class T>
std::vector<std::pair<T, size_t>>
argmin_pair(rowmajor_matrix_local<T>& mat, 
            int axis = -1) {
  std::vector<std::pair<T, size_t>> ret;
  if (axis == 0) ret = min_of_rows(mat);
  else if (axis == 1) {
    std::vector<size_t> pos;
    auto v = min_of_cols(mat, pos);
    ret = make_key_value_pair(v, pos);
  }
  else ret = min_of_elements(mat);
  return ret;
}

template <class T>
std::vector<std::pair<T, size_t>>
argmax_pair(rowmajor_matrix_local<T>& mat,
            int axis = -1) {
  std::vector<std::pair<T, size_t>> ret;
  if (axis == 0) ret = max_of_rows(mat);
  else if (axis == 1) {
    std::vector<size_t> pos;
    auto v = max_of_cols(mat, pos);
    ret = make_key_value_pair(v, pos);
  }
  else ret = max_of_elements(mat);
  return ret;
}
       
template <class T>    
rowmajor_matrix_local<T> 
extract_cols(rowmajor_matrix_local<T>& mat, 
             size_t start, size_t end) {
  if(get_selfid() == 0) {  
    std::string msg1 = "start column index must be less than end column index";                     
    require(end > start, msg1);
    std::string msg2 = "given indices are out of range";                                               
    require(start >= 0 && end <= mat.local_num_col, msg2);
  }    
  auto ret_ncol = end - start;
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  rowmajor_matrix_local<T> ret(nrow, ret_ncol); 
  auto sptr = mat.val.data() + start;
  auto dptr = ret.val.data();
  auto c = 0;
  for(; c+7 < ret_ncol; c += 8) {    
    for(size_t r = 0; r < nrow; r++) { 
      dptr[r * ret_ncol + c] = sptr[r * ncol + c];    
      dptr[r * ret_ncol + c + 1] = sptr[r * ncol + c + 1];
      dptr[r * ret_ncol + c + 2] = sptr[r * ncol + c + 2];
      dptr[r * ret_ncol + c + 3] = sptr[r * ncol + c + 3];
      dptr[r * ret_ncol + c + 4] = sptr[r * ncol + c + 4];
      dptr[r * ret_ncol + c + 5] = sptr[r * ncol + c + 5];
      dptr[r * ret_ncol + c + 6] = sptr[r * ncol + c + 6];
      dptr[r * ret_ncol + c + 7] = sptr[r * ncol + c + 7];
    }
  }
  for(; c+3 < ret_ncol; c += 4) {    
    for(size_t r=0; r < nrow; r++) { 
      dptr[r * ret_ncol + c] = sptr[r * ncol + c];    
      dptr[r * ret_ncol + c + 1] = sptr[r * ncol + c + 1];
      dptr[r * ret_ncol + c + 2] = sptr[r * ncol + c + 2];
      dptr[r * ret_ncol + c + 3] = sptr[r * ncol + c + 3];
    }
  }
  for(; c+1 < ret_ncol; c += 2) {    
    for(size_t r=0; r < nrow; r++) { 
      dptr[r * ret_ncol + c] = sptr[r * ncol + c];    
      dptr[r * ret_ncol + c + 1] = sptr[r * ncol + c + 1];
    }
  }
  for(; c < ret_ncol; c++) {    
    for(size_t r=0; r < nrow; r++) { 
      dptr[r * ret_ncol + c] = sptr[r * ncol + c];
    }
  }

  return ret;  
}

template <class T>    
rowmajor_matrix_local<T> 
rmm_extract_cols(rowmajor_matrix_local<T>& mat, 
                 size_t start, size_t end) {
  return extract_cols(mat, start, end);  
}    

template <class T>    
rowmajor_matrix<T> 
extract_cols(rowmajor_matrix<T>& mat, 
             size_t start, size_t end) {   
  rowmajor_matrix<T> ret(mat.data.map(rmm_extract_cols<T>, 
                         broadcast(start), broadcast(end)));
  ret.num_row = mat.num_row;
  ret.num_col = end - start; 
  return ret;
} 
    
template <class T>
rowmajor_matrix_local<T>
rmm_extract_rows(rowmajor_matrix_local<T>& mat,
                 const std::vector<size_t>& rowids) {
  return extract_rows(mat, rowids);
}

inline size_t count_sizes(std::vector<size_t>& vec) { return vec.size(); }

template <class MATRIX, class I>
lvec<I> align_global_index(MATRIX& mat,
                           const std::vector<I>& gidx) { // assumes to be sorted
  auto nrows = mat.get_local_num_rows();
  auto nproc = nrows.size();
  std::vector<size_t> myst(nproc); myst[0] = 0;
  auto mystp = myst.data();
  for(size_t i = 1; i < nproc; ++i) mystp[i] = mystp[i - 1] + nrows[i - 1];

  std::vector<I> search_target(nproc); auto sptr = search_target.data();
  for(size_t i = 0; i < nproc - 1; ++i) sptr[i] = mystp[i + 1]; // next-start
  sptr[nproc - 1] = mat.num_row;

  std::vector<size_t> lb(nproc + 1); lb[0] = 0;
  auto lbptr = lb.data();
  auto gptr = gidx.data();
  lower_bound(gptr, gidx.size(), sptr, nproc, lbptr + 1);

  std::vector<std::vector<I>> ret(nproc);
  for(size_t i = 0; i < nproc; ++i) {
    auto sz = lbptr[i + 1] - lbptr[i];
    ret[i].resize(sz);
    auto tptr = ret[i].data();
    for(size_t j = 0; j < sz; ++j) tptr[j] = gptr[j] - mystp[i];
    gptr += sz; // move gptr pointer 'sz' step ahead...
  }
  return make_node_local_scatter(ret);
}

template <class T>
rowmajor_matrix<T>
extract_rows(rowmajor_matrix<T>& mat,
             lvec<size_t>& rowids,
             bool need_transpose = false) {
  rowmajor_matrix<T> ret = mat.data.map(rmm_extract_rows<T>, rowids); 
  ret.num_row = rowids.map(count_sizes).reduce(add<size_t>);
  ret.num_col = mat.num_col;
  return need_transpose ? ret.transpose() : ret;
}

template <class T>
rowmajor_matrix<T>
extract_rows(rowmajor_matrix<T>& mat,
             const std::vector<size_t>& rowids, // must be sorted
             bool need_transpose = false) {
  auto loc_ids = align_global_index(mat, rowids);
  return extract_rows(mat, loc_ids, need_transpose);
}

// vectorization on left-right direction (ncol > nrow)
template <class T>
std::vector<T>
rmm_logsumexp_vector_x_axis(rowmajor_matrix_local<T>& mat, 
                            int axis = -1) {
  std::vector<T> ret;
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  auto mvalp = mat.val.data();
  if (axis == -1) ret.push_back(vector_logsumexp(mat.val));
  else if (axis == 1) {
    ret.resize(nrow); auto rptr = ret.data();
    for(size_t i = 0; i < nrow; ++i) 
      rptr[i] = vector_logsumexp_impl(mvalp + i * ncol, ncol, 1);
  }
  else if (axis == 0) {
    ret.resize(ncol); auto rptr = ret.data();
    for(size_t i = 0; i < ncol; ++i) 
      rptr[i] = vector_logsumexp_impl(mvalp + i, nrow, ncol);
  }
  else REPORT_ERROR(USER_ERROR, "logsumexp: supported axis is 0, 1 or -1\n");
  return ret;
}    

// vectorization on top-bottom direction (nrow > ncol)
template <class T>
std::vector<T>
rmm_logsumexp_vector_y_axis(rowmajor_matrix_local<T>& mat, 
                            int axis = -1) {
  std::vector<T> ret;
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  auto mvalp = mat.val.data();
  if (axis == -1) ret.push_back(vector_logsumexp(mat.val));
  else {
    auto max = matrix_amax(mat, axis); auto maxp = max.data();
    if (axis == 1) {
      ret.resize(nrow, 0); auto rptr = ret.data();
#pragma _NEC nointerchange
      for(size_t j = 0; j < ncol; ++j) {
        for(size_t i = 0; i < nrow; ++i) {
          rptr[i] += exp(mvalp[i * ncol + j] - maxp[i]);
        }
      }
      for(size_t i = 0; i < nrow; ++i) rptr[i] = maxp[i] + log(rptr[i]);
    }
    else if (axis == 0) {
      ret.resize(ncol); auto rptr = ret.data();
#pragma _NEC nointerchange
      for(size_t j = 0; j < ncol; ++j) {
        T lse = 0;
        for(size_t i = 0; i < nrow; ++i) {
          lse += exp(mvalp[i * ncol + j] - maxp[j]);
        }
        rptr[j] = maxp[j] + log(lse);
      }
    }
    else REPORT_ERROR(USER_ERROR, "logsumexp: supported axis is 0, 1 or -1\n");
  }
  return ret;
}    

template <class T>
std::vector<T>
logsumexp(rowmajor_matrix_local<T>& mat, 
          int axis = -1) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  if(nrow > ncol) return rmm_logsumexp_vector_y_axis(mat, axis);
  else            return rmm_logsumexp_vector_x_axis(mat, axis);
}

}
#endif
