#ifndef _CLUSTERING_COMMON_
#define _CLUSTERING_COMMON_

#include <math.h>
#include <frovedis/matrix/blas_wrapper.hpp>

namespace frovedis {

//x: index of base point, y: index of target point to be swapped
template <class T>
void swap_vec_elem(std::vector<T>& vec,
                   size_t x, size_t y){
  auto tmp = vec[x];
  vec[x] = vec[y];
  vec[y] = tmp;
}

template <class T>
std::vector<T> get_block(T sz) { 
  T nodesize = static_cast<T>(get_nodesize());
  std::vector<T> block_size(nodesize);
  T each = ceil_div(sz, nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    if(sz > each) {
      block_size[i] = each;
      sz -= each;
    } else {
      block_size[i] = sz;
      break;
    }
  }
  return block_size;
}

template <class T>
rowmajor_matrix_local<T>
get_global_data(rowmajor_matrix<T>& mat) {
  auto g_data = mat.data.gather();
  auto nrows = g_data[0].local_num_row;
  auto ncols = g_data[0].local_num_col;
  for(size_t i=1; i<g_data.size(); ++i) nrows += g_data[i].local_num_row;
  rowmajor_matrix_local<T> ret(nrows, ncols);
  auto rptr = ret.val.data();
  size_t k = 0;
  for(size_t i=0; i<g_data.size(); ++i) {
     auto temp = g_data[i].val;
     auto dptr = temp.data();
     for(size_t j=0; j<temp.size(); ++j)  rptr[k+j] = dptr[j];
     k += temp.size();
  }
  return ret;
}

template <class T>
std::vector<T>
alloc_vector(size_t size, bool incremental = false) {
  std::vector<T> ret(size, 0);
  auto rptr = ret.data();
  if(incremental) for(size_t i = 0; i < size; ++i) rptr[i] = i;
  return ret;
}


// construction of distance matrix in distributed way using gemm
template <class T>
rowmajor_matrix_local<T>
construct_distance_matrix_helper(rowmajor_matrix_local<T>& mat,
                                 std::vector<T>& sum_sq_col,
                                 size_t myst, size_t mysize,
                                 bool is_sq_euclidian = false) {
  auto nrow = mat.local_num_row;
  // returning empty matrix for process not owning any data
  if(!mysize) return rowmajor_matrix_local<T>(0,nrow); 
  auto myend = myst + mysize - 1;
  sum_sq_col = squared_sum_of_cols(mat); // A^2 or ^B2
  auto twice_ab = mult_sliceA_trans_sliceB<T>(mat,myst,myend,0,nrow-1,2.0,1.0);
  T *a2_ptr = sum_sq_col.data(); // a2_ptr
  T *twoab_ptr = twice_ab.val.data();
  auto dm_nrow = twice_ab.local_num_row;
  auto dm_ncol = twice_ab.local_num_col;

  // computing the distance using (sqrt(A^2 + B^2 - 2AB))
  if (is_sq_euclidian) { 
    for(size_t i = 0; i < dm_nrow; ++i) {
      for(size_t j = 0; j < dm_ncol; ++j) { // dm_ncol >> dm_nrow (as for distance 2AB)
        auto tmp = twoab_ptr[i*dm_ncol+j];
        twoab_ptr[i*dm_ncol+j] = a2_ptr[myst+i] + a2_ptr[j] - tmp;
      }
    }
  }
  else {
    for(size_t i = 0; i < dm_nrow; ++i) {
      for(size_t j = 0; j < dm_ncol; ++j) { // dm_ncol >> dm_nrow (as for distance 2AB)
        auto tmp = twoab_ptr[i*dm_ncol+j];
        twoab_ptr[i*dm_ncol+j] = std::sqrt(a2_ptr[myst+i] + a2_ptr[j] - tmp);
      }
    }
  }
  return twice_ab; // in-place updated with distance
}


template <class T>
rowmajor_matrix<T>
construct_distance_matrix(node_local<rowmajor_matrix_local<T>>& mat,
                          lvec<T>& sum_sq_col,
                          size_t nrow,
                          bool is_sq_euclidian = false) {
  // load distribution
  auto nrows = get_block(nrow);
  std::vector<size_t> sidx(nrows.size(),0);
  for(size_t i=1; i<nrows.size(); ++i) sidx[i] = sidx[i-1] + nrows[i-1];
  auto myst = make_node_local_scatter(sidx);
  auto mysz = make_node_local_scatter(nrows);
  rowmajor_matrix<T> dist_mat(mat.map(construct_distance_matrix_helper<T>,
                              sum_sq_col,myst,mysz,broadcast(is_sq_euclidian)));
  dist_mat.num_row = dist_mat.num_col = nrow;
  return dist_mat;
}

template <class T>
rowmajor_matrix<T>
construct_distance_matrix(rowmajor_matrix_local<T>& mat,
                          lvec<T>& sum_sq_col,
                          bool input_movable = false,
                          bool is_sq_euclidian = false) {
  auto nrow = mat.local_num_row;
  auto bdata = broadcast(mat);
  if(input_movable) mat.clear(); // clear rvalue input matrix, after broadcast
  return construct_distance_matrix(bdata, sum_sq_col, nrow, is_sq_euclidian);
}

template <class T>
rowmajor_matrix<T>
construct_distance_matrix(rowmajor_matrix_local<T>& mat,
                          bool input_movable = false,
                          bool is_sq_euclidian = false) {
  auto sum_sq_col = make_node_local_allocate<std::vector<T>>();
  return construct_distance_matrix(mat, sum_sq_col, input_movable, is_sq_euclidian);
}

// constructing only the upper-triangle of the symmetric distance matrix
// and store them in 1D vector
template <class T = size_t>
T get_distance_vector_size(T nrow, T ncol, T myst) {
  auto a = nrow * ncol;
  auto b = (nrow % 2 == 0) ? (nrow/2) * (nrow+1) : nrow * ((nrow+1)/2);
  auto c = myst * nrow;
  return a - b - c;
}

template <class T>
std::vector<T>
construct_condensed_distance_matrix_helper(rowmajor_matrix_local<T>& mat,
                                           size_t myst, size_t mysz,
                                           bool is_sq_euclidian = false) {
  if(!mysz) return std::vector<T>();
  auto nrow = mat.local_num_row;
  auto myend = myst + mysz - 1;
  auto sum_sq_col = squared_sum_of_cols(mat); // A^2 or ^B2
  //std::cout << "[rank: " << get_selfid() << "] myst: " << myst << ", myend: " << myend << std::endl;
  auto twice_ab = mult_sliceA_trans_sliceB<T>(mat,myst,myend,0,nrow-1,2.0,1.0);
  auto a2_ptr = sum_sq_col.data(); // a2_ptr
  auto twoab_ptr = twice_ab.val.data();

  auto dm_nrow = twice_ab.local_num_row;
  auto dm_ncol = twice_ab.local_num_col; // same as nrow
  auto size = get_distance_vector_size(dm_nrow, dm_ncol, myst);
  std::vector<T> ret(size); // contains a^2 + b^2 - 2ab
  size_t k = 0;

  for(size_t i = myst; i <= myend; ++i) {
    for(size_t j = i+1; j < dm_ncol; ++j) {
      auto local_i = i - myst;
      auto tmp = twoab_ptr[local_i*dm_ncol+j];
      if (is_sq_euclidian) ret[k++] = a2_ptr[i] + a2_ptr[j] - tmp;
      else                 ret[k++] = std::sqrt(a2_ptr[i] + a2_ptr[j] - tmp);
    }
  }
  return ret;
}

template <class T>
std::vector<T>
construct_condensed_distance_matrix(rowmajor_matrix_local<T>& mat) {
  auto nrow = mat.local_num_row;
  auto nrows = get_block(nrow);
  std::vector<size_t> sidx(nrows.size(),0);
  for(size_t i=1; i<nrows.size(); ++i) sidx[i] = sidx[i-1] + nrows[i-1];
  auto myst = make_node_local_scatter(sidx);
  auto mysz = make_node_local_scatter(nrows);
  auto bmat = broadcast(mat);
  return bmat.map(construct_condensed_distance_matrix_helper<T>, 
                  myst,mysz,broadcast(false))
             .template moveto_dvector<T>().gather();
}

// construction of affinity matrix in distributed way
// using rbf kernel exp(-gamma*pow(dist,2))
template <class T>
struct construct_affinity_matrix_helper {
  construct_affinity_matrix_helper() {}
  construct_affinity_matrix_helper(double gm): gamma(gm) {}
  // in-place conversion of distance matrix -> affinity matrix using rbf kernel
  void operator()(rowmajor_matrix_local<T>& dist_mat) {
    auto nrow = dist_mat.local_num_row;
    auto ncol = dist_mat.local_num_col;
    T* mptr = dist_mat.val.data();
    for(size_t i=0; i < nrow*ncol; ++i) {
      mptr[i] = exp(-gamma * mptr[i] * mptr[i]); // in-place update
    }
  }
  double gamma;
  SERIALIZE(gamma)
};


template <class T>
void construct_affinity_matrix_inplace(rowmajor_matrix<T>& mat,
                                       double gamma = 1.0) {
  mat.data.mapv(construct_affinity_matrix_helper<T>(gamma));
}

template <class T>
rowmajor_matrix<T>
construct_affinity_matrix(rowmajor_matrix<T>& mat,
                          double gamma = 1.0) {
  rowmajor_matrix<T> ret(mat);
  construct_affinity_matrix_inplace(ret,gamma);
  return ret;
}

template <class T>
std::vector<T>
sum_of_cols_without_diagonal(rowmajor_matrix_local<T>& aff_loc,
                             size_t myst) {
  auto nrow = aff_loc.local_num_row;
  auto ncol = aff_loc.local_num_col;
  std::vector<T> con_vec(nrow,0);
  auto cptr = con_vec.data();
  auto mptr = aff_loc.val.data();
  for(size_t i = 0; i < nrow; ++i){
    auto glob_i = i + myst;
    for(size_t j = 0; j < ncol; ++j){
      // high affinity value (1.0) in diagonal affect the floating point calculation
      if( j != glob_i) cptr[i] += mptr[i*ncol+j];
    }
  }
  return con_vec;
}

template <class T>
node_local<size_t> get_start_indices(rowmajor_matrix<T>& mat) {
  auto nrows = mat.data.map(rowmajor_get_local_num_row<T>).gather();
  std::vector<size_t> sidx(nrows.size(),0);
  for(size_t i=1; i<nrows.size(); ++i) sidx[i] = sidx[i-1] + nrows[i-1];
  return make_node_local_scatter(sidx);
}

template <class T>
lvec<T> construct_connectivity_diagonals(rowmajor_matrix<T>& mat,
                                         node_local<size_t>& myst) {
  return mat.data.map(sum_of_cols_without_diagonal<T>, myst);
}

template <class T>
void one_by_sqrt_inplace(std::vector<T>& conn_vec) {
  auto cptr = conn_vec.data();
  for(size_t i = 0; i < conn_vec.size(); ++i) cptr[i] = 1 / sqrt(cptr[i]);
}

// construction of laplace matrix from affinity matrix and degree matrix
// L = D - A
// normalized laplacian can be constructed with norm = true
template <class T>
struct construct_laplace_matrix_helper {
  construct_laplace_matrix_helper() {}
  construct_laplace_matrix_helper(bool norm): norm_laplacian(norm) {}
  // in-place conversion of affinity matrix -> lapalce matrix
  void operator()(rowmajor_matrix_local<T>& mat, 
                  std::vector<T>& conn_vec,
                  size_t myst) {
    auto nrow = mat.local_num_row;
    auto ncol = mat.local_num_col;
    auto mptr = mat.val.data();
    auto cptr = conn_vec.data();
    if(norm_laplacian) {
      for(size_t i = 0; i < nrow; ++i) {
        auto diag_j = i + myst; 
        for(size_t j = 0; j < ncol; ++j) {
          mptr[i*ncol+j] *= -1.0 * cptr[j] * cptr[diag_j]; 
        }
        mptr[i * ncol + diag_j] = 1.0; // taking care of diagonals
      }
    }
    else {
      for(size_t i = 0; i < nrow*ncol; ++i) mptr[i] *= -1.0; // -A 
      for(size_t i = 0; i < nrow; ++i) {
        auto diag_j = i + myst; 
        mptr[i * ncol + diag_j] = cptr[diag_j]; // taking care of diagonals
      }
    }
  }
  bool norm_laplacian;
  SERIALIZE(norm_laplacian)
};

template <class T>
void construct_laplace_matrix_inplace(rowmajor_matrix<T>& aff,
                                      lvec<T>& conn_vec,
                                      node_local<size_t>& myst,
                                      bool norm = false) {
  aff.data.mapv(construct_laplace_matrix_helper<T>(norm), 
                conn_vec, myst);
}

template <class T>
rowmajor_matrix<T>
construct_laplace_matrix(rowmajor_matrix<T>& aff,
                         lvec<T>& conn_vec,
                         node_local<size_t>& myst,
                         bool norm = false) {
  rowmajor_matrix<T> lap(aff);
  construct_laplace_matrix_inplace(lap,conn_vec,myst,norm);
  return lap;
}

template <class T>
void negate_inplace(rowmajor_matrix_local<T>& mat) {
    auto nrow = mat.local_num_row;
    auto ncol = mat.local_num_col;
    auto mptr = mat.val.data();
    for(size_t i=0; i < nrow*ncol; ++i) mptr[i] *= -1; 
}

template <class T>
rowmajor_matrix_local<T>
construct_embed_matrix_helper(rowmajor_matrix_local<T>& eig_vec,
                              std::vector<T>& conn_vec,
                              size_t myst,
                              bool norm = false ) {
  auto nrow = eig_vec.local_num_row;
  auto ncol = eig_vec.local_num_col;
  rowmajor_matrix_local<T> ret(nrow, ncol);
  auto mptr = eig_vec.val.data();
  auto rptr = ret.val.data();

  if (norm) {
    auto cptr = conn_vec.data();
    for(size_t i = 0; i < nrow; ++i) {
      for(size_t j = 0; j < ncol; ++j) {
        auto rev_j = ncol - 1 - j;
        rptr[i * ncol + j] = mptr[i * ncol + rev_j] * cptr[i + myst];
      }      
    }
  }
  else {
    for(size_t i = 0; i < nrow; ++i) {
      for(size_t j = 0; j < ncol; ++j) {
        auto rev_j = ncol - 1 - j;
        rptr[i * ncol + j] = mptr[i * ncol + rev_j];
      }      
    }
  }
  return ret;
}

template <class T>
rowmajor_matrix<T>
construct_embed_matrix(rowmajor_matrix<T>& eig_vec,
                       lvec<T>& conn_vec,
                       node_local<size_t>& myst,
                       bool norm = false ) {

  auto embed = eig_vec.data.map(construct_embed_matrix_helper<T>, 
                                conn_vec, myst, broadcast(norm));
  rowmajor_matrix<T> ret(std::move(embed));
  ret.num_row = eig_vec.num_row;
  ret.num_col = eig_vec.num_col;
  return ret;
}

} // end of namespace

#endif
