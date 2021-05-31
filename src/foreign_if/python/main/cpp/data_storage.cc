#include "python_client_headers.hpp"
#include "exrpc_data_storage.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

template <class T>
std::vector<frovedis::crs_matrix_local<T,int,size_t>> 
  prepare_scattered_crs_matrices(T* datavalp, int* dataidxp, long* dataoffp,
                                 ulong nrow, ulong ncol, ulong total,
                                 size_t node_size) {
  size_t each_size = frovedis::ceil_div(total, node_size);
  std::vector<size_t> divide_row(node_size+1);
  for(size_t i = 0; i < node_size; i++) {
    auto it = std::lower_bound(dataoffp, dataoffp + nrow + 1, each_size * i);
    if(it != dataoffp + nrow + 1) {
      divide_row[i] = it - dataoffp;
    } else {
      divide_row[i] = nrow;
    }
  }
  divide_row[node_size] = nrow;
  std::vector<frovedis::crs_matrix_local<T,int,size_t>> vret(node_size);
#pragma omp parallel for num_threads(node_size)
  for(size_t i = 0; i < node_size; i++) {
    vret[i].local_num_col = ncol;
    size_t start_row = divide_row[i];
    size_t end_row = divide_row[i+1];
    vret[i].local_num_row = end_row - start_row;
    size_t start_off = dataoffp[start_row];
    size_t end_off = dataoffp[end_row];
    size_t off_size = end_off - start_off;
    vret[i].val.resize(off_size);
    vret[i].idx.resize(off_size);
    vret[i].off.resize(end_row - start_row + 1); // off[0] == 0 by ctor
    T* valp = &vret[i].val[0];
    int* idxp = &vret[i].idx[0];
    size_t* offp = &vret[i].off[0];
    for(size_t j = 0; j < off_size; j++) {
      valp[j] = datavalp[j + start_off];
      idxp[j] = dataidxp[j + start_off];
    }
    for(size_t j = 0; j < end_row - start_row; j++) {
      offp[j+1] = offp[j] + (dataoffp[start_row + j + 1] -
                             dataoffp[start_row + j]);
    }
  }
  return vret;
}

template <class T>
std::vector<frovedis::crs_matrix_local<T,size_t,size_t>> 
  prepare_scattered_crs_matrices(T* datavalp, long* dataidxp, long* dataoffp,
                                 ulong nrow, ulong ncol, ulong total,
                                 size_t node_size) {
  size_t each_size = frovedis::ceil_div(total, node_size);
  std::vector<size_t> divide_row(node_size+1);
  for(size_t i = 0; i < node_size; i++) {
    auto it = std::lower_bound(dataoffp, dataoffp + nrow + 1, each_size * i);
    if(it != dataoffp + nrow + 1) {
      divide_row[i] = it - dataoffp;
    } else {
      divide_row[i] = nrow;
    }
  }
  divide_row[node_size] = nrow;
  std::vector<frovedis::crs_matrix_local<T,size_t,size_t>> vret(node_size);
#pragma omp parallel for num_threads(node_size)
  for(size_t i = 0; i < node_size; i++) {
    vret[i].local_num_col = ncol;
    size_t start_row = divide_row[i];
    size_t end_row = divide_row[i+1];
    vret[i].local_num_row = end_row - start_row;
    size_t start_off = dataoffp[start_row];
    size_t end_off = dataoffp[end_row];
    size_t off_size = end_off - start_off;
    vret[i].val.resize(off_size);
    vret[i].idx.resize(off_size);
    vret[i].off.resize(end_row - start_row + 1); // off[0] == 0 by ctor
    T* valp = &vret[i].val[0];
    size_t* idxp = &vret[i].idx[0];
    size_t* offp = &vret[i].off[0];
    for(size_t j = 0; j < off_size; j++) {
      valp[j] = datavalp[j + start_off];
      idxp[j] = dataidxp[j + start_off];
    }
    for(size_t j = 0; j < end_row - start_row; j++) {
      offp[j+1] = offp[j] + (dataoffp[start_row + j + 1] -
                             dataoffp[start_row + j]);
    }
  }
  return vret;
}

template <class T>
std::vector<rowmajor_matrix_local<T>>
prepare_scattered_rowmajor_matrices(T* valp, size_t nrow, size_t ncol,
                                    size_t wsize) {
  auto rows = get_block_sizes(nrow, wsize);
  std::vector<size_t> sizevec(wsize);
  auto sizevecp = sizevec.data();
  auto rowsp = rows.data();
  for(size_t i = 0; i < wsize; i++) {
    sizevecp[i] = rowsp[i] * ncol;
  }
  std::vector<size_t> sizepfx(wsize);
  auto sizepfxp = sizepfx.data();
  for(size_t i = 0; i < wsize-1; i++) {
    sizepfxp[i+1] = sizepfxp[i] + sizevecp[i];
  }
  std::vector<rowmajor_matrix_local<T>> ret(wsize);
#pragma omp parallel for num_threads(wsize)
  for(size_t i = 0; i < wsize; i++) {
    ret[i].val.resize(sizevecp[i]);
    auto retp = ret[i].val.data();
    auto srcp = valp + sizepfxp[i];
    for(size_t j = 0; j < sizevecp[i]; j++) {
      retp[j] = srcp[j];
    }
    ret[i].set_local_num(rows[i], ncol);
  }
  return ret;
}
                           
extern "C" {

  // --- frovedis crs matrx create/load/save/view/release ---
  std::vector<exrpc_ptr_t>
  get_each_II_crs_matrix_local_pointers(const char* host, int port,
                                        ulong nrow, ulong ncol,
                                        int* vv, int* ii, long* oo, 
                                        ulong nelem) {
    ASSERT_PTR(vv); ASSERT_PTR(ii); ASSERT_PTR(oo);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering scipy crs-data in (python) client side
    auto mdist = prepare_scattered_crs_matrices(vv, ii, oo, nrow, ncol, nelem,
                                                wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i=0; i<wsize; ++i) {
      res[i] = exrpc_async(nodes[i],load_local_data<S_LMAT44>,mdist[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  std::vector<exrpc_ptr_t>
  get_each_IL_crs_matrix_local_pointers(const char* host, int port,
                                        ulong nrow, ulong ncol,
                                        int* vv, long* ii, long* oo, 
                                        ulong nelem) {
    ASSERT_PTR(vv); ASSERT_PTR(ii); ASSERT_PTR(oo);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering scipy crs-data in (python) client side    
    auto mdist = prepare_scattered_crs_matrices(vv, ii, oo, nrow, ncol, nelem,
                                                wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i=0; i<wsize; ++i) {
      res[i] = exrpc_async(nodes[i],load_local_data<S_LMAT45>,mdist[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  std::vector<exrpc_ptr_t>
  get_each_LI_crs_matrix_local_pointers(const char* host, int port,
                                        ulong nrow, ulong ncol,
                                        long* vv, int* ii, long* oo, 
                                        ulong nelem) {
    ASSERT_PTR(vv); ASSERT_PTR(ii); ASSERT_PTR(oo);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering scipy crs-data in (python) client side    
    auto mdist = prepare_scattered_crs_matrices(vv, ii, oo, nrow, ncol, nelem,
                                                wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i=0; i<wsize; ++i) {
      res[i] = exrpc_async(nodes[i],load_local_data<S_LMAT34>,mdist[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  std::vector<exrpc_ptr_t>
  get_each_LL_crs_matrix_local_pointers(const char* host, int port,
                                        ulong nrow, ulong ncol,
                                        long* vv, long* ii, long* oo, 
                                        ulong nelem) {
    ASSERT_PTR(vv); ASSERT_PTR(ii); ASSERT_PTR(oo);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering scipy crs-data in (python) client side    
    auto mdist = prepare_scattered_crs_matrices(vv, ii, oo, nrow, ncol, nelem,
                                                wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i=0; i<wsize; ++i) {
      res[i] = exrpc_async(nodes[i],load_local_data<S_LMAT35>,mdist[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  std::vector<exrpc_ptr_t>
  get_each_FI_crs_matrix_local_pointers(const char* host, int port,
                                        ulong nrow, ulong ncol,
                                        float* vv, int* ii, long* oo, 
                                        ulong nelem) {
    ASSERT_PTR(vv); ASSERT_PTR(ii); ASSERT_PTR(oo);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering scipy crs-data in (python) client side    
    auto mdist = prepare_scattered_crs_matrices(vv, ii, oo, nrow, ncol, nelem,
                                                wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i=0; i<wsize; ++i) {
      res[i] = exrpc_async(nodes[i],load_local_data<S_LMAT24>,mdist[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  std::vector<exrpc_ptr_t>
  get_each_FL_crs_matrix_local_pointers(const char* host, int port,
                                        ulong nrow, ulong ncol,
                                        float* vv, long* ii, long* oo, 
                                        ulong nelem) {
    ASSERT_PTR(vv); ASSERT_PTR(ii); ASSERT_PTR(oo);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering scipy crs-data in (python) client side    
    auto mdist = prepare_scattered_crs_matrices(vv, ii, oo, nrow, ncol, nelem,
                                                wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i=0; i<wsize; ++i) {
      res[i] = exrpc_async(nodes[i],load_local_data<S_LMAT25>,mdist[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  std::vector<exrpc_ptr_t>
  get_each_DI_crs_matrix_local_pointers(const char* host, int port,
                                        ulong nrow, ulong ncol,
                                        double* vv, int* ii, long* oo, 
                                        ulong nelem) {
    ASSERT_PTR(vv); ASSERT_PTR(ii); ASSERT_PTR(oo);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering scipy crs-data in (python) client side    
    auto mdist = prepare_scattered_crs_matrices(vv, ii, oo, nrow, ncol, nelem,
                                                wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i=0; i<wsize; ++i) {
      res[i] = exrpc_async(nodes[i],load_local_data<S_LMAT14>,mdist[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  std::vector<exrpc_ptr_t>
  get_each_DL_crs_matrix_local_pointers(const char* host, int port,
                                        ulong nrow, ulong ncol,
                                        double* vv, long* ii, long* oo, 
                                        ulong nelem) {
    ASSERT_PTR(vv); ASSERT_PTR(ii); ASSERT_PTR(oo);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering scipy crs-data in (python) client side    
    auto mdist = prepare_scattered_crs_matrices(vv, ii, oo, nrow, ncol, nelem,
                                                wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i=0; i<wsize; ++i) {
      res[i] = exrpc_async(nodes[i],load_local_data<S_LMAT15>,mdist[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  void get_dist_crs_II_matrix_local(exrpc_node& fm_node,
                                    std::vector<exrpc_ptr_t>& eps,
                                    DT4 *vv, DT4 *ii, DT3 *oo,
                                    ulong vsize, ulong osize){
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    std::vector<std::exception> exps_val(wsize);
    std::vector<int> is_except_val(wsize);
    std::vector<frovedis::crs_matrix_local<DT4,DT4,DT5>> vret_crs(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        vret_crs[i] = exrpc_async(nodes[i],(get_crs_matrix_local<DT4,DT4,DT5>),eps[i]).get(); 
      } catch (std::exception& e) {
          exps_val[i] = e;
          is_except_val[i] = true;
      }
    }
    for(size_t i = 0; i < wsize; ++i) {
      if(is_except_val[i]) throw exps_val[i];
    }
    merge_scattered_crs_matrices_impl(vret_crs, vv, ii, oo, vsize, osize); 
  }

  void get_dist_crs_IL_matrix_local(exrpc_node& fm_node,
                                    std::vector<exrpc_ptr_t>& eps,
                                    DT4 *vv, DT3 *ii, DT3 *oo, //, ii is of python c_long type
                                    ulong vsize, ulong osize){
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    std::vector<std::exception> exps_val(wsize);
    std::vector<int> is_except_val(wsize);
    std::vector<frovedis::crs_matrix_local<DT4,DT5,DT5>> vret_crs(wsize);

#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        vret_crs[i] = exrpc_async(nodes[i],(get_crs_matrix_local<DT4,DT5,DT5>),eps[i]).get(); 
      } catch (std::exception& e) {
          exps_val[i] = e;
          is_except_val[i] = true;
      }
    }
    for(size_t i = 0; i < wsize; ++i) {
      if(is_except_val[i]) throw exps_val[i];
    }
    merge_scattered_crs_matrices_impl(vret_crs, vv, ii, oo, vsize, osize); 
  }

  void get_dist_crs_LI_matrix_local(exrpc_node& fm_node,
                                    std::vector<exrpc_ptr_t>& eps,
                                    DT3 *vv, DT4 *ii, DT3 *oo,
                                    ulong vsize, ulong osize){
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    std::vector<std::exception> exps_val(wsize);
    std::vector<int> is_except_val(wsize);
    std::vector<frovedis::crs_matrix_local<DT3,DT4,DT5>> vret_crs(wsize);

#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        vret_crs[i] = exrpc_async(nodes[i],(get_crs_matrix_local<DT3,DT4,DT5>),eps[i]).get(); 
      } catch (std::exception& e) {
          exps_val[i] = e;
          is_except_val[i] = true;
      }
    }
    for(size_t i = 0; i < wsize; ++i) {
      if(is_except_val[i]) throw exps_val[i];
    }
    merge_scattered_crs_matrices_impl(vret_crs, vv, ii, oo, vsize, osize); 
  }

  void get_dist_crs_LL_matrix_local(exrpc_node& fm_node,
                                    std::vector<exrpc_ptr_t>& eps,
                                    DT3 *vv, DT3 *ii, DT3 *oo,
                                    ulong vsize, ulong osize){
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    std::vector<std::exception> exps_val(wsize);
    std::vector<int> is_except_val(wsize);
    std::vector<frovedis::crs_matrix_local<DT3,DT5,DT5>> vret_crs(wsize);

#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        vret_crs[i] = exrpc_async(nodes[i],(get_crs_matrix_local<DT3,DT5,DT5>),eps[i]).get(); 
      } catch (std::exception& e) {
          exps_val[i] = e;
          is_except_val[i] = true;
      }
    }
    for(size_t i = 0; i < wsize; ++i) {
      if(is_except_val[i]) throw exps_val[i];
    }
    merge_scattered_crs_matrices_impl(vret_crs, vv, ii, oo, vsize, osize); 
  }

  void get_dist_crs_FI_matrix_local(exrpc_node& fm_node,
                                    std::vector<exrpc_ptr_t>& eps,
                                    DT2 *vv, DT4 *ii, DT3 *oo,
                                    ulong vsize, ulong osize){
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    std::vector<std::exception> exps_val(wsize);
    std::vector<int> is_except_val(wsize);
    std::vector<frovedis::crs_matrix_local<DT2,DT4,DT5>> vret_crs(wsize);

#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        vret_crs[i] = exrpc_async(nodes[i],(get_crs_matrix_local<DT2,DT4,DT5>),eps[i]).get(); 
      } catch (std::exception& e) {
          exps_val[i] = e;
          is_except_val[i] = true;
      }
    }
    for(size_t i = 0; i < wsize; ++i) {
      if(is_except_val[i]) throw exps_val[i];
    }
    merge_scattered_crs_matrices_impl(vret_crs, vv, ii, oo, vsize, osize); 
  }

  void get_dist_crs_FL_matrix_local(exrpc_node& fm_node,
                                    std::vector<exrpc_ptr_t>& eps,
                                    DT2 *vv, DT3 *ii, DT3 *oo,
                                    ulong vsize, ulong osize){
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    std::vector<std::exception> exps_val(wsize);
    std::vector<int> is_except_val(wsize);
    std::vector<frovedis::crs_matrix_local<DT2,DT5,DT5>> vret_crs(wsize);

#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        vret_crs[i] = exrpc_async(nodes[i],(get_crs_matrix_local<DT2,DT5,DT5>),eps[i]).get(); 
      } catch (std::exception& e) {
          exps_val[i] = e;
          is_except_val[i] = true;
      }
    }
    for(size_t i = 0; i < wsize; ++i) {
      if(is_except_val[i]) throw exps_val[i];
    }
    merge_scattered_crs_matrices_impl(vret_crs, vv, ii, oo, vsize, osize); 
  }

  void get_dist_crs_DI_matrix_local(exrpc_node& fm_node,
                                    std::vector<exrpc_ptr_t>& eps,
                                    DT1 *vv, DT4 *ii, DT3 *oo,
                                    ulong vsize, ulong osize){
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    std::vector<std::exception> exps_val(wsize);
    std::vector<int> is_except_val(wsize);
    std::vector<frovedis::crs_matrix_local<DT1,DT4,DT5>> vret_crs(wsize);

#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        vret_crs[i] = exrpc_async(nodes[i],(get_crs_matrix_local<DT1,DT4,DT5>),eps[i]).get(); 
      } catch (std::exception& e) {
          exps_val[i] = e;
          is_except_val[i] = true;
      }
    }
    for(size_t i = 0; i < wsize; ++i) {
      if(is_except_val[i]) throw exps_val[i];
    }
    merge_scattered_crs_matrices_impl(vret_crs, vv, ii, oo, vsize, osize); 
  }

  void get_dist_crs_DL_matrix_local(exrpc_node& fm_node,
                                    std::vector<exrpc_ptr_t>& eps,
                                    DT1 *vv, DT3 *ii, DT3 *oo,
                                    ulong vsize, ulong osize){
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    std::vector<std::exception> exps_val(wsize);
    std::vector<int> is_except_val(wsize);
    std::vector<frovedis::crs_matrix_local<DT1,DT5,DT5>> vret_crs(wsize);

#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        vret_crs[i] = exrpc_async(nodes[i],(get_crs_matrix_local<DT1,DT5,DT5>),eps[i]).get(); 
      } catch (std::exception& e) {
          exps_val[i] = e;
          is_except_val[i] = true;
      }
    }
    for(size_t i = 0; i < wsize; ++i) {
      if(is_except_val[i]) throw exps_val[i];
    }
    merge_scattered_crs_matrices_impl(vret_crs, vv, ii, oo, vsize, osize); 
  }

void get_crs_matrix_components(const char* host, int port,
                               ulong dptr, void* vv, void* ii, void* oo,
                               short dtype, short itype, 
                               ulong vsize, ulong osize){

  if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
  exrpc_node fm_node(host,port);
  auto f_dptr = (exrpc_ptr_t) dptr;
  std::vector<exrpc_ptr_t> eps;
  try{
     /* creating a vector of crs_matrix local*/
    if(itype == INT) {
      switch(dtype){
        case INT:
          eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT4,DT4,DT5>),f_dptr).get();
          get_dist_crs_II_matrix_local(fm_node, eps, (DT4*) vv, (DT4*) ii, (DT3*) oo, vsize, osize);
          break;
        case LONG:
          eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT3,DT4,DT5>),f_dptr).get();
          get_dist_crs_LI_matrix_local(fm_node, eps, (DT3*) vv, (DT4*) ii, (DT3*) oo, vsize, osize);
          break;
        case FLOAT:
          eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT2,DT4,DT5>),f_dptr).get();
          get_dist_crs_FI_matrix_local(fm_node, eps, (DT2*) vv, (DT4*) ii, (DT3*) oo, vsize, osize);
          break;
        case DOUBLE:
          eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT1,DT4,DT5>),f_dptr).get();
          get_dist_crs_DI_matrix_local(fm_node, eps, (DT1*) vv, (DT4*) ii, (DT3*) oo, vsize, osize);
          break;
        default : REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
      }
    }
    else if(itype == LONG) {
      switch(dtype){
        case INT:
          eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT4,DT5,DT5>),f_dptr).get();
          get_dist_crs_IL_matrix_local(fm_node, eps, (DT4*) vv, (DT3*) ii, (DT3*) oo, vsize, osize);
          break;
        case LONG:
          eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT3,DT5,DT5>),f_dptr).get();
          get_dist_crs_LL_matrix_local(fm_node, eps, (DT3*) vv, (DT3*) ii, (DT3*) oo, vsize, osize);
          break;
        case FLOAT:
          eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT2,DT5,DT5>),f_dptr).get();
          get_dist_crs_FL_matrix_local(fm_node, eps, (DT2*) vv, (DT3*) ii, (DT3*) oo, vsize, osize);
          break;
        case DOUBLE:
          eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT1,DT5,DT5>),f_dptr).get();
          get_dist_crs_DL_matrix_local(fm_node, eps, (DT1*) vv, (DT3*) ii, (DT3*) oo, vsize, osize);
          break;
        default : REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
      }
    }
    else REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
}


/* transpose of crs_matrix */
PyObject* transpose_frovedis_sparse_matrix(const char* host, int port,
                                            long dptr, short dtype, short itype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    dummy_matrix ret;
    try {
      if(itype == INT){
         switch(dtype) {
           case DOUBLE: ret = exrpc_async(fm_node,(transpose_matrix<S_MAT14,S_LMAT14>),f_dptr).get(); break;
           case FLOAT:  ret = exrpc_async(fm_node,(transpose_matrix<S_MAT24,S_LMAT24>),f_dptr).get(); break;
           case LONG:   ret = exrpc_async(fm_node,(transpose_matrix<S_MAT34,S_LMAT34>),f_dptr).get(); break;
           case INT:    ret = exrpc_async(fm_node,(transpose_matrix<S_MAT44,S_LMAT44>),f_dptr).get(); break;
           default :    REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
         }
      }
      else if(itype == LONG){
         switch(dtype) {
           case DOUBLE: ret = exrpc_async(fm_node,(transpose_matrix<S_MAT15,S_LMAT15>),f_dptr).get(); break;
           case FLOAT:  ret = exrpc_async(fm_node,(transpose_matrix<S_MAT25,S_LMAT25>),f_dptr).get(); break;
           case LONG:   ret = exrpc_async(fm_node,(transpose_matrix<S_MAT35,S_LMAT35>),f_dptr).get(); break;
           case INT:    ret = exrpc_async(fm_node,(transpose_matrix<S_MAT45,S_LMAT45>),f_dptr).get(); break;
           default :    REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
         }
      }
      else REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }

/* crs_matrix to rowmajor_matrix*/
PyObject* csr_to_rowmajor_matrix(const char* host, int port,
                                 long dptr, short dtype, short itype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    dummy_matrix ret;
    try {
      if(itype == INT){
         switch(dtype) {
           case DOUBLE: ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT1,S_MAT14>),f_dptr).get(); break;
           case FLOAT:  ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT2,S_MAT24>),f_dptr).get(); break;
           case LONG:   ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT3,S_MAT34>),f_dptr).get(); break;
           case INT:    ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT4,S_MAT44>),f_dptr).get(); break;
           default :    REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
         }
      }
      else if(itype == LONG){
         switch(dtype) {
           case DOUBLE: ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT1,S_MAT15>),f_dptr).get(); break;
           case FLOAT:  ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT2,S_MAT25>),f_dptr).get(); break;
           case LONG:   ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT3,S_MAT35>),f_dptr).get(); break;
           case INT:    ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT4,S_MAT45>),f_dptr).get(); break;
           default :    REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
         }
      }
      else REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }

/* crs_matrix to colmajor_matrix*/
PyObject* csr_to_colmajor_matrix(const char* host, int port,
                                 long dptr, short dtype, short itype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    dummy_matrix ret;
    try {
      if(itype == INT){
         switch(dtype) {
           case DOUBLE: ret = exrpc_async(fm_node,(to_colmajor_matrix<DT1,S_MAT14>),f_dptr).get(); break;
           case FLOAT:  ret = exrpc_async(fm_node,(to_colmajor_matrix<DT2,S_MAT24>),f_dptr).get(); break;
           case LONG:   ret = exrpc_async(fm_node,(to_colmajor_matrix<DT3,S_MAT34>),f_dptr).get(); break;
           case INT:    ret = exrpc_async(fm_node,(to_colmajor_matrix<DT4,S_MAT44>),f_dptr).get(); break;
           default :    REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
         }
      }
      else if(itype == LONG){
         switch(dtype) {
           case DOUBLE: ret = exrpc_async(fm_node,(to_colmajor_matrix<DT1,S_MAT15>),f_dptr).get(); break;
           case FLOAT:  ret = exrpc_async(fm_node,(to_colmajor_matrix<DT2,S_MAT25>),f_dptr).get(); break;
           case LONG:   ret = exrpc_async(fm_node,(to_colmajor_matrix<DT3,S_MAT35>),f_dptr).get(); break;
           case INT:    ret = exrpc_async(fm_node,(to_colmajor_matrix<DT4,S_MAT45>),f_dptr).get(); break;
           default :    REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
         }
      }
      else REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }

  PyObject* compute_spmv(const char* host, int port,
                         long dptr, long vptr, 
                         short dtype, short itype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    auto f_vptr = (exrpc_ptr_t) vptr;
    dummy_vector dv;
    try {
      if(itype == INT) {
        switch(dtype) {
          case INT:    dv = exrpc_async(fm_node,(get_computed_spmv<DT4,DT4,DT5>),
                                     f_dptr,f_vptr,dtype).get(); break;
          case LONG:   dv = exrpc_async(fm_node,(get_computed_spmv<DT3,DT4,DT5>),
                                     f_dptr,f_vptr,dtype).get(); break;
          case FLOAT:  dv = exrpc_async(fm_node,(get_computed_spmv<DT2,DT4,DT5>),
                                     f_dptr,f_vptr,dtype).get(); break;
          case DOUBLE: dv = exrpc_async(fm_node,(get_computed_spmv<DT1,DT4,DT5>),
                                     f_dptr,f_vptr,dtype).get(); break;
          default: REPORT_ERROR(USER_ERROR, "compute_spmv: Unknown dtype is encountered!\n");
        }
      }
      else if(itype == LONG) {
        switch(dtype) {
          case INT:    dv = exrpc_async(fm_node,(get_computed_spmv<DT4,DT5,DT5>),
                                     f_dptr,f_vptr,dtype).get(); break;
          case LONG:   dv = exrpc_async(fm_node,(get_computed_spmv<DT3,DT5,DT5>),
                                     f_dptr,f_vptr,dtype).get(); break;
          case FLOAT:  dv = exrpc_async(fm_node,(get_computed_spmv<DT2,DT5,DT5>),
                                     f_dptr,f_vptr,dtype).get(); break;
          case DOUBLE: dv = exrpc_async(fm_node,(get_computed_spmv<DT1,DT5,DT5>),
                                     f_dptr,f_vptr,dtype).get(); break;
          default: REPORT_ERROR(USER_ERROR, "compute_spmv: Unknown dtype is encountered!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR, "Unknown crs matrix index type is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
   return to_py_dummy_vector(dv);
  }

#if 0
void get_crs_II_matrix_components(const char* host, int port,
                         ulong dptr, int* vv, int* ii, size_t* oo){

    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    //auto r = static_cast<size_t>(nrow);
    //auto nz_elem = static_cast<size_t>(nzelem);
    std::vector<exrpc_ptr_t> eps;
    try{
          /* creating a vector of crs_matrix local*/
        eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT4,DT4,DT5>),f_dptr).get();
        get_dist_crs_II_matrix_local(fm_node, eps, vv, ii, oo);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
   }

void get_crs_IL_matrix_components(const char* host, int port,
                         ulong dptr, int* vv, long* ii, size_t* oo){

    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    //auto r = static_cast<size_t>(nrow);
    //auto nz_elem = static_cast<size_t>(nzelem);
    std::vector<exrpc_ptr_t> eps;
    try{
          /* creating a vector of crs_matrix local*/
        eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT4,DT3,DT5>),f_dptr).get(); 
        get_dist_crs_IL_matrix_local(fm_node, eps, vv, ii, oo);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
   }

void get_crs_LI_matrix_components(const char* host, int port,
                         ulong dptr, long* vv, int* ii, size_t* oo){

    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    //auto r = static_cast<size_t>(nrow);
    //auto nz_elem = static_cast<size_t>(nzelem);
    std::vector<exrpc_ptr_t> eps;
    try{
          /* creating a vector of crs_matrix local*/
        eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT3,DT4,DT5>),f_dptr).get(); 
        get_dist_crs_LI_matrix_local(fm_node, eps, vv, ii, oo);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
   }

void get_crs_LL_matrix_components(const char* host, int port,
                         ulong dptr, long* vv, long* ii, size_t* oo){

    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    //auto r = static_cast<size_t>(nrow);
    //auto nz_elem = static_cast<size_t>(nzelem);
    std::vector<exrpc_ptr_t> eps;
    try{
          /* creating a vector of crs_matrix local*/
        eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT3,DT3,DT5>),f_dptr).get(); 
        get_dist_crs_LL_matrix_local(fm_node, eps, vv, ii, oo);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
   }

void get_crs_FI_matrix_components(const char* host, int port,
                         ulong dptr, float* vv, int* ii, size_t* oo){

    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    //auto r = static_cast<size_t>(nrow);
    //auto nz_elem = static_cast<size_t>(nzelem);
    std::vector<exrpc_ptr_t> eps;
    try{
          /* creating a vector of crs_matrix local*/
        eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT2,DT4,DT5>),f_dptr).get(); 
        get_dist_crs_FI_matrix_local(fm_node, eps, vv, ii, oo);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
   }

void get_crs_FL_matrix_components(const char* host, int port,
                         ulong dptr, float* vv, long* ii, size_t* oo){

    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    //auto r = static_cast<size_t>(nrow);
    //auto nz_elem = static_cast<size_t>(nzelem);
    std::vector<exrpc_ptr_t> eps;
    try{
          /* creating a vector of crs_matrix local*/
        eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT2,DT3,DT5>),f_dptr).get(); 
        get_dist_crs_FL_matrix_local(fm_node, eps, vv, ii, oo);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
   }

void get_crs_DI_matrix_components(const char* host, int port,
                         ulong dptr, double* vv, int* ii, size_t* oo){

    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    //auto r = static_cast<size_t>(nrow);
    //auto nz_elem = static_cast<size_t>(nzelem);
    std::vector<exrpc_ptr_t> eps;
    try{
          /* creating a vector of crs_matrix local*/
        eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT1,DT4,DT5>),f_dptr).get(); 
        get_dist_crs_DI_matrix_local(fm_node, eps, vv, ii, oo);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
   }


void get_crs_DL_matrix_components(const char* host, int port,
                         ulong dptr, double* vv, long* ii, size_t* oo){

    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    //auto r = static_cast<size_t>(nrow);
    //auto nz_elem = static_cast<size_t>(nzelem);
    std::vector<exrpc_ptr_t> eps;
    try{
          /* creating a vector of crs_matrix local*/
        eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT1,DT3,DT5>),f_dptr).get(); 
        get_dist_crs_DL_matrix_local(fm_node, eps, vv, ii, oo);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
   }
#endif


  PyObject* create_frovedis_crs_II_matrix(const char* host, int port,
                                          ulong nrow, ulong ncol,
                                          int* vv, int* ii, long* oo,
                                          ulong nelem) {
    auto eps = get_each_II_crs_matrix_local_pointers(host,port,nrow,ncol,
                                                     vv,ii,oo,nelem);
    // creating (frovedis) server side global matrix from dist local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = 0;
    dummy_matrix dmat;
    try {
      m = exrpc_async(fm_node,(create_crs_data<DT4,DT4,DT5>),eps,r,c).get();
      dmat = exrpc_async(fm_node,(to_dummy_matrix<S_MAT44,S_LMAT44>),m).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

  PyObject* create_frovedis_crs_IL_matrix(const char* host, int port,
                                          ulong nrow, ulong ncol,
                                          int* vv, long* ii, long* oo,
                                          ulong nelem) {
    auto eps = get_each_IL_crs_matrix_local_pointers(host,port,nrow,ncol,
                                                     vv,ii,oo,nelem);
    // creating (frovedis) server side global matrix from dist local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = 0;
    dummy_matrix dmat;
    try {
      m = exrpc_async(fm_node,(create_crs_data<DT4,DT5,DT5>),eps,r,c).get();
      dmat = exrpc_async(fm_node,(to_dummy_matrix<S_MAT45,S_LMAT45>),m).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

  PyObject* create_frovedis_crs_LI_matrix(const char* host, int port,
                                          ulong nrow, ulong ncol,
                                          long* vv, int* ii, long* oo,
                                          ulong nelem) {
    auto eps = get_each_LI_crs_matrix_local_pointers(host,port,nrow,ncol,
                                                     vv,ii,oo,nelem);
    // creating (frovedis) server side global matrix from dist local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = 0;
    dummy_matrix dmat;
    try {
      m = exrpc_async(fm_node,(create_crs_data<DT3,DT4,DT5>),eps,r,c).get();
      dmat = exrpc_async(fm_node,(to_dummy_matrix<S_MAT34,S_LMAT34>),m).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

  PyObject* create_frovedis_crs_LL_matrix(const char* host, int port,
                                          ulong nrow, ulong ncol,
                                          long* vv, long* ii, long* oo,
                                          ulong nelem) {
    auto eps = get_each_LL_crs_matrix_local_pointers(host,port,nrow,ncol,
                                                     vv,ii,oo,nelem);
    // creating (frovedis) server side global matrix from dist local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = 0;
    dummy_matrix dmat;
    try {
      m = exrpc_async(fm_node,(create_crs_data<DT3,DT5,DT5>),eps,r,c).get();
      dmat = exrpc_async(fm_node,(to_dummy_matrix<S_MAT35,S_LMAT35>),m).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

  PyObject* create_frovedis_crs_FI_matrix(const char* host, int port,
                                          ulong nrow, ulong ncol,
                                          float* vv, int* ii, long* oo,
                                          ulong nelem) {
    auto eps = get_each_FI_crs_matrix_local_pointers(host,port,nrow,ncol,
                                                     vv,ii,oo,nelem);
    // creating (frovedis) server side global matrix from dist local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = 0;
    dummy_matrix dmat;
    try {
      m = exrpc_async(fm_node,(create_crs_data<DT2,DT4,DT5>),eps,r,c).get();
      dmat = exrpc_async(fm_node,(to_dummy_matrix<S_MAT24,S_LMAT24>),m).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

  PyObject* create_frovedis_crs_FL_matrix(const char* host, int port,
                                          ulong nrow, ulong ncol,
                                          float* vv, long* ii, long* oo,
                                          ulong nelem) {
    auto eps = get_each_FL_crs_matrix_local_pointers(host,port,nrow,ncol,
                                                     vv,ii,oo,nelem);
    // creating (frovedis) server side global matrix from dist local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = 0;
    dummy_matrix dmat;
    try {
      m = exrpc_async(fm_node,(create_crs_data<DT2,DT5,DT5>),eps,r,c).get();
      dmat = exrpc_async(fm_node,(to_dummy_matrix<S_MAT25,S_LMAT25>),m).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  } 

  PyObject* create_frovedis_crs_DI_matrix(const char* host, int port,
                                          ulong nrow, ulong ncol,
                                          double* vv, int* ii, long* oo,
                                          ulong nelem) {
    auto eps = get_each_DI_crs_matrix_local_pointers(host,port,nrow,ncol,
                                                     vv,ii,oo,nelem);
    // creating (frovedis) server side global matrix from dist local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = 0;
    dummy_matrix dmat;
    try {
      m = exrpc_async(fm_node,(create_crs_data<DT1,DT4,DT5>),eps,r,c).get();
      dmat = exrpc_async(fm_node,(to_dummy_matrix<S_MAT14,S_LMAT14>),m).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

  PyObject* create_frovedis_crs_DL_matrix(const char* host, int port,
                                          ulong nrow, ulong ncol,
                                          double* vv, long* ii, long* oo,
                                          ulong nelem) {
    auto eps = get_each_DL_crs_matrix_local_pointers(host,port,nrow,ncol,
                                                     vv,ii,oo,nelem);
    // creating (frovedis) server side global matrix from dist local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = 0;
    dummy_matrix dmat;
    try {
      m = exrpc_async(fm_node,(create_crs_data<DT1,DT5,DT5>),eps,r,c).get();
      dmat = exrpc_async(fm_node,(to_dummy_matrix<S_MAT15,S_LMAT15>),m).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

  PyObject* load_frovedis_crs_matrix(const char* host, int port,
                                     const char* fname, bool bin,
                                     short dtype, short itype) {
    ASSERT_PTR(fname);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string path(fname);
    dummy_matrix m;
    try {
      if(itype == INT) {
         switch(dtype) {
           case INT:    m = exrpc_async(fm_node,(load_crs_matrix<DT4,DT4,DT5>),path,bin).get(); break;
           case LONG:   REPORT_ERROR(USER_ERROR,"long type crs matrix data can't be loaded from file!\n");
           case FLOAT:  m = exrpc_async(fm_node,(load_crs_matrix<DT2,DT4,DT5>),path,bin).get(); break;
           case DOUBLE: m = exrpc_async(fm_node,(load_crs_matrix<DT1,DT4,DT5>),path,bin).get(); break;
           default:     REPORT_ERROR(USER_ERROR,"Unsupported dtype for crs_matrix is encountered!\n");
         }
      }
      else if(itype == LONG) {
         switch(dtype) {
           case INT:    m = exrpc_async(fm_node,(load_crs_matrix<DT4,DT5,DT5>),path,bin).get(); break;
           case LONG:   REPORT_ERROR(USER_ERROR,"long type crs matrix data can't be loaded from file!\n");
           case FLOAT:  m = exrpc_async(fm_node,(load_crs_matrix<DT2,DT5,DT5>),path,bin).get(); break;
           case DOUBLE: m = exrpc_async(fm_node,(load_crs_matrix<DT1,DT5,DT5>),path,bin).get(); break;
           default:     REPORT_ERROR(USER_ERROR,"Unsupported dtype for crs_matrix is encountered!\n");
         }
      }
      else REPORT_ERROR(USER_ERROR, "Unsupported itype for crs_matrix is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(m);
  }

  void save_frovedis_crs_matrix(const char* host, int port,
                                long dptr, const char* path,
                                bool bin, short dtype, short itype) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::string fs_path(path);
    try {
      if(itype == INT){
         switch(dtype) {
           case INT:    exrpc_oneway(fm_node,save_matrix<S_MAT44>,f_dptr,fs_path,bin); break;
           case LONG:   exrpc_oneway(fm_node,save_matrix<S_MAT34>,f_dptr,fs_path,bin); break;
           case FLOAT:  exrpc_oneway(fm_node,save_matrix<S_MAT24>,f_dptr,fs_path,bin); break;
           case DOUBLE: exrpc_oneway(fm_node,save_matrix<S_MAT14>,f_dptr,fs_path,bin); break;
           default:     REPORT_ERROR(USER_ERROR,"Unsupported dtype for crs_matrix is encountered!\n");
         }
      }
      else if(itype == LONG){
         switch(dtype) {
           case INT:    exrpc_oneway(fm_node,save_matrix<S_MAT45>,f_dptr,fs_path,bin); break;
           case LONG:   exrpc_oneway(fm_node,save_matrix<S_MAT35>,f_dptr,fs_path,bin); break;
           case FLOAT:  exrpc_oneway(fm_node,save_matrix<S_MAT25>,f_dptr,fs_path,bin); break;
           case DOUBLE: exrpc_oneway(fm_node,save_matrix<S_MAT15>,f_dptr,fs_path,bin); break;
           default:     REPORT_ERROR(USER_ERROR,"Unsupported dtype for crs_matrix is encountered!\n");
         }
      }
      else REPORT_ERROR(USER_ERROR, "Unsupported itype for crs_matrix is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void release_frovedis_crs_matrix(const char* host, int port,
                                   long dptr, short dtype, short itype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    try {
      if(itype == INT){
         switch(dtype) {
           case INT:    exrpc_oneway(fm_node,release_data<S_MAT44>,f_dptr); break;
           case LONG:   exrpc_oneway(fm_node,release_data<S_MAT34>,f_dptr); break;
           case FLOAT:  exrpc_oneway(fm_node,release_data<S_MAT24>,f_dptr); break;
           case DOUBLE: exrpc_oneway(fm_node,release_data<S_MAT14>,f_dptr); break;
           default:     REPORT_ERROR(USER_ERROR,"Unsupported dtype for crs_matrix is encountered!\n");
         }
      }
      else if(itype == LONG){
         switch(dtype) {
           case INT:    exrpc_oneway(fm_node,release_data<S_MAT45>,f_dptr); break;
           case LONG:   exrpc_oneway(fm_node,release_data<S_MAT35>,f_dptr); break;
           case FLOAT:  exrpc_oneway(fm_node,release_data<S_MAT25>,f_dptr); break;
           case DOUBLE: exrpc_oneway(fm_node,release_data<S_MAT15>,f_dptr); break;
           default:     REPORT_ERROR(USER_ERROR,"Unsupported dtype for crs_matrix is encountered!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR, "Unsupported itype for crs_matrix is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void show_frovedis_crs_matrix(const char* host, int port,
                                long dptr, short dtype, short itype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    try {
      if(itype == INT){
         switch(dtype) {
           case INT:    exrpc_oneway(fm_node,show_data<S_MAT44>,f_dptr); break;
           case LONG:   exrpc_oneway(fm_node,show_data<S_MAT34>,f_dptr); break;
           case FLOAT:  exrpc_oneway(fm_node,show_data<S_MAT24>,f_dptr); break;
           case DOUBLE: exrpc_oneway(fm_node,show_data<S_MAT14>,f_dptr); break;
           default:     REPORT_ERROR(USER_ERROR,"Unsupported dtype for crs_matrix is encountered!\n");
         }
       }
      else if(itype == LONG){
         switch(dtype) {
           case INT:    exrpc_oneway(fm_node,show_data<S_MAT45>,f_dptr); break;
           case LONG:   exrpc_oneway(fm_node,show_data<S_MAT35>,f_dptr); break;
           case FLOAT:  exrpc_oneway(fm_node,show_data<S_MAT25>,f_dptr); break;
           case DOUBLE: exrpc_oneway(fm_node,show_data<S_MAT15>,f_dptr); break;
           default:     REPORT_ERROR(USER_ERROR,"Unsupported dtype for crs_matrix is encountered!\n");
         }
       }
    else REPORT_ERROR(USER_ERROR, "Unsupported itype for crs_matrix is encountered!\n");
   }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- Frovedis Dense matrices load/save/transpose/view/release ---
  std::vector<exrpc_ptr_t>
  get_each_double_rml_pointers_from_numpy_matrix(const char* host, int port,
                                                 ulong nrow, ulong ncol, 
                                                 double* vv) {
    ASSERT_PTR(vv); 
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering numpy matrix in (python) client side
    auto mdist = prepare_scattered_rowmajor_matrices(vv,nrow,ncol,wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i=0; i<wsize; ++i) { 
      res[i] = exrpc_async(nodes[i],load_local_data<R_LMAT1>,mdist[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  std::vector<exrpc_ptr_t>
  get_each_float_rml_pointers_from_numpy_matrix(const char* host, int port,
                                                ulong nrow, ulong ncol, 
                                                float* vv) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering numpy matrix in (python) client side
    auto mdist = prepare_scattered_rowmajor_matrices(vv,nrow,ncol,wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i=0; i<wsize; ++i) {
      res[i] = exrpc_async(nodes[i],load_local_data<R_LMAT2>,mdist[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }
  
  std::vector<exrpc_ptr_t>
  get_each_long_rml_pointers_from_numpy_matrix(const char* host, int port,
                                               ulong nrow, ulong ncol, 
                                               long* vv) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering numpy matrix in (python) client side
    auto mdist = prepare_scattered_rowmajor_matrices(vv,nrow,ncol,wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i=0; i<wsize; ++i) {
      res[i] = exrpc_async(nodes[i],load_local_data<R_LMAT3>,mdist[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  std::vector<exrpc_ptr_t>
  get_each_int_rml_pointers_from_numpy_matrix(const char* host, int port,
                                              ulong nrow, ulong ncol, 
                                              int* vv) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering numpy matrix in (python) client side
    auto mdist = prepare_scattered_rowmajor_matrices(vv,nrow,ncol,wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i=0; i<wsize; ++i) {
      res[i] = exrpc_async(nodes[i],load_local_data<R_LMAT4>,mdist[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  // create from python (numpy) data
  PyObject* create_frovedis_double_dense_matrix(const char* host, int port,
                                                ulong nrow, ulong  ncol, 
                                                double* vv, char mtype) {
    auto eps = get_each_double_rml_pointers_from_numpy_matrix(host,port,nrow,ncol,vv);
    // creating (frovedis) server side dense matrix from local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = 0;
    try {
      switch(mtype) {
        case 'R': m = exrpc_async(fm_node,create_rmm_data<DT1>,eps,r,c).get(); break;
        case 'C': m = exrpc_async(fm_node,create_cmm_data<DT1>,eps,r,c).get(); break;
        case 'B': m = exrpc_async(fm_node,create_bcm_data<DT1>,eps,r,c).get(); break;
        default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dummy_matrix(m, r, c));
  }
   
  PyObject* create_frovedis_float_dense_matrix(const char* host, int port,
                                               ulong nrow, ulong ncol,
                                               float* vv, char mtype) {
    auto eps = get_each_float_rml_pointers_from_numpy_matrix(host,port,nrow,ncol,vv);
    // creating (frovedis) server side dense matrix from local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = 0;
    try {
      switch(mtype) {
        case 'R': m = exrpc_async(fm_node,create_rmm_data<DT2>,eps,r,c).get(); break;
        case 'C': m = exrpc_async(fm_node,create_cmm_data<DT2>,eps,r,c).get(); break;
        case 'B': m = exrpc_async(fm_node,create_bcm_data<DT2>,eps,r,c).get(); break;
        default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }  
    return to_py_dummy_matrix(dummy_matrix(m, r, c));
  }

  PyObject* create_frovedis_long_dense_matrix(const char* host, int port,
                                              ulong nrow, ulong ncol,
                                              long* vv, char mtype) {
    auto eps = get_each_long_rml_pointers_from_numpy_matrix(host,port,nrow,ncol,vv);
    // creating (frovedis) server side dense matrix from local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = 0;
    try {
      switch(mtype) {
        case 'R': m = exrpc_async(fm_node,create_rmm_data<DT3>,eps,r,c).get(); break;
        case 'C': m = exrpc_async(fm_node,create_cmm_data<DT3>,eps,r,c).get(); break;
        case 'B': REPORT_ERROR(USER_ERROR,"blockcyclic_matrix<long> is not supported!\n"); 
        default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    } 
    return to_py_dummy_matrix(dummy_matrix(m, r, c));
  }
  
  PyObject* create_frovedis_int_dense_matrix(const char* host, int port,
                                             ulong nrow, ulong ncol,
                                             int* vv, char mtype) {
    auto eps = get_each_int_rml_pointers_from_numpy_matrix(host,port,nrow,ncol,vv);
    // creating (frovedis) server side dense matrix from local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = 0;
    try {
      switch(mtype) {
        case 'R': m = exrpc_async(fm_node,create_rmm_data<DT4>,eps,r,c).get(); break;
        case 'C': m = exrpc_async(fm_node,create_cmm_data<DT4>,eps,r,c).get(); break;
        case 'B': REPORT_ERROR(USER_ERROR,"blockcyclic_matrix<int> is not supported!\n"); 
        default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dummy_matrix(m, r, c));
  }

  // load from file
  PyObject* load_frovedis_dense_matrix(const char* host, int port,
                                       const char* path, 
                                       bool bin, char mtype, short dtype) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    dummy_matrix ret;
    try {
      if(mtype == 'R'){
         switch(dtype) {
           case DOUBLE: ret = exrpc_async(fm_node,load_rmm_matrix<DT1>,fs_path,bin).get(); break;
           case FLOAT:  ret = exrpc_async(fm_node,load_rmm_matrix<DT2>,fs_path,bin).get(); break;
           case LONG:   REPORT_ERROR(USER_ERROR,"rowmajor_matrix<long> can't be loaded from file!\n");
           case INT:    ret = exrpc_async(fm_node,load_rmm_matrix<DT4>,fs_path,bin).get(); break;
           default:     REPORT_ERROR(USER_ERROR,"Unknown rowmajor_matrix type is encountered!\n");
         }
      }
      else if(mtype == 'C'){
         switch(dtype) {
           case DOUBLE: ret = exrpc_async(fm_node,load_cmm_matrix<DT1>,fs_path,bin).get(); break;
           case FLOAT:  ret = exrpc_async(fm_node,load_cmm_matrix<DT2>,fs_path,bin).get(); break;
           case LONG:   REPORT_ERROR(USER_ERROR,"colmajor_matrix<long> can't be loaded from file!\n");
           case INT:    ret = exrpc_async(fm_node,load_cmm_matrix<DT4>,fs_path,bin).get(); break;
           default:     REPORT_ERROR(USER_ERROR,"Unknown colmajor_matrix type is encountered!\n");
         }
      }
      else if(mtype == 'B'){
         switch(dtype) {
           case DOUBLE: ret = exrpc_async(fm_node,load_bcm_matrix<DT1>,fs_path,bin).get(); break;
           case FLOAT:  ret = exrpc_async(fm_node,load_bcm_matrix<DT2>,fs_path,bin).get(); break;
           default:     REPORT_ERROR(USER_ERROR,"Unknown blockcyclic_matrix type is encountered!\n");
         }
      }
      else  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }

  void save_frovedis_dense_matrix(const char* host, int port,
                                  long dptr, const char* path, 
                                  bool bin, char mtype, short dtype) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::string fs_path(path);
    try {
      if(mtype == 'R') {
         switch(dtype) {
           case DOUBLE: exrpc_oneway(fm_node,save_matrix<R_MAT1>,f_dptr,fs_path,bin); break;
           case FLOAT:  exrpc_oneway(fm_node,save_matrix<R_MAT2>,f_dptr,fs_path,bin); break;
           case LONG:   exrpc_oneway(fm_node,save_matrix<R_MAT3>,f_dptr,fs_path,bin); break;
           case INT:    exrpc_oneway(fm_node,save_matrix<R_MAT4>,f_dptr,fs_path,bin); break;
           default:     REPORT_ERROR(USER_ERROR,"Unknown rowmajor_matrix type is encountered!\n");
         }
      }
      else if(mtype == 'C'){
         switch(dtype) {
           case DOUBLE: exrpc_oneway(fm_node,save_matrix<C_MAT1>,f_dptr,fs_path,bin); break;
           case FLOAT:  exrpc_oneway(fm_node,save_matrix<C_MAT2>,f_dptr,fs_path,bin); break;
           case LONG:   exrpc_oneway(fm_node,save_matrix<C_MAT3>,f_dptr,fs_path,bin); break;
           case INT:    exrpc_oneway(fm_node,save_matrix<C_MAT4>,f_dptr,fs_path,bin); break;
           default:     REPORT_ERROR(USER_ERROR,"Unknown colmajor_matrix type is encountered!\n");
         }
      }
      else if(mtype == 'B'){
         switch(dtype) {
           case DOUBLE:exrpc_oneway(fm_node,save_matrix<B_MAT1>,f_dptr,fs_path,bin); break;
           case FLOAT: exrpc_oneway(fm_node,save_matrix<B_MAT2>,f_dptr,fs_path,bin); break;
           default:    REPORT_ERROR(USER_ERROR,"Unknown blockcyclic_matrix type is encountered!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR, "Unknown dense matrix kind is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  PyObject* copy_frovedis_dense_matrix(const char* host, int port, 
                                       long dptr, char mtype, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    dummy_matrix ret;
    try {
      if(mtype == 'R') 
        REPORT_ERROR(USER_ERROR,"deepcopy a rowmajor_matrix is not supported!\n");
      else if(mtype == 'C') 
        REPORT_ERROR(USER_ERROR,"deepcopy a colmajor_matrix is not supported!\n");
      else if(mtype == 'B'){ 
         switch(dtype) {
           case DOUBLE: ret = exrpc_async(fm_node,(copy_matrix<B_MAT1,B_LMAT1>),f_dptr).get(); break; 
           case FLOAT:  ret = exrpc_async(fm_node,(copy_matrix<B_MAT2,B_LMAT2>),f_dptr).get(); break; 
           default:     REPORT_ERROR(USER_ERROR,"Unknown blockcyclic_matrix type is encountered!\n");
         }
       }
      else REPORT_ERROR(USER_ERROR, "Unknown dense matrix kind is encountered!\n");
    } 
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }
  
  PyObject* transpose_frovedis_dense_matrix(const char* host, int port, 
                                            long dptr, char mtype,short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    dummy_matrix ret;
    try {
      if(mtype == 'R'){
         switch(dtype) {
           case DOUBLE: ret = exrpc_async(fm_node,(transpose_matrix<R_MAT1,R_LMAT1>),f_dptr).get(); break;
           case FLOAT:  ret = exrpc_async(fm_node,(transpose_matrix<R_MAT2,R_LMAT2>),f_dptr).get(); break;
           case LONG:   ret = exrpc_async(fm_node,(transpose_matrix<R_MAT3,R_LMAT3>),f_dptr).get(); break;
           case INT:    ret = exrpc_async(fm_node,(transpose_matrix<R_MAT4,R_LMAT4>),f_dptr).get(); break;
           default:     REPORT_ERROR(USER_ERROR,"Unknown rowmajor_matrix type is encountered!\n");
         }
      }
      else if(mtype == 'C')
        REPORT_ERROR(USER_ERROR,"transpose on colmajor_matrix is not supported currently!\n");
      
      else if(mtype == 'B'){
         switch(dtype) {
           case DOUBLE: ret = exrpc_async(fm_node,(transpose_matrix<B_MAT1,B_LMAT1>),f_dptr).get(); break;
           case FLOAT:  ret = exrpc_async(fm_node,(transpose_matrix<B_MAT2,B_LMAT2>),f_dptr).get(); break;
           default:     REPORT_ERROR(USER_ERROR,"Unknown blockcyclic_matrix type is encountered!\n");
         }
      }
      else REPORT_ERROR(USER_ERROR, "Unknown dense matrix kind is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }
 
  // converts matrix to rowmajor_matrix
  PyObject* get_frovedis_rowmatrix(const char* host, int port, 
                                   long dptr, 
                                   ulong nrow, ulong ncol, 
                                   char mtype, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    dummy_matrix ret;
    try {
      if(mtype == 'R') ret = dummy_matrix(f_dptr,nrow,ncol);
      else if(mtype == 'C') {
        switch(dtype) {
          case DOUBLE: ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT1,C_MAT1>),f_dptr).get(); break;
          case FLOAT:  ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT2,C_MAT2>),f_dptr).get(); break;
          case LONG:   ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT3,C_MAT3>),f_dptr).get(); break;
          case INT:    ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT4,C_MAT4>),f_dptr).get(); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown colmajor_matrix type is encountered!\n");
        }
      }
      else if(mtype == 'B'){
        switch(dtype) {
          case DOUBLE: ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT1,B_MAT1>),f_dptr).get(); break;
          case FLOAT:  ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT2,B_MAT2>),f_dptr).get(); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown blockcyclic_matrix type is encountered!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR, "Unknown dense matrix kind is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }

  // --- Frovedis dense distributed matrix to Rowmajor Array Conversion ---
  std::vector<std::vector<double>> 
  get_double_rowmajor_local_arrays(const char* host, int port, 
                                   long dptr, char mtype, ulong sz) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<std::vector<double>> evs;
    try {
      // 1. convert input dense matrix to rowmajor_matrix at Frovedis server side
      // 2. get each rowmajor_matrix_local pointers
      std::vector<exrpc_ptr_t> eps;
      switch(mtype) {
        case 'R': eps = exrpc_async(fm_node,(get_all_local_pointers<R_MAT1,R_LMAT1>),f_dptr).get(); break;
        case 'C': eps = exrpc_async(fm_node,(convert_and_get_all_rml_pointers<DT1,C_MAT1>),f_dptr).get(); break;
        case 'B': eps = exrpc_async(fm_node,(convert_and_get_all_rml_pointers<DT1,B_MAT1>),f_dptr).get(); break;
        default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
      }
      // 3. gather local vectors from Frovedis worker nodes one-by-one
      auto nodes = get_worker_nodes(fm_node);
      auto wsize = nodes.size();
      std::vector<std::exception> exps(wsize);
      std::vector<int> is_except(wsize);
      evs.resize(wsize);
#pragma omp parallel for num_threads(wsize)
      for(size_t i = 0; i < wsize; ++i) {
        try {
          evs[i] = exrpc_async(nodes[i],(get_local_array<DT1,R_LMAT1>),eps[i]).get();
        } catch (std::exception& e) {
          exps[i] = e;
          is_except[i] = true;
        }
      }
      size_t total = 0;
      for(size_t i = 0; i < wsize; ++i) { 
        if(is_except[i]) throw exps[i];
        else total += evs[i].size();
      }
      // The gathered size and expected size from client side should match
      checkAssumption(total == sz);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return evs;
  }

  void get_double_rowmajor_array_as_int_array(const char* host, int port,
                                              long dptr, char mtype,
                                              int* ret, ulong sz) {
    auto evs = get_double_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side int buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<int>(evs[i][j]);
      }
    }
  }

  void get_double_rowmajor_array_as_long_array(const char* host, int port,
                                               long dptr, char mtype,
                                               long* ret, ulong sz) {
    auto evs = get_double_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side long buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<long>(evs[i][j]);
      }
    }
  }

  void get_double_rowmajor_array_as_float_array(const char* host, int port,
                                                long dptr, char mtype,
                                                float* ret, ulong sz) {
    auto evs = get_double_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side float buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<float>(evs[i][j]);
      }
    }
  }

  void get_double_rowmajor_array_as_double_array(const char* host, int port,
                                                 long dptr, char mtype,
                                                 double* ret, ulong sz) {
    auto evs = get_double_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side double buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<double>(evs[i][j]);
      }
    }
  }

  std::vector<std::vector<float>> 
  get_float_rowmajor_local_arrays(const char* host, int port,
                                  long dptr, char mtype, ulong sz) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<std::vector<float>> evs;
    try {
      // 1. convert input dense matrix to rowmajor_matrix at Frovedis server side
      // 2. get each rowmajor_matrix_local pointers
      std::vector<exrpc_ptr_t> eps;
      switch(mtype) {
        case 'R': eps = exrpc_async(fm_node,(get_all_local_pointers<R_MAT2,R_LMAT2>),f_dptr).get(); break;
        case 'C': eps = exrpc_async(fm_node,(convert_and_get_all_rml_pointers<DT2,C_MAT2>),f_dptr).get(); break;
        case 'B': eps = exrpc_async(fm_node,(convert_and_get_all_rml_pointers<DT2,B_MAT2>),f_dptr).get(); break;
        default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
      }
      // 3. gather local vectors from Frovedis worker nodes one-by-one
      auto nodes = get_worker_nodes(fm_node);
      auto wsize = nodes.size();
      std::vector<std::exception> exps(wsize);
      std::vector<int> is_except(wsize);
      evs.resize(wsize);
#pragma omp parallel for num_threads(wsize)
      for(size_t i = 0; i < wsize; ++i) {
        try {
          evs[i] = exrpc_async(nodes[i],(get_local_array<DT2,R_LMAT2>),eps[i]).get();
        } catch (std::exception& e) {
          exps[i] = e;
          is_except[i] = true;
        }
      }
      size_t total = 0;
      for(size_t i = 0; i < wsize; ++i) { 
        if(is_except[i]) throw exps[i];
        else total += evs[i].size();
      }
      // The gathered size and expected size from client side should match
      checkAssumption(total == sz);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return evs;
  }

  void get_float_rowmajor_array_as_int_array(const char* host, int port,
                                             long dptr, char mtype,
                                             int* ret, ulong sz) {
    auto evs = get_float_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side int buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<int>(evs[i][j]);
      }
    }
  }

  void get_float_rowmajor_array_as_long_array(const char* host, int port,
                                              long dptr, char mtype,
                                              long* ret, ulong sz) {
    auto evs = get_float_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side long buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<long>(evs[i][j]);
      }
    }
  }

  void get_float_rowmajor_array_as_float_array(const char* host, int port,
                                               long dptr, char mtype,
                                               float* ret, ulong sz) {
    auto evs = get_float_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side float buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<float>(evs[i][j]);
      }
    }
  }

  void get_float_rowmajor_array_as_double_array(const char* host, int port,
                                                long dptr, char mtype,
                                                double* ret, ulong sz) {
    auto evs = get_float_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side double buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<double>(evs[i][j]);
      }
    }
  }

  std::vector<std::vector<long>> 
  get_long_rowmajor_local_arrays(const char* host, int port,
                                 long dptr, char mtype, ulong sz) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<std::vector<long>> evs;
    try {
      // 1. convert input dense matrix to rowmajor_matrix at Frovedis server side
      // 2. get each rowmajor_matrix_local pointers
      std::vector<exrpc_ptr_t> eps;
      switch(mtype) {
        case 'R': eps = exrpc_async(fm_node,(get_all_local_pointers<R_MAT3,R_LMAT3>),f_dptr).get(); break;
        case 'C': eps = exrpc_async(fm_node,(convert_and_get_all_rml_pointers<DT3,C_MAT3>),f_dptr).get(); break;
        case 'B': REPORT_ERROR(USER_ERROR,"blockcylic matrix<long> is not supported!\n");break;
        default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
      }
      // 3. gather local vectors from Frovedis worker nodes one-by-one
      auto nodes = get_worker_nodes(fm_node);
      auto wsize = nodes.size();
      std::vector<std::exception> exps(wsize);
      std::vector<int> is_except(wsize);
      evs.resize(wsize);
#pragma omp parallel for num_threads(wsize)
      for(size_t i = 0; i < wsize; ++i) {
        try {
          evs[i] = exrpc_async(nodes[i],(get_local_array<DT3,R_LMAT3>),eps[i]).get();
        } catch (std::exception& e) {
          exps[i] = e;
          is_except[i] = true;
        }
      }
      size_t total = 0;
      for(size_t i = 0; i < wsize; ++i) { 
        if(is_except[i]) throw exps[i];
        else total += evs[i].size();
      }
      // The gathered size and expected size from client side should match
      checkAssumption(total == sz);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return evs;
  }

  void get_long_rowmajor_array_as_int_array(const char* host, int port,
                                            long dptr, char mtype,
                                            int* ret, ulong sz) {
    auto evs = get_long_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side int buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<int>(evs[i][j]);
      }
    }
  }

  void get_long_rowmajor_array_as_long_array(const char* host, int port,
                                             long dptr, char mtype,
                                             long* ret, ulong sz) {
    auto evs = get_long_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side long buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<long>(evs[i][j]);
      }
    }
  }

  void get_long_rowmajor_array_as_float_array(const char* host, int port,
                                              long dptr, char mtype,
                                              float* ret, ulong sz) {
    auto evs = get_long_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side float buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<float>(evs[i][j]);
      }
    }
  }

  void get_long_rowmajor_array_as_double_array(const char* host, int port,
                                               long dptr, char mtype,
                                               double* ret, ulong sz) {
    auto evs = get_long_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side double buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<double>(evs[i][j]);
      }
    }
  }

  std::vector<std::vector<int>> 
  get_int_rowmajor_local_arrays(const char* host, int port,
                                long dptr, char mtype, ulong sz) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<std::vector<int>> evs;
    try {
      // 1. convert input dense matrix to rowmajor_matrix at Frovedis server side
      // 2. get each rowmajor_matrix_local pointers
      std::vector<exrpc_ptr_t> eps;
      switch(mtype) {
        case 'R': eps = exrpc_async(fm_node,(get_all_local_pointers<R_MAT4,R_LMAT4>),f_dptr).get(); break;
        case 'C': eps = exrpc_async(fm_node,(convert_and_get_all_rml_pointers<DT4,C_MAT4>),f_dptr).get(); break;
        case 'B': REPORT_ERROR(USER_ERROR,"blockcylic_matrix<int> is not supported!\n");break;
        default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
      }
      // 3. gather local vectors from Frovedis worker nodes one-by-one
      auto nodes = get_worker_nodes(fm_node);
      auto wsize = nodes.size();
      std::vector<std::exception> exps(wsize);
      std::vector<int> is_except(wsize);
      evs.resize(wsize);
#pragma omp parallel for num_threads(wsize)
      for(size_t i = 0; i < wsize; ++i) {
        try {
          evs[i] = exrpc_async(nodes[i],(get_local_array<DT4,R_LMAT4>),eps[i]).get();
        } catch (std::exception& e) {
          exps[i] = e;
          is_except[i] = true;
        }
      }
      size_t total = 0;
      for(size_t i = 0; i < wsize; ++i) { 
        if(is_except[i]) throw exps[i];
        else total += evs[i].size();
      }
      // The gathered size and expected size from client side should match
      checkAssumption(total == sz);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return evs;
  }

  void get_int_rowmajor_array_as_int_array(const char* host, int port,
                                           long dptr, char mtype,
                                           int* ret, ulong sz) {
    auto evs = get_int_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side int buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<int>(evs[i][j]);
      }
    }
  }

  void get_int_rowmajor_array_as_long_array(const char* host, int port,
                                            long dptr, char mtype,
                                            long* ret, ulong sz) {
    auto evs = get_int_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side long buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<long>(evs[i][j]);
      }
    }
  }

  void get_int_rowmajor_array_as_float_array(const char* host, int port,
                                             long dptr, char mtype,
                                             float* ret, ulong sz) {
    auto evs = get_int_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side float buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<float>(evs[i][j]);
      }
    }
  }

  void get_int_rowmajor_array_as_double_array(const char* host, int port,
                                              long dptr, char mtype,
                                              double* ret, ulong sz) {
    auto evs = get_int_rowmajor_local_arrays(host, port, dptr, mtype, sz);
    auto size = evs.size();
    std::vector<size_t> sizepfx(size, 0);
    for(size_t i = 0; i < size-1; i++) sizepfx[i+1] = sizepfx[i] + evs[i].size();
    // 4. fill the gathered data in client side double buffer (ret)
#pragma omp parallel for num_threads(size)
    for(size_t i = 0; i < size; ++i) {
      for(size_t j = 0; j < evs[i].size(); ++j) {
        ret[sizepfx[i]+j] = static_cast<double>(evs[i][j]);
      }
    }
  }

// Following cast_and_copy methods are C wrapper for python functions 
// defined in dense.py. It does not use frovedis server to do any task...
  void I2I_cast_and_copy_array(int* src, int* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = target[i];
    }
  }

  void I2L_cast_and_copy_array(int* src, long* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = static_cast<int>(target[i]);
    }
  }

  void I2F_cast_and_copy_array(int* src, float* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = static_cast<int>(target[i]);
    }
  }

  void I2D_cast_and_copy_array(int* src, double* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = static_cast<int>(target[i]);
    }
  }

  void L2I_cast_and_copy_array(long* src, int* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = static_cast<long>(target[i]);
    }
  }

  void L2L_cast_and_copy_array(long* src, long* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = target[i];
    }
  }

  void L2F_cast_and_copy_array(long* src, float* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = static_cast<long>(target[i]);
    }
  }

  void L2D_cast_and_copy_array(long* src, double* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = static_cast<long>(target[i]);
    }
  }

  void F2I_cast_and_copy_array(float* src, int* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = static_cast<float>(target[i]);
    }
  }

  void F2L_cast_and_copy_array(float* src, long* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = static_cast<float>(target[i]);
    }
  }

  void F2F_cast_and_copy_array(float* src, float* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = target[i];
    }
  }

  void F2D_cast_and_copy_array(float* src, double* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = static_cast<float>(target[i]);
    }
  }

  void D2I_cast_and_copy_array(double* src, int* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = static_cast<double>(target[i]);
    }
  }

  void D2L_cast_and_copy_array(double* src, long* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = static_cast<double>(target[i]);
    }
  }

  void D2F_cast_and_copy_array(double* src, float* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = static_cast<double>(target[i]);
    }
  }

  void D2D_cast_and_copy_array(double* src, double* target, ulong size) {
    for(size_t i = 0; i < size; ++i) {
      src[i] = target[i];
    }
  }

  void show_frovedis_dense_matrix(const char* host, int port, 
                                  long dptr, char mtype, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    try {
      if(mtype == 'R'){
        switch(dtype) {
          case DOUBLE: exrpc_oneway(fm_node,show_data<R_MAT1>,f_dptr); break;
          case FLOAT:  exrpc_oneway(fm_node,show_data<R_MAT2>,f_dptr); break;
          case LONG:   exrpc_oneway(fm_node,show_data<R_MAT3>,f_dptr); break;
          case INT:    exrpc_oneway(fm_node,show_data<R_MAT4>,f_dptr); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown rowmajor_matrix type is encountered!\n");
        }
      }
      else if(mtype == 'C'){
        switch(dtype) {
          case DOUBLE: exrpc_oneway(fm_node,show_data<C_MAT1>,f_dptr); break;
          case FLOAT:  exrpc_oneway(fm_node,show_data<C_MAT2>,f_dptr); break;
          case LONG:   exrpc_oneway(fm_node,show_data<C_MAT3>,f_dptr); break;
          case INT:    exrpc_oneway(fm_node,show_data<C_MAT4>,f_dptr); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown colmajor_matrix type is encountered!\n");
        }
      }
      else if(mtype == 'B'){
        switch(dtype) {
          case DOUBLE: exrpc_oneway(fm_node,show_data<B_MAT1>,f_dptr); break;
          case FLOAT:  exrpc_oneway(fm_node,show_data<B_MAT2>,f_dptr); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown blockcyclic_matrix type is encountered!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR, "Unknown dense matrix kind is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void release_frovedis_dense_matrix(const char* host, int port, 
                                   long dptr, char mtype, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    try {
      if(mtype == 'R'){
        switch(dtype) {
          case DOUBLE: exrpc_oneway(fm_node,release_data<R_MAT1>,f_dptr); break;
          case FLOAT:  exrpc_oneway(fm_node,release_data<R_MAT2>,f_dptr); break;
          case LONG:   exrpc_oneway(fm_node,release_data<R_MAT3>,f_dptr); break;
          case INT:    exrpc_oneway(fm_node,release_data<R_MAT4>,f_dptr); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown rowmajor_matrix type is encountered!\n");
        }
     }
     else if(mtype == 'C'){
       switch(dtype) {
         case DOUBLE: exrpc_oneway(fm_node,release_data<C_MAT1>,f_dptr); break;
         case FLOAT:  exrpc_oneway(fm_node,release_data<C_MAT2>,f_dptr); break;
         case LONG:   exrpc_oneway(fm_node,release_data<C_MAT3>,f_dptr); break;
         case INT:    exrpc_oneway(fm_node,release_data<C_MAT4>,f_dptr); break;
         default:     REPORT_ERROR(USER_ERROR,"Unknown colmajor_matrix type is encountered!\n");
       }
     }
     else if(mtype == 'B'){
       switch(dtype) {
         case DOUBLE: exrpc_oneway(fm_node,release_data<B_MAT1>,f_dptr); break;
         case FLOAT:  exrpc_oneway(fm_node,release_data<B_MAT2>,f_dptr); break;
         default:     REPORT_ERROR(USER_ERROR,"Unknown blockcyclic_matrix type is encountered!\n");
       }
     }
     else REPORT_ERROR(USER_ERROR, "Unknown dense matrix kind is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

}
