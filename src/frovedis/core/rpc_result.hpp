#ifndef RPC_RESULT_HPP
#define RPC_RESULT_HPP

#include "mpi_rpc.hpp"
#include <string>
#include <stdexcept>
#include <sstream>

namespace frovedis {

template <class T>
class rpc_result {
public:
  rpc_result(int tag, int from) : tag(tag), from(from), get_done(false) {}
  T get();
private:
  int tag;
  int from;
  bool get_done;
};

template <class T>
T rpc_result<T>::get() {
// this causes performance penalty on SX
#if !(defined(_SX) || defined(__ve__))
  if(get_done)
    throw std::runtime_error
      ("rpc_result: get() called already got variable");
  else {
#endif
    MPI_Status s;
    size_t count;
    MPI_Recv(reinterpret_cast<char*>(&count), sizeof(count), MPI_CHAR,
             from, tag, frovedis_comm_rpc, &s);
    std::string buf;
    buf.resize(count);
    char* recv_data = &buf[0];
    size_t pos = 0;
    for(; pos < count; pos += mpi_max_count) {
      size_t recv_size = std::min(count - pos, mpi_max_count);
      MPI_Recv(recv_data + pos, recv_size, MPI_CHAR, from, tag,
               frovedis_comm_rpc, &s);
    }
    // TODO: remove this copy
    std::istringstream ss(buf);
    my_iarchive ar(ss);
    T ret;
    ar >> ret;
    get_done = true;
    return ret;
#if !(defined(_SX) || defined(__ve__))
  }
#endif
}

}

#endif
