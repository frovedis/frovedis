#include "python_client_headers.hpp"

using namespace frovedis;

std::vector<size_t>
get_starts(const std::vector<size_t>& sizes) {
  auto size = sizes.size();
  std::vector<size_t> sidx(size); sidx[0] = 0;
  for(size_t i = 1; i < size; ++i) sidx[i] = sidx[i - 1] + sizes[i - 1];
  return sidx;
}

void get_start_size_info(size_t vecsz, size_t wsize,
                         std::vector<size_t>& starts,
                         std::vector<size_t>& sizes) {
  sizes = get_block_sizes(vecsz, wsize);
  starts = get_starts(sizes);
}

std::vector<size_t>
get_crs_row_division(long* dataoffp, size_t nrow, size_t nelem, size_t wsize) {
  size_t each_size = frovedis::ceil_div(nelem, wsize);
  std::vector<size_t> divide_row(wsize + 1);
  for(size_t i = 0; i < wsize; i++) {
    auto it = std::lower_bound(dataoffp, dataoffp + nrow + 1, each_size * i);
    if(it != dataoffp + nrow + 1) {
      divide_row[i] = it - dataoffp;
    } else {
      divide_row[i] = nrow;
    }
  }
  divide_row[wsize] = nrow;
  return divide_row;
}

extern "C" {
  // std::string => python string object
  PyObject* to_python_string_object (const std::string& val) {
    return Py_BuildValue("s",val.c_str());
  }
  // std::vector<std::string> => python List of strings
  PyObject* to_python_string_list (const std::vector<std::string>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) PyList_Append(PList,Py_BuildValue("s",each.c_str()));
    return Py_BuildValue("O",PList);
  }

  // std::vector<std::string> => python List of doubles
  PyObject* to_python_double_list_from_str_vector (const std::vector<std::string>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) {
      auto val = std::stod(each);
      PyList_Append(PList,Py_BuildValue("d",val));
    }
    return Py_BuildValue("O",PList);
  }

  // std::vector<int> => python List of integers
  PyObject* to_python_int_list (const std::vector<int>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) PyList_Append(PList,Py_BuildValue("i",each));
    return Py_BuildValue("O",PList);
  }

  // std::vector<long> => python List of long integers
  PyObject* to_python_long_list (const std::vector<long>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) PyList_Append(PList,Py_BuildValue("l",each));
    return Py_BuildValue("O",PList);
  }

  // std::vector<size_t> => python List of long integers
  PyObject* to_python_llong_list (const std::vector<size_t>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) {
      auto leach = static_cast<long>(each);
      PyList_Append(PList,Py_BuildValue("l",leach)); // FIXME
    }
    return Py_BuildValue("O",PList);
  }

  // std::vector<unsigned long> => python List of long integers
  PyObject* to_python_ulong_list (const std::vector<unsigned long>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) {
      auto leach = static_cast<long>(each);
      PyList_Append(PList,Py_BuildValue("l",leach)); // FIXME
    }
    return Py_BuildValue("O",PList);
  }

  // std::vector<float> => python List of floats
  PyObject* to_python_float_list (const std::vector<float>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) PyList_Append(PList,Py_BuildValue("f",each));
    return Py_BuildValue("O",PList);
  }

  // std::vector<double> => python List of doubles
  PyObject* to_python_double_list (const std::vector<double>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) PyList_Append(PList,Py_BuildValue("d",each));
    return Py_BuildValue("O",PList);
  }

  PyObject* to_py_dummy_df(const dummy_dftable& obj) {
    return Py_BuildValue("{s:l, s:l, s:O, s:O}", 
                        "dfptr", (long)obj.dfptr, 
                        "nrow",  (long)obj.nrow,
                        "names", to_python_string_list(obj.names), 
                        "types", to_python_int_list(obj.types));
  }

  // std::vector<dummy_dftable> => python List of dummy_dftable (dict)
  PyObject* to_py_dataframe_list (const std::vector<dummy_dftable>& val) {
    PyObject *PList = PyList_New(0);
    for(auto& each: val) PyList_Append(PList, to_py_dummy_df(each));
    return Py_BuildValue("O",PList);
  }

  // --- Frovedis Data structure to Python Data structure ---
  PyObject* to_py_bfs_result(const bfs_result<DT5>& result) {
    return Py_BuildValue("{s:O, s:O, s:O}",
                         "destids", to_python_llong_list(result.destids),
                         "distances", to_python_llong_list(result.distances),
                         "predecessors", to_python_llong_list(result.predecessors));
  }

  PyObject* to_py_sssp_result(const sssp_result<DT1,DT5>& result) {
    return Py_BuildValue("{s:O, s:O, s:O}",
                         "destids", to_python_llong_list(result.destids),
                         "distances", to_python_double_list(result.distances),
                         "predecessors", to_python_llong_list(result.predecessors));
  }

  PyObject* to_py_pagerank_result(const py_pagerank_result<double>& result) {
    return Py_BuildValue("{s:O, s:O}",
                         "nodeid", to_python_long_list(result.nodeid),
                         "rank", to_python_double_list(result.rank));
  }


  PyObject* to_py_dummy_matrix(const dummy_matrix& m) {
    return Py_BuildValue("{s:l, s:i, s:i, s:i}", 
                         "dptr", (long)m.mptr, 
                         "nrow", m.nrow, 
                         "ncol", m.ncol,
                         "n_nz", m.active_elems);
  }

  PyObject* to_py_mfm_info(const dummy_mfm& m) {
    return Py_BuildValue("{s:i, s:i, s:i}", 
                         "rank", m.rank, 
                         "nrow", m.nrow, 
                         "ncol", m.ncol);
  }

  PyObject* to_py_dummy_vector(const dummy_vector& dv) {
    return Py_BuildValue("{s:l, s:i, s:i}", 
                         "dptr", (long)dv.vptr, 
                         "size", (int)dv.size, 
                         "vtype", (int)dv.dtype);
  }

  PyObject* to_py_svd_result(const svd_result& obj,
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

  PyObject* to_py_eigen_result(const eigen_result& obj, bool isU) {
    long uptr = isU ? (long)obj.umat_ptr : 0;
    long sptr = (long)obj.svec_ptr;
    return Py_BuildValue("{s:l, s:l, s:i, s:i, s:i}",
                         "eigenval", sptr, "eigenvec", uptr,
                         "m", obj.m, "n", obj.n,
                         "k", obj.k);
  }

  PyObject* to_py_pca_result(const pca_result& obj,
                             char mtype) {
    if (mtype != 'C') 
      REPORT_ERROR(INTERNAL_ERROR, "pca output should be colmajor matrix!");
    auto mt = "C";
    long comp_ptr = (long)obj.comp_ptr; // colmajor_matrix
    long score_ptr = (long)obj.score_ptr; // colmajor_matrix
    long var_ratio_ptr = (long)obj.var_ratio_ptr; // vector
    long eig_ptr = (long)obj.eig_ptr; // vector
    long sval_ptr = (long)obj.sval_ptr; // vector
    long mean_ptr = (long)obj.mean_ptr; // vector
    return Py_BuildValue("{s:s, s:l, s:l, s:l, s:l, s:l, s:l, s:i, s:i, s:i, s:d}", 
                         "mtype", mt, 
                         "pc_ptr", comp_ptr, 
                         "var_ptr", var_ratio_ptr,
                         "score_ptr", score_ptr,
                         "exp_var_ptr", eig_ptr,
                         "singular_val_ptr", sval_ptr,
                         "mean_ptr", mean_ptr,
                         "n_components", obj.n_components,
                         "n_samples",  obj.n_samples,
                         "n_features", obj.n_features,
                         "noise", obj.noise);
  }
  
  PyObject* to_py_tsne_result(const tsne_result& obj) {
    long embedding_ptr = (long)obj.embedding_ptr; // rowmajor_matrix
    return Py_BuildValue("{s:l, s:i, s:i, s:i, s:d}",
                         "embedding_ptr", embedding_ptr,
                         "n_samples",  obj.n_samples,
                         "n_comps", obj.n_comps,
                         "n_iter_", obj.n_iter_,
                         "kl_divergence_", obj.kl_divergence_);
  }

  PyObject* to_py_kmeans_result(const kmeans_result& result) {
    return Py_BuildValue("{s:O, s:i, s:f, s:i, s:l, s:i}",
                         "labels", to_python_int_list(result.label_),
                         "n_iter", result.n_iter_,
                         "inertia", result.inertia_,
                         "n_clusters", result.n_clusters_,
                         "mptr", result.trans_mat_ptr,
                         "n_samples", result.trans_mat_nsamples);
  }

  PyObject* to_py_gmm_result(const gmm_result& result) {
    return Py_BuildValue("{s:i, s:d}",
                         "n_iter", result.n_iter_,
                         "likelihood", result.likelihood_);
  } 

  PyObject* to_py_float_lnr_result(const lnr_result<float>& result) {
    return Py_BuildValue("{s:i, s:i, s:O}",
                         "n_iter", result.n_iter,
                         "rank", result.rank,
                         "singular", to_python_float_list(result.singular));
  } 

  PyObject* to_py_double_lnr_result(const lnr_result<double>& result) {
    return Py_BuildValue("{s:i, s:i, s:O}",
                         "n_iter", result.n_iter,
                         "rank", result.rank,
                         "singular", to_python_double_list(result.singular));
  } 

  PyObject* to_py_knn_result(const knn_result& obj,
                             char mtype) {
    if (mtype != 'R') 
      REPORT_ERROR(INTERNAL_ERROR, "knn output should be rowmajor matrix!");
    auto mt = "R";
    return Py_BuildValue("{s:s, s:l, s:l, s:l, s:l, s:l, s:l, s:i}", 
                         "mtype", mt, 
                         "indices_ptr", (long) obj.indices_ptr, 
                         "distances_ptr", (long) obj.distances_ptr,
                         "nrow_ind", (long) obj.nrow_ind,
                         "ncol_ind",  (long) obj.ncol_ind,
                         "nrow_dist", (long) obj.nrow_dist,
                         "ncol_dist", (long) obj.ncol_dist,
                         "k", obj.k);
  }  

  PyObject* to_py_lu_fact_result(const lu_fact_result& obj,char mtype) {
    auto mt = (mtype == 'C') ? "C" : "B";
    long dptr = (long)obj.ipiv_ptr;
    return Py_BuildValue("{s:s, s:l, s:i}", 
                         "mtype", mt, 
                         "dptr", dptr, 
                         "info", obj.info);
  }

  PyObject* to_py_dummy_lda_result(const dummy_lda_result& m) {
    return Py_BuildValue("{s:l, s:i, s:i, s:d, s:d}",
                         "dist_mat", (long)m.dist_mat.mptr,
                         "nrow", m.dist_mat.nrow,
                         "ncol", m.dist_mat.ncol,
                         "perplexity", m.perplexity,
                         "likelihood", m.likelihood);
  }

  PyObject* to_py_dummy_graph(const dummy_graph& obj){  
    long dptr = static_cast<long>(obj.dptr);
    long num_edges = static_cast<long>(obj.num_edges);
    long num_vertices = static_cast<long>(obj.num_nodes);
    return Py_BuildValue("{s:l, s:l, s:l}",
                         "dptr", dptr,
                         "nEdges", num_edges,
                         "nNodes", num_vertices);
  }

  // converison : const char** => std::vector<std::string>
  std::vector<std::string> 
  to_string_vector(const char** data, ulong sz) {
    std::vector<std::string> vec(sz);
    for(size_t i=0; i<sz; ++i) vec[i] = std::string(data[i]);
    return vec;
  }

  // converison : short* (pointer-to-short-array) => std::vector<short>
  std::vector<short> 
  to_short_vector(short* data, ulong sz) {
    std::vector<short> vec(sz);
    for(size_t i = 0; i < sz; ++i) vec[i] = data[i];
    return vec;
  }

  // converison : char* (pointer-to-char-array) => std::vector<char>
  std::vector<char>
  to_char_vector(char* data, ulong sz) {
    std::vector<char> vec(sz);
    for(size_t i = 0; i < sz; ++i) vec[i] = data[i];
    return vec;
  }

  // converison : int* (pointer-to-int-array) => std::vector<int>
  std::vector<int> 
  to_int_vector(int* data, ulong sz) {
    std::vector<int> vec(sz);
    for(size_t i = 0; i < sz; ++i) vec[i] = data[i];
    return vec;
  }

  // converison : char* (pointer-to-char-array) => std::vector<int>
  std::vector<int>
  to_int_vector_from_char(char* data, ulong sz) {
    size_t size = sz / 4; // assumes sz is correct
    std::vector<int> vec(size);
    auto iptr = reinterpret_cast<int*>(data);
    for(size_t i = 0; i < size; ++i) vec[i] = iptr[i];
    return vec;
  }

  // converison : long* (pointer-to-long-array) => std::vector<long>
  std::vector<long> 
  to_long_vector(long* data, ulong sz) {
    std::vector<long> vec(sz);
    for(size_t i = 0; i < sz; ++i) vec[i] = data[i];
    return vec;
  }

  // converison : ulong* (pointer-to-unsigned-long-array) => std::vector<unsigned long>
  std::vector<unsigned long> 
  to_ulong_vector(unsigned long* data, ulong sz) {
    std::vector<unsigned long> vec(sz);
    for(size_t i = 0; i < sz; ++i) vec[i] = data[i];
    return vec;
  }

  // converison : float* (pointer-to-float-array) => std::vector<float>
  std::vector<float> 
  to_float_vector(float* data, ulong sz) {
    std::vector<float> vec(sz);
    for(size_t i = 0; i < sz; ++i) vec[i] = data[i];
    return vec;
  }

  // converison : double* (pointer-to-double-array) => std::vector<double>
  std::vector<double> 
  to_double_vector(double* data, ulong sz) {
    std::vector<double> vec(sz);
    for(size_t i = 0; i < sz; ++i) vec[i] = data[i];
    return vec;
  }

  // converison : double* (pointer-to-double-array) => std::vector<float>
  std::vector<float> 
  double_to_float_vector(double* data, ulong sz) {
    std::vector<float> vec(sz);
    for(size_t i = 0; i < sz; ++i) vec[i] = static_cast<float>(data[i]);
    return vec;
  }

  void get_exrpc_result(std::vector<frovedis::exrpc_ptr_t>& eps,
                        std::vector<frovedis::exrpc_result
                        <frovedis::exrpc_ptr_t>>& res,
                        size_t wsize) {
    size_t i = 0;
    try {
      for(; i < wsize; ++i) eps[i] = res[i].get();
    } catch(std::exception& e) {
      set_status(true,e.what());
      try { // consume other result
        for(; i < wsize; ++i) eps[i] = res[i].get();
      } catch (std::exception& e) {
        ; // already get the exception
      }
    }
  }

}
