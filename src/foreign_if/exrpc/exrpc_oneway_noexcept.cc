#include "exrpc_oneway.hpp"
#include <unistd.h>

namespace frovedis {

void send_exrpcreq_oneway_noexcept(exrpc_node& n, const std::string& funcname,
                                   const std::string& serialized_arg) {
  int sockfd = send_exrpcreq(exrpc_type::exrpc_oneway_noexcept_type, n,
                             funcname, serialized_arg);
  ::close(sockfd);
}

void exrpc_function_oneway_noexcept(exrpc_node& n, std::string fn,
                                    void(*f)()) {
  my_portable_ostream outss;
  my_portable_oarchive outar(outss);
  PORTABLE_OSTREAM_TO_STRING(outss, str);
  send_exrpcreq_oneway_noexcept(n, fn, str);
}

}
