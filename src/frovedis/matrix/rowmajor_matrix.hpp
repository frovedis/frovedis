#ifndef ROWMAJOR_MATRIX_HPP
#define ROWMAJOR_MATRIX_HPP

#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../core/dvector.hpp"
#include "../core/mpihelper.hpp"
#include "diag_matrix.hpp"

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
  void debug_print();
  void save(const std::string& file) {
    std::ofstream str(file.c_str());
    str << *this;
  }
  void savebinary(const std::string&);
  rowmajor_matrix_local<T> transpose() const;
  rowmajor_matrix_local<T> pow_val(T exponent) const;
  node_local<rowmajor_matrix_local<T>> broadcast(); // for performance
  std::vector<T> val;
  size_t local_num_row;
  size_t local_num_col;

  SERIALIZE(val, local_num_row, local_num_col)
};

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
void rowmajor_matrix_local<T>::debug_print() {
  std::cout << "node = " << get_selfid()
            << ", local_num_row = " << local_num_row
            << ", local_num_col = " << local_num_col
            << ", val = ";
  for(auto i: val){ std::cout << i << " "; }
  std::cout << std::endl;
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

template <class T>
rowmajor_matrix_local<T>
make_rowmajor_matrix_local_load(const std::string& file) {
  std::ifstream str(file.c_str());
  return make_rowmajor_matrix_local_readstream<T>(str);
}

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
  for(size_t j = 0; j < jmax; j++) {
    for(size_t i = 0; i < imax; i++) {
        cp[i * jmax + j] = ap[i * jmax + j] * bp[j];
    }
  }
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
  for(size_t i = 0; i < imax; i++) {
    for(size_t j = 0; j < jmax; j++) {
        cp[i * jmax + j] = ap[i] * bp[i * jmax + j];
    }
  }
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
  std::ifstream numstr;
  numstr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  numstr.open(numsfile.c_str());
  size_t num_row, num_col;
  numstr >> num_row >> num_col;
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
  std::ofstream numstr;
  numstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  numstr.open(numsfile.c_str());
  numstr << local_num_row << "\n" << local_num_col << std::endl;
  auto tmp = make_dvector_scatter(val);
  tmp.savebinary(valfile);
}

/*
  distributed version: rowmajor_matrix
 */

template <class T>
void call_debug_print(T& m) {
  m.debug_print();
}

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

  void debug_print() {data.mapv(call_debug_print<rowmajor_matrix_local<T>>);}
  void save(const std::string& file) {
    std::ofstream str(file.c_str());
    str << *this;
  }
  void savebinary(const std::string&);
  frovedis::node_local<rowmajor_matrix_local<T>> data;
  size_t num_row;
  size_t num_col;
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
rowmajor_matrix<T> make_rowmajor_matrix_load(const std::string& input) {
  auto dvec = make_dvector_loadline(input);
  dvec.align_block();
  rowmajor_matrix<T>
    ret(dvec.moveto_node_local().map(make_rowmajor_matrix_local_vectorstring<T>));
  ret.num_row = ret.data.map(rowmajor_get_local_num_row<T>).reduce(add<size_t>);
  ret.num_col = ret.data.map(rowmajor_get_local_num_col<T>).gather()[0];
  return ret;
}

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
  std::ifstream numstr;
  numstr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  numstr.open(numsfile.c_str());
  size_t num_row, num_col;
  numstr >> num_row >> num_col;
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
  for(size_t i = 0; i < sizes.size(); i++) {
    sizes[i] = rows[i] * num_col;
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
  std::ofstream numstr;
  numstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  numstr.open(numsfile.c_str());
  numstr << num_row << "\n" << num_col << std::endl;
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

std::vector<size_t> get_block_sizes(size_t num_row);
std::vector<size_t> get_block_sizes(size_t num_row, size_t wsize);

template <class T>
rowmajor_matrix<T> make_rowmajor_matrix_scatter(rowmajor_matrix_local<T>& m,
                                                std::vector<size_t>& rows) {
  std::vector<size_t> sizes(get_nodesize());
  for(size_t i = 0; i < sizes.size(); i++) {
    sizes[i] = rows[i] * m.local_num_col;
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
get_scattered_vectors(std::vector<T>& vec, 
                      size_t nrow, size_t ncol, size_t wsize) {
  auto rows = get_block_sizes(nrow, wsize);
  std::vector<size_t> sizevec(wsize);
  for(size_t i = 0; i < wsize; i++) {
    sizevec[i] = rows[i] * ncol;
  }
  std::vector<std::vector<T>> src2(wsize);
  const T* srcp = &vec[0];
  for(size_t i = 0; i < wsize; i++) {
    src2[i].resize(sizevec[i]);
    for(size_t j = 0; j < sizevec[i]; j++) {
      src2[i][j] = srcp[j];
    }
    srcp += sizevec[i];
  }
  return src2;
}

template <class T>
std::vector<std::vector<T>>
get_scattered_vectors(std::vector<T>& vec, 
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
rowmajor_matrix_local<T> rowmajor_matrix<T>::gather() {
  auto locals = data.gather();
  size_t num_row = 0;
  for(auto& i: locals) num_row += i.local_num_row;
  size_t num_col = locals[0].local_num_col;
  rowmajor_matrix_local<T> ret(num_row, num_col);
  T* datap = &ret.val[0];
  size_t global_row = 0;
  for(size_t n = 0; n < locals.size(); n++) {
    for(size_t local_row = 0; local_row < locals[n].local_num_row;
        local_row++, global_row++) {
      T* local_datap = &locals[n].val[0];
      for(size_t c = 0; c < num_col; c++) {
        datap[global_row * num_col + c] = local_datap[local_row * num_col + c];
      }
    }
  }
  return ret;
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
    for(size_t i = 0; i < node_size; i++) {
      send_size[i] = rows[i] * m.local_num_col;
    }
    std::vector<size_t> recv_size(node_size);
    MPI_Alltoall(&send_size[0], sizeof(size_t), MPI_CHAR,
                 &recv_size[0], sizeof(size_t), MPI_CHAR, MPI_COMM_WORLD);
    size_t total_size = 0;
    for(size_t i = 0; i < node_size; i++) {
      total_size += recv_size[i];
    }
    std::vector<size_t> send_displ(node_size);
    std::vector<size_t> recv_displ(node_size);
    send_displ[0] = 0; recv_displ[0] = 0;
    for(size_t i = 1; i < node_size; i++) {
      send_displ[i] = send_displ[i-1] + send_size[i-1];
      recv_displ[i] = recv_displ[i-1] + recv_size[i-1];
    }
    std::vector<T> tmpval(ret.val.size());
    large_alltoallv(sizeof(T),
                    reinterpret_cast<char*>(&m.val[0]), send_size, send_displ, 
                    reinterpret_cast<char*>(&tmpval[0]), recv_size, recv_displ, 
                    MPI_COMM_WORLD);
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
  for(size_t i = 0; i < dst.size(); i++) dsttotal += dst[i];
  for(size_t i = 0; i < mysizes.size(); i++) selftotal += mysizes[i];
  if(dsttotal != selftotal)
    throw std::runtime_error
      ("align_as: total size of src and dst does not match");
  bool is_same = true;
  for(size_t i = 0; i < dst.size(); i++) {
    if(dst[i] != mysizes[i]) {
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

}
#endif
