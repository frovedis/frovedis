#ifndef MPI_RPC_HPP
#define MPI_RPC_HPP

#include "config.hpp"

#include <mpi.h>
#include <stdint.h>

#ifdef USE_THREAD
#include <pthread.h>
#endif

#ifdef USE_CEREAL
#include <cereal/archives/binary.hpp>
#endif
#ifdef USE_BOOST_SERIALIZATION
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#endif

#ifdef USE_CEREAL_FOR_RPC
namespace frovedis{
typedef cereal::BinaryInputArchive my_iarchive;
typedef cereal::BinaryOutputArchive my_oarchive;
}
#else
namespace frovedis{
typedef boost::archive::binary_oarchive my_oarchive;
typedef boost::archive::binary_iarchive my_iarchive;
}
#endif

namespace frovedis {

typedef int NID;

extern MPI_Comm frovedis_comm_rpc;
extern int frovedis_self_rank;
extern int frovedis_comm_size;

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
