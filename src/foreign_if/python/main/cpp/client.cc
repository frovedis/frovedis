#include <Python.h>
#include <typeinfo>
#include "exrpc_request_headers.hpp"

using namespace frovedis;

extern "C" {

  // std::vector<std::string> => python List of strings
  PyObject* to_python_string_list (std::vector<std::string>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) PyList_Append(PList,Py_BuildValue("s",each.c_str()));
    return Py_BuildValue("O",PList);
  }

  // std::vector<std::string> => python List of doubles
  PyObject* to_python_double_list_from_str_vector (std::vector<std::string>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) {
      auto val = std::stod(each);
      PyList_Append(PList,Py_BuildValue("d",val));
    }
    return Py_BuildValue("O",PList);
  }

  // std::vector<int> => python List of integers
  PyObject* to_python_int_list (std::vector<int>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) PyList_Append(PList,Py_BuildValue("i",each));
    return Py_BuildValue("O",PList);
  }

  // std::vector<long> => python List of long integers
  PyObject* to_python_long_list (std::vector<long>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) PyList_Append(PList,Py_BuildValue("l",each));
    return Py_BuildValue("O",PList);
  }

  // std::vector<float> => python List of floats
  PyObject* to_python_float_list (std::vector<float>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) PyList_Append(PList,Py_BuildValue("f",each));
    return Py_BuildValue("O",PList);
  }

  // std::vector<double> => python List of doubles
  PyObject* to_python_double_list (std::vector<double>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) PyList_Append(PList,Py_BuildValue("d",each));
    return Py_BuildValue("O",PList);
  }

  // --- Frovedis server Initialization, Finalization, Query ---
  PyObject* initialize_server(const char *cmd) {
    if(!cmd) REPORT_ERROR(USER_ERROR,"Invalid server command!!");
    auto n = invoke_frovedis_server(cmd);
    auto host = n.hostname.c_str();
    auto port = n.rpcport; 
    return Py_BuildValue("{s:s, s:i}", "hostname", host, "rpcport", port); 
  }
  
  int get_worker_size(const char* host, int port) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    return exrpc_async0(fm_node, get_nodesize).get();
  }

  std::vector<exrpc_node> get_worker_nodes(exrpc_node& fm_node) {  
    auto info = prepare_parallel_exrpc(fm_node);
    auto nodes = get_parallel_exrpc_nodes(fm_node, info);
    wait_parallel_exrpc(fm_node, info);
    return nodes;
  }

  void clean_server(const char* host, int port) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    exrpc_oneway0(fm_node, cleanup_frovedis_server);
  }

  void finalize_server(const char* host, int port) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    finalize_frovedis_server(fm_node);
  }

  // --- Frovedis Data structure to Python Data structure ---
  PyObject* to_py_dummy_matrix(dummy_matrix& m) {
    return Py_BuildValue("{s:l, s:i, s:i}", 
                         "dptr", (long)m.mptr, "nrow", m.nrow, "ncol", m.ncol);
  }

  PyObject* to_py_mfm_info(dummy_mfm& m) {
    return Py_BuildValue("{s:i, s:i, s:i}", 
                         "rank", m.rank, "nrow", m.nrow, "ncol", m.ncol);
  }

  PyObject* to_py_dummy_vector(exrpc_ptr_t& ptr, int size, int vtype) {
    return Py_BuildValue("{s:l, s:i, s:i}", "dptr", (long)ptr, "size", size, "vtype", vtype);

  }

  PyObject* to_py_gesvd_result(gesvd_result& obj,
                               char mtype, bool isU, bool isV) {
    auto mt = (mtype == 'C') ? "C" : "B";
    long uptr = isU ? (long)obj.umat_ptr : 0;
    long vptr = isV ? (long)obj.vmat_ptr : 0;
    long sptr = (long)obj.svec_ptr;
    return Py_BuildValue("{s:s, s:l, s:l, s:l, s:i, s:i, s:i, s:i}", 
                         "mtype", mt, 
                         "uptr", uptr, "vptr", vptr, "sptr", sptr,
                         "m", obj.m, "n", obj.n, "k", obj.k, 
                         "info", obj.info);
  }

  PyObject* to_py_getrf_result(getrf_result& obj,char mtype) {
    auto mt = (mtype == 'C') ? "C" : "B";
    long dptr = (long)obj.ipiv_ptr;
    return Py_BuildValue("{s:s, s:l, s:i}", 
                         "mtype", mt, "dptr", dptr, "info", obj.info);
  }

  // --- Frovedis sparse matrices load/save/view/release ---
  std::vector<exrpc_ptr_t>
  get_each_crs_matrix_local_pointers(const char* host, int port,
                                     int nrow, int ncol, 
                                     double* vv, int* ii, int* oo, int nelem) {
    ASSERT_PTR(vv); ASSERT_PTR(ii); ASSERT_PTR(oo);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering scipy crs-data in (python) client side
    crs_matrix_local<double> mloc(nrow, ncol);
    mloc.copy_from_jarray(oo,ii,vv,nelem);
    auto mdist = get_scattered_crs_matrices(mloc,wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    for(size_t i=0; i<wsize; ++i) { 
      eps[i] = exrpc_async(nodes[i],load_local_data<S_LMAT1>,mdist[i]).get();
    }
    return eps;
  }

  PyObject* create_frovedis_sparse_matrix(const char* host, int port,
                                        int nrow, int ncol, 
                                        double* vv, int* ii, int* oo, 
                                        int nelem, char mtype) {
    auto eps = get_each_crs_matrix_local_pointers(host,port,nrow,ncol,
                                                  vv,ii,oo,nelem);
    // creating (frovedis) server side global matrix from dist local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = -1;
    switch(mtype) {
      case 'R': m = exrpc_async(fm_node,create_crs_data<DT1>,eps,r,c).get(); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown sparse matrix kind is encountered!\n");
    }
    dummy_matrix dmat(m,r,c);
    return to_py_dummy_matrix(dmat);
  }
  
  PyObject* load_frovedis_sparse_matrix(const char* host, int port,
                                      const char* fname, bool bin,
                                      char mtype) {
    ASSERT_PTR(fname);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string path(fname);
    dummy_matrix m;
    switch(mtype) {
      case 'R': m = exrpc_async(fm_node, load_crs_matrix<DT1>, path, bin).get(); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown sparse matrix kind is encountered!\n");
    }
    return to_py_dummy_matrix(m);
  }

  void save_frovedis_sparse_matrix(const char* host, int port,
                                 long dptr, const char* path,
                                 bool bin, char mtype) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::string fs_path(path);
    switch(mtype) {
      case 'R': exrpc_oneway(fm_node,save_matrix<S_MAT1>,f_dptr,fs_path,bin); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown sparse matrix kind is encountered!\n");
    }
  }

  void release_frovedis_sparse_matrix(const char* host, int port, 
                                    long dptr, char mtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    switch(mtype) {
      case 'R': exrpc_oneway(fm_node,release_data<S_MAT1>,f_dptr); break;
      default:  REPORT_ERROR(USER_ERROR,
                "Unknown sparse matrix kind is encountered!\n");
    }
  }
  
  void show_frovedis_sparse_matrix(const char* host, int port, 
                                 long dptr, char mtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    switch(mtype) {
      case 'R': exrpc_oneway(fm_node,show_data<S_MAT1>,f_dptr); break;
      default:  REPORT_ERROR(USER_ERROR,
                "Unknown sparse matrix kind is encountered!\n");
    }
  }
  
  void show_frovedis_dvector(const char* host, int port, long dptr, int vtype) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
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
  
  void release_frovedis_dvector(const char* host, int port, long dptr, int vtype) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
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
    for(size_t i=0; i<wsize; ++i) {
      eps[i] = exrpc_async(nodes[i],(load_local_data<std::vector<int>>),evs[i]).get();
    }
    return eps;
  }

  PyObject* create_frovedis_int_dvector(const char* host, int port,
                                  int* vv, int size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto eps = get_each_dist_vec_int_pointers(host,port,vv,size);
    auto dptr = exrpc_async(fm_node,create_and_set_dvector<int>,eps).get();
    return to_py_dummy_vector(dptr,size,INT);
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
    for(size_t i=0; i<wsize; ++i) {
      eps[i] = exrpc_async(nodes[i],(load_local_data<std::vector<long>>),evs[i]).get();
    }
    return eps;
  }

  PyObject* create_frovedis_long_dvector(const char* host, int port,
                                  long* vv, int size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto eps = get_each_dist_vec_long_pointers(host,port,vv,size);
    auto dptr = exrpc_async(fm_node,create_and_set_dvector<long>,eps).get();
    return to_py_dummy_vector(dptr,size,LONG);
  }

  //For Float Support
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
    for(size_t i=0; i<wsize; ++i) {
      eps[i] = exrpc_async(nodes[i],(load_local_data<std::vector<float>>),evs[i]).get();
    }
    return eps;
  }

  PyObject* create_frovedis_float_dvector(const char* host, int port,
                                  float* vv, int size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto eps = get_each_dist_vec_float_pointers(host,port,vv,size);
    auto dptr = exrpc_async(fm_node,create_and_set_dvector<float>,eps).get();
    return to_py_dummy_vector(dptr,size,FLOAT);
  }

  //For Double Support
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
    for(size_t i=0; i<wsize; ++i) {
      eps[i] = exrpc_async(nodes[i],(load_local_data<std::vector<double>>),evs[i]).get();
    }
    return eps;
  }

  PyObject* create_frovedis_double_dvector(const char* host, int port,
                                  double* vv, int size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto eps = get_each_dist_vec_double_pointers(host,port,vv,size);
    auto dptr = exrpc_async(fm_node,create_and_set_dvector<double>,eps).get();
    return to_py_dummy_vector(dptr,size,DOUBLE);
  }

  //For String Support
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
    for(size_t i=0; i<wsize; ++i) {
      eps[i] = exrpc_async(nodes[i],(load_local_data<std::vector<std::string>>),evs[i]).get();
    }
    return eps;
  }

  PyObject* create_frovedis_string_dvector(const char* host, int port,
                                         const char** vv, int size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto eps = get_each_dist_vec_string_pointers(host,port,vv,size);
    auto dptr = exrpc_async(fm_node,create_and_set_dvector<std::string>,eps).get();
    return to_py_dummy_vector(dptr,size,STRING);
  }

  // To create dataframe from dvector proxies
  long create_frovedis_dataframe(const char* host, int port, short* types,
                                 const char** col_name, long* dvec, int size){
    ASSERT_PTR(host); 
    std::vector<short> col_types(size);
    std::vector<exrpc_ptr_t> dvec_arr(size);
    for(int i = 0 ; i < size ; ++i) 
      col_types[i] = types[i];
    for(int i = 0; i < size; ++i) {
      dvec_arr[i] = static_cast<exrpc_ptr_t>(dvec[i]);
    }
    std::vector<std::string> col_names(size);
    for(int i = 0 ; i < size ; ++i) col_names[i] = col_name[i];
    exrpc_node fm_node(host,port);
    auto df_proxy = exrpc_async(fm_node,create_dataframe,col_types,col_names,dvec_arr).get();
    return (static_cast<long>(df_proxy));
  }

  // To Print data frame
  void show_frovedis_dataframe(const char* host, int port, long proxy){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    exrpc_oneway(fm_node,show_dataframe,df_proxy);
  }

  // To support df operator
  long get_frovedis_dfoperator(const char* host, int port, const char* op1,
                             const char* op2, short dtype, short opt, 
                             bool isImmed){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    std::string opd1(op1);
    std::string opd2(op2);
    exrpc_ptr_t ret_proxy = -1;
    switch(dtype) {
      case INT:
        ret_proxy = exrpc_async(fm_node,get_dfoperator<int>,opd1,opd2,opt,isImmed).get();
        break;
      case LONG:
        ret_proxy = exrpc_async(fm_node,get_dfoperator<long>,opd1,opd2,opt,isImmed).get();
        break;
      case FLOAT:
        ret_proxy = exrpc_async(fm_node, get_dfoperator<float>,opd1,opd2,opt,isImmed).get();
        break;
      case DOUBLE:
        ret_proxy = exrpc_async(fm_node, get_dfoperator<double>,opd1,opd2,opt,isImmed).get();
        break;
      case STRING:
        ret_proxy = exrpc_async(fm_node, get_str_dfoperator, opd1,opd2,opt,isImmed).get();
        break;
      default: 
        REPORT_ERROR(USER_ERROR,"Unknown type is encountered!\n");  
        break;
    }
    return (static_cast<long>(ret_proxy));
  }                  

  //To support dfAND operator
  long get_frovedis_dfANDoperator(const char* host,int port,long op1,long op2){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto lopt_proxy = static_cast<exrpc_ptr_t> (op1); 
    auto ropt_proxy = static_cast<exrpc_ptr_t> (op2);
    auto ret_proxy = exrpc_async(fm_node, get_dfANDoperator,lopt_proxy,ropt_proxy).get();
    return (static_cast<long>(ret_proxy));
  }
  
  //To support dfOR operator
  long get_frovedis_dfORoperator(const char* host,int port,long op1,long op2){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto lopt_proxy = static_cast<exrpc_ptr_t> (op1);
    auto ropt_proxy = static_cast<exrpc_ptr_t> (op2);
    auto ret_proxy = exrpc_async(fm_node, get_dfORoperator,lopt_proxy,ropt_proxy).get();
    return (static_cast<long>(ret_proxy));
  }
  
  //To release data frame
  void release_frovedis_dataframe(const char* host,int port,long proxy){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    exrpc_oneway(fm_node,release_data<dftable>,df_proxy);
  }
  
  //To release dfoperator
  void release_dfoperator(const char* host,int port,
                          long proxy){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    exrpc_oneway(fm_node,(release_data<std::shared_ptr<dfoperator>>),df_proxy);
  }
  
  // To filter rows from the given dataframe based on the given condition
  long filter_frovedis_dataframe(const char* host, int port, long proxy1, 
                                 long proxy2){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto pro1 = static_cast<exrpc_ptr_t> (proxy1);
    auto pro2 = static_cast<exrpc_ptr_t> (proxy2); 
    auto proxy = exrpc_async(fm_node, filter_df,pro1, pro2).get();
    return (static_cast<long>(proxy));
  } 
 
  // To select requested columns from given dataframe
  long select_frovedis_dataframe(const char* host, int port, long proxy, 
                               const char **cols, int size){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> targets(size);
    for(size_t i=0; i<size; ++i) targets[i] = std::string(cols[i]); 
    auto ret_proxy = exrpc_async(fm_node,select_df,df_proxy,targets).get();
    return (static_cast<long>(ret_proxy));
  }
   
  // To sort dataframe entries based on given column and requested order
  long sort_frovedis_dataframe(const char* host, int port, long proxy, 
                               const char** cols, int size, bool asc){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> targets(size);
    for(size_t i=0; i<size; ++i) targets[i] = std::string(cols[i]);
    auto isdesc = asc ? false : true;
    auto ret_proxy = exrpc_async(fm_node,sort_df,df_proxy,targets,isdesc).get();
    return (static_cast<long>(ret_proxy));
  }

  // To perform groupBy based on given columns 
  long group_frovedis_dataframe(const char* host, int port, long proxy, 
                               const char** cols, int size){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> targets(size);
    for(size_t i=0; i<size; ++i) targets[i] = std::string(cols[i]);
    auto ret_proxy = exrpc_async(fm_node,group_by_df,df_proxy,targets).get();
    return (static_cast<long>(ret_proxy));
  }
   
  // To perform join operation 
  long join_frovedis_dataframe(const char* host, int port,  
                               long proxy1, long proxy2, long proxy3,
                               const char* kind, const char* type){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto left = static_cast<exrpc_ptr_t> (proxy1);
    auto right = static_cast<exrpc_ptr_t> (proxy2); 
    auto opt = static_cast<exrpc_ptr_t> (proxy3); 
    std::string k(kind), t(type);
    auto ret_proxy = exrpc_async(fm_node, join_df, left, right, opt, k, t).get();
    return (static_cast<long>(ret_proxy));
  } 

  // To rename dataframe columns (returns new dataframe) 
  long rename_frovedis_dataframe(const char* host, int port, long proxy,
                                 const char** cols, const char ** new_cols, 
                                 int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> targets(size), new_targets(size);
    for(size_t i=0; i<size; ++i) { 
       targets[i] = std::string(cols[i]);
       new_targets[i] = std::string(new_cols[i]);
    }
    auto ret_proxy = exrpc_async(fm_node,frovedis_df_rename,df_proxy,targets,new_targets).get();
    return (static_cast<long>(ret_proxy));
  }

  PyObject* min_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, short* types,
                                   int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    std::vector<short> tt(size);
    for(size_t i=0; i<size; ++i) {
       cc[i] = std::string(cols[i]);
       tt[i] = types[i];
    }
    auto ret = exrpc_async(fm_node,frovedis_df_min,df_proxy,cc,tt).get();
    return to_python_double_list_from_str_vector(ret);
  }

  PyObject* max_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, short* types,
                                   int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    std::vector<short> tt(size);
    for(size_t i=0; i<size; ++i) {
       cc[i] = std::string(cols[i]);
       tt[i] = types[i];
    }
    auto ret = exrpc_async(fm_node,frovedis_df_max,df_proxy,cc,tt).get();
    return to_python_double_list_from_str_vector(ret);
  }

  PyObject* sum_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, short* types,
                                   int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    std::vector<short> tt(size);
    for(size_t i=0; i<size; ++i) {
       cc[i] = std::string(cols[i]);
       tt[i] = types[i];
    }
    auto ret = exrpc_async(fm_node,frovedis_df_sum,df_proxy,cc,tt).get();
    return to_python_double_list_from_str_vector(ret);
  }

  PyObject* avg_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    for(size_t i=0; i<size; ++i) {
       cc[i] = std::string(cols[i]);
    }
    auto ret = exrpc_async(fm_node,frovedis_df_avg,df_proxy,cc).get();
    return to_python_double_list_from_str_vector(ret);
  }

  PyObject* cnt_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    for(size_t i=0; i<size; ++i) {
       cc[i] = std::string(cols[i]);
    }
    auto ret = exrpc_async(fm_node,frovedis_df_cnt,df_proxy,cc).get();
    return to_python_double_list_from_str_vector(ret);
  }

  PyObject* std_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, short* types,
                                   int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    std::vector<short> tt(size);
    for(size_t i=0; i<size; ++i) {
       cc[i] = std::string(cols[i]);
       tt[i] = types[i];
    }
    auto ret = exrpc_async(fm_node,frovedis_df_std,df_proxy,cc,tt).get();
    return to_python_double_list_from_str_vector(ret);
  }

  PyObject* get_frovedis_col(const char* host, int port, long proxy,
                             const char* col_name, short tid){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    auto cname = std::string(col_name);
    switch (tid) {
      case INT: { 
          auto ret = exrpc_async(fm_node,get_df_int_col,f_dptr,cname).get();
          return to_python_int_list(ret); }
      case LONG: {
          auto ret = exrpc_async(fm_node,get_df_long_col,f_dptr,cname).get();
          return to_python_long_list(ret); }
      case FLOAT: {
          auto ret = exrpc_async(fm_node,get_df_float_col,f_dptr,cname).get();
          return to_python_float_list(ret); }
      case DOUBLE: {
          auto ret = exrpc_async(fm_node,get_df_double_col,f_dptr,cname).get();
          return to_python_double_list(ret); }
      case STRING: {
          auto ret = exrpc_async(fm_node,get_df_string_col,f_dptr,cname).get();
          return to_python_string_list(ret); }
      default:  REPORT_ERROR(USER_ERROR,"Unknown type for frovedis dataframe!");
    }
  }

  // --- Frovedis Dense matrices load/save/transpose/view/release ---
  std::vector<exrpc_ptr_t>
  get_each_rml_pointers_from_numpy_matrix(const char* host, int port,
                                          int nrow, int ncol, double* vv) {
    ASSERT_PTR(vv); 
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    // getting frovedis worker information
    exrpc_node fm_node(host,port);
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    // scattering numpy matrix in (python) client side
    rowmajor_matrix_local<double> mloc(nrow,ncol,vv);
    auto mdist = get_scattered_rowmajor_matrices(mloc,wsize);
    std::vector<exrpc_ptr_t> eps(wsize);
    for(size_t i=0; i<wsize; ++i) { 
      eps[i] = exrpc_async(nodes[i],load_local_data<R_LMAT1>,mdist[i]).get();
    }
    return eps;
  }

  // create from python (numpy) data
  PyObject* create_frovedis_dense_matrix(const char* host, int port,
                                       int nrow, int ncol, 
                                       double* vv, char mtype) {
    auto eps = get_each_rml_pointers_from_numpy_matrix(host,port,nrow,ncol,vv);
    // creating (frovedis) server side dense matrix from local pointers
    exrpc_node fm_node(host,port);
    auto r = static_cast<size_t>(nrow);
    auto c = static_cast<size_t>(ncol);
    exrpc_ptr_t m = -1;
    switch(mtype) {
      case 'R': m = exrpc_async(fm_node,create_rmm_data<DT1>,eps,r,c).get(); break;
      case 'C': m = exrpc_async(fm_node,create_cmm_data<DT1>,eps,r,c).get(); break;
      case 'B': m = exrpc_async(fm_node,create_bcm_data<DT1>,eps,r,c).get(); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
    dummy_matrix dmat(m,r,c);
    return to_py_dummy_matrix(dmat);
  }

  // load from file
  PyObject* load_frovedis_dense_matrix(const char* host, int port,
                                     const char* path, 
                                     bool bin, char mtype) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    dummy_matrix ret;
    switch(mtype) {
      case 'R': ret = exrpc_async(fm_node,load_rmm_matrix<DT1>,fs_path,bin).get(); break;
      case 'C': ret = exrpc_async(fm_node,load_cmm_matrix<DT1>,fs_path,bin).get(); break;
      case 'B': ret = exrpc_async(fm_node,load_bcm_matrix<DT1>,fs_path,bin).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
    return to_py_dummy_matrix(ret);
  }

  void save_frovedis_dense_matrix(const char* host, int port,
                                long dptr, const char* path, 
                                bool bin, char mtype) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::string fs_path(path);
    switch(mtype) {
      case 'R': exrpc_oneway(fm_node,save_matrix<R_MAT1>,f_dptr,fs_path,bin); break;
      case 'C': exrpc_oneway(fm_node,save_matrix<C_MAT1>,f_dptr,fs_path,bin); break;
      case 'B': exrpc_oneway(fm_node,save_matrix<B_MAT1>,f_dptr,fs_path,bin); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
  }

  PyObject* copy_frovedis_dense_matrix(const char* host, int port, 
                                     long dptr, char mtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    dummy_matrix ret;
    switch(mtype) {
      case 'R': REPORT_ERROR(USER_ERROR,"Frovedis doesn't support this deepcopy currently!\n"); 
      case 'C': REPORT_ERROR(USER_ERROR,"Frovedis doesn't support this deepcopy currently!\n"); 
      case 'B': ret = exrpc_async(fm_node,copy_matrix<B_MAT1>,f_dptr).get(); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
    return to_py_dummy_matrix(ret);
  }

  PyObject* transpose_frovedis_dense_matrix(const char* host, int port, 
                                          long dptr, char mtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    dummy_matrix ret;
    switch(mtype) {
      case 'R': ret = exrpc_async(fm_node,transpose_matrix<R_MAT1>,f_dptr).get(); break;
      case 'C': REPORT_ERROR(USER_ERROR,"Frovedis doesn't support this transpose currently!\n");
      case 'B': ret = exrpc_async(fm_node,transpose_matrix<B_MAT1>,f_dptr).get(); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
    return to_py_dummy_matrix(ret);
  }

  // converts matrix to rowmajor_matrix<double>
  PyObject* get_frovedis_rowmatrix(const char* host, int port, 
                                 long dptr, 
                                 int nrow, int ncol, char mtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    dummy_matrix ret(f_dptr,nrow,ncol);
    switch(mtype) {
      case 'R': break; // if already rowmajor, return *this
      case 'C': ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT1,C_MAT1>),f_dptr).get(); break;
      case 'B': ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT1,B_MAT1>),f_dptr).get(); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
    return to_py_dummy_matrix(ret);
  }

  // --- Frovedis dense distributed matrix to Rowmajor Array Conversion ---
  // 1. convert input dense matrix to rowmajor_matrix at Frovedis server side
  // 2. get each rowmajor_matrix_local pointers
  // 3. gather local vectors from Frovedis worker nodes one-by-one
  // 4. fill the gathered data in client side buffer (ret)
  void get_rowmajor_array(const char* host, int port, 
                          long dptr, char mtype, double* ret, int sz) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    // (1) and (2)
    std::vector<exrpc_ptr_t> eps;
    switch(mtype) {
      case 'R': eps = exrpc_async(fm_node,(get_all_local_pointers<R_MAT1,R_LMAT1>),f_dptr).get(); break;
      case 'C': eps = exrpc_async(fm_node,(convert_and_get_all_rml_pointers<DT1,C_MAT1>),f_dptr).get(); break;
      case 'B': eps = exrpc_async(fm_node,(convert_and_get_all_rml_pointers<DT1,B_MAT1>),f_dptr).get(); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
    // (3) 
    auto nodes = get_worker_nodes(fm_node);
    auto wsize = nodes.size();
    std::vector<std::vector<double>> evs(wsize);
    int k = 0, total = 0;
    for(size_t i=0; i<wsize; ++i) { 
      evs[i] = exrpc_async(nodes[i],(get_local_array<DT1,R_LMAT1>),eps[i]).get();
      total += evs[i].size();
    }
    // The gathered size and expected size from client side should match
    checkAssumption(total == sz);
    // (4)
    for(size_t i=0; i<wsize; ++i) {
      for(size_t j=0; j<evs[i].size(); ++j) ret[k++] = evs[i][j];
    }
  }

  void show_frovedis_dense_matrix(const char* host, int port, 
                                long dptr, char mtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    switch(mtype) {
      case 'R': exrpc_oneway(fm_node,show_data<R_MAT1>,f_dptr); break;
      case 'C': exrpc_oneway(fm_node,show_data<C_MAT1>,f_dptr); break;
      case 'B': exrpc_oneway(fm_node,show_data<B_MAT1>,f_dptr); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
  }

  void release_frovedis_dense_matrix(const char* host, int port, 
                                   long dptr, char mtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    switch(mtype) {
      case 'R': exrpc_oneway(fm_node,release_data<R_MAT1>,f_dptr); break;
      case 'C': exrpc_oneway(fm_node,release_data<C_MAT1>,f_dptr); break;
      case 'B': exrpc_oneway(fm_node,release_data<B_MAT1>,f_dptr); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
  }

// --- Frovedis ML Models ---
  void show_frovedis_model(const char* host, int port, int mid, int mkind) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    switch(mkind) {
      case LRM:    exrpc_oneway(fm_node,show_model<LRM1>,mid); break;
      case SVM:    exrpc_oneway(fm_node,show_model<SVM1>,mid); break;
      case LNRM:   exrpc_oneway(fm_node,show_model<LNRM1>,mid); break;
      case MFM:    exrpc_oneway(fm_node,show_model<MFM1>,mid); break;
      case KMEANS: exrpc_oneway(fm_node,show_model<KMM1>,mid); break;
      default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }

  void release_frovedis_model(const char* host, int port, int mid, int mkind) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    switch(mkind) {
      case LRM:    exrpc_oneway(fm_node,release_model<LRM1>,mid); break;
      case SVM:    exrpc_oneway(fm_node,release_model<SVM1>,mid); break;
      case LNRM:   exrpc_oneway(fm_node,release_model<LNRM1>,mid); break;
      case MFM:    exrpc_oneway(fm_node,release_model<MFM1>,mid); break;
      case KMEANS: exrpc_oneway(fm_node,release_model<KMM1>,mid); break;
      default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }

  void save_frovedis_model(const char* host, int port, 
                         int mid, int mkind, const char* path) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    switch(mkind) {
      case LRM:    exrpc_oneway(fm_node,save_model<LRM1>,mid,fs_path); break;
      case SVM:    exrpc_oneway(fm_node,save_model<SVM1>,mid,fs_path); break;
      case LNRM:   exrpc_oneway(fm_node,save_model<LNRM1>,mid,fs_path); break;
      case MFM:    exrpc_oneway(fm_node,save_model<MFM1>,mid,fs_path); break;
      case KMEANS: exrpc_oneway(fm_node,save_model<KMM1>,mid,fs_path); break;
      default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }

  void load_frovedis_model(const char* host, int port, 
                         int mid, int mkind, const char* path) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    switch(mkind) {
      case LRM:  exrpc_async(fm_node,load_glm<LRM1>,mid,LRM,fs_path).get(); break;
      case SVM:  exrpc_async(fm_node,load_glm<SVM1>,mid,SVM,fs_path).get(); break;
      case LNRM: exrpc_async(fm_node,load_lnrm<DT1>,mid,LNRM,fs_path).get(); break;
      case KMEANS: exrpc_async(fm_node,load_kmm<DT1>,mid,KMEANS,fs_path).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }

  PyObject* load_frovedis_mfm(const char* host, int port,
                            int mid, int mkind, const char* path) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    auto ret = exrpc_async(fm_node,load_mfm<DT1>,mid,MFM,fs_path).get();
    return to_py_mfm_info(ret);
  }

  void parallel_glm_predict(const char* host, int port,
                            int mid, int mkind, long dptr, 
                            double* ret, int len, bool prob) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<double> pred;
    switch(mkind) {
      case LRM: pred = exrpc_async(fm_node,(pgp2<DT1,S_MAT1,S_LMAT1,LRM1>),f_dptr,mid,prob).get(); break;
      case SVM: pred = exrpc_async(fm_node,(pgp2<DT1,S_MAT1,S_LMAT1,SVM1>),f_dptr,mid,prob).get(); break;
      case LNRM: pred = exrpc_async(fm_node,(p_lnrm_p2<DT1,S_MAT1,S_LMAT1>),f_dptr,mid).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
    int sz = pred.size();
    checkAssumption(len == sz);
    for(int i=0; i<sz; ++i) ret[i] = pred[i];
  }

  void parallel_kmeans_predict(const char* host, int port,
                               int mid, int mkind, long dptr,
                               int* ret, int len) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<int> pred;
    pred = exrpc_async(fm_node,(pkp2<S_MAT1,S_LMAT1,KMM1>),f_dptr,mid).get();
    int sz = pred.size();
    checkAssumption(len == sz);
    for(int i=0; i<sz; ++i) ret[i] = pred[i];
  }

  void als_predict(const char* host, int port, int mid, 
                   int* ids, double* ret, int sz) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<std::pair<size_t,size_t>> ID(sz);
    for(int i=0; i<sz; ++i) {
      ID[i] = std::make_pair(ids[2*i],ids[2*i+1]);
    }
    auto pd = exrpc_async(fm_node,frovedis_als_predict<DT1>,mid,ID).get();
    for(int i=0; i<sz; ++i) ret[i] = pd[i]; 
  }

  void als_rec_users(const char* host, int port, int mid, int pid, int k, 
                     int* uids, double* scores) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto pd = exrpc_async(fm_node,recommend_users<DT1>,mid,pid,k).get();
    for(int i=0; i<k; ++i) {
      uids[i] = pd[i].first;
      scores[i] = pd[i].second;
    }
  }

  void als_rec_prods(const char* host, int port, int mid, int uid, int k, 
                     int* pids, double* scores) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto pd = exrpc_async(fm_node,recommend_products<DT1>,mid,uid,k).get();
    for(int i=0; i<k; ++i) {
      pids[i] = pd[i].first;
      scores[i] = pd[i].second;
    }
  }

// --- Frovedis ML Trainers ---
  void lr_sgd(const char* host, int port, long xptr, long yptr,
              int iter, double al, int rtype, bool icpt, double tol,
              int vb, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    double rprm = 0.01; // default
    bool mvbl = false; // auto-managed at python side
    exrpc_oneway(fm_node,(frovedis_lr_sgd<DT1,S_MAT1>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl);
  }

  void lr_lbfgs(const char* host, int port, long xptr, long yptr,
                int iter, double al, int rtype, bool icpt, double tol,
                int vb, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;   // default
    double rprm = 0.01; // default
    bool mvbl = false; // auto-managed at python side
    exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT1,S_MAT1>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
  }

  void svm_sgd(const char* host, int port, long xptr, long yptr,
               int iter, double al, int rtype, bool icpt, double tol,
               int vb, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    double rprm = 0.01; // default
    bool mvbl = false; // auto-managed at python side
    exrpc_oneway(fm_node,(frovedis_svm_sgd<DT1,S_MAT1>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl);
  }

  void svm_lbfgs(const char* host, int port, long xptr, long yptr,
                 int iter, double al, int rtype, bool icpt, double tol,
                 int vb, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;   // default
    double rprm = 0.01; // default
    bool mvbl = false; // auto-managed at python side
    exrpc_oneway(fm_node,(frovedis_svm_lbfgs<DT1,S_MAT1>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
  }

  void lnr_sgd(const char* host, int port, long xptr, long yptr,
               bool icpt, int vb, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int iter = 1000;  //default
    double al = 0.01;  //default
    double mbf = 1.0;  // default
    bool mvbl = false;  // auto-managed at python side
    exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT1,S_MAT1>),f_dptr,iter,al,mbf,icpt,vb,mid,mvbl);
  }

  void lnr_lbfgs(const char* host, int port, long xptr, long yptr,
                 bool icpt, int vb, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int iter = 1000;  //default
    double al = 0.01;  //default
    int hs = 10;  // default
    bool mvbl = false; // auto-managed at python side
    exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT1,S_MAT1>),f_dptr,iter,al,hs,icpt,vb,mid,mvbl);
  }

  void lasso_sgd(const char* host, int port, long xptr, long yptr,
                 int iter, double al, bool icpt, double tol,
                 int vb, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    double rprm = 0.01; // default
    bool mvbl = false; // auto-managed at python side
    exrpc_oneway(fm_node,(frovedis_lasso_sgd<DT1,S_MAT1>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
  }

  void lasso_lbfgs(const char* host, int port, long xptr, long yptr,
                   int iter, double al, bool icpt, double tol,
                   int vb, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;   // default
    double rprm = 0.01; // default
    bool mvbl = false; // auto-managed at python side
    exrpc_oneway(fm_node,(frovedis_lasso_lbfgs<DT1,S_MAT1>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
  }

  void ridge_sgd(const char* host, int port, long xptr, long yptr,
                 int iter, double al, bool icpt, double tol,
                 int vb, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    double rprm = 0.01; // default
    bool mvbl = false; // auto-managed at python side
    exrpc_oneway(fm_node,(frovedis_ridge_sgd<DT1,S_MAT1>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
  }

  void ridge_lbfgs(const char* host, int port, long xptr, long yptr,
                   int iter, double al, bool icpt, double tol,
                   int vb, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;   // default
    double rprm = 0.01; // default
    bool mvbl = false; // auto-managed at python side
    exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT1,S_MAT1>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
  }

  void kmeans_train(const char* host, int port, long xptr, int k,
                    int iter, long seed, double eps,
                    int vb, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    bool mvbl = false; // auto-managed at python side
    exrpc_oneway(fm_node,(frovedis_kmeans<DT1,S_MAT1>),f_xptr,k,iter,seed,eps,vb,mid,mvbl);
  }

  void als_train(const char* host, int port, long dptr, int rank,
                 int iter, double al, double rprm, long seed,
                 int vb, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    bool mvbl = false; // auto-managed at python side
    exrpc_oneway(fm_node,(frovedis_mf_als<DT1,S_MAT1>),f_dptr,rank,iter,al,rprm,seed,vb,mid,mvbl);
  }

// --- Frovedis PBLAS L1 Wrapper ---
  void pswap(const char* host, int port, long vptr1, long vptr2) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    exrpc_oneway(fm_node,(frovedis_swap<DT1,B_MAT1>),v1,v2);
  }

  void pcopy(const char* host, int port, long vptr1, long vptr2) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    exrpc_oneway(fm_node,(frovedis_copy<DT1,B_MAT1>),v1,v2);
  }

  void pscal(const char* host, int port, long vptr, double al) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v = (exrpc_ptr_t) vptr;
    exrpc_oneway(fm_node,(frovedis_scal<DT1,B_MAT1>),v,al);
  }

  void paxpy(const char* host, int port, long vptr1, long vptr2, double al) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    exrpc_oneway(fm_node,(frovedis_axpy<DT1,B_MAT1>),v1,v2,al);
  }

  double pdot(const char* host, int port, long vptr1, long vptr2) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    return exrpc_async(fm_node,(frovedis_dot<DT1,B_MAT1>),v1,v2).get();
  }

  double pnrm2(const char* host, int port, long vptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v = (exrpc_ptr_t) vptr;
    return exrpc_async(fm_node,(frovedis_nrm2<DT1,B_MAT1>),v).get();
  }

// --- Frovedis PBLAS L2 Wrapper ---
  PyObject* pgemv(const char* host, int port, 
                  long mptr, long vptr, 
                  bool trans, double al, double be) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    auto v = (exrpc_ptr_t) vptr;
    auto r = exrpc_async(fm_node,(frovedis_gemv<DT1,B_MAT1>),m,v,trans,al,be).get();
    return to_py_dummy_matrix(r);
  }

  PyObject* pger(const char* host, int port, 
                 long vptr1, long vptr2, double al) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    auto r = exrpc_async(fm_node,(frovedis_ger<DT1,B_MAT1>),v1,v2,al).get();
    return to_py_dummy_matrix(r);
  }

// --- Frovedis PBLAS L3 Wrapper ---
  PyObject* pgemm(const char* host, int port, 
                  long mptr1, long mptr2, 
                  bool trans1, bool trans2,
                  double al, double be) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m1 = (exrpc_ptr_t) mptr1;
    auto m2 = (exrpc_ptr_t) mptr2;
    auto r = exrpc_async(fm_node,(frovedis_gemm<DT1,B_MAT1>),m1,m2,trans1,trans2,al,be).get();
    return to_py_dummy_matrix(r);
  }

  void pgeadd(const char* host, int port, 
              long mptr1, long mptr2, 
              bool trans, double al, double be) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m1 = (exrpc_ptr_t) mptr1;
    auto m2 = (exrpc_ptr_t) mptr2;
    exrpc_oneway(fm_node,(frovedis_geadd<DT1,B_MAT1>),m1,m2,trans,al,be);
  }

// --- Frovedis SCALAPACK Wrapper ---
  PyObject* pgetrf(const char* host, int port, long mptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    auto ret = exrpc_async(fm_node,(frovedis_getrf<DT1,B_MAT1,lvec<int>>),m).get();
    return to_py_getrf_result(ret,'B');
  }

  int pgetri(const char* host, int port, 
             long mptr, long ipiv_ptr) { 
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    auto ipiv = (exrpc_ptr_t) ipiv_ptr;
    return exrpc_async(fm_node,(frovedis_getri<DT1,B_MAT1,lvec<int>>),m,ipiv).get();
  }

  int pgetrs(const char* host, int port, 
             long mptrA, long mptrB, 
             long ipiv_ptr, bool trans) { 
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mA = (exrpc_ptr_t) mptrA;
    auto mB = (exrpc_ptr_t) mptrB;
    auto ipiv = (exrpc_ptr_t) ipiv_ptr;
    return exrpc_async(fm_node,(frovedis_getrs<DT1,B_MAT1,lvec<int>>),mA,mB,ipiv,trans).get();
  }

  int pgesv(const char* host, int port, 
            long mptrA, long mptrB) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mA = (exrpc_ptr_t) mptrA;
    auto mB = (exrpc_ptr_t) mptrB;
    return exrpc_async(fm_node,(frovedis_gesv<DT1,B_MAT1,lvec<int>>),mA,mB).get();
  }

  int pgels(const char* host, int port, 
            long mptrA, long mptrB, bool trans) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mA = (exrpc_ptr_t) mptrA;
    auto mB = (exrpc_ptr_t) mptrB;
    return exrpc_async(fm_node,(frovedis_gels<DT1,B_MAT1>),mA,mB,trans).get();
  }

  PyObject* pgesvd(const char* host, int port, 
                   long mptr, bool isU, bool isV) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    auto ret = exrpc_async(fm_node,(frovedis_gesvd<DT1,B_MAT1>),m,isU,isV).get();
    return to_py_gesvd_result(ret,'B',isU,isV);
  }

// --- Frovedis ARPACK Wrapper ---
  PyObject* compute_sparse_svd(const char* host, int port, 
                               long mptr, int k) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    bool wantU = true;
    bool wantV = true;
    bool mvbl = false;
    char mtype = 'C';
    auto ret = exrpc_async(fm_node,(frovedis_sparse_svd<DT1,S_MAT1>),m,k,mvbl).get();
    return to_py_gesvd_result(ret,mtype,wantU,wantV);
  }

// --- Frovedis Scalapack Wrapper Results ---
  void release_ipiv(const char* host, int port, char mtype, long ipiv_ptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) ipiv_ptr;
    switch(mtype) {
      case 'C': exrpc_oneway(fm_node,(release_data<std::vector<int>>),f_dptr); break;
      case 'B': exrpc_oneway(fm_node,(release_data<lvec<int>>),f_dptr); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }

  void get_double_array(const char* host, int port, 
                        long dptr, double* ret) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    auto arr = exrpc_async(fm_node,vec_to_array<DT1>,f_dptr).get();
    double *arrp = &arr[0];
    // filling the output array (avoiding memory leak)
    for(size_t i=0; i<arr.size(); ++i) ret[i] = arrp[i];
  }

  void release_double_array(const char* host, int port, long dptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    exrpc_oneway(fm_node,(release_data<std::vector<double>>),f_dptr);
  }

  void save_as_diag_matrix(const char* host, int port, long dptr,
                           const char* path, bool bin) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::string fs_path(path);
    exrpc_oneway(fm_node,save_as_diag_matrix_local<DT1>,f_dptr,fs_path,bin);
  }

  PyObject* get_svd_results_from_file(const char* host, int port,
                                      const char* spath, 
                                      const char* upath, 
                                      const char* vpath, 
                                      bool bin, bool isU, bool isV,
                                      char mtype) {
    // upath/vpath can be empty (""), but not null
    ASSERT_PTR(spath); ASSERT_PTR(upath); ASSERT_PTR(vpath);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string sfl(spath);
    std::string ufl(upath);
    std::string vfl(vpath);
    gesvd_result ret;
    switch(mtype) {
      case 'C': ret = exrpc_async(fm_node,load_cmm_svd_results<DT1>,sfl,ufl,vfl,isU,isV,bin).get(); break;
      case 'B': ret = exrpc_async(fm_node,load_bcm_svd_results<DT1>,sfl,ufl,vfl,isU,isV,bin).get(); break;
      default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
    return to_py_gesvd_result(ret,mtype,isU,isV);    
  }
}
