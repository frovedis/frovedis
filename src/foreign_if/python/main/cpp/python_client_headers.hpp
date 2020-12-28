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

  // std::vector<float> => python List of floats
  PyObject* to_python_float_list (const std::vector<float>& v); 

  // std::vector<double> => python List of doubles
  PyObject* to_python_double_list (const std::vector<double>& v);

  // --- Frovedis Data structure to Python Data structure ---
  PyObject* to_py_bfs_result(const bfs_result<DT5>& result);
  PyObject* to_py_sssp_result(const sssp_result<DT1,DT5>& result);
  PyObject* to_py_pagerank_result(const py_pagerank_result<double>& result);
  PyObject* to_py_dummy_matrix(const dummy_matrix& m);
  PyObject* to_py_mfm_info(const dummy_mfm& m);
  PyObject* to_py_dummy_vector(const dummy_vector& dv);
  PyObject* to_py_svd_result(const svd_result& obj, char, bool, bool);
  PyObject* to_py_lu_fact_result(const lu_fact_result& obj, char);
  PyObject* to_py_pca_result(const pca_result& obj, char);
  PyObject* to_py_tsne_result(const tsne_result& obj);
  PyObject* to_py_knn_result(const knn_result& obj, char);
  PyObject* to_py_kmeans_result(const kmeans_result& obj);
  PyObject* to_py_dummy_lda_result(const dummy_lda_result& m);
  PyObject* to_py_dummy_graph(const dummy_graph& obj);

  // --- server info ---
  std::vector<exrpc_node> get_worker_nodes(exrpc_node& fm_node);

  // conversion
  std::vector<std::string> to_string_vector(const char** data, ulong sz);
}

#endif
