#include "python_client_headers.hpp"
#include "exrpc_data_storage.hpp"
#include "short_hand_dtype.hpp"

template <class T>
std::vector<std::vector<T>>
prepare_scattered_vectors(const std::vector<T>& vec,
                          size_t vecsz, size_t wsize) {
  auto sizevec = get_block_sizes(vecsz, wsize);
  std::vector<size_t> sidx(wsize); sidx[0] = 0;
  for(size_t i = 1; i < wsize; ++i) sidx[i] = sidx[i - 1] + sizevec[i - 1];
  std::vector<std::vector<T>> ret(wsize);
  const T* vecp = &vec[0];
#pragma omp parallel for num_threads(wsize)
  for(size_t i = 0; i < wsize; i++) {
    ret[i].resize(sizevec[i]);
    auto srcp = vecp + sidx[i];
    for(size_t j = 0; j < sizevec[i]; j++) ret[i][j] = srcp[j];
  }
  //for (auto &v: ret) show("local-vec: ", v);
  return ret;
}

template <class T>
std::vector<T>
merge_vectors(const std::vector<std::vector<T>>& vec) {
  auto nvec = vec.size();
  if(nvec == 0) return std::vector<T>();
  size_t rsize = 0;
  for(size_t i = 0; i < nvec; ++i) rsize += vec[i].size();
  std::vector<T> ret(rsize);
  auto rdata = ret.data();
  size_t k = 0;
  for(size_t i = 0; i < nvec; ++i) {
    auto vdata = vec[i].data();
    auto vsize = vec[i].size();
    for(size_t j = 0; j < vsize; ++j) rdata[k + j] = vdata[j];
    k += vsize;
  }
  return ret;
}

extern "C" {

  int get_distinct_count(const char* host, int port, long dptr, int vtype) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    int count = 0;
    try {
      switch(vtype) {
        case INT:    count = exrpc_async(fm_node,count_distinct<int>,
                                         f_dptr).get(); break;
        case LONG:   count = exrpc_async(fm_node,count_distinct<long>,
                                         f_dptr).get(); break;
        case ULONG:  count = exrpc_async(fm_node,count_distinct<unsigned long>,
                                         f_dptr).get(); break;
        case FLOAT:  count = exrpc_async(fm_node,count_distinct<float>,
                                         f_dptr).get(); break;
        case DOUBLE: count = exrpc_async(fm_node,count_distinct<double>,
                                         f_dptr).get(); break;
        //case STRING: count = exrpc_async(fm_node,count_distinct<std::string>,f_dptr).get(); break;
        default:  REPORT_ERROR(USER_ERROR,
                  "Unknown type for frovedis dvector: " + std::to_string(vtype));
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return count;
  }

  long encode_frovedis_int_dvector(const char* host, int port, long dptr,
                                   int* src, int* target, ulong size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    auto src_vec = to_int_vector(src, size);
    auto target_vec = to_int_vector(target, size);
    exrpc_ptr_t proxy = 0;
    try {
      proxy = exrpc_async(fm_node, get_encoded_dvector<int>,
                          f_dptr, src_vec, target_vec).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (long) proxy;
  }

  long encode_frovedis_long_dvector(const char* host, int port, long dptr,
                                    long* src, long* target, ulong size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    auto src_vec = to_long_vector(src, size);
    auto target_vec = to_long_vector(target, size);
    exrpc_ptr_t proxy = 0;
    try {
      proxy = exrpc_async(fm_node, get_encoded_dvector<long>,
                          f_dptr, src_vec, target_vec).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (long) proxy;
  }

  long encode_frovedis_ulong_dvector(const char* host, int port, long dptr,
                                     unsigned long* src, 
                                     unsigned long* target, ulong size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    auto src_vec = to_ulong_vector(src, size);
    auto target_vec = to_ulong_vector(target, size);
    exrpc_ptr_t proxy = 0;
    try {
      proxy = exrpc_async(fm_node, get_encoded_dvector<unsigned long>,
                          f_dptr, src_vec, target_vec).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (long) proxy;
  }

  long encode_frovedis_float_dvector(const char* host, int port, long dptr,
                                     float* src, float* target, ulong size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    auto src_vec = to_float_vector(src, size);
    auto target_vec = to_float_vector(target, size);
    exrpc_ptr_t proxy = 0;
    try {
      proxy = exrpc_async(fm_node, get_encoded_dvector<float>,
                          f_dptr, src_vec, target_vec).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (long) proxy;
  }

  long encode_frovedis_double_dvector(const char* host, int port, long dptr,
                                      double* src, double* target, ulong size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    auto src_vec = to_double_vector(src, size);
    auto target_vec = to_double_vector(target, size);
    exrpc_ptr_t proxy = 0;
    try {
      proxy = exrpc_async(fm_node, get_encoded_dvector<double>,
                          f_dptr, src_vec, target_vec).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (long) proxy;
  }

  long encode_frovedis_dvector_zero_based(const char* host, int port,
                                          long dptr, short dtype) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    exrpc_ptr_t proxy = 0;
    try {
      switch(dtype) {
        case INT:     proxy = exrpc_async(fm_node,get_encoded_dvector_zero_based<int>,f_dptr).get();
                      break;
        case LONG:    proxy = exrpc_async(fm_node,get_encoded_dvector_zero_based<long>,f_dptr).get();
                      break;
        case ULONG:   proxy = exrpc_async(fm_node,get_encoded_dvector_zero_based<unsigned long>,f_dptr).get();
                      break;
        case FLOAT:   proxy = exrpc_async(fm_node,get_encoded_dvector_zero_based<float>,f_dptr).get();
                      break;
        case DOUBLE:  proxy = exrpc_async(fm_node,get_encoded_dvector_zero_based<double>,f_dptr).get();
                      break;
        default:  REPORT_ERROR(USER_ERROR, "encode: Supported dtypes are int/long/float/double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (long) proxy;
  }

  PyObject* distinct_elements(const char* host, int port, long dptr, int vtype) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    PyObject* ret = NULL;
    try {
      switch(vtype) {
        case INT:   { auto int_col  = exrpc_async(fm_node,get_distinct_elements<int>,f_dptr).get(); 
                      ret = to_python_int_list(int_col); break;
                    }
        case LONG:  { auto long_col  = exrpc_async(fm_node,get_distinct_elements<long>,f_dptr).get(); 
                      ret = to_python_long_list(long_col); break;
                    }
        case ULONG: { auto ulong_col  = exrpc_async(fm_node,get_distinct_elements<unsigned long>,f_dptr).get(); 
                      ret = to_python_ulong_list(ulong_col); break;
                    }
        case FLOAT: { auto float_col  = exrpc_async(fm_node,get_distinct_elements<float>,f_dptr).get(); 
                      ret = to_python_float_list(float_col); break;
                    }
        case DOUBLE:{ auto double_col  = exrpc_async(fm_node,get_distinct_elements<double>,f_dptr).get(); 
                      ret = to_python_double_list(double_col); break;
                    }
        default:  REPORT_ERROR(USER_ERROR,
                "Unknown type for frovedis dvector: " + std::to_string(vtype));
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  // --- dvector creation/show/release ---
  void show_frovedis_dvector(const char* host, int port, long dptr, int vtype) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    try {
      switch(vtype) {
        case INT: 
          exrpc_oneway(fm_node,show_dvector<int>,f_dptr);
          break;
        case LONG:
          exrpc_oneway(fm_node,show_dvector<long>,f_dptr);
          break;
        case ULONG:
          exrpc_oneway(fm_node,show_dvector<unsigned long>,f_dptr);
          break;
        case FLOAT:
          exrpc_oneway(fm_node,show_dvector<float>,f_dptr);
          break;
        case DOUBLE: 
          exrpc_oneway(fm_node,show_dvector<double>,f_dptr);
          break;
        case STRING: 
          exrpc_oneway(fm_node,show_dvector<std::string>,f_dptr);
          break;
        default:  REPORT_ERROR(USER_ERROR,
                "Unknown type for frovedis dvector: " + std::to_string(vtype));
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void release_frovedis_dvector(const char* host, int port, long dptr, int vtype) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    try {
      switch(vtype) {
        case INT:
          exrpc_oneway(fm_node,release_dvector<int>,f_dptr);
          break;
        case LONG:
          exrpc_oneway(fm_node,release_dvector<long>,f_dptr);
          break;
        case ULONG:
          exrpc_oneway(fm_node,release_dvector<unsigned long>,f_dptr);
          break;
        case FLOAT:
          exrpc_oneway(fm_node,release_dvector<float>,f_dptr);
          break;
        case DOUBLE:
          exrpc_oneway(fm_node,release_dvector<double>,f_dptr);
          break;
        case STRING:
          exrpc_oneway(fm_node,release_dvector<std::string>,f_dptr);
          break;
        default:  REPORT_ERROR(USER_ERROR,
                "Unknown type for frovedis dvector: " + std::to_string(vtype));
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- data transfer ---
  std::vector<exrpc_ptr_t>
  get_each_dist_vec_int_pointers(const char* host, int port,
                                 int* vv, ulong size) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering vector locally at client side
    auto vec = to_int_vector(vv, size);
    auto evs = prepare_scattered_vectors(vec, size, wsize);
    // sending the scattered pices to Frovedis server
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      res[i] = exrpc_async(nodes[i], (load_local_data<std::vector<int>>), evs[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  PyObject* create_frovedis_int_dvector(const char* host, int port,
                                        int* vv, ulong size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto eps = get_each_dist_vec_int_pointers(host,port,vv,size);
    exrpc_ptr_t dptr = 0;
    try {
      dptr = exrpc_async(fm_node,create_and_set_dvector<int>,eps).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_vector(dummy_vector(dptr,size,INT));
  }
  
  std::vector<exrpc_ptr_t>
  get_each_dist_vec_long_pointers(const char* host, int port,
                                  long* vv, ulong size) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting Frovedis server information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering vector locally at client side
    auto vec = to_long_vector(vv, size);
    auto evs = prepare_scattered_vectors(vec, size, wsize);
    // sending the scattered pices to Frovedis server
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      res[i] = exrpc_async(nodes[i], (load_local_data<std::vector<long>>), evs[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  PyObject* create_frovedis_long_dvector(const char* host, int port,
                                         long* vv, ulong size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto eps = get_each_dist_vec_long_pointers(host,port,vv,size);
    exrpc_ptr_t dptr = 0;
    try {
      dptr = exrpc_async(fm_node,create_and_set_dvector<long>,eps).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_vector(dummy_vector(dptr,size,LONG));
  }

  std::vector<exrpc_ptr_t>
  get_each_dist_vec_ulong_pointers(const char* host, int port,
                                   unsigned long* vv, ulong size) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting Frovedis server information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering vector locally at client side
    auto vec = to_ulong_vector(vv, size);
    auto evs = prepare_scattered_vectors(vec, size, wsize);
    // sending the scattered pices to Frovedis server
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      res[i] = exrpc_async(nodes[i], (load_local_data<std::vector<unsigned long>>), evs[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  PyObject* create_frovedis_ulong_dvector(const char* host, int port,
                                          unsigned long* vv, ulong size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host,port);
    auto eps = get_each_dist_vec_ulong_pointers(host,port,vv,size);
    exrpc_ptr_t dptr = 0;
    try {
      dptr = exrpc_async(fm_node,create_and_set_dvector<unsigned long>,eps).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_vector(dummy_vector(dptr,size,ULONG));
  }

  std::vector<exrpc_ptr_t>
  get_each_dist_vec_float_pointers(const char* host, int port,
                                   float* vv, ulong size) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting Frovedis server information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering vector locally at client side
    auto vec = to_float_vector(vv, size); 
    auto evs = prepare_scattered_vectors(vec, size, wsize);
    // sending the scattered pices to Frovedis server
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      res[i] = exrpc_async(nodes[i], (load_local_data<std::vector<float>>), evs[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  PyObject* create_frovedis_float_dvector(const char* host, int port,
                                          float* vv, ulong size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto eps = get_each_dist_vec_float_pointers(host,port,vv,size);
    exrpc_ptr_t dptr = 0;
    try {
      dptr = exrpc_async(fm_node,create_and_set_dvector<float>,eps).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_vector(dummy_vector(dptr,size,FLOAT));
  }

  std::vector<exrpc_ptr_t>
  get_each_dist_vec_double_pointers(const char* host, int port,
                                    double* vv, ulong size) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting Frovedis server information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering vector locally at client side
    auto vec = to_double_vector(vv, size); 
    auto evs = prepare_scattered_vectors(vec, size, wsize);
    // sending the scattered pices to Frovedis server
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      res[i] = exrpc_async(nodes[i], (load_local_data<std::vector<double>>), evs[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  PyObject* create_frovedis_double_dvector(const char* host, int port,
                                           double* vv, ulong size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto eps = get_each_dist_vec_double_pointers(host,port,vv,size);
    exrpc_ptr_t dptr = 0;
    try {
      dptr = exrpc_async(fm_node,create_and_set_dvector<double>,eps).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_vector(dummy_vector(dptr,size,DOUBLE));
  }

  std::vector<exrpc_ptr_t>
  get_each_dist_vec_string_pointers(const char* host, int port,
                                    const char** vv, ulong size) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting Frovedis server information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    
    // scattering vector locally at client side
    auto vec = to_string_vector(vv, size);
    auto evs = prepare_scattered_vectors(vec, size, wsize);
    // sending the scattered pices to Frovedis server
    std::vector<exrpc_ptr_t> eps(wsize);
    std::vector<exrpc_result<exrpc_ptr_t>> res(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      res[i] = exrpc_async(nodes[i], (load_local_data<std::vector<std::string>>), evs[i]);
    }
    get_exrpc_result(eps, res, wsize);
    return eps;
  }

  PyObject* create_frovedis_string_dvector(const char* host, int port,
                                           const char** vv, ulong size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto eps = get_each_dist_vec_string_pointers(host,port,vv,size);
    exrpc_ptr_t dptr = 0;
    try {
      dptr = exrpc_async(fm_node,create_and_set_dvector<std::string>,eps).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_vector(dummy_vector(dptr,size,STRING));
  }

  std::vector<std::vector<int>>
  get_local_int_vectors(std::vector<exrpc_node>& nodes,
                        std::vector<exrpc_ptr_t>& eps,
                        std::vector<std::exception>& exps,
                        std::vector<int>& is_except,
                        size_t wsize, ulong dvsz) {
    std::vector<std::vector<int>> evs(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        evs[i] = exrpc_async(nodes[i], get_local_vector<int>, eps[i]).get();
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
    checkAssumption(total == dvsz);
    return evs;
  }

  std::vector<std::vector<long>>
  get_local_long_vectors(std::vector<exrpc_node>& nodes,
                         std::vector<exrpc_ptr_t>& eps,
                         std::vector<std::exception>& exps,
                         std::vector<int>& is_except,
                         size_t wsize, ulong dvsz) {
    std::vector<std::vector<long>> evs(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        evs[i] = exrpc_async(nodes[i], get_local_vector<long>, eps[i]).get();
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
    checkAssumption(total == dvsz);
    return evs;
  }

  std::vector<std::vector<unsigned long>>
  get_local_ulong_vectors(std::vector<exrpc_node>& nodes,
                          std::vector<exrpc_ptr_t>& eps,
                          std::vector<std::exception>& exps,
                          std::vector<int>& is_except,
                          size_t wsize, ulong dvsz) {
    std::vector<std::vector<unsigned long>> evs(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        evs[i] = exrpc_async(nodes[i], get_local_vector<unsigned long>, eps[i]).get();
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
    checkAssumption(total == dvsz);
    return evs;
  }

  std::vector<std::vector<float>>
  get_local_float_vectors(std::vector<exrpc_node>& nodes,
                          std::vector<exrpc_ptr_t>& eps,
                          std::vector<std::exception>& exps,
                          std::vector<int>& is_except,
                          size_t wsize, ulong dvsz) {
    std::vector<std::vector<float>> evs(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        evs[i] = exrpc_async(nodes[i], get_local_vector<float>, eps[i]).get();
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
    checkAssumption(total == dvsz);
    return evs;
  }

  std::vector<std::vector<double>>
  get_local_double_vectors(std::vector<exrpc_node>& nodes,
                           std::vector<exrpc_ptr_t>& eps,
                           std::vector<std::exception>& exps,
                           std::vector<int>& is_except,
                           size_t wsize, ulong dvsz) {
    std::vector<std::vector<double>> evs(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        evs[i] = exrpc_async(nodes[i], get_local_vector<double>, eps[i]).get();
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
    checkAssumption(total == dvsz);
    return evs;
  }

  std::vector<std::vector<std::string>>
  get_local_string_vectors(std::vector<exrpc_node>& nodes,
                           std::vector<exrpc_ptr_t>& eps,
                           std::vector<std::exception>& exps,
                           std::vector<int>& is_except,
                           size_t wsize, ulong dvsz) {
    std::vector<std::vector<std::string>> evs(wsize);
#pragma omp parallel for num_threads(wsize)
    for(size_t i = 0; i < wsize; ++i) {
      try {
        evs[i] = exrpc_async(nodes[i], get_local_vector<std::string>, eps[i]).get();
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
    checkAssumption(total == dvsz);
    return evs;
  }

  PyObject* dvector_to_numpy_array(const char* host, int port,
                                   long proxy, short dtype, ulong dvsz) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto proxy_ = (exrpc_ptr_t) proxy;
    PyObject* ret = NULL;
    try {
      // step 1. get each local pointers
      std::vector<exrpc_ptr_t> eps;
      switch(dtype) {
        case INT:    eps = exrpc_async(fm_node, get_dvector_local_pointers<int>, 
                                       proxy_).get(); break;
        case LONG:   eps = exrpc_async(fm_node, get_dvector_local_pointers<long>, 
                                       proxy_).get(); break;
        case ULONG:  eps = exrpc_async(fm_node, get_dvector_local_pointers<unsigned long>, 
                                       proxy_).get(); break;
        case FLOAT:  eps = exrpc_async(fm_node, get_dvector_local_pointers<float>, 
                                       proxy_).get(); break;
        case DOUBLE: eps = exrpc_async(fm_node, get_dvector_local_pointers<double>, 
                                       proxy_).get(); break;
        case STRING: eps = exrpc_async(fm_node, get_dvector_local_pointers<std::string>, 
                                       proxy_).get(); break;
        default:  REPORT_ERROR(USER_ERROR,
                  "Unknown type for frovedis dvector: " + std::to_string(dtype));
      }
      // step 2. gather local vectors from Frovedis worker nodes one-by-one
      auto nodes = get_worker_nodes(fm_node);
      auto wsize = nodes.size();
      std::vector<std::exception> exps(wsize);
      std::vector<int> is_except(wsize);
      switch(dtype) {
        case INT:    { auto evs = get_local_int_vectors(nodes, eps, exps,
                                                        is_except, wsize, dvsz);
                       ret = to_python_int_list(merge_vectors(evs)); break;
                     }
        case LONG:   { auto evs = get_local_long_vectors(nodes, eps, exps,
                                                         is_except, wsize, dvsz);
                       ret = to_python_long_list(merge_vectors(evs)); break;
                     }
        case ULONG:  { auto evs = get_local_ulong_vectors(nodes, eps, exps,
                                                          is_except, wsize, dvsz);
                       ret = to_python_ulong_list(merge_vectors(evs)); break;
                     }
        case FLOAT:  { auto evs = get_local_float_vectors(nodes, eps, exps,
                                                          is_except, wsize, dvsz);
                       ret = to_python_float_list(merge_vectors(evs)); break;
                     }
        case DOUBLE: { auto evs = get_local_double_vectors(nodes, eps, exps,
                                                           is_except, wsize, dvsz);
                       ret = to_python_double_list(merge_vectors(evs)); break;
                     }
        case STRING: { auto evs = get_local_string_vectors(nodes, eps, exps,
                                                           is_except, wsize, dvsz);
                       ret = to_python_string_list(merge_vectors(evs)); break;
                     }
        default:  REPORT_ERROR(USER_ERROR,
                  "Unknown type for frovedis dvector: " + std::to_string(dtype));
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }
}
