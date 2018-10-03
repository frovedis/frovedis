#include "python_client_headers.hpp"

using namespace frovedis;

extern "C" {
  // std::string => python string object
  PyObject* to_python_string_object (std::string& val) {
    return Py_BuildValue("s",val.c_str());
  }
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
}
