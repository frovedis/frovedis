#ifndef _KNN_HPP_
#define _KNN_HPP_

// decide number of elements of 1M memory size
#define CHUNK_SIZE 1024 * 1024 

#define COMPUTE_ONLY_B2_FOR_PARTIAL_DISTANCE
//#define MANUAL_LOOP_COLLAPSE_IN_EXTRACTION 
//#define DEBUG_SAVE
//#define NEED_TRANS_FOR_MULT

#include <frovedis/core/radix_sort.hpp>
#include <frovedis/matrix/blas_wrapper.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

namespace frovedis {

struct create_graph_helper {
  create_graph_helper() {}
  create_graph_helper(size_t nsamples_, std::string& mode_): 
    nsamples(nsamples_), mode(mode_) {}

  template <class T, class I, class O>
  void operator()(crs_matrix_local<T,I,O>& graph,
                  rowmajor_matrix_local<I>& indices,
                  rowmajor_matrix_local<T>& distances) {
    auto nrow = indices.local_num_row;
    auto ncol = indices.local_num_col;
    graph.val.resize(nrow * ncol);
    graph.idx.resize(nrow * ncol);
    graph.off.resize(nrow + 1);
    graph.local_num_row = nrow;
    graph.local_num_col = nsamples;
    auto ret_valptr = graph.val.data();
    auto ret_idxptr = graph.idx.data();
    auto ret_offptr = graph.off.data();
    auto idxptr = indices.val.data();
    for(size_t i = 0; i <= nrow; ++i) ret_offptr[i] = i * ncol;
    if (mode == "connectivity") {
      for(size_t i = 0; i < nrow * ncol; ++i) {
        ret_valptr[i] = 1.0;
        ret_idxptr[i] = idxptr[i];
      }
    }
    else { // mode = "distance"
      if (nrow != distances.local_num_row &&
          ncol != distances.local_num_col)
        REPORT_ERROR(USER_ERROR, 
        "model distance matrix size does not match with indices matrix size!\n");
      auto dstptr = distances.val.data();
      for(size_t i = 0; i < nrow * ncol; ++i) {
        ret_valptr[i] = dstptr[i];
        ret_idxptr[i] = idxptr[i];
      }
    }
    //graph.debug_pretty_print(); // for DEBUG
  }
  size_t nsamples;
  std::string mode;
  SERIALIZE(nsamples, mode)
};

template <class T, class I = size_t>
struct knn_model {
  knn_model() {}
  knn_model(int kk): k(kk) {}
  void save(const std::string& fname) {
    indices.save(fname + "/indices");
    distances.save(fname + "/distances");
  }
  template <class O = size_t>
  crs_matrix<T, I, O>
  create_graph(const std::string& mode) {
    if (mode != "connectivity" && mode != "distance")
      REPORT_ERROR(USER_ERROR, "Unknown mode for graph creation!\n");
    if (mode == "distance" && distances.num_row == 0) {
      std::string msg = "Input model does not have distance information stored.\n";
      msg += "Please regenerate model with need_distance = true.\n";
      REPORT_ERROR(USER_ERROR, msg);
    }
    crs_matrix<T, I, O> ret;
    auto nsamples = indices.num_row;
    std::string mode_ = mode; // removing const-ness
    ret.data = make_node_local_allocate<crs_matrix_local<T,I,O>>();
    ret.data.mapv(create_graph_helper(nsamples, mode_), 
                  indices.data, distances.data);
    ret.num_row = ret.num_col = nsamples; // squared matrix
    return ret;
  }
  rowmajor_matrix<I> indices;
  rowmajor_matrix<T> distances;
  int k;
  SERIALIZE(indices, distances, k)
};

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
    sm1.sliced_num_row = a_ncol;
    sm1.sliced_num_col = a_nrow;

    sliced_colmajor_matrix_local<T> sm2;
    sm2.ldm = b.local_num_col;
    sm2.data = b.val.data();
    sm2.sliced_num_row = b_ncol;
    sm2.sliced_num_col = b_nrow;

    rowmajor_matrix_local<T> ret(a_nrow, b_nrow); // ret = a * trans(b)
    sliced_colmajor_matrix_local<T> sm3;
    sm3.ldm = b_nrow;
    sm3.data = ret.val.data();
    sm3.sliced_num_row = b_nrow;
    sm3.sliced_num_col = a_nrow;
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

template <class T>
size_t get_rows_per_chunk(size_t nrow, size_t ncol,
                          float MB = 1.0) {
  if(MB <= 0) REPORT_ERROR(USER_ERROR, "chunk_size should be a positive non-zero value!\n");
  size_t tot_elems_per_chunk = MB / sizeof(T) * CHUNK_SIZE;
  size_t rows_per_chunk = ceil_div(tot_elems_per_chunk, ncol);
  return std::min(nrow, rows_per_chunk);
}
  
template <class T, class I>
void sort_segmented_rows(T* dptr, I* iptr, 
                         size_t size,
                         size_t ncol,
                         bool need_distance) {
  std::vector<T> copy_dist;
  if (need_distance) {
    copy_dist.resize(size);
    auto copy_dptr = copy_dist.data();
    for(size_t i = 0; i < size; ++i) copy_dptr[i] = dptr[i]; // copying before sort
  }
  // iptr contains physical indices, but reused later as col-index.
  for(size_t i = 0; i < size; ++i) iptr[i] = i;
  radix_sort(dptr, iptr, size); // sort entire data in dptr(size) in one go
  auto one_by_ncol = 1.0 / ncol;
  std::vector<I> row_indx(size);
  auto riptr = row_indx.data();
  for(size_t i = 0; i < size; ++i) {
    riptr[i] = iptr[i] * one_by_ncol;
    iptr[i] = iptr[i] - (riptr[i] * ncol); // physical index to col index conversion: reuse of memory 
  }
  radix_sort(riptr, iptr, size); // sort to know positional changes in each row
  if (need_distance) {
    auto copy_dptr = copy_dist.data();
    for(size_t k = 0; k < size; ++k) {
      auto i = riptr[k];
      auto j = iptr[k]; // actually col-index
      dptr[k] = copy_dptr[i * ncol + j]; // update dptr in-place using its copy (after sort)
    }
  }
}

template <class T>
void extract_k_cols(const T* srcptr, T* dstptr,
                    size_t nrow, size_t ncol,
                    size_t k) {
  if (k == ncol) { // direct copy
    for(size_t ij = 0; ij < nrow * ncol; ++ij) dstptr[ij] = srcptr[ij];
  }
  else {
#ifdef MANUAL_LOOP_COLLAPSE_IN_EXTRACTION
    auto one_by_k = 1.0 / k;
    for(size_t ij = 0; ij < nrow * k; ++ij) {
      size_t i = ij * one_by_k;
      size_t j = ij - (i * k);
      dstptr[ij] = srcptr[i * ncol + j];
    }
#else
    if (nrow > k) {
#pragma _NEC nointerchange
      for(size_t i = 0; i < k; ++i) {
        for(size_t j = 0; j < nrow; ++j) {
          dstptr[j * k + i] = srcptr[j * ncol + i];
        }
      }
    }
    else { // case when number of rows in a chunk is very less due to large size of distance matrix 
#pragma _NEC nointerchange
      for(size_t j = 0; j < nrow; ++j) {
        for(size_t i = 0; i < k; ++i) {
          dstptr[j * k + i] = srcptr[j * ncol + i];
        }
      }
    }
#endif
  }
}

struct find_kneighbor {
  find_kneighbor() {}
  find_kneighbor(int kk, bool need_dist, float c_sz): 
    k(kk), need_distance(need_dist), chunk_size(c_sz) {}

  template <class T, class I>
  void operator()(rowmajor_matrix_local<T>& dist_mat,
                  rowmajor_matrix_local<T>& model_dist,
                  rowmajor_matrix_local<I>& model_indx) {
    auto nrow = dist_mat.local_num_row;
    auto ncol = dist_mat.local_num_col;

    // allocating output memory for model indx
    model_indx.val.resize(nrow * k);
    model_indx.set_local_num(nrow, k);
    
    // decide each chunk of rows (startring index and total nrow in each chunk)
    auto rows_per_chunk = get_rows_per_chunk<T>(nrow, ncol, chunk_size);
    auto n_iter = ceil_div(nrow, rows_per_chunk);
    std::vector<size_t> rows(n_iter + 1);
    rows[0] = 0;
    auto rows_ptr = rows.data();
    for(size_t i = 1; i <= n_iter; ++i) rows_ptr[i]  = i * rows_per_chunk;
    if (rows[n_iter] > nrow) rows[n_iter] = nrow;
    //display(rows);

    std::vector<I> indx_buffer(rows_per_chunk * ncol); // reusable buffer of CHUNK_SIZE
    auto indx_bfptr = indx_buffer.data();
    time_spent sort_t(INFO), extract_t(INFO);
    for(size_t i = 0; i < n_iter; ++i) {
      //std::cout << "sorting: " << rows[i] << " -> " << rows[i+1] << std::endl;
      // creating chunk for dist_mat.val for sorting
      auto mat_dptr = dist_mat.val.data() + (rows[i] * ncol);
      auto row_in_chunk = rows[i+1] - rows[i];
      auto size = row_in_chunk * ncol;
      // sorting on distance and column index
      sort_t.lap_start();
      sort_segmented_rows(mat_dptr, indx_bfptr, size, ncol, need_distance);
      sort_t.lap_stop();
      // extracting first k from sorted column index chunk
      extract_t.lap_start();
      auto model_iptr = model_indx.val.data() + (rows[i] * k);
      extract_k_cols(indx_bfptr, model_iptr, row_in_chunk, ncol, k); 
      extract_t.lap_stop();
    }
    // extracting first k from entire sorted distance matrix
    extract_t.lap_start();
    if (need_distance) {
      if (k != ncol) {
        model_dist.val.resize(nrow * k);
        model_dist.set_local_num(nrow, k);
        auto mat_dptr = dist_mat.val.data();
        auto model_dptr = model_dist.val.data();
        extract_k_cols(mat_dptr, model_dptr, nrow, ncol, k);
      }
      else {
        model_dist = std::move(dist_mat); // dist_mat will be destroyed at this stage
      } 
    }
    else {
      model_dist.set_local_num(0, 0); // empty distance matrix for model component
    }
    extract_t.lap_stop();
    if(get_selfid() == 0) { // logging only by rank 0
      sort_t.show_lap("sorting time: ");
      extract_t.show_lap("extraction time: ");
    }
  }
  int k;
  bool need_distance;
  float chunk_size;
  SERIALIZE(k, need_distance, chunk_size)
};

template <class T, class I = size_t>
knn_model<T, I> knn(rowmajor_matrix<T>& x_mat,
                    rowmajor_matrix<T>& y_mat,
                    int k,
                    const std::string& algorithm = "brute",
                    const std::string& metric = "euclidean",
                    bool need_distance = false,
                    float chunk_size = 1.0) {
  auto nsamples = x_mat.num_row;
  auto nquery   = y_mat.num_row;
  std::string dist_metric = metric; // removing const-ness

  if (k <= 0 || k > nsamples) 
    REPORT_ERROR(USER_ERROR, std::string("Invalid value for k: ") + 
                             std::to_string(k) + std::string("\n"));

  if (algorithm != "brute")
    REPORT_ERROR(USER_ERROR, 
      "Currently frovedis knn supports only brute force implementation!\n");

  if (metric != "euclidean" && metric != "seuclidean")
    REPORT_ERROR(USER_ERROR, 
      "Currently frovedis knn supports only euclidean/seuclidean distance!\n");

  rowmajor_matrix<T> dist_mat;
  time_spent calc_dist(INFO);

  // a simple heuristic to decide which matrix to broadcast
  if (nsamples/100 <= nquery) {
    auto g_x_mat = x_mat.gather();
    calc_dist.lap_start();
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
    calc_dist.lap_stop();
    calc_dist.show_lap("dist calculation: ");
  }

  else { // nsamples >> nquery (more than 100 times)
    auto g_y_mat = y_mat.gather();
    calc_dist.lap_start();
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
    calc_dist.lap_stop();
    calc_dist.show_lap("dist calculation: ");
  }

#ifdef DEBUG_SAVE
  dist_mat.save("unsorted_distance_matrix");
#endif

  knn_model<T, I> ret(k);
  ret.distances.data = make_node_local_allocate<rowmajor_matrix_local<T>>();
  if (need_distance) {
    ret.distances.num_row = nquery;
    ret.distances.num_col = k;
  }
  else {
    ret.distances.num_row = ret.distances.num_col = 0;
  }
  ret.indices.data = make_node_local_allocate<rowmajor_matrix_local<I>>();
  ret.indices.num_row = nquery;
  ret.indices.num_col = k;

  dist_mat.data.mapv(find_kneighbor(k, need_distance, chunk_size), 
                     ret.distances.data, ret.indices.data);
  return ret;
}

template <class T, class I = size_t>
knn_model<T, I> knn(rowmajor_matrix<T>& mat,
                    int k,
                    const std::string& algorithm = "brute",
                    const std::string& metric = "euclidean",
                    bool need_distance = false,
                    float chunk_size = 1.0) {
  return knn(mat, mat, k, algorithm, metric, need_distance, chunk_size);
}


// sklearn-like interface
template <class T>
struct nearest_neighbors {
  nearest_neighbors(int k, float rad, const std::string& algo,
                    const std::string& met, float c_sz):
    n_neighbors(k), radius(rad), algorithm(algo), 
    metric(met), chunk_size(c_sz) {}
  
  void fit(rowmajor_matrix<T>& mat) {
    observation_data = mat;
  }

  template <class I = size_t>
  knn_model<T,I>
  kneighbors(rowmajor_matrix<T>& enquiry_data, 
             int k = 0, 
             bool need_distance = true) {
    if (k == 0) k = n_neighbors;
    return knn(observation_data, enquiry_data, 
                k, algorithm, metric, need_distance, chunk_size);
  }

  template <class I = size_t, class O = size_t>
  crs_matrix<T, I, O>
  kneighbors_graph(rowmajor_matrix<T>& enquiry_data,
                   int k = 0,
                   const std::string& mode = "connectivity") {
    auto need_distance = (mode == std::string("distance"));
    auto model = kneighbors(enquiry_data, k, need_distance);
    return model.create_graph(mode); 
  }

  rowmajor_matrix<T> observation_data;
  int n_neighbors;
  float radius;
  std::string algorithm;
  std::string metric;
  float chunk_size;
  SERIALIZE(observation_data, n_neighbors, radius, algorithm, metric, chunk_size)
};

}

#endif
