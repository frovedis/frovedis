#ifndef TRUNCATED_SVD
#define TRUNCATED_SVD

// reuse the definition
#include "sparse_svd.hpp"
#include "blas_wrapper.hpp"

namespace frovedis {

template <class T, class MATRIX_LOCAL>
struct calc_truncated_svd {
  calc_truncated_svd() {}
  calc_truncated_svd(int k) : k(k) {}
  void operator()(MATRIX_LOCAL& mat,
                  colmajor_matrix_local<T>& u,
                  diag_matrix_local<T>& s,
                  colmajor_matrix_local<T>& v,
                  MATRIX_LOCAL& trans_mat) {
    svd_mpi<T, MATRIX_LOCAL>(mat, trans_mat, u, s, v, k);
  }
  int k;
  SERIALIZE(k)
};

template <class MATRIX_LOCAL, class T>
MATRIX_LOCAL dense_trans_and_convert(rowmajor_matrix_local<T>& m) {
  return MATRIX_LOCAL(m.transpose());
}

template <class MATRIX, class MATRIX_LOCAL, class T>
void truncated_svd(rowmajor_matrix<T>& mat,
                   colmajor_matrix<T>& u,
                   diag_matrix_local<T>& s,
                   colmajor_matrix<T>& v,
                   int k,
                   bool allow_transpose = true) {
  auto stmp = make_node_local_allocate<diag_matrix_local<T>>();
  u.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  if(mat.num_col <= mat.num_row || allow_transpose == false) {
    frovedis::time_spent t(DEBUG);  
    auto local_trans = mat.data.map(dense_trans_and_convert<MATRIX_LOCAL,T>);
    t.show("local transpose and convert matrix format: ");
    MATRIX new_mat(mat);
    t.show("convert matrix format: ");
    new_mat.data.mapv(calc_truncated_svd<T, MATRIX_LOCAL>(k), 
                      u.data, stmp, v.data, local_trans);
  } else {
    frovedis::time_spent t(DEBUG);
    auto trans_mat = mat.transpose();
    t.show("transpose: ");
    auto local_trans = trans_mat.data.map(dense_trans_and_convert
                                          <MATRIX_LOCAL,T>);
    t.show("local transpose and convert matrix format: ");
    MATRIX new_trans_mat(trans_mat);
    t.show("convert matrix format: ");
    new_trans_mat.data.mapv(calc_truncated_svd<T, MATRIX_LOCAL>(k), 
                            v.data, stmp, u.data, local_trans);
  }
  s = *stmp.get_dvid().get_selfdata();
  u.set_num(mat.num_row, k);
  v.set_num(mat.num_col, k);
}

template <class MATRIX, class MATRIX_LOCAL, class T>
void truncated_svd(rowmajor_matrix<T>&& mat,
                   colmajor_matrix<T>& u,
                   diag_matrix_local<T>& s,
                   colmajor_matrix<T>& v,
                   int k,
                   bool allow_transpose = true) {
  auto stmp = make_node_local_allocate<diag_matrix_local<T>>();
  u.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  if(mat.num_col <= mat.num_row || allow_transpose == false) {
    frovedis::time_spent t(DEBUG);  
    auto local_trans = mat.data.map(dense_trans_and_convert
                                    <MATRIX_LOCAL, T>);
    t.show("local transpose and convert matrix format: ");
    MATRIX new_mat(mat);
    t.show("convert matrix format: ");
    new_mat.data.mapv(calc_truncated_svd<T, MATRIX_LOCAL>(k), 
                      u.data, stmp, v.data, local_trans);
  } else {
    frovedis::time_spent t(DEBUG);
    auto trans_mat = mat.transpose();
    mat.clear();
    t.show("transpose: ");
    auto local_trans = trans_mat.data.map(dense_trans_and_convert
                                          <MATRIX_LOCAL, T>);
    t.show("local transpose and convert matrix format: ");
    MATRIX new_trans_mat(trans_mat);
    t.show("convert matrix format: ");
    new_trans_mat.data.mapv(calc_truncated_svd<T, MATRIX_LOCAL>(k), 
                            v.data, stmp, u.data, local_trans);
  }
  s = *stmp.get_dvid().get_selfdata();
  u.set_num(mat.num_row, k);
  v.set_num(mat.num_col, k);
}

template <class T>
void truncated_svd(rowmajor_matrix<T>& mat,
                   colmajor_matrix<T>& u,
                   diag_matrix_local<T>& s,
                   colmajor_matrix<T>& v,
                   int k,
                   bool allow_transpose = true) {
  auto stmp = make_node_local_allocate<diag_matrix_local<T>>();
  u.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  if(mat.num_col <= mat.num_row || allow_transpose == false) {
    frovedis::time_spent t(DEBUG);
    auto local_trans = mat.data.map(rowmajor_matrix_call_transpose<T>);
    t.show("local transpose: ");
    mat.data.mapv(calc_truncated_svd<T,rowmajor_matrix_local<T>>(k),
                  u.data, stmp, v.data, local_trans);
  } else {
    frovedis::time_spent t(DEBUG);  
    auto trans_mat = mat.transpose();
    t.show("transpose: ");
    auto local_trans = trans_mat.data.map(rowmajor_matrix_call_transpose<T>);
    t.show("local transpose: ");
    trans_mat.data.mapv(calc_truncated_svd<T,rowmajor_matrix_local<T>>(k),
                        v.data, stmp, u.data, local_trans);
  }
  s = *stmp.get_dvid().get_selfdata();
  u.set_num(mat.num_row, k);
  v.set_num(mat.num_col, k);
}


}
#endif
