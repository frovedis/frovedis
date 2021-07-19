#ifndef _KNN_HPP_
#define _KNN_HPP_

// decide number of elements of 1M memory size
#define CHUNK_SIZE 1024 * 1024 

#define THRESHOLD 50000

//#define MANUAL_LOOP_COLLAPSE_IN_EXTRACTION 
//#define DEBUG_SAVE

#include <frovedis/core/radix_sort.hpp>
#include <frovedis/core/partition_sort.hpp>
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
void sort_segmented_rows(T* dist_buf_ptr,       // distance buffer pointer (destroyed)
                         I* imat_ptr,           // chunk-partitioned index matrix pointer 
                         T* dptr, I* iptr,      // model member pointers
                         size_t size, size_t k, // nrow_in_buffer = size / k
                         size_t ncol,           // ncol of indx_mat: imat_ptr
                         bool need_distance,
                         time_spent& radix_t,
                         time_spent& extract_t) {
  std::vector<T> copy_dist;
  if (need_distance) {
    copy_dist.resize(size);
    auto copy_dptr = copy_dist.data();
    for(size_t i = 0; i < size; ++i) copy_dptr[i] = dist_buf_ptr[i]; // copying before sort
  }
  // iptr contains physical indices, but reused later as col-index.
  for(size_t i = 0; i < size; ++i) iptr[i] = i;
  radix_t.lap_start();
  radix_sort(dist_buf_ptr, iptr, size); // sort entire data in distance buffer in one go
  radix_t.lap_stop();
  auto one_by_k = 1.0 / k;
  std::vector<I> row_indx(size);
  auto riptr = row_indx.data();
  for(size_t i = 0; i < size; ++i) {
    riptr[i] = iptr[i] * one_by_k;
    iptr[i] = iptr[i] - (riptr[i] * k); // physical index to col index conversion: reuse of memory 
  }
  radix_t.lap_start();
  radix_sort(riptr, iptr, size); // sort to know positional changes in each row
  radix_t.lap_stop();
  extract_t.lap_start();
  if (need_distance) {
    auto copy_dptr = copy_dist.data();
    for(size_t ij = 0; ij < size; ++ij) {
      auto i = riptr[ij];
      auto j = iptr[ij]; // actually col-index
      dptr[ij] = copy_dptr[i * k + j]; // update model dptr in-place
      iptr[ij] = imat_ptr[i * ncol + j]; // update model iptr in-place, with actual index
    }
  }
  else {
    for(size_t ij = 0; ij < size; ++ij) {
      auto i = riptr[ij];
      auto j = iptr[ij]; // actually col-index
      iptr[ij] = imat_ptr[i * ncol + j]; // update model iptr in-place, with actual index
    }
  }
  extract_t.lap_stop();
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

template <class I>
void set_index(rowmajor_matrix_local<I>& mat) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  auto mptr = mat.val.data();
  for(size_t i = 0; i < nrow; ++i) {
    for(size_t j = 0; j < ncol; ++j) {
      mptr[i * ncol + j] = j;
    }
  }
}

struct find_kneighbor {
  find_kneighbor() {}
  find_kneighbor(int kk, bool need_dist, float c_sz): 
    k(kk), need_distance(need_dist), chunk_size(c_sz) {}

  template <class T, class I>
  void operator()(rowmajor_matrix_local<T>& dist_mat,
                  rowmajor_matrix_local<T>& model_dist,
                  rowmajor_matrix_local<I>& model_indx,
                  size_t batch_size,
                  size_t iter) {
    auto nrow = dist_mat.local_num_row;
    auto ncol = dist_mat.local_num_col;

    /*
    model_dist and model_indx is already allocated. Fetch the index from where
    the assignment will begin. 
    */  
    auto start = std::min(iter * batch_size, model_dist.local_num_row);   
    
    auto model_iptr = model_indx.val.data();
    auto model_dptr = model_dist.val.data();
    
    // decide each chunk of rows (startring index and total nrow in each chunk)
    auto rows_per_chunk = get_rows_per_chunk<T>(nrow, k, chunk_size);
    auto n_iter = ceil_div(nrow, rows_per_chunk);  
    RLOG(DEBUG) << "distance sorting problem will be solved in " 
                  + std::to_string(n_iter) + " steps!\n";
    std::vector<size_t> rows(n_iter + 1);
    rows[0] = 0;
    auto rows_ptr = rows.data();
    for(size_t i = 1; i <= n_iter; ++i) rows_ptr[i]  = i * rows_per_chunk;
    if (rows[n_iter] > nrow) rows[n_iter] = nrow;
    //display(rows);

    time_spent partition_t(DEBUG), sort_each_t(TRACE), extract_t(DEBUG), radix_t(DEBUG);
    time_spent comp_t(DEBUG), copy_t(DEBUG);
    for(size_t i = 0; i < n_iter; ++i) {
      RLOG(DEBUG) << "working on chunk [" << rows[i] << " : " << rows[i+1] - 1 << "]\n";
      size_t nrow_in_chunk = rows[i+1] - rows[i] ;
      rowmajor_matrix_local<I> indx_mat(nrow_in_chunk, ncol);
      set_index(indx_mat);
      auto dist_mptr = dist_mat.val.data() + (rows[i] * ncol);
      auto indx_mptr = indx_mat.val.data();
      partition_t.lap_start();
      partition_sort(dist_mptr, indx_mptr, nrow_in_chunk, ncol, k, comp_t, copy_t);
      partition_t.lap_stop();

      extract_t.lap_start();
      std::vector<T> dist_buffer(nrow_in_chunk * k);
      auto dist_buf_ptr = dist_buffer.data();
      extract_k_cols(dist_mptr, dist_buf_ptr, nrow_in_chunk, ncol, k);
      extract_t.lap_stop();

      // sorting on distance and column index
      auto b_size = dist_buffer.size();
      auto dptr = model_dptr;
      if (need_distance) dptr = dptr + (start * k) + (rows[i] * k);
      auto iptr = model_iptr + (start * k) + (rows[i] * k);
      sort_each_t.lap_start();
      sort_segmented_rows(dist_buf_ptr,        // partitioned distance buffer pointer
                          indx_mptr,           // partitioned sorted indx_mat pointer
                          dptr, iptr,          // model pointers
                          b_size, k, ncol,
                          need_distance, 
                          radix_t, extract_t);
      sort_each_t.lap_stop();      
      if(get_selfid() == 0) {
        auto chunk = "[" + std::to_string(rows[i])     + ":" + 
                           std::to_string(rows[i+1]-1) + "]";
        sort_each_t.show_lap(chunk + " chunk-wise sorting time: "); // includes distance extraction time
      }
      sort_each_t.reset();
    }
    if(get_selfid() == 0) { // logging only by rank 0
      partition_t.show_lap("partition time: ");
      comp_t.show_lap("  \\_ comparison time: ");
      copy_t.show_lap("  \\_ copy back time: ");
      radix_t.show_lap("radix sorting time: ");
      extract_t.show_lap("extraction time: ");
    }      
  }    

  template <class T, class I>
  void operator()(rowmajor_matrix_local<T>& dist_mat,
                  rowmajor_matrix_local<T>& model_dist,
                  rowmajor_matrix_local<I>& model_indx) {            
    auto nrow = dist_mat.local_num_row;
    auto ncol = dist_mat.local_num_col;

    // allocating output memory for model parameters
    model_indx.val.resize(nrow * k);
    model_indx.set_local_num(nrow, k);
    if (need_distance) {
      model_dist.val.resize(nrow * k);
      model_dist.set_local_num(nrow, k);
    }
    else {
      model_dist.set_local_num(0, 0); // empty distance matrix for model component
    }
    auto model_iptr = model_indx.val.data();
    auto model_dptr = model_dist.val.data();
    
    // decide each chunk of rows (startring index and total nrow in each chunk)
    auto rows_per_chunk = get_rows_per_chunk<T>(nrow, k, chunk_size);

    auto n_iter = ceil_div(nrow, rows_per_chunk);

    RLOG(DEBUG) << "distance sorting problem will be solved in " 
                  + std::to_string(n_iter) + " steps!\n";
    std::vector<size_t> rows(n_iter + 1);
    rows[0] = 0;
    auto rows_ptr = rows.data();
    for(size_t i = 1; i <= n_iter; ++i) rows_ptr[i]  = i * rows_per_chunk;
    if (rows[n_iter] > nrow) rows[n_iter] = nrow;
    //display(rows);

    time_spent partition_t(DEBUG), sort_each_t(TRACE), extract_t(DEBUG), radix_t(DEBUG);
    time_spent comp_t(DEBUG), copy_t(DEBUG);
    for(size_t i = 0; i < n_iter; ++i) {
      RLOG(DEBUG) << "working on chunk [" << rows[i] << " : " << rows[i+1] - 1 << "]\n";
      size_t nrow_in_chunk = rows[i+1] - rows[i] ;
      rowmajor_matrix_local<I> indx_mat(nrow_in_chunk, ncol);

      set_index(indx_mat);
      auto dist_mptr = dist_mat.val.data() + (rows[i] * ncol);
      auto indx_mptr = indx_mat.val.data();
      partition_t.lap_start();
      partition_sort(dist_mptr, indx_mptr, nrow_in_chunk, ncol, k, comp_t, copy_t);
      partition_t.lap_stop();

      extract_t.lap_start();
      std::vector<T> dist_buffer(nrow_in_chunk * k);
      auto dist_buf_ptr = dist_buffer.data();
      extract_k_cols(dist_mptr, dist_buf_ptr, nrow_in_chunk, ncol, k);     
      extract_t.lap_stop();

      // sorting on distance and column index
      auto b_size = dist_buffer.size();
      auto dptr = model_dptr;
      if (need_distance) dptr = dptr + (rows[i] * k);
      auto iptr = model_iptr + (rows[i] * k);
      sort_each_t.lap_start();
      sort_segmented_rows(dist_buf_ptr,        // partitioned distance buffer pointer
                          indx_mptr,           // partitioned sorted indx_mat pointer
                          dptr, iptr,          // model pointers
                          b_size, k, ncol,
                          need_distance, 
                          radix_t, extract_t);
      sort_each_t.lap_stop();      
      if(get_selfid() == 0) {
        auto chunk = "[" + std::to_string(rows[i])     + ":" + 
                           std::to_string(rows[i+1]-1) + "]";
        sort_each_t.show_lap(chunk + " chunk-wise sorting time: "); // includes distance extraction time
      }
      sort_each_t.reset();
    }
    if(get_selfid() == 0) { // logging only by rank 0
      partition_t.show_lap("partition time: ");
      comp_t.show_lap("  \\_ comparison time: ");
      copy_t.show_lap("  \\_ copy back time: ");
      radix_t.show_lap("radix sorting time: ");
      extract_t.show_lap("extraction time: ");
    }
  }
  int k;
  bool need_distance;
  float chunk_size;
  SERIALIZE(k, need_distance, chunk_size)
};

template <class T>
void pre_allocate(rowmajor_matrix_local<T>& rmat_local, size_t row, size_t col) {
  rmat_local.val.resize(row * col); 
  rmat_local.set_local_num(row, col);
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
rowmajor_matrix_local<T> extract_rmm_batch(rowmajor_matrix_local<T>& mat,
                                           size_t batch_size, size_t iter) {
  auto local_row = mat.local_num_row;
  auto local_col = mat.local_num_col;  
  
  auto start = std::min(iter * batch_size, local_row);  
  auto end = std::min((iter+1) * batch_size, local_row);
        
  require(end >= start, "start row index must be less than end column index");
  require(start >= 0 && end <= local_row, "given indices are out of range"); 
    
  auto nrows = end - start;  
  rowmajor_matrix_local<T> ret(nrows, local_col);
  
  auto retptr = ret.val.data();
  auto matptr = mat.val.data();  

  //Copy
  for(size_t j = 0; j < local_col; ++j) {
    for(size_t i = 0; i < nrows; ++i)
      retptr[local_col * i + j] = matptr[local_col * (i + start) + j];     
  }  
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
  auto end = std::min((iter+1) * batch_size, local_row);                   

  require(end >= start, "start row index must be less than end column index");
  require(start >= 0 && end <= local_row, "given indices are out of range");  
  auto nrows = end - start;  
  
  size_t num_elements = 0;
  auto offp = &mat.off[0];
  auto idxp = &mat.idx[0];
  auto valp = &mat.val[0];
  for(size_t i = start; i < end; ++i)
    num_elements += offp[i + 1] - offp[i];

  crs_matrix_local<T,I,O> ret(nrows, local_col);
  ret.val.resize(num_elements);
  ret.idx.resize(num_elements);
  ret.off.resize(nrows+1);  
      
  auto roffp = &ret.off[0];
  auto ridxp = &ret.idx[0];
  auto rvalp = &ret.val[0];
  
  //Copy
  size_t counter = 0, off_iter=1;  
  for(size_t i = start; i < end; ++i) {
    for(O o = offp[i]; o < offp[i+1]; ++o) {
      ridxp[counter] = idxp[o];
      rvalp[counter] = valp[o];
      ++counter;
    }  
    roffp[off_iter++] = counter;    
  }
    
  return ret;  
}    
    
template <class T, class I, class O>
crs_matrix<T,I,O> extract_batch(crs_matrix<T,I,O>& mat, 
                                node_local<size_t> batch_size, 
                                node_local<size_t> iter) {
    crs_matrix<T,I,O> ret = mat.data.map(extract_crs_batch<T,I,O>, batch_size, iter);
    return ret;
}
    
template <class T>
rowmajor_matrix<T> extract_batch(rowmajor_matrix<T>& mat, 
                                 node_local<size_t> batch_size, 
                                 node_local<size_t> iter) {
    rowmajor_matrix<T> ret = mat.data.map(extract_rmm_batch<T>, batch_size, iter);
    return ret;
}      
    
    
template <class T, class I = size_t, 
          class MATRIX1 = rowmajor_matrix<T>,
          class MATRIX2 = rowmajor_matrix<T>>
knn_model<T, I> compute_kneigbor_in_batch(MATRIX1& x_mat,
                                          MATRIX2& y_mat,
                                          int k,
                                          const std::string& metric,
                                          bool need_distance,
                                          float chunk_size, 
                                          size_t batch_size) {

  auto nquery = y_mat.num_row;
  auto nrows = y_mat.get_local_num_rows();
  auto nl_rows = make_node_local_scatter(nrows);
  //Pre-allocate
  knn_model<T, I> ret(k);            
  ret.distances.data = make_node_local_allocate<rowmajor_matrix_local<T>>();
  if(need_distance) {
    ret.distances.num_row = nquery;
    ret.distances.num_col = k;            
    ret.distances.data.mapv(pre_allocate<T>, nl_rows, broadcast(k));
  }
  else {
    ret.distances.num_row = ret.distances.num_col = 0;
    ret.distances.data.mapv(pre_allocate<T>, broadcast(0), broadcast(0));//All local shouldn't have garbage row,col  
  }
              
  ret.indices = make_node_local_allocate<rowmajor_matrix_local<I>>();
  ret.indices.num_row = nquery;
  ret.indices.num_col = k;
  ret.indices.data.mapv(pre_allocate<I>, nl_rows, broadcast(k));
  
  //Get number of iterations needed
  auto niters = get_num_iterations(nrows, batch_size);                         
  auto nl_batch_size = broadcast(batch_size);
              
  for(size_t i=0; i < niters; ++i) {
   
    auto partial_query = extract_batch(y_mat, nl_batch_size, broadcast(i)); 
    
    auto partial_dist_mat = construct_distance_matrix<T>(x_mat, partial_query, metric, need_distance);
            
    partial_dist_mat.data.mapv(find_kneighbor(k, need_distance, chunk_size), 
                               ret.distances.data, ret.indices.data, nl_batch_size, broadcast(i));      
  }         
  return ret;                          
}
    
template <class T, class I = size_t, 
          class MATRIX1 = rowmajor_matrix<T>,
          class MATRIX2 = rowmajor_matrix<T>>
knn_model<T, I> compute_kneigbor(MATRIX1& x_mat,
                                 MATRIX2& y_mat,
                                 int k,
                                 const std::string& metric,
                                 bool need_distance,
                                 float chunk_size) {           
  auto nquery   = y_mat.num_row;              
  auto dist_mat = construct_distance_matrix<T>(x_mat, y_mat, metric, need_distance);
            
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
        
template <class T, class I = size_t, 
          class MATRIX1 = rowmajor_matrix<T>,
          class MATRIX2 = rowmajor_matrix<T>>
knn_model<T, I> knn(MATRIX1& x_mat,
                    MATRIX2& y_mat,
                    int k,
                    const std::string& algorithm = "brute",
                    const std::string& metric = "euclidean",
                    bool need_distance = false,
                    float chunk_size = 1.0, 
                    double batch_fraction = std::numeric_limits<double>::max()) {  
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

  if (batch_fraction < 0)
    REPORT_ERROR(USER_ERROR, 
      "Batch fraction value should be between 0.0 and 1.0\n");
        
  if(batch_fraction == std::numeric_limits<double>::max()) { // No batch provided
    if (nquery > THRESHOLD) { // Compute with batches of distance matrix
      auto node_size = get_nodesize();  
      size_t batch_size = THRESHOLD/node_size;
      return compute_kneigbor_in_batch<T, I>(x_mat, y_mat, k, metric, need_distance, 
                                             chunk_size, batch_size);  
    }
    else { // Compute entire matrix at once 
      return compute_kneigbor<T, I>(x_mat, y_mat, k, metric, need_distance, chunk_size);
    }
  }
  else { // Divide as per batch value provided
    auto node_size = get_nodesize();
    auto global_batch = static_cast<size_t>(batch_fraction * nquery);  
    size_t batch_size = global_batch/node_size;
      
    return compute_kneigbor_in_batch<T, I>(x_mat, y_mat, k, metric, need_distance, 
                                           chunk_size, batch_size);   
  }              
}

    
template <class T, class I = size_t, 
          class MATRIX = rowmajor_matrix<T>>
knn_model<T, I> knn(MATRIX& mat,
                    int k,
                    const std::string& algorithm = "brute",
                    const std::string& metric = "euclidean",
                    bool need_distance = false,
                    float chunk_size = 1.0, 
                    double batch_fraction = std::numeric_limits<double>::max()) {
  return knn(mat, mat, k, algorithm, metric, need_distance, chunk_size, batch_fraction);
}

template <class R, class T, class I = size_t, class O = size_t, 
          class MATRIX1 = rowmajor_matrix<T>,
          class MATRIX2 = rowmajor_matrix<T>>
crs_matrix<R, I, O> 
knn_radius(MATRIX1& x_mat,
           MATRIX2& y_mat,
           float radius,
           double batch_fraction = std::numeric_limits<double>::max(), 
           const std::string& algorithm = "brute",
           const std::string& metric = "euclidean",
           const std::string& mode = "distance") {
  if (radius <= 0)
    REPORT_ERROR(USER_ERROR, "Input radius should be a positive number!\n");

  if (algorithm != "brute")
    REPORT_ERROR(USER_ERROR,
      "Currently frovedis knn supports only brute force implementation!\n");

  if (metric != "euclidean" && metric != "seuclidean")
    REPORT_ERROR(USER_ERROR,
      "Currently frovedis knn supports only euclidean/seuclidean distance!\n");

  if (mode != "distance" && mode != "connectivity")
    REPORT_ERROR(USER_ERROR,
      "Currently frovedis knn supports only distance or connectivity as for mode of radius_graph!");

  bool need_distance = true; // needs correct distance for checking within radius

  // TODO: implement batch-wise distance calculation here...
  auto dist_mat = construct_distance_matrix<T>(x_mat, y_mat, metric, need_distance);
#ifdef DEBUG_SAVE
  dist_mat.save("unsorted_distance_matrix");
#endif

  bool include_self = true;
  bool need_weight = (mode == "distance");
  return construct_connectivity_graph<R,T,I,O>(dist_mat, radius, 
                                               include_self, need_weight);
}

}

#endif
