#ifndef _CLUSTERING_COMMON_
#define _CLUSTERING_COMMON_

#include <math.h>
#include <frovedis/matrix/blas_wrapper.hpp>

#define COMPUTE_ONLY_B2_FOR_PARTIAL_DISTANCE
//#define NEED_TRANS_FOR_MULT

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
get_global_data(rowmajor_matrix<T>& mat) { // mat.gather()
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

template <class T>
void display(std::vector<T>& vec) {
  for(auto i: vec) std::cout << i << " "; std::cout << std::endl;
}

template <class T>
rowmajor_matrix_local<T>
mult_a_with_trans_b (rowmajor_matrix_local<T>& a,
                     rowmajor_matrix_local<T>& b) {
    auto a_nrow = a.local_num_row;
    auto b_nrow = b.local_num_row;
    auto a_ncol = a.local_num_col;
    auto b_ncol = a.local_num_col;

    sliced_colmajor_matrix_local<T> sm1;
    sm1.ldm = a.local_num_col;
    sm1.data = a.val.data();
    sm1.local_num_row = a_ncol;
    sm1.local_num_col = a_nrow;

    sliced_colmajor_matrix_local<T> sm2;
    sm2.ldm = b.local_num_col;
    sm2.data = b.val.data();
    sm2.local_num_row = b_ncol;
    sm2.local_num_col = b_nrow;

    rowmajor_matrix_local<T> ret(a_nrow, b_nrow); // ret = a * trans(b)
    sliced_colmajor_matrix_local<T> sm3;
    sm3.ldm = b_nrow;
    sm3.data = ret.val.data();
    sm3.local_num_row = b_nrow;
    sm3.local_num_col = a_nrow;
    gemm<T>(sm2, sm1, sm3, 'T', 'N');
    return ret;
}

struct compute_dist {
  compute_dist() {}
  compute_dist(std::string& metric, bool is_trans_b,
               bool need_distance) {
    this->metric = metric;
    this->is_trans_b = is_trans_b;
    this->need_distance = need_distance;
  }

  template <class T>
  rowmajor_matrix_local<T>
  operator()(rowmajor_matrix_local<T>& a,
             rowmajor_matrix_local<T>& b) {
    if (!a.local_num_row || !b.local_num_row) return std::vector<T>();
    std::vector<T> a2, b2;
    const T *a2ptr, *b2ptr;
    a2ptr = b2ptr = NULL;
#ifdef COMPUTE_ONLY_B2_FOR_PARTIAL_DISTANCE
    if (need_distance) {
      a2 = squared_sum_of_cols(a);
      a2ptr = a2.data();
    }
    if (is_trans_b) b2 = squared_sum_of_rows(b); // b is transposed
    else b2 = squared_sum_of_cols(b); // b is not transposed
    b2ptr = b2.data();
#else
    a2 = squared_sum_of_cols(a);
    a2ptr = a2.data();
    if (need_distance) {
      if (is_trans_b) b2 = squared_sum_of_rows(b); // b is transposed
      else b2 = squared_sum_of_cols(b); // b is not transposed
      b2ptr = b2.data();
    }
#endif
    rowmajor_matrix_local<T> ab;
    if (is_trans_b) ab = a * b;
    else ab = mult_a_with_trans_b(a, b); // without physical transpose
    auto abptr = ab.val.data();
    auto nrow = ab.local_num_row;
    auto ncol = ab.local_num_col;
    if (metric == "euclidean" && need_distance) {
      if (nrow > ncol) {
#pragma _NEC nointerchange
        for(size_t j = 0; j < ncol; ++j) {
          for(size_t i = 0; i < nrow; ++i) {
            auto sq_dist = a2ptr[i] + b2ptr[j] - 2 * abptr[i*ncol+j];
            if (sq_dist > 0) abptr[i*ncol+j] = std::sqrt(sq_dist);
            else abptr[i*ncol+j] = 0;
          }
        }
      }
      else {
#pragma _NEC nointerchange
        for(size_t i = 0; i < nrow; ++i) {
          for(size_t j = 0; j < ncol; ++j) {
            auto sq_dist = a2ptr[i] + b2ptr[j] - 2 * abptr[i*ncol+j];
            if (sq_dist > 0) abptr[i*ncol+j] = std::sqrt(sq_dist);
            else abptr[i*ncol+j] = 0;
          }
        }
      }
    }
    else if (metric == "euclidean" && !need_distance) {
      if (nrow > ncol) {
#pragma _NEC nointerchange
        for(size_t j = 0; j < ncol; ++j) {
          for(size_t i = 0; i < nrow; ++i) {
            abptr[i*ncol+j] = b2ptr[j] - 2 * abptr[i*ncol+j];
          }
        }
      }
      else {
#pragma _NEC nointerchange
        for(size_t i = 0; i < nrow; ++i) {
          for(size_t j = 0; j < ncol; ++j) {
            abptr[i*ncol+j] = b2ptr[j] - 2 * abptr[i*ncol+j];
          }
        }
      }
    }
    else if (metric == "seuclidean" and need_distance) {
      if (nrow > ncol) {
#pragma _NEC nointerchange
        for(size_t j = 0; j < ncol; ++j) {
          for(size_t i = 0; i < nrow; ++i) {
            auto sq_dist = a2ptr[i] + b2ptr[j] - 2 * abptr[i*ncol+j];
            if (sq_dist > 0) abptr[i*ncol+j] = sq_dist;
            else abptr[i*ncol+j] = 0;
          }
        }
      }
      else {
#pragma _NEC nointerchange
        for(size_t i = 0; i < nrow; ++i) {
          for(size_t j = 0; j < ncol; ++j) {
            auto sq_dist = a2ptr[i] + b2ptr[j] - 2 * abptr[i*ncol+j];
            if (sq_dist > 0) abptr[i*ncol+j] = sq_dist;
            else abptr[i*ncol+j] = 0;
          }
        }
      }
    }
    else if (metric == "seuclidean" and !need_distance) {
      if (nrow > ncol) {
#pragma _NEC nointerchange
        for(size_t j = 0; j < ncol; ++j) {
          for(size_t i = 0; i < nrow; ++i) {
            abptr[i*ncol+j] = b2ptr[j] - 2 * abptr[i*ncol+j];
          }
        }
      }
      else {
#pragma _NEC nointerchange
        for(size_t i = 0; i < nrow; ++i) {
          for(size_t j = 0; j < ncol; ++j) {
            abptr[i*ncol+j] = b2ptr[j] - 2 * abptr[i*ncol+j];
          }
        }
      }
    }
    else
      REPORT_ERROR(USER_ERROR,
      "Currently frovedis knn supports only euclidean/seuclidean distance!\n");
    return ab;
  }
  std::string metric;
  bool is_trans_b, need_distance;
  SERIALIZE(metric, is_trans_b, need_distance)
};

// construct distance of points in x_mat from the points in y_mat
// output: y_mat.num_row * x_mat.num_row
// rows: <y1, y2, ..., yn>
// cols: <x1, x2, ..., xn>
template <class T>
rowmajor_matrix<T>
construct_distance_matrix(rowmajor_matrix<T>& x_mat,
                          rowmajor_matrix<T>& y_mat,
                          const std::string& metric,
                          bool need_distance = true) {
  auto nsamples = x_mat.num_row;
  auto nquery   = y_mat.num_row;
  std::string dist_metric = metric; // removing const-ness
  rowmajor_matrix<T> dist_mat;

  try {
    time_spent calc_dist(DEBUG);
    calc_dist.lap_start();
    // a simple heuristic to decide which matrix to broadcast
    if (nsamples/100 <= nquery) {
      auto g_x_mat = x_mat.gather();
#ifdef NEED_TRANS_FOR_MULT
      auto b_mat = broadcast(g_x_mat.transpose());
      auto trans_b = true;
#else
      auto b_mat = broadcast(g_x_mat);
      auto trans_b = false;
#endif
      g_x_mat.clear(); // releasing memory for gathered data
      auto loc_dist_mat = y_mat.data.map(compute_dist(dist_metric,
                                         trans_b,need_distance), b_mat);
      dist_mat.data = std::move(loc_dist_mat);
      dist_mat.num_row = nquery;
      dist_mat.num_col = nsamples; // no tranpose required, since Y * Xt is performed
    }
    else { // nsamples >> nquery (more than 100 times)
      auto g_y_mat = y_mat.gather();
#ifdef NEED_TRANS_FOR_MULT
      auto b_mat = broadcast(g_y_mat.transpose());
      auto trans_b = true;
#else
      auto b_mat = broadcast(g_y_mat);
      auto trans_b = false;
#endif
      g_y_mat.clear(); // releasing memory for gathered data
      auto loc_dist_mat = x_mat.data.map(compute_dist(dist_metric,
                                         trans_b,need_distance), b_mat);
      rowmajor_matrix<T> tmp_dist_mat(std::move(loc_dist_mat));
      tmp_dist_mat.num_row = nsamples;
      tmp_dist_mat.num_col = nquery;
      dist_mat = tmp_dist_mat.transpose(); // transpose is needed, since X * Yt is performed
    }
    calc_dist.lap_stop();
    calc_dist.show_lap("dist calculation: ");
  }
  catch(std::exception& excpt) {
    double reqsz = nsamples * nquery * sizeof(T);
    auto reqsz_gb = reqsz / 1024.0 / 1024.0 / 1024.0;
    std::string msg = "out-of-memory error occured while computing distance matrix!";
    msg += "\nrequired memory size for the target distance matrix: ";
    msg += std::to_string(reqsz_gb) + " GB\n";
    REPORT_ERROR(INTERNAL_ERROR, msg);
  }
  return dist_mat;
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
        auto sq_dist = (a2_ptr[myst+i] + a2_ptr[j] - tmp);
        if (sq_dist > 0) // for fixing nan outcome, due to possible numerical error
          twoab_ptr[i*dm_ncol+j] = std::sqrt(sq_dist);
        else
          twoab_ptr[i*dm_ncol+j] = 0;
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
  rowmajor_matrix<T> dist_mat;
  try {
    dist_mat.data = mat.map(construct_distance_matrix_helper<T>,
                            sum_sq_col,myst,mysz,broadcast(is_sq_euclidian));
    dist_mat.num_row = dist_mat.num_col = nrow;
  }
  catch(std::exception& excpt) {
    double reqsz = nrow * nrow * sizeof(T);
    auto reqsz_gb = reqsz / 1024.0 / 1024.0 / 1024.0;
    std::string msg = "out-of-memory error occured while computing distance matrix!";
    msg += "\nrequired memory size for the target distance matrix: ";
    msg += std::to_string(reqsz_gb) + " GB\n";
    REPORT_ERROR(INTERNAL_ERROR, msg);
  }
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

  if (is_sq_euclidian) { 
    for(size_t i = myst; i <= myend; ++i) {
      for(size_t j = i+1; j < dm_ncol; ++j) {
        auto local_i = i - myst;
        auto tmp = twoab_ptr[local_i*dm_ncol+j];
        ret[k++] = a2_ptr[i] + a2_ptr[j] - tmp;
      }
    }
  }
  else { 
    for(size_t i = myst; i <= myend; ++i) {
      for(size_t j = i+1; j < dm_ncol; ++j) {
        auto local_i = i - myst;
        auto tmp = twoab_ptr[local_i*dm_ncol+j];
        auto sq_dist = (a2_ptr[i] + a2_ptr[j] - tmp);
        if (sq_dist > 0) ret[k++] = std::sqrt(sq_dist); // fixing of possible nan outcome
        else ret[k++] = 0;
      }
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
lvec<T> construct_connectivity_diagonals(rowmajor_matrix<T>& mat,
                                         node_local<size_t>& myst) {
  return mat.data.map(sum_of_cols_without_diagonal<T>, myst);
}

template <class T>
std::vector<int> 
one_by_sqrt_inplace(std::vector<T>& conn_vec) {
  auto nsamples = conn_vec.size();
  std::vector<int> isolated(nsamples);
  auto cptr = conn_vec.data();
  auto isoptr = isolated.data();
  for(size_t i = 0; i < nsamples; ++i) {
    if (cptr[i] == 0) cptr[i] = isoptr[i] = 1;
    else {
      isoptr[i] = 0;
      cptr[i] = 1 / sqrt(cptr[i]);
    }
  }
  return isolated;
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
    // algo: scipy.sparse.csgraph.laplacian
    if(norm_laplacian) {
      auto isolated = one_by_sqrt_inplace(conn_vec); // returns vector(1 or 0) 
      auto cptr = conn_vec.data();
      auto isoptr = isolated.data();
      for(size_t i = 0; i < nrow; ++i) {
        auto diag_j = i + myst; 
        for(size_t j = 0; j < ncol; ++j) {
          mptr[i*ncol+j] *= -1.0 * cptr[j] * cptr[diag_j];  
        }
        mptr[i * ncol + diag_j] = 1.0 - isoptr[i]; // taking care of diagonals
      }
    }
    else {
      auto cptr = conn_vec.data();
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
template <class R, class T, class I = size_t, class O = size_t>
crs_matrix_local<R,I,O>
construct_connectivity_graph_helper(rowmajor_matrix_local<T>& dmat,
                                    double eps,
                                    bool to_include_self = true,
                                    bool needs_weight = false) {
  auto nrow = dmat.local_num_row;
  auto ncol = dmat.local_num_col;
  std::vector<size_t> oneD_indx;
  if (to_include_self) oneD_indx = vector_find_le(dmat.val, (T) eps);
  else oneD_indx = vector_find_le_and_neq(dmat.val, (T) eps, (T) 0);
  auto count = oneD_indx.size();
  std::vector<I> index(count);
  auto idxp = index.data();
  auto gidxp = oneD_indx.data();
  std::vector<O> retoff;
  if (to_include_self) {
    for(size_t i = 0; i < count; ++i) idxp[i] = gidxp[i] / ncol; // 0-based row-index
    // every row would have at least one entry (since it includes self)
    // hence, only set_separate() would produce required offset for resultant graph
    if (std::is_same<O, size_t>::value)  retoff = set_separate(index);
    else retoff = vector_astype<O>(set_separate(index));
    for(size_t i = 0; i < count; ++i) {
       idxp[i] = gidxp[i] - (idxp[i] * ncol); // col-index [same as gidxp[i] % ncol]
    }
  }
  else {
    // making 1-based row index to avoid adding 0 at beginining of bincount result
    for(size_t i = 0; i < count; ++i) idxp[i] = 1 + (gidxp[i] / ncol); // 1-based row-index
    retoff = prefix_sum(vector_bincount<O>(index));
    for(size_t i = 0; i < count; ++i) {
      idxp[i] = gidxp[i] - ((idxp[i] - 1) * ncol); // col-index [same as gidxp[i] % ncol]
    }
  }
  crs_matrix_local<R,I,O> ret;
  if (needs_weight) {
    ret.val = vector_take<R>(dmat.val, oneD_indx);
  }
  else {
    oneD_indx.clear(); // 1-D index is no longer needed, hence freed
    ret.val = vector_ones<R>(count);
  }
  ret.idx.swap(index);
  ret.off.swap(retoff);
  ret.local_num_row = nrow;
  ret.local_num_col = ncol;
  return ret;
}

template <class R, class T, class I = size_t, class O = size_t>
crs_matrix<R,I,O>
construct_connectivity_graph(rowmajor_matrix<T>& dmat,
                             double eps,
                             bool to_include_self = true,
                             bool needs_weight = false) {
  if (dmat.num_row != dmat.num_col)
    REPORT_ERROR(USER_ERROR, "input distance matrix is not a square matrix!\n");
  crs_matrix<R,I,O> ret(dmat.data.map(construct_connectivity_graph_helper<R,T,I,O>,
                                      broadcast(eps),
                                      broadcast(to_include_self),
                                      broadcast(needs_weight)));
  ret.num_row = ret.num_col = dmat.num_row; // square matrix
  return ret;
}

} // end of namespace

#endif
