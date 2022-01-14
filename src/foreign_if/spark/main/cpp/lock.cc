#include "spark_client_headers.hpp"

pthread_mutex_t parallel_management_lock = PTHREAD_MUTEX_INITIALIZER;

extern "C" {

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_lockParallel
  (JNIEnv *env, jclass thisClass) {
  pthread_mutex_lock(&parallel_management_lock);
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_unlockParallel
  (JNIEnv *env, jclass thisClass) {
  pthread_mutex_unlock(&parallel_management_lock);
}

}
