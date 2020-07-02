#ifndef EXRPC_HPP
#define EXRPC_HPP

// using hostname is more robust and flexible for routing
//#define USE_IP_EXRPC 
#define NO_PROGRAM_OPTION // sometimes VE compiler does not like it

/* tentative: at client side do not throw exception; just print error.
   later make it throw in client language like python and scala */
//#define CLIENT_DONOT_THROW_EXCEPTION_AND_PRINT

#include "frovedis.hpp"
#include "frovedis/core/exceptions.hpp"

#if defined(USE_YAS)
#include <yas/types/std/vector.hpp>
#include <yas/types/std/string.hpp>
#include <yas/types/std/map.hpp>
#include <yas/types/std/pair.hpp>
#elif defined(USE_CEREAL)
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/utility.hpp>
#elif defined(USE_BOOST_SERIALIZATION)
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/utility.hpp>
#endif

#if defined(USE_YAS_FOR_EXRPC)
namespace frovedis{
typedef yas::mem_istream my_portable_istream;
typedef yas::mem_ostream my_portable_ostream;
typedef yas::binary_iarchive<yas::mem_istream,yas::binary|yas::no_header>
  my_portable_iarchive;
typedef yas::binary_oarchive<yas::mem_ostream,yas::binary|yas::no_header>
  my_portable_oarchive;
}
#define STRING_TO_PORTABLE_ISTREAM(is, str) \
  my_portable_istream is(str.c_str(), str.size())
#define PORTABLE_OSTREAM_TO_STRING(os, str)           \
  std::string str;                                    \
  auto os ## _yas_buf = os.get_intrusive_buffer();    \
  str.assign(os ## _yas_buf.data, os ## _yas_buf.size)

#elif defined(USE_CEREAL_FOR_EXRPC)
namespace frovedis{
typedef std::istringstream my_portable_istream;
typedef std::ostringstream my_portable_ostream;
typedef cereal::BinaryInputArchive my_portable_iarchive;
typedef cereal::BinaryOutputArchive my_portable_oarchive;
}
#define STRING_TO_PORTABLE_ISTREAM(is, str) my_portable_istream is(str)
#define PORTABLE_OSTREAM_TO_STRING(os, r) auto r = os.str()

#elif defined(USE_BOOST_FOR_EXRPC)
#define NO_EXPLICIT_TEMPLATE_INSTANTIATION
#include "portable_iarchive.hpp"
#include "portable_oarchive.hpp"
namespace frovedis{
typedef std::istringstream my_portable_istream;
typedef std::ostringstream my_portable_ostream;
typedef eos::portable_iarchive my_portable_iarchive;
typedef eos::portable_oarchive my_portable_oarchive;
}
#define STRING_TO_PORTABLE_ISTREAM(is, str) my_portable_istream is(str)
#define PORTABLE_OSTREAM_TO_STRING(os, r) auto r = os.str()
#endif

namespace frovedis {

struct exrpc_node {
  std::string hostname;
  int rpcport;
  exrpc_node(std::string hostname, int rpcport) :
    hostname(hostname), rpcport(rpcport) {}
  exrpc_node(){}
  SERIALIZE(hostname, rpcport)
};

enum exrpc_type {
  exrpc_async_type,
  exrpc_oneway_type,
  exrpc_oneway_noexcept_type,
  exrpc_finalize_type
};

typedef uint64_t exrpc_count_t; // to make sure it is 64bit
typedef int64_t exrpc_ptr_t; // basically intptr_t; to make sure it is 64bit

struct exrpc_header {
  exrpc_type type;
  std::string funcname;
  exrpc_count_t arg_count;
  SERIALIZE(type, funcname, arg_count)
};

int send_exrpcreq(exrpc_type, exrpc_node&, const std::string&, const std::string&);
void mywrite(int fd, const char* write_data, size_t to_write);
void myread(int fd, char* read_data, size_t to_read);
void send_exrpc_finish(exrpc_node&);

int handle_exrpc_listen(int& port);
bool handle_exrpc_accept(int sockfd, int timeout, int& new_sockfd);
int handle_exrpc_connect(const std::string& hostname, int rpcport);
bool handle_exrpc_process(int new_sockfd);
bool handle_exrpc_onereq(int sockfd, int timeout = 0);

exrpc_node invoke_frovedis_server(const std::string& command);
void init_frovedis_server(int argc, char* argv[]);
void finalize_frovedis_server(exrpc_node&);

struct exrpc_info {
  exrpc_node node;
  int sockfd;
  SERIALIZE(node, sockfd)
};

template <class T>
struct exptr {
  exptr() : ptr(0) {}
  exptr(T* p) : ptr(reinterpret_cast<exrpc_ptr_t>(p)) {}
  T* to_ptr() {return reinterpret_cast<T*>(ptr);}
  exrpc_ptr_t ptr;
  SERIALIZE(ptr)
};

template <class T>
exptr<T> make_exptr(T* p){return exptr<T>(p);}

template <class T>
void delete_exptr(exptr<T>& p){delete reinterpret_cast<T*>(p.ptr);}

exptr<node_local<exrpc_info>> prepare_parallel_exrpc(exrpc_node&);
exptr<node_local<exrpc_info>> prepare_parallel_exrpc_server();
std::vector<exrpc_node> get_parallel_exrpc_nodes(exrpc_node&,exptr<node_local<exrpc_info>>&);
std::vector<exrpc_node> get_parallel_exrpc_nodes_server(exptr<node_local<exrpc_info>>&);
void wait_parallel_exrpc(exrpc_node&,exptr<node_local<exrpc_info>>&);
void wait_parallel_exrpc_server(exptr<node_local<exrpc_info>>&);

/*
  Copied from dvector.cc
  TODO: change to share the code!
 */
inline bool is_bigendian() {
  int i = 1;
  if(*((char*)&i)) return false;
  else return true;
}

inline uint64_t swap64(uint64_t val) {
  return ( (((val) >> 56) & 0x00000000000000FF) |
           (((val) >> 40) & 0x000000000000FF00) |
           (((val) >> 24) & 0x0000000000FF0000) |
           (((val) >>  8) & 0x00000000FF000000) |
           (((val) <<  8) & 0x000000FF00000000) |
           (((val) << 24) & 0x0000FF0000000000) |
           (((val) << 40) & 0x00FF000000000000) |
           (((val) << 56) & 0xFF00000000000000) );
}

inline uint64_t myhtonll(uint64_t a){
  if(is_bigendian()) return a;
  else return(swap64(a));
}

inline uint64_t myntohll(uint64_t a){
  return myhtonll(a);
}

}
#endif
