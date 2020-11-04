#include "python_client_headers.hpp"
#include "exrpc_data_storage.hpp"
#include "short_hand_dtype.hpp"

extern "C" {

  int get_distinct_count(const char* host, int port, long dptr, int vtype) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    int count = 0;
    try {
      switch(vtype) {
        case INT:    count = exrpc_async(fm_node,count_distinct<int>,f_dptr).get(); break;
        case LONG:   count = exrpc_async(fm_node,count_distinct<long>,f_dptr).get(); break;
        case FLOAT:  count = exrpc_async(fm_node,count_distinct<float>,f_dptr).get(); break;
        case DOUBLE: count = exrpc_async(fm_node,count_distinct<double>,f_dptr).get(); break;
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

  long encode_frovedis_int_dvector(const char* host, int port, 
                                   long dptr, int* src, int* target, long size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<int> src_vec(size), target_vec(size);
    for(size_t i = 0; i < size; ++i) {
      src_vec[i] = src[i];
      target_vec[i] = target[i];
    }
    exrpc_ptr_t proxy = 0;
    try {
      proxy = exrpc_async(fm_node,get_encoded_dvector<int>,f_dptr,src_vec,target_vec).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (long) proxy;
  }

  long encode_frovedis_long_dvector(const char* host, int port,
                                    long dptr, long* src, long* target, long size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<long> src_vec(size), target_vec(size);
    for(size_t i = 0; i < size; ++i) {
      src_vec[i] = src[i];
      target_vec[i] = target[i];
    }
    exrpc_ptr_t proxy = 0;
    try {
      proxy = exrpc_async(fm_node,get_encoded_dvector<long>,f_dptr,src_vec,target_vec).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (long) proxy;
  }

  long encode_frovedis_float_dvector(const char* host, int port,
                                     long dptr, float* src, float* target, long size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<float> src_vec(size), target_vec(size);
    for(size_t i = 0; i < size; ++i) {
      src_vec[i] = src[i];
      target_vec[i] = target[i];
    }
    exrpc_ptr_t proxy = 0;
    try {
      proxy = exrpc_async(fm_node,get_encoded_dvector<float>,f_dptr,src_vec,target_vec).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (long) proxy;
  }

  long encode_frovedis_double_dvector(const char* host, int port,
                                      long dptr, double* src, double* target, long size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<double> src_vec(size), target_vec(size);
    for(size_t i = 0; i < size; ++i) {
      src_vec[i] = src[i];
      target_vec[i] = target[i];
    }
    exrpc_ptr_t proxy = 0;
    try {
      proxy = exrpc_async(fm_node,get_encoded_dvector<double>,f_dptr,src_vec,target_vec).get(); 
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

  std::vector<exrpc_ptr_t>
  get_each_dist_vec_int_pointers(const char* host, int port,
                                 int* vv, int size) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering vector locally at client side
    std::vector<int> vec(size);
    for(size_t i=0; i<size; ++i) vec[i] = vv[i];
    auto evs = get_scattered_vectors<int>(vec,size,1,wsize);
    // sending the scattered pices to Frovedis server
    std::vector<exrpc_ptr_t> eps(wsize);
    try {
      for(size_t i=0; i<wsize; ++i) {
        eps[i] = exrpc_async(nodes[i],(load_local_data<std::vector<int>>),evs[i]).get();
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return eps;
  }

  PyObject* create_frovedis_int_dvector(const char* host, int port,
                                        int* vv, int size) {
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
                                  long* vv, int size) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting Frovedis server information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering vector locally at client side
    std::vector<long> vec(size);
    for(size_t i=0; i<size; ++i) vec[i] = vv[i];
    auto evs = get_scattered_vectors<long>(vec,size,1,wsize);
    // sending the scattered pices to Frovedis server
    std::vector<exrpc_ptr_t> eps(wsize);
    try {
      for(size_t i=0; i<wsize; ++i) {
        eps[i] = exrpc_async(nodes[i],(load_local_data<std::vector<long>>),evs[i]).get();
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return eps;    
  }

  PyObject* create_frovedis_long_dvector(const char* host, int port,
                                         long* vv, int size) {
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
  get_each_dist_vec_float_pointers(const char* host, int port,
                                   float* vv, int size) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting Frovedis server information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering vector locally at client side
    std::vector<float> vec(size);
    for(size_t i=0; i<size; ++i) vec[i] = vv[i];
    auto evs = get_scattered_vectors<float>(vec,size,1,wsize);
    // sending the scattered pices to Frovedis server
    std::vector<exrpc_ptr_t> eps(wsize);
    try {
      for(size_t i=0; i<wsize; ++i) {
        eps[i] = exrpc_async(nodes[i],(load_local_data<std::vector<float>>),evs[i]).get();
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return eps;
  }

  PyObject* create_frovedis_float_dvector(const char* host, int port,
                                          float* vv, int size) {
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
                                    double* vv, int size) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting Frovedis server information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering vector locally at client side
    std::vector<double> vec(size);
    for(size_t i=0; i<size; ++i) vec[i] = vv[i];
    auto evs = get_scattered_vectors<double>(vec,size,1,wsize);
    // sending the scattered pices to Frovedis server
    std::vector<exrpc_ptr_t> eps(wsize);
    try {
      for(size_t i=0; i<wsize; ++i) {
        eps[i] = exrpc_async(nodes[i],(load_local_data<std::vector<double>>),evs[i]).get();
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return eps;
  }

  PyObject* create_frovedis_double_dvector(const char* host, int port,
                                           double* vv, int size) {
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
                                    const char** vv, int size) {
    ASSERT_PTR(vv);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting Frovedis server information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    
    // scattering vector locally at client side
    std::vector<std::string> vec(size);
    for(size_t i=0; i<size; ++i) vec[i] = std::string(vv[i]);
    //for(size_t i =0; i<size; i++) cout << vv[i] << " "; cout << endl;
    auto evs = get_scattered_vectors<std::string>(vec,size,1,wsize);
    // sending the scattered pices to Frovedis server
    std::vector<exrpc_ptr_t> eps(wsize);
    try {
      for(size_t i=0; i<wsize; ++i) {
        eps[i] = exrpc_async(nodes[i],(load_local_data<std::vector<std::string>>),evs[i]).get();
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return eps;
  }

  PyObject* create_frovedis_string_dvector(const char* host, int port,
                                           const char** vv, int size) {
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

}
