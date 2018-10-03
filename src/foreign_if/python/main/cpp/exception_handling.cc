#include "python_client_headers.hpp"

using namespace frovedis;

bool status = false;
std::string info = "";

extern "C" {
  // --- routines related to exception handling ---
  void set_status(bool stat, const std::string& what) {
    // if previous exception is not cleared
    if (status) info += "\n -> " + what;
    else {
      status = stat;
      info = "Exception occured at Frovedis Server: \n -> " + what;
    }
  }
  void reset_status() {
    status = false;
    info = "";
  }
  PyObject* check_server_exception() {
    auto ret = Py_BuildValue("{s:i, s:s}", "status", (int)status, "info", info.c_str()); 
    reset_status();
    return ret;
  }
}
