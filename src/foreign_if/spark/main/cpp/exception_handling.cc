#include "spark_client_headers.hpp"

// --- global exception tracking variables ---
bool status = false;
std::string info = "";

extern "C" {

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

std::string get_info() { 
    auto ret = info;
    reset_status();
    return ret;
}

JNIEXPORT jstring JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_checkServerException
  (JNIEnv *env, jclass thisClass) {
  auto info = get_info();
  return env->NewStringUTF(info.c_str());
}

}
