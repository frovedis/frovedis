#ifndef EXRPC_RESULT_HPP
#define EXRPC_RESULT_HPP

#include <string>
#include <stdexcept>
#include <sstream>
#include <unistd.h>

#include "exrpc.hpp"

namespace frovedis {

template <class T>
class exrpc_result {
public:
  exrpc_result() : sockfd(-1), get_done(true) {}
  exrpc_result(int sockfd) : sockfd(sockfd), get_done(false) {}
  T get();
private:
  int sockfd;
  bool get_done;
};

template <class T>
T exrpc_result<T>::get() {
  if(get_done)
    throw std::runtime_error
      ("exrpc_result: get() called already got variable");
  else {
    char caught_exception;
    myread(sockfd, &caught_exception, 1);
    exrpc_count_t count_nw;
    myread(sockfd, reinterpret_cast<char*>(&count_nw), sizeof(count_nw));
    exrpc_count_t count = myntohll(count_nw);
    std::string buf;
    buf.resize(count);
    char* recv_data = &buf[0];
    myread(sockfd, recv_data, count);
    auto it = send_connection_lock.find(sockfd);
    if(it == send_connection_lock.end()) {
      throw std::runtime_error("internal error in get");
    } else {
      pthread_mutex_unlock(it->second);
    }
    if(!caught_exception) {
      STRING_TO_PORTABLE_ISTREAM(ss, buf);
      my_portable_iarchive ar(ss);
      T ret;
      ar & ret;
      get_done = true;
      return ret;
    } else {
      get_done = true;
#ifdef CLIENT_DONOT_THROW_EXCEPTION_AND_PRINT
      std::cout << "Error occurred at frovedis_server: " << std::endl;
      std::cout << buf << std::endl;
      return T();
#else
      throw std::runtime_error(buf);
#endif
    }
  }
}

}

#endif
