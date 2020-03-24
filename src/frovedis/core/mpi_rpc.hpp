#ifndef MPI_RPC_HPP
#define MPI_RPC_HPP

#include "config.hpp"

#include <mpi.h>
#include <stdint.h>
#include <vector>
#include <string>

#ifdef USE_THREAD
#include <pthread.h>
#endif

#ifdef USE_YAS
#include <yas/mem_streams.hpp>
#include <yas/binary_iarchive.hpp>
#include <yas/binary_oarchive.hpp>
#endif
#ifdef USE_CEREAL
#include <sstream>
#include <cereal/archives/binary.hpp>
#endif
#ifdef USE_BOOST_SERIALIZATION
#include <sstream>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#endif

#if defined(USE_YAS_FOR_RPC)
namespace frovedis{
typedef yas::mem_istream my_istream;
typedef yas::mem_ostream my_ostream;
typedef yas::binary_iarchive<yas::mem_istream,yas::binary|yas::no_header>
  my_iarchive;
typedef yas::binary_oarchive<yas::mem_ostream,yas::binary|yas::no_header>
  my_oarchive;
}
#define STRING_TO_ISTREAM(is, str) my_istream is(str.c_str(), str.size())
#define OSTREAM_TO_STRING(os, str)                    \
  std::string str;                                    \
  auto os ## _yas_buf = os.get_intrusive_buffer();    \
  str.assign(os ## _yas_buf.data, os ## _yas_buf.size)
#elif defined(USE_CEREAL_FOR_RPC)
namespace frovedis{
typedef std::istringstream my_istream;
typedef std::ostringstream my_ostream;
typedef cereal::BinaryInputArchive my_iarchive;
typedef cereal::BinaryOutputArchive my_oarchive;
}
#define STRING_TO_ISTREAM(is, str) my_istream is(str)
#define OSTREAM_TO_STRING(os, r) auto r = os.str()
#else
namespace frovedis{
typedef std::istringstream my_istream;
typedef std::ostringstream my_ostream;
typedef boost::archive::binary_iarchive my_iarchive;
typedef boost::archive::binary_oarchive my_oarchive;
}
#define STRING_TO_ISTREAM(is, str) my_istream is(str)
#define OSTREAM_TO_STRING(os, r) auto r = os.str()
#endif

namespace frovedis {

typedef int NID;

extern MPI_Comm frovedis_comm_rpc;
extern int frovedis_self_rank;
extern int frovedis_comm_size;

// for shared memory between MPI processes
extern bool frovedis_shm_init;
extern MPI_Comm frovedis_shm_comm;
extern int frovedis_shm_self_rank;
extern int frovedis_shm_comm_size;
extern MPI_Comm frovedis_shmroot_comm;
extern int frovedis_shmroot_self_rank;
extern int frovedis_shmroot_comm_size;

#ifdef USE_THREAD
extern pthread_mutex_t mpi_tag_lock;
#endif
extern int mpi_tag;

// INT_MAX is OK, but make it power of 2
const size_t mpi_max_count = 1 << 30; // 1G

const int system_tag_fence = 1 << 14; // 16384
const int bcast_tag = system_tag_fence + 1;

inline NID get_selfid(){return frovedis_self_rank;}
inline int get_nodesize(){return frovedis_comm_size;}

inline int get_shm_id(){return frovedis_shm_self_rank;}
inline int get_shm_size(){return frovedis_shm_comm_size;}
inline bool is_shm_root(){return frovedis_shm_self_rank == 0 ? true : false;}

enum rpc_type {
  rpc_async_type,
  rpc_oneway_type,
  bcast_rpc_type,
  bcast_rpc_oneway_type,
  finalize_type
};

struct rpc_header {
  rpc_type type;
  int tag;
  intptr_t function_addr;
  intptr_t wrapper_addr;
  size_t count;
};

int get_mpi_tag();
void handle_req();
bool handle_one_req();
int send_rpcreq(rpc_type, NID, intptr_t, intptr_t, const std::string&);        
void send_bcast_rpcreq(rpc_type, intptr_t, intptr_t, const std::string&,
                       std::vector<std::string>&);
void send_bcast_rpcreq_oneway(rpc_type, intptr_t, intptr_t,
                              const std::string&);
}

#endif
