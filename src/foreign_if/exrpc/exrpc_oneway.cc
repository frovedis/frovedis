#include "exrpc_oneway.hpp"
#include <unistd.h>

namespace frovedis {

void send_exrpcreq_oneway(exrpc_node& n, const std::string& funcname,
                          const std::string& serialized_arg) {
  int sockfd = send_exrpcreq(exrpc_type::exrpc_oneway_type, n, funcname,
			     serialized_arg);
  char caught_exception;
  myread(sockfd, &caught_exception, 1);
  if(caught_exception) {
    exrpc_count_t count_nw;
    myread(sockfd, reinterpret_cast<char*>(&count_nw), sizeof(count_nw));
    exrpc_count_t count = myntohll(count_nw);
    std::string buf;
    buf.resize(count);
    char* recv_data = &buf[0];
    myread(sockfd, recv_data, count);
    ::close(sockfd);
#ifdef CLIENT_DONOT_THROW_EXCEPTION_AND_PRINT
    std::cout << "Error occurred at frovedis_server: " << std::endl;
    std::cout << buf << std::endl;
#else
    throw std::runtime_error(buf);
#endif
  } else {
    ::close(sockfd);
  }
}

void exrpc_function_oneway(exrpc_node& n, std::string fn, void(*f)()) {
  my_portable_ostream outss;
  my_portable_oarchive outar(outss);
  PORTABLE_OSTREAM_TO_STRING(outss, str);
  send_exrpcreq_oneway(n, fn, str);
}

}
