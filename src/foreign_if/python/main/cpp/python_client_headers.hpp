#ifndef _PYTHON_CLIENT_HEADERS_
#define _PYTHON_CLIENT_HEADERS_

#include <Python.h>
#include <typeinfo>
#include "exrpc_util_headers.hpp"
#include "short_hand_sparse_type.hpp"

// --- global exception tracking variables ---
extern bool status;
extern std::string info;

using namespace frovedis;

std::vector<size_t> get_starts(const std::vector<size_t>& sizes);

void get_start_size_info(size_t vecsz, size_t wsize,
                         std::vector<size_t>& starts,
                         std::vector<size_t>& sizes);
std::vector<size_t>
get_crs_row_division(long* dataoffp, size_t nrow, 
                     size_t nelem, size_t wsize);

extern "C" {
  // --- exception handling ---
  void set_status(bool stat, const std::string&);
  void reset_status();
  PyObject* check_server_exception();

  // std::string => python string object
  PyObject* to_python_string_object (const std::string& str);

  // std::vector<std::string> => python List of strings
  PyObject* to_python_string_list (const std::vector<std::string>& v);

  // std::vector<std::string> => python List of doubles
  PyObject* to_python_double_list_from_str_vector (const std::vector<std::string>& v);

  // std::vector<int> => python List of integers
  PyObject* to_python_int_list (const std::vector<int>& v);

  // std::vector<long> => python List of long integers
  PyObject* to_python_long_list (const std::vector<long>& v);

  // std::vector<size_t> => python List of long integers
  PyObject* to_python_llong_list (const std::vector<size_t>& v);

  // std::vector<unsigned long> => python List of long integers
  PyObject* to_python_ulong_list (const std::vector<unsigned long>& v);

  // std::vector<float> => python List of floats
  PyObject* to_python_float_list (const std::vector<float>& v); 

  // std::vector<double> => python List of doubles
  PyObject* to_python_double_list (const std::vector<double>& v);

  // std::vector<dummy_dftable> => python List of dummy_dftable (dict)
  PyObject* to_py_dataframe_list(const std::vector<dummy_dftable>& v);

  // --- Frovedis Data structure to Python Data structure ---
  PyObject* to_py_bfs_result(const bfs_result<DT5>& result);
  PyObject* to_py_sssp_result(const sssp_result<DT1,DT5>& result);
  PyObject* to_py_pagerank_result(const py_pagerank_result<double>& result);
  PyObject* to_py_dummy_matrix(const dummy_matrix& m);
  PyObject* to_py_mfm_info(const dummy_mfm& m);
  PyObject* to_py_dummy_vector(const dummy_vector& dv);
  PyObject* to_py_svd_result(const svd_result& obj, char, bool, bool);
  PyObject* to_py_eigen_result(const eigen_result& obj, bool);
  PyObject* to_py_lu_fact_result(const lu_fact_result& obj, char);
  PyObject* to_py_pca_result(const pca_result& obj, char);
  PyObject* to_py_tsne_result(const tsne_result& obj);
  PyObject* to_py_knn_result(const knn_result& obj, char);
  PyObject* to_py_kmeans_result(const kmeans_result& obj);
  PyObject* to_py_gmm_result(const gmm_result& result);
  PyObject* to_py_float_lnr_result(const lnr_result<float>& result);
  PyObject* to_py_double_lnr_result(const lnr_result<double>& result);
  PyObject* to_py_dummy_lda_result(const dummy_lda_result& m);
  PyObject* to_py_dummy_graph(const dummy_graph& obj);
  PyObject* to_py_dummy_df(const dummy_dftable& obj);  

  // --- server info ---
  std::vector<exrpc_node> get_worker_nodes(exrpc_node& fm_node);
  std::vector<exrpc_node> get_worker_nodes_for_vector_rawsend(exrpc_node& fm_node);
  std::vector<exrpc_node>
  get_worker_nodes_for_multi_exrpc(exrpc_node& fm_node,
                                   std::vector<size_t> blocksz);

  // conversion
  std::vector<std::string> to_string_vector(const char** data, ulong sz);
  std::vector<short> to_short_vector(short* data, ulong sz);
  std::vector<char> to_char_vector(char* data, ulong sz);
  std::vector<int> to_int_vector(int* data, ulong sz);
  std::vector<int> to_int_vector_from_char(char* data, ulong sz);
  std::vector<long> to_long_vector(long* data, ulong sz);
  std::vector<unsigned long> to_ulong_vector(unsigned long* data, ulong sz);
  std::vector<float> to_float_vector(float* data, ulong sz);
  std::vector<double> to_double_vector(double* data, ulong sz);
  std::vector<float> double_to_float_vector(double* data, ulong sz);

  // parallel data transfer
  void get_exrpc_result(std::vector<frovedis::exrpc_ptr_t>& eps,
                        std::vector<frovedis::exrpc_result
                        <frovedis::exrpc_ptr_t>>& res,
                        size_t wsize);

}

#endif
