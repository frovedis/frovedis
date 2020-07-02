#ifndef COLMAJOR_MATRIX_HPP
#define COLMAJOR_MATRIX_HPP

#include "rowmajor_matrix.hpp"
#include "../core/exceptions.hpp"

namespace frovedis {

template <class T>
struct colmajor_matrix_local {
  colmajor_matrix_local(){}
  colmajor_matrix_local(size_t r, size_t c)
    : local_num_row(r), local_num_col(c) {
    val.resize(r*c);
  }
  colmajor_matrix_local(colmajor_matrix_local<T>&& m) {
    val.swap(m.val);
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
  }
  colmajor_matrix_local(const colmajor_matrix_local<T>& m) {
    val = m.val;
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
  }
  colmajor_matrix_local(const rowmajor_matrix_local<T>& m) {
    if(m.local_num_col > 1) {
      auto tmp = m.transpose();
      val.swap(tmp.val);
    } else {
      val = m.val; // 'm' is lvalue vector (so just copied)
    }
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
  }
  colmajor_matrix_local(rowmajor_matrix_local<T>&& m) {
    if(m.local_num_col > 1) {
      auto tmp = m.transpose();
      val.swap(tmp.val);
    } else {
      val.swap(m.val); // 'm' is rvalue vector (so just moved)
    }
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
  }
  colmajor_matrix_local<T>&
  operator=(const colmajor_matrix_local<T>& m) {
    val = m.val;
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
    return *this;
  }
  colmajor_matrix_local<T>&
  operator=(colmajor_matrix_local<T>&& m) {
    val.swap(m.val);
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
    return *this;
  }
  rowmajor_matrix_local<T> moveto_rowmajor() {
    if(local_num_col > 1) 
      REPORT_ERROR(USER_ERROR,"Invalid move operation requested!\n"); 
    rowmajor_matrix_local<T> ret(std::move(val));
    return ret;
  }
  rowmajor_matrix_local<T> to_rowmajor() {
    rowmajor_matrix_local<T> ret;
    if(local_num_col > 1) {
      auto tmp = transpose();
      ret.val.swap(tmp.val);
    } else {
      ret.val = val;
    }
    ret.local_num_row = local_num_row;
    ret.local_num_col = local_num_col;
    return ret;
  }

  void clear() {
    std::vector<T> tmpval; tmpval.swap(val);
    local_num_row = 0;
    local_num_col = 0;
  }
  void debug_print(size_t n = 0);
  size_t get_nrows() { return local_num_row; }
  size_t get_ncols() { return local_num_col; }
  colmajor_matrix_local<T> transpose() const;
  node_local<colmajor_matrix_local<T>> broadcast(); // for performance
  std::vector<T> val;
  size_t local_num_row;
  size_t local_num_col;

  SERIALIZE(val, local_num_row, local_num_col)
};

template <class T>
struct colmajor_matrix_broadcast_helper {
  colmajor_matrix_broadcast_helper() {}
  colmajor_matrix_broadcast_helper(size_t local_num_row,
                                   size_t local_num_col) :
    local_num_row(local_num_row), local_num_col(local_num_col) {}
  colmajor_matrix_local<T> operator()(std::vector<T>& v) {
    colmajor_matrix_local<T> ret;
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
node_local<colmajor_matrix_local<T>> colmajor_matrix_local<T>::broadcast() {
  auto bval = make_node_local_broadcast(val);
  return bval.template map<colmajor_matrix_local<T>>
    (colmajor_matrix_broadcast_helper<T>(local_num_row, local_num_col));
}

template <class T>
void colmajor_matrix_local<T>::debug_print(size_t n) {
  std::cout << "node = " << get_selfid()
            << ", local_num_row = " << local_num_row
            << ", local_num_col = " << local_num_col
            << ", val = ";
  debug_print_vector(val, n);
}

/*
template <class T>
colmajor_matrix_local<T> operator*(const colmajor_matrix_local<T>& a,
                                   const colmajor_matrix_local<T>& b) {
  if(a.local_num_col != b.local_num_row)
    throw std::runtime_error("invalid size for matrix multiplication");
  size_t imax = a.local_num_row;
  size_t jmax = b.local_num_col;
  size_t kmax = a.local_num_col; // == b.local_num_row
  colmajor_matrix_local<T> c(imax, jmax);
  const T* ap = &a.val[0];
  const T* bp = &b.val[0];
  T* cp = &c.val[0];
  // let the SX compiler detect matmul
  for(size_t i = 0; i < imax; i++) {
    for(size_t j = 0; j < jmax; j++) {
      for(size_t k = 0; k < kmax; k++) {
        //cp[i][j] += ap[i][k] * bp[k][j];
        cp[i + imax * j] += ap[i + imax * k] * bp[k + kmax * j];
      }
    }
  }
  return c;
}
*/

template <class T>
colmajor_matrix_local<T> operator*(const colmajor_matrix_local<T>& a,
                                   const diag_matrix_local<T>& b) {
  if(a.local_num_col != b.local_num())
    throw std::runtime_error("invalid size for matrix multiplication");
  size_t imax = a.local_num_row;
  size_t jmax = a.local_num_col; // == b.local_num_row
  colmajor_matrix_local<T> c(imax, jmax);
  const T* ap = &a.val[0];
  const T* bp = &b.val[0];
  T* cp = &c.val[0];
  for(size_t j = 0; j < jmax; j++) {
    for(size_t i = 0; i < imax; i++) {
        cp[i + imax * j] = ap[i + imax * j] * bp[j];
    }
  }
  return c;
}

template <class T>
colmajor_matrix_local<T> colmajor_matrix_local<T>::transpose() const {
  colmajor_matrix_local<T> ret(local_num_col, local_num_row);
  T* retp = &ret.val[0];
  const T* valp = &val[0];
  for(size_t i = 0; i < local_num_row; i++) {
    for(size_t j = 0; j < local_num_col; j++) {
      retp[j + local_num_col * i] = valp[i + local_num_row * j];
    }
  }
  return ret;
}

/*
  distributed version
 */

template <class T>
rowmajor_matrix_local<T> 
copy_col_to_rowmajor(colmajor_matrix_local<T>& m) {
  return m.to_rowmajor();
}

template <class T>
rowmajor_matrix_local<T> 
move_col_to_rowmajor(colmajor_matrix_local<T>& m) {
  return m.moveto_rowmajor();
}

template <class T>
colmajor_matrix_local<T> 
copy_row_to_colmajor(rowmajor_matrix_local<T>& m) {
  return colmajor_matrix_local<T>(m);
}

template <class T>
colmajor_matrix_local<T> 
move_row_to_colmajor(rowmajor_matrix_local<T>& m) {
  return colmajor_matrix_local<T>(std::move(m));
}

template <class T>
void colmajor_clear_helper(colmajor_matrix_local<T>& mat) {mat.clear();}

template <class T>
size_t cmm_get_local_num_row(colmajor_matrix_local<T>& m) {
  return m.local_num_row;
}

template <class T>
struct colmajor_matrix {
  colmajor_matrix(){}
  colmajor_matrix(frovedis::node_local<colmajor_matrix_local<T>>&& d) :
    data(std::move(d)) {}
  colmajor_matrix(const rowmajor_matrix<T>& m) {
    // m need to be const but map does not support const!
    auto& m2 = const_cast<rowmajor_matrix<T>&>(m);
    data = std::move(m2.data.map(copy_row_to_colmajor<T>));
    set_num(m.num_row, m.num_col);
  }
  colmajor_matrix(rowmajor_matrix<T>&& m) {
    data = std::move(m.data.map(move_row_to_colmajor<T>));
    set_num(m.num_row, m.num_col);
  }
  rowmajor_matrix<T> to_rowmajor() {
    rowmajor_matrix<T> ret(data.map(copy_col_to_rowmajor<T>));
    ret.set_num(num_row, num_col);
    return ret;
  }
  rowmajor_matrix<T> moveto_rowmajor() {
    if(num_col > 1) REPORT_ERROR(USER_ERROR,"Invalid move operation requested!\n");
    rowmajor_matrix<T> ret(data.map(move_col_to_rowmajor<T>));
    ret.set_num(num_row, num_col);
    return ret;
  }
  void save(const std::string& path) {
    to_rowmajor().save(path);
  }
  void savebinary(const std::string& path) {
    to_rowmajor().savebinary(path);
  }
  void debug_print(size_t n = 0) {
    std::cout << "num_row = " << num_row
              << ", num_col = " << num_col << std::endl;
    auto g = data.gather();
    for(size_t i = 0; i < g.size(); i++) {
      std::cout << "node " << i << std::endl;
      g[i].debug_print(n);
    }
  }
  void set_num(size_t r, size_t c) {
    num_row = r; num_col = c;
  }
  void clear() {
    data.mapv(colmajor_clear_helper<T>);
    num_row = 0;
    num_col = 0;
  }
  std::vector<size_t> get_local_num_rows() {
    return data.map(cmm_get_local_num_row<T>).gather();
  }
  frovedis::node_local<colmajor_matrix_local<T>> data;
  size_t num_row;
  size_t num_col;
};

template <class T>
colmajor_matrix<T> operator*(const colmajor_matrix<T>& aa,
                             const diag_matrix_local<T>& b) {
  colmajor_matrix<T>& a = const_cast<colmajor_matrix<T>&>(aa);
  if(a.num_col != b.local_num())
    throw std::runtime_error("invalid size for matrix multiplication");
  auto bb = broadcast(b);
  auto retdata = a.data.map(+[](colmajor_matrix_local<T>& al,
                                diag_matrix_local<T>& b){return al * b;}, bb);
  colmajor_matrix<T> ret(std::move(retdata));
  ret.num_row = a.num_row;
  ret.num_col = a.num_col;
  return ret;
}

template <class T>
std::vector<T>
sum_of_rows(colmajor_matrix_local<T>& mat) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  std::vector<T> ret(ncol, 0);
  auto retp = ret.data();
  auto mvalp = mat.val.data();
  for (size_t i = 0; i < ncol; ++i) {
    for (size_t j = 0; j < nrow; ++j) retp[i] += mvalp[i * nrow + j];
  }
  return ret;
}

template <class T>
std::vector<T>
squared_sum_of_rows(colmajor_matrix_local<T>& mat) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  std::vector<T> ret(ncol, 0);
  auto retp = ret.data();
  auto mvalp = mat.val.data();
  for (size_t i = 0; i < ncol; ++i) {
    for (size_t j = 0; j < nrow; ++j) {
      retp[i] += (mvalp[i * nrow + j] * mvalp[i * nrow + j]);
    }
  }
  return ret;
}

template <class T>
void colmajor_sub_vector_row(colmajor_matrix_local<T>& m, std::vector<T>& v) {
  size_t ncol = m.local_num_col;
  size_t nrow = m.local_num_row;
  if(ncol != v.size())
    throw std::runtime_error("sub_vector_row: size mismatch");
  auto valp = m.val.data();
  auto vp = v.data();
  for (size_t j = 0; j < ncol; j++) {
    for(size_t i = 0; i < nrow; i++) valp[j * nrow + i] -= vp[j];
  }
}

template <class T>
void colmajor_add_vector_row(colmajor_matrix_local<T>& m, std::vector<T>& v) {
  size_t ncol = m.local_num_col;
  size_t nrow = m.local_num_row;
  if(ncol != v.size())
    throw std::runtime_error("add_vector_row: size mismatch");
  auto valp = m.val.data();
  auto vp = v.data();
  for (size_t j = 0; j < ncol; j++) {
    for(size_t i = 0; i < nrow; i++) valp[j * nrow + i] += vp[j];
  }
}

template <class T>
void colmajor_mul_vector_row(colmajor_matrix_local<T>& m, std::vector<T>& v) {
  size_t ncol = m.local_num_col;
  size_t nrow = m.local_num_row;
  if(ncol != v.size())
    throw std::runtime_error("mul_vector_row: size mismatch");
  auto valp = m.val.data();
  auto vp = v.data();
  for (size_t j = 0; j < ncol; j++) {
    for(size_t i = 0; i < nrow; i++) valp[j * nrow + i] *= vp[j];
  }
}

template <class T>
void scale_matrix(colmajor_matrix<T>& mat, std::vector<T>& vec) {
  mat.data.mapv(colmajor_mul_vector_row<T>, broadcast(vec));
} 

// TODO: Add axis parameter to support row-wise mean
template <class T>
std::vector<T>
compute_mean(colmajor_matrix<T>& mat) { // column-wise mean
  if(mat.num_row  == 0)
    throw std::runtime_error("matrix with ZERO rows for mean computation!");
  auto ret = mat.data.map(+[](colmajor_matrix_local<T>& m)
                       {return sum_of_rows(m);}).vector_sum();
  T to_mul = static_cast<T>(1)/static_cast<T>(mat.num_row); // for performance
  auto retp = ret.data();
  for(size_t i = 0; i < ret.size(); ++i) retp[i] *= to_mul; // average
  return ret;
}

template <class T>
void centerize(colmajor_matrix<T>& mat, std::vector<T>& mean) {
  if(mat.num_row < 2)
    throw std::runtime_error("cannot centerize if number of row is 0 or 1");
  mat.data.mapv(colmajor_sub_vector_row<T>, broadcast(mean));
}

template <class T>
void centerize(colmajor_matrix<T>& mat) {
  auto mean = compute_mean(mat, 0); // column-wise mean
  centerize(mat, mean);
}

template <class T>
void decenterize(colmajor_matrix<T>& mat, std::vector<T>& mean) {
  if(mat.num_row < 2)
    throw std::runtime_error("cannot decenterize if number of row is 0 or 1");
  mat.data.mapv(colmajor_add_vector_row<T>, broadcast(mean));
}

template <class T>
std::vector<T>
compute_stddev(colmajor_matrix<T>& mat,
               std::vector<T>& mean,
               bool sample_stddev = true) {
  if(mat.num_row < 2)
    throw std::runtime_error("cannot compute stddev if number of row is 0 or 1");
  centerize(mat, mean); // m = m - mean
  auto ret = mat.data.map(+[](colmajor_matrix_local<T>& m)
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
compute_stddev(colmajor_matrix<T>& mat, bool sample_stddev = true) {
  auto mean = compute_mean(mat, 0); // column-wise mean
  return compute_stddev(mat, mean, sample_stddev);
}

template <class T>
void standardize(colmajor_matrix<T>& mat,
                 std::vector<T>& mean, 
                 bool sample_stddev = true) {
  if(mat.num_row < 2)
    throw std::runtime_error("cannot standardize if number of row is 0 or 1");
  centerize(mat, mean);
  auto tmp = mat.data.map(+[](colmajor_matrix_local<T>& m)
                         {return squared_sum_of_rows(m);}).vector_sum();
  auto tmpp = tmp.data();
  T to_div;
  if(sample_stddev) to_div = static_cast<T>(mat.num_row - 1);
  else to_div = static_cast<T>(mat.num_row);
  for(size_t i = 0; i < mat.num_col; ++i) {
    if(tmpp[i] == 0) tmpp[i] = 1.0; // data is zero so can be anything
    tmpp[i] = sqrt(to_div / tmpp[i]);
  }
  mat.data.mapv(colmajor_mul_vector_row<T>, broadcast(tmp));
}

template <class T>
void standardize(colmajor_matrix<T>& mat, bool sample_stddev = true) {
  auto mean = compute_mean(mat); // column-wise mean
  standardize(mat, mean, sample_stddev);
}

}
#endif
