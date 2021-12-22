#ifndef _CLUSTERING_COMMON_
#define _CLUSTERING_COMMON_

#include <math.h>
#include <frovedis/matrix/blas_wrapper.hpp>
#include <frovedis/matrix/spgemm.hpp>

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
 
//For example nrows: [5, 5, 5, 4] and given batch_size is 10: Then we process ceil(5/10) = 1 cycles
//batch_size is 3: Then we process ceil(5/3) = 2 cycles
template <class T>    
size_t get_num_iterations(std::vector<T>& nrows, T batch_size) {   
  auto max_nrow = vector_amax(nrows);
  auto iters = ceil_div(max_nrow, batch_size);
  return iters;  
}
    
template <class T>    
size_t get_batch_size_per_node(T global_batch) {
  T node_size = get_nodesize();  
  return ceil_div(global_batch, node_size);
}
  
template <class T>        
rowmajor_matrix_local<T> 
extract_rmm_batch(rowmajor_matrix_local<T>& mat,
                  size_t batch_size, size_t iter) {
  auto local_row = mat.local_num_row;
  auto local_col = mat.local_num_col;  
  
  auto start = std::min(iter * batch_size, local_row);  
  auto end = std::min((iter + 1) * batch_size, local_row);
  require(end >= start, "start row index must be less than end column index");
    
  auto nrows = end - start;  
  rowmajor_matrix_local<T> ret(nrows, local_col);
  auto ret_sz = ret.val.size();
  auto retptr = ret.val.data();
  auto matptr = mat.val.data() + local_col * start;
  for(size_t i = 0; i < ret_sz; ++i) retptr[i] = matptr[i]; 
  return ret;  
}   

    
template <class T, class I, class O>
crs_matrix_local<T,I,O>       
extract_crs_batch(crs_matrix_local<T,I,O>& mat,
                  size_t batch_size, 
                  size_t iter) {
  auto local_row = mat.local_num_row;
  auto local_col = mat.local_num_col;

  auto start = std::min(iter * batch_size, local_row);  
  auto end = std::min((iter + 1) * batch_size, local_row);                   
  require(end >= start, "start row index must be less than end column index");

  auto nrows = end - start;    
  auto offp = mat.off.data();
  auto idxp = mat.idx.data();
  auto valp = mat.val.data();
  size_t num_elements = offp[end] - offp[start];

  crs_matrix_local<T,I,O> ret(nrows, local_col);
  ret.val.resize(num_elements);
  ret.idx.resize(num_elements);
  ret.off.resize(nrows + 1);  
      
  auto roffp = ret.off.data();
  auto ridxp = ret.idx.data();
  auto rvalp = ret.val.data();
  
  valp = valp + offp[start];
  idxp = idxp + offp[start];     
  for(size_t i = 0; i < num_elements; ++i) {
    rvalp[i] = valp[i];
    ridxp[i] = idxp[i];  
  }

  roffp[0] = 0; offp = offp + start;
  for(size_t j = 0; j < nrows; ++j) roffp[j + 1] = offp[j + 1] - offp[0]; 
  return ret;  
}     

    
template <class T, class I, class O>
crs_matrix<T,I,O> 
extract_batch(crs_matrix<T,I,O>& mat, 
              size_t batch_size_per_node, 
              size_t iter) {
  crs_matrix<T,I,O> ret = mat.data.map(extract_crs_batch<T,I,O>, 
                                       broadcast(batch_size_per_node), 
                                       broadcast(iter));
  ret.num_row = ret.data.map(crs_get_local_num_row<T,I,O>).reduce(add<size_t>); 
  ret.num_col = mat.num_col;
  return ret;
}
        
template <class T>
rowmajor_matrix<T> 
extract_batch(rowmajor_matrix<T>& mat, 
              size_t batch_size_per_node, 
              size_t iter) {
  rowmajor_matrix<T> ret = mat.data.map(extract_rmm_batch<T>, 
                                        broadcast(batch_size_per_node), 
                                        broadcast(iter));
  ret.num_row = ret.data.map(rowmajor_get_local_num_row<T>).reduce(add<size_t>);
  ret.num_col = mat.num_col;
  return ret;
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

template <class T, class I, class O>
crs_matrix_local<T,I,O>
get_global_data(crs_matrix<T,I,O>& mat) { 
  auto g_data = mat.data.gather();
  auto nrows = g_data[0].local_num_row;
  auto ncols = g_data[0].local_num_col;
  size_t n_elements = g_data[0].off[nrows];
  for(size_t i=1; i<g_data.size(); ++i) {
    nrows += g_data[i].local_num_row;
    n_elements += g_data[i].off[g_data[i].local_num_row];
  }
  crs_matrix_local<T,I,O> ret(nrows, ncols);
  ret.off.resize(nrows + 1);
  ret.val.resize(n_elements);
  ret.idx.resize(n_elements);
  auto roffp = &ret.off[0];
  auto rvalp = &ret.val[0];
  auto ridxp = &ret.idx[0];
  size_t k = 0, counter = 0;
  for(size_t i=0; i<g_data.size(); ++i) {
    auto offp = &g_data[i].off[0];
    auto valp = &g_data[i].val[0];
    auto idxp = &g_data[i].idx[0];
    auto size = g_data[i].local_num_row;
    for(size_t j = 0; j < size; ++j)  {
      for(O o = offp[j]; o < offp[j + 1]; ++o) {
        rvalp[counter] = valp[o];
        ridxp[counter++] = idxp[o];
      }
      roffp[k + j + 1] = counter;
    }
    k += size;
  }
  return ret;
}

template <class T>
std::vector<T>
alloc_vector(size_t size, bool incremental = false) {
  return incremental ? vector_arrange<T>(size) : vector_zeros<T>(size);
}

template <class T>
void display(std::vector<T>& vec) {
  debug_print_vector(vec);
}

template <class T, class I, class O>
rowmajor_matrix_local<T>
mult_a_with_trans_b (crs_matrix_local<T,I,O>& a,
                     crs_matrix_local<T,I,O>& b,
                     bool needs_transpose = false, 
                     bool use_spgemm = false) {
  auto& trans_b = b; //assuming b is already a transposed matrix
  if(needs_transpose) trans_b = b.transpose();
  if(use_spgemm) return spgemm(a, trans_b).to_rowmajor();
  else           return crs_mm(a, trans_b); 
}

template <class T, class I, class O>
rowmajor_matrix_local<T>
mult_a_with_trans_b (crs_matrix_local<T,I,O>& a,
                     rowmajor_matrix_local<T>& b,
                     bool needs_transpose = false,
                     bool use_spgemm = false) { //Meanigless for this function, just kept for interface unifomity
  if(!needs_transpose) return a * b; //assuming b is already a transposed matrix
  auto num_row1 = a.local_num_row;
  auto num_row2 = b.local_num_row;
  auto num_col1 = a.local_num_col;
  auto num_col2 = b.local_num_col;
  if(num_col1 != num_col2)
    throw std::runtime_error("invalid size for matrix multiplication");
  rowmajor_matrix_local<T> ret(num_row1, num_row2);

  const T* valp1 = &a.val[0];
  const I* idxp1 = &a.idx[0];
  const O* offp1 = &a.off[0];

  auto retp = ret.val.data();
  auto bp = b.val.data();
  for(size_t r1 = 0; r1 < num_row1; ++r1) {
    for(size_t r2 = 0; r2 < num_row2; ++r2) {
      for(O c1 = offp1[r1]; c1 < offp1[r1 + 1]; ++c1) {
        retp[r1 * num_row2 + r2] += valp1[c1] * bp[r2 * num_col2 + idxp1[c1]];
      }
    }
  }
  return ret;
}

template <class T, class I, class O>
rowmajor_matrix_local<T>
mult_a_with_trans_b (rowmajor_matrix_local<T>& a,
                     crs_matrix_local<T,I,O>& b,
                     bool needs_transpose = false,
                     bool use_spgemm = false) { //Meanigless for this function, just kept for interface unifomity
  if(!needs_transpose) return a * b; //assuming b is already a transposed matrix
  auto num_row1 = a.local_num_row;
  auto num_row2 = b.local_num_row;
  auto num_col1 = a.local_num_col;
  auto num_col2 = b.local_num_col;
  if(num_col1 != num_col2)
    throw std::runtime_error("invalid size for matrix multiplication");
  rowmajor_matrix_local<T> ret(num_row1, num_row2);

  const T* valp2 = &b.val[0];
  const I* idxp2 = &b.idx[0];
  const O* offp2 = &b.off[0];

  auto retp = ret.val.data();
  auto ap = a.val.data();
  for(size_t r1 = 0; r1 < num_row1; ++r1) {
    for(size_t r2 = 0; r2 < num_row2; ++r2) {
      for(O c2 = offp2[r2]; c2 < offp2[r2 + 1]; ++c2) {
        retp[r1 * num_row2 + r2] += valp2[c2] * ap[r1 * num_col1 + idxp2[c2]];
      }
    }
  }
  return ret;
}

template <class T>
rowmajor_matrix_local<T>
mult_a_with_trans_b (rowmajor_matrix_local<T>& a,
                     rowmajor_matrix_local<T>& b,
                     bool needs_transpose = false,
                     bool use_spgemm = false) { //Meanigless for this function, just kept for interface unifomity
  if(!needs_transpose) return a * b; //assuming b is already a transposed matrix
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

template <class T>
struct compute_dist {
  compute_dist() {}
  compute_dist(std::string& metric, bool is_trans_b,
               bool need_distance, bool use_spgemm = false) {
    this->metric = metric;
    this->is_trans_b = is_trans_b;
    this->need_distance = need_distance;
    this->use_spgemm = use_spgemm;
  }

  template <class MATRIX1, class MATRIX2>
  rowmajor_matrix_local<T>
  operator()(MATRIX1& a,
             MATRIX2& b) {
    std::vector<T> a2, b2;
    const T *a2ptr, *b2ptr;
    a2ptr = b2ptr = NULL;
#ifdef COMPUTE_ONLY_B2_FOR_PARTIAL_DISTANCE
    /*partial distance only applies currently for euclidean distances*/
    if(((metric == "euclidean" || metric == "seuclidean")  
        && need_distance) || (metric == "cosine")) {
        a2 = squared_sum_of_cols(a);
        a2ptr = a2.data();
    }
    if (is_trans_b) b2 = squared_sum_of_rows(b); // b is transposed
    else b2 = squared_sum_of_cols(b); // b is not transposed
    b2ptr = b2.data();
#else
    a2 = squared_sum_of_cols(a);
    a2ptr = a2.data();       
    if(((metric == "euclidean" || metric == "seuclidean")  
        && need_distance) || (metric == "cosine")) {
      if (is_trans_b) b2 = squared_sum_of_rows(b); // b is transposed
      else b2 = squared_sum_of_cols(b); // b is not transposed
      b2ptr = b2.data();        
    }
#endif
    auto ab = mult_a_with_trans_b(a, b, !is_trans_b, use_spgemm);
    auto abptr = ab.val.data();
    auto nrow = ab.local_num_row;
    auto ncol = ab.local_num_col;

    if(metric == "cosine") {
      if (nrow > ncol) {
#pragma _NEC nointerchange
        for(size_t j = 0; j < ncol; ++j) {
          for(size_t i = 0; i < nrow; ++i) {
            auto den = std::sqrt(a2ptr[i] * b2ptr[j]);
            if (den > 0) abptr[i*ncol+j] = 1.0 - (abptr[i*ncol+j] / den);
            else abptr[i*ncol+j] = 0;
          }
        }
      }
      else {
#pragma _NEC nointerchange
        for(size_t i = 0; i < nrow; ++i) {
          for(size_t j = 0; j < ncol; ++j) {
            auto den = std::sqrt(a2ptr[i] * b2ptr[j]);
            if (den > 0) abptr[i*ncol+j] = 1.0 - (abptr[i*ncol+j] / den);
            else abptr[i*ncol+j] = 0;
          }
        }
      }        
    }  
    else if (metric == "euclidean" && need_distance) {
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
      "Currently frovedis supports only euclidean/seuclidean/cosine distance!\n");
    return ab;
  }
  std::string metric;
  bool is_trans_b, need_distance, use_spgemm;
  SERIALIZE(metric, is_trans_b, need_distance, use_spgemm)
};

// construct distance of points in x_mat from the points in y_mat
// output: y_mat.num_row * x_mat.num_row
// rows: <y1, y2, ..., yn>
// cols: <x1, x2, ..., xn>
template <class T, class M1, class M2>
rowmajor_matrix<T>
construct_distance_matrix(M1& x_mat,
                          M2& y_mat,
                          const std::string& metric,
                          bool need_distance = true,
                          bool use_spgemm = false) {
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
      auto loc_dist_mat = y_mat.data.map(compute_dist<T>(dist_metric,
                                         trans_b,need_distance,
                                         use_spgemm), b_mat);
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
      auto loc_dist_mat = x_mat.data.map(compute_dist<T>(dist_metric,
                                         trans_b,need_distance,
                                         use_spgemm), b_mat);
      rowmajor_matrix<T> tmp_dist_mat(std::move(loc_dist_mat));
      tmp_dist_mat.num_row = nsamples;
      tmp_dist_mat.num_col = nquery;
      dist_mat = tmp_dist_mat.transpose(); // transpose is needed, since X * Yt is performed
    }
    calc_dist.lap_stop();
    calc_dist.show_lap("dist calculation: ");
  }
  catch(std::exception& excpt) {
    std::string msg = excpt.what();
    if(msg.find("bad_alloc") != std::string::npos ) {
      double reqsz = nsamples * nquery * sizeof(T);
      auto reqsz_gb = reqsz / 1024.0 / 1024.0 / 1024.0;
      std::string e = "out-of-memory error occured while computing distance matrix!";
      e += "\nrequired memory size for the target distance matrix: ";
      e += std::to_string(reqsz_gb) + " GB\n";
      REPORT_ERROR(INTERNAL_ERROR, e);
    }
    else REPORT_ERROR(INTERNAL_ERROR, msg);
  }
  return dist_mat;
}

template<class T>
rowmajor_matrix_local<T> 
twice_a_b_transpose(rowmajor_matrix_local<T>& mat, size_t myst, size_t myend, 
                    bool use_spgemm) { //Meanigless for this function, just kept for interface unifomity
  return mult_sliceA_trans_sliceB<T>(mat, myst, myend, 0, 
                                     mat.local_num_row -1 , 2.0, 1.0);
}

template<class T, class I, class O>
rowmajor_matrix_local<T> 
twice_a_b_transpose(crs_matrix_local<T,I,O>& mat, size_t myst, size_t myend, 
                    bool use_spgemm = false) { 
  auto nrow = myend - myst + 1;
  auto ncol = mat.local_num_col;
  crs_matrix_local<T,I,O> mat_sliced(nrow, ncol);
  size_t num_elements = mat.off[myend + 1] - mat.off[myst];
  mat_sliced.off.resize(nrow + 1);
  mat_sliced.val.resize(num_elements);
  mat_sliced.idx.resize(num_elements);
  auto offp1 = &mat.off[myst];
  auto valp1 = &mat.val[offp1[0]];
  auto idxp1 = &mat.idx[offp1[0]];

  auto offp2 = &mat_sliced.off[0];
  auto valp2 = &mat_sliced.val[0];
  auto idxp2 = &mat_sliced.idx[0];
  auto n = offp1[0];
  for(size_t i = 0; i <= nrow; ++i) 
    offp2[i] = offp1[i] - n;

  for(size_t i = 0; i < num_elements; ++i) {
    valp2[i] = valp1[i];
    idxp2[i] = idxp1[i];
  }
  auto ab = mult_a_with_trans_b(mat_sliced, mat, true, use_spgemm); 
  auto valp = ab.val.data();
  auto size = ab.val.size();
  for(size_t i = 0; i < size; ++i) valp[i] *= 2.0;
  return ab;
}

// construction of distance matrix in distributed way using gemm
template <class T, class MATRIX>
rowmajor_matrix_local<T>
construct_distance_matrix_helper(MATRIX& mat,
                                 std::vector<T>& sum_sq_col,
                                 size_t myst, size_t mysize,
                                 bool is_sq_euclidian = false,
                                 bool use_spgemm = false) {
  auto nrow = mat.local_num_row;
  // returning empty matrix for process not owning any data
  if(!mysize) return rowmajor_matrix_local<T>(0,nrow); 
  auto myend = myst + mysize - 1;
  sum_sq_col = squared_sum_of_cols(mat); // A^2 or ^B2
  auto twice_ab = twice_a_b_transpose<T>(mat,myst,myend,use_spgemm);
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

template <class T, class MATRIX>
rowmajor_matrix<T>
construct_distance_matrix(node_local<MATRIX>& mat,
                          lvec<T>& sum_sq_col,
                          size_t nrow,
                          bool is_sq_euclidian = false,
                          bool use_spgemm = false) {
  // load distribution
  auto nrows = get_block(nrow);
  std::vector<size_t> sidx(nrows.size(),0);
  for(size_t i=1; i<nrows.size(); ++i) sidx[i] = sidx[i-1] + nrows[i-1];
  auto myst = make_node_local_scatter(sidx);
  auto mysz = make_node_local_scatter(nrows);
  rowmajor_matrix<T> dist_mat;
  try {
    dist_mat.data = mat.map(construct_distance_matrix_helper<T, MATRIX>,
                            sum_sq_col,myst,mysz,broadcast(is_sq_euclidian),
                            broadcast(use_spgemm));
    dist_mat.num_row = dist_mat.num_col = nrow;
  }
  catch(std::exception& excpt) {
    std::string msg = excpt.what();
    if(msg.find("bad_alloc") != std::string::npos ) {
      double reqsz = nrow * nrow * sizeof(T);
      auto reqsz_gb = reqsz / 1024.0 / 1024.0 / 1024.0;
      std::string e = "out-of-memory error occured while computing distance matrix!";
      e += "\nrequired memory size for the target distance matrix: ";
      e += std::to_string(reqsz_gb) + " GB\n";
      REPORT_ERROR(INTERNAL_ERROR, e);
    }
    else REPORT_ERROR(INTERNAL_ERROR, msg);
  }
  return dist_mat;
}

template <class T, class MATRIX>
rowmajor_matrix<T>
construct_distance_matrix(MATRIX& mat,
                          lvec<T>& sum_sq_col,
                          bool input_movable = false,
                          bool is_sq_euclidian = false,
                          bool use_spgemm = false) {
  auto nrow = mat.local_num_row;
  auto bdata = broadcast(mat);
  if(input_movable) mat.clear(); // clear rvalue input matrix, after broadcast
  return construct_distance_matrix(bdata, sum_sq_col, nrow, is_sq_euclidian,
                                   use_spgemm);
}

template <class T, class MATRIX>
rowmajor_matrix<T>
construct_distance_matrix(MATRIX& mat,
                          bool input_movable = false,
                          bool is_sq_euclidian = false,
                          bool use_spgemm = false) {
  auto sum_sq_col = make_node_local_allocate<std::vector<T>>();
  return construct_distance_matrix(mat, sum_sq_col, input_movable, is_sq_euclidian,
                                   use_spgemm);
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

template <class T, class MATRIX>
std::vector<T>
construct_condensed_distance_matrix_helper(MATRIX& mat,
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

template <class T, class MATRIX>
std::vector<T>
construct_condensed_distance_matrix(MATRIX& mat) {
  auto nrow = mat.local_num_row;
  auto nrows = get_block(nrow);
  std::vector<size_t> sidx(nrows.size(),0);
  for(size_t i=1; i<nrows.size(); ++i) sidx[i] = sidx[i-1] + nrows[i-1];
  auto myst = make_node_local_scatter(sidx);
  auto mysz = make_node_local_scatter(nrows);
  auto bmat = broadcast(mat);
  return bmat.map(construct_condensed_distance_matrix_helper<T, MATRIX>, 
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
rmm_sum_of_cols_without_diagonal(rowmajor_matrix_local<T>& aff_loc,
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
  return mat.data.map(rmm_sum_of_cols_without_diagonal<T>, myst);
}
    
template <class T, class I, class O>
std::vector<T>
crs_sum_of_cols_without_diagonal(crs_matrix_local<T,I,O>& aff_loc,
                                 size_t myst) {
  auto nrow = aff_loc.local_num_row;
  std::vector<T> sum_vec(nrow, 0);
  auto mvalp = aff_loc.val.data();
  auto moffp = aff_loc.off.data();
  auto idxp = aff_loc.idx.data();  
  auto sumvecp = sum_vec.data();
  size_t maxlen = 0;
  for (size_t i = 0; i < nrow; ++i) {
    auto rowlen = moffp[i+1] - moffp[i];  
    if (rowlen > maxlen) maxlen = rowlen;
  }
  for (size_t i = 0; i < maxlen; ++i) { // iterates on columns (left -> right)
    for (size_t j = 0; j < nrow; ++j) { // iterates on rows (top -> bottom)
      int rem = moffp[j+1] - moffp[j] - i; 
      if (rem > 0) {
        auto col = moffp[j] + i;
        auto glob_row = j + myst;
        //only non-diagonal  
        if(glob_row != idxp[col]) sumvecp[j] += mvalp[col];
      }
    }
  }
  return sum_vec;
}
    

template <class T, class I, class O>
lvec<T> construct_connectivity_diagonals(crs_matrix<T,I,O>& mat,
                                         node_local<size_t>& myst) {  
  return mat.data.map(crs_sum_of_cols_without_diagonal<T,I,O>, myst);
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
struct rmm_construct_laplace_matrix_helper {
  rmm_construct_laplace_matrix_helper() {}
  rmm_construct_laplace_matrix_helper(bool norm): norm_laplacian(norm) {}
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
  aff.data.mapv(rmm_construct_laplace_matrix_helper<T>(norm), 
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

template <class T, class I, class O>
struct crs_construct_laplace_matrix_helper {
  crs_construct_laplace_matrix_helper() {}
  crs_construct_laplace_matrix_helper(bool norm): norm_laplacian(norm) {}
  // in-place conversion of affinity matrix -> lapalce matrix
  void operator()(crs_matrix_local<T,I,O>& mat, 
                  std::vector<T>& conn_vec,
                  size_t myst) {
    auto nrow = mat.local_num_row;
    auto mptr = mat.val.data();
    auto idxp = mat.idx.data();
    auto offp = mat.off.data();      
    auto sz = mat.val.size();
    size_t count_diag_not_present = 0;
      
    // algo: scipy.sparse.csgraph.laplacian
    if(norm_laplacian) {
      auto isolated = one_by_sqrt_inplace(conn_vec); // returns vector(1 or 0) 
      auto cptr = conn_vec.data();
      auto isoptr = isolated.data();
      for(size_t i = 0; i < nrow; ++i) {
        bool diag_found = false;  
        auto diag = i + myst;          
        for(O pos = offp[i]; pos < offp[i+1]; pos++) { 
          mptr[pos] *= -1.0 * cptr[idxp[pos]] * cptr[diag];
          // taking care of diagonals   
          if(idxp[pos] == diag) {
            diag_found = true;
            mptr[pos] = 1.0 - isoptr[i];
          }            
        }
        //count if diagonal element is not present in this row 
        if(!diag_found) count_diag_not_present++; 
      }    
    }
    else {
      auto cptr = conn_vec.data();
      for(size_t i = 0; i < sz; ++i) mptr[i] *= -1.0; // -A 
             
      for(size_t i = 0; i < nrow; ++i) {
        bool diag_found = false;  
        auto diag = i + myst;    
        for(O pos = offp[i]; pos < offp[i+1]; pos++) {
          // taking care of diagonals   
          if(idxp[pos] == diag) {
            diag_found = true;
            mptr[pos] =  cptr[diag];  
          }                 
        }
        //count if diagonal element is not present in this row 
        if(!diag_found) count_diag_not_present++;   
      }
    }

    //if diagonal is not present, give warning
    if(count_diag_not_present > 0)
      REPORT_WARNING(WARNING_MESSAGE, "Affinty matrix should have diagonal values");           
  }
  bool norm_laplacian;
  SERIALIZE(norm_laplacian)
};
    
    
template <class T, class I, class O>
void construct_laplace_matrix_inplace(crs_matrix<T,I,O>& aff,
                                       lvec<T>& conn_vec,
                                       node_local<size_t>& myst,
                                       bool norm = false) {
  aff.data.mapv(crs_construct_laplace_matrix_helper<T,I,O>(norm), 
                conn_vec, myst);
}
    
    
template <class T, class I, class O>
crs_matrix<T,I,O>
construct_laplace_matrix(crs_matrix<T,I,O>& aff,
                          lvec<T>& conn_vec,
                          node_local<size_t>& myst,
                          bool norm = false) {
  crs_matrix<T,I,O> lap(aff);
  construct_laplace_matrix_inplace(lap,conn_vec,myst,norm);
  return lap;
}

template <class T, class I, class O>
void crs_negate_inplace(crs_matrix_local<T,I,O>& mat) {
    auto mptr = mat.val.data();
    auto sz = mat.val.size(); 
    for(size_t i=0; i < sz; ++i) mptr[i] *= -1; 
}
    
    
template <class T>
void rmm_negate_inplace(rowmajor_matrix_local<T>& mat) {
    auto nrow = mat.local_num_row;
    auto ncol = mat.local_num_col;
    auto mptr = mat.val.data();
    for(size_t i=0; i < nrow*ncol; ++i) mptr[i] *= -1; 
}

template <class T, class I, class O>
void negate_inplace(crs_matrix<T,I,O>& mat) {
  mat.data.mapv(crs_negate_inplace<T,I,O>);  
}

    
template <class T>
void negate_inplace(rowmajor_matrix<T>& mat) {
  mat.data.mapv(rmm_negate_inplace<T>);  
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
  auto oneD_indx = vector_find_le(dmat.val, (T) eps);
  if (!to_include_self) {
    std::vector<size_t> self_index(nrow); auto selfp = self_index.data();
    for (size_t i = 0; i < nrow; ++i) selfp[i] = i * ncol + i;  
    oneD_indx = set_difference(oneD_indx, self_index);
  }
  auto count = oneD_indx.size();
  std::vector<I> index(count);
  auto idxp = index.data();
  auto gidxp = oneD_indx.data();
  // making 1-based row index to avoid adding 0 at beginining of bincount result
  for(size_t i = 0; i < count; ++i) idxp[i] = 1 + (gidxp[i] / ncol); // 1-based row-index
  I max_bin = nrow; // required, since last row(s) of dmat can have values > eps
  auto retoff = prefix_sum(vector_bincount<O>(index, max_bin));
  for(size_t i = 0; i < count; ++i) {
    idxp[i] = gidxp[i] - ((idxp[i] - 1) * ncol); // col-index [same as gidxp[i] % ncol]
  }
  crs_matrix_local<R,I,O> ret;
  if (needs_weight) ret.val = vector_take<R>(dmat.val, oneD_indx);
  else              ret.val = vector_ones<R>(count); 
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
  // dmat: might not be asquare matrix because of 
  // batch-wise distance calculation in knn, dbscan etc.
  crs_matrix<R,I,O> ret(dmat.data.map(construct_connectivity_graph_helper<R,T,I,O>,
                                      broadcast(eps),
                                      broadcast(to_include_self),
                                      broadcast(needs_weight)));
  ret.num_row = ret.num_col = dmat.num_row; // square matrix
  return ret;
}

} // end of namespace

#endif
