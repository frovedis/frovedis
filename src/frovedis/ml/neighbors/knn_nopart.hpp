#ifndef _KNN_HPP_
#define _KNN_HPP_

// decide number of elements of 1M memory size
#define CHUNK_SIZE 1024 * 1024 

//#define MANUAL_LOOP_COLLAPSE_IN_EXTRACTION 
//#define DEBUG_SAVE

#include <frovedis/core/radix_sort.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
#include <frovedis/ml/clustering/common.hpp>

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
    if (distances.num_row != 0) distances.save(fname + "/distances");
  }
  template <class O = size_t>
  crs_matrix<T, I, O>
  create_graph(const std::string& mode,
               size_t nsamples) {
    if (mode != "connectivity" && mode != "distance")
      REPORT_ERROR(USER_ERROR, "Unknown mode for graph creation!\n");
    if (mode == "distance" && distances.num_row == 0) {
      std::string msg = "Input model does not have distance information stored.\n";
      msg += "Please regenerate model with need_distance = true.\n";
      REPORT_ERROR(USER_ERROR, msg);
    }
    crs_matrix<T, I, O> ret;
    std::string mode_ = mode; // removing const-ness
    ret.data = make_node_local_allocate<crs_matrix_local<T,I,O>>();
    ret.data.mapv(create_graph_helper(nsamples, mode_), 
                  indices.data, distances.data);
    ret.num_row = indices.num_row;
    ret.num_col = nsamples;
    return ret;
  }
  rowmajor_matrix<I> indices;
  rowmajor_matrix<T> distances;
  int k;
  SERIALIZE(indices, distances, k)
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
                         bool need_distance,
                         time_spent& radix_t) {
  std::vector<T> copy_dist;
  if (need_distance) {
    copy_dist.resize(size);
    auto copy_dptr = copy_dist.data();
    for(size_t i = 0; i < size; ++i) copy_dptr[i] = dptr[i]; // copying before sort
  }
  // iptr contains physical indices, but reused later as col-index.
  for(size_t i = 0; i < size; ++i) iptr[i] = i;
  radix_t.lap_start();
  radix_sort(dptr, iptr, size); // sort entire data in dptr(size) in one go
  radix_t.lap_stop();
  auto one_by_ncol = 1.0 / ncol;
  std::vector<I> row_indx(size);
  auto riptr = row_indx.data();
  for(size_t i = 0; i < size; ++i) {
    riptr[i] = iptr[i] * one_by_ncol;
    iptr[i] = iptr[i] - (riptr[i] * ncol); // physical index to col index conversion: reuse of memory 
  }
  radix_t.lap_start();
  radix_sort(riptr, iptr, size); // sort to know positional changes in each row
  radix_t.lap_stop();
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
    if(get_selfid() == 0) {
      RLOG(DEBUG) << "distance sorting problem will be solved in " 
                  << n_iter << " steps!\n";
    }
    std::vector<size_t> rows(n_iter + 1);
    rows[0] = 0;
    auto rows_ptr = rows.data();
    for(size_t i = 1; i <= n_iter; ++i) rows_ptr[i]  = i * rows_per_chunk;
    if (rows[n_iter] > nrow) rows[n_iter] = nrow;
    //display(rows);

    std::vector<I> indx_buffer(rows_per_chunk * ncol); // reusable buffer of CHUNK_SIZE
    auto indx_bfptr = indx_buffer.data();
    time_spent sort_t(DEBUG), sort_each_t(TRACE), extract_t(DEBUG), radix_t(DEBUG);
    for(size_t i = 0; i < n_iter; ++i) {
      //std::cout << "sorting: " << rows[i] << " -> " << rows[i+1] << std::endl;
      // creating chunk for dist_mat.val for sorting
      auto mat_dptr = dist_mat.val.data() + (rows[i] * ncol);
      auto row_in_chunk = rows[i+1] - rows[i];
      auto size = row_in_chunk * ncol;
      // sorting on distance and column index
      sort_each_t.lap_start();
      sort_t.lap_start();
      sort_segmented_rows(mat_dptr, indx_bfptr, size, ncol, need_distance, radix_t);
      sort_t.lap_stop();
      sort_each_t.lap_stop();
      if(get_selfid() == 0) sort_each_t.show_lap("chunk-wise sorting time: ");
      sort_each_t.reset();
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
      radix_t.show_lap("radix sorting time: ");
      sort_t.show_lap("overall sorting time: ");
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

  if (k <= 0 || k > nsamples) 
    REPORT_ERROR(USER_ERROR, std::string("Invalid value for k: ") + 
                             std::to_string(k) + std::string("\n"));

  if (algorithm != "brute")
    REPORT_ERROR(USER_ERROR, 
      "Currently frovedis knn supports only brute force implementation!\n");

  if (metric != "euclidean" && metric != "seuclidean")
    REPORT_ERROR(USER_ERROR, 
      "Currently frovedis knn supports only euclidean/seuclidean distance!\n");

  auto dist_mat = construct_distance_matrix(x_mat, y_mat, metric, need_distance);
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

struct find_radius_neighbor {
  find_radius_neighbor() {}
  find_radius_neighbor(float rad, size_t ns):
    radius(rad), nsamples(ns) {}

  template <class T, class I, class O>
  void operator()(rowmajor_matrix_local<T>& dist_mat,
                  crs_matrix_local<T,I,O>& ret) {
    auto nrow = dist_mat.local_num_row;
    auto ncol = dist_mat.local_num_col;
    auto dmatptr = dist_mat.val.data();
    size_t count = 0;
    // counting number of in-radius elements for getting size of val/idx of ret
    for(size_t i = 0; i < nrow * ncol; ++i) {
      if (dmatptr[i] <= radius) { // && dmatptr[i] != 0) {
        count++;
      }
    }
    ret.val.resize(count);
    ret.idx.resize(count);
    ret.off.resize(nrow + 1);
    ret.off[0] = 0;
    ret.local_num_row = nrow;
    ret.local_num_col = nsamples;
    auto retvalptr = ret.val.data();
    auto retidxptr = ret.idx.data();
    auto retoffptr = ret.off.data();
    size_t curidx = 0;
    for(size_t i = 0; i < nrow; ++i) {
      for(size_t j = 0; j < ncol; ++j) {
        auto dist = dmatptr[i * ncol + j];
        if (dist <= radius) { // && dist != 0) {
          retvalptr[curidx] = dist;
          retidxptr[curidx] = j;
          curidx++;
        }
      }
      retoffptr[i+1] = curidx;
    }
  }
  float radius;
  size_t nsamples;
  SERIALIZE(radius, nsamples)
};

template <class T, class I, class O>
void convert_to_connectivity_graph(crs_matrix_local<T,I,O>& mat) {
  auto vptr = mat.val.data();
  for(size_t i = 0; i < mat.val.size(); ++i) vptr[i] = 1.0; // means connected
}

template <class T, class I, class O>
crs_matrix<T,I,O>
create_radius_graph(crs_matrix<T,I,O>& mat, const std::string& mode) {
  crs_matrix<T,I,O> ret;
  if (mode == "distance") ret = mat;
  else if (mode == "connectivity") { // copy mat and replace distnace values with 1.0
    ret = mat;
    ret.data.mapv(convert_to_connectivity_graph<T,I,O>);
  }
  else REPORT_ERROR(USER_ERROR, "Unknown mode is encountered for graph creation!\n");
  return ret;
}

template <class T, class I = size_t, class O = size_t>
crs_matrix<T, I, O> 
knn_radius(rowmajor_matrix<T>& x_mat,
           rowmajor_matrix<T>& y_mat,
           float radius,
           const std::string& algorithm = "brute",
           const std::string& metric = "euclidean",
           bool need_distance = false,
           float chunk_size = 1.0) {
  auto nsamples = x_mat.num_row;
  auto nquery   = y_mat.num_row;

  if (radius <= 0)
    REPORT_ERROR(USER_ERROR, "Input radius should be a positive number!\n");

  if (algorithm != "brute")
    REPORT_ERROR(USER_ERROR,
      "Currently frovedis knn supports only brute force implementation!\n");

  if (metric != "euclidean" && metric != "seuclidean")
    REPORT_ERROR(USER_ERROR,
      "Currently frovedis knn supports only euclidean/seuclidean distance!\n");

  auto dist_mat = construct_distance_matrix(x_mat, y_mat, metric, need_distance);
#ifdef DEBUG_SAVE
  dist_mat.save("unsorted_distance_matrix");
#endif

  crs_matrix<T,I,O> knn_radius_model;
  knn_radius_model.data = make_node_local_allocate<crs_matrix_local<T,I,O>>();
  dist_mat.data.mapv(find_radius_neighbor(radius, nsamples),
                     knn_radius_model.data);
  knn_radius_model.num_row = nquery;
  knn_radius_model.num_col = nsamples;
  return knn_radius_model;
}

}

#endif
