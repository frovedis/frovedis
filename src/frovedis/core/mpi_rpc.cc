#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <malloc.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "mpi_rpc.hpp"
#include "mpihelper.hpp"
#include "utility.hpp"

using namespace std;

namespace frovedis {

MPI_Comm frovedis_comm_rpc;
int frovedis_self_rank;
int frovedis_comm_size;

bool frovedis_shm_init = false;
MPI_Comm frovedis_shm_comm;
int frovedis_shm_self_rank;
int frovedis_shm_comm_size;
MPI_Comm frovedis_shmroot_comm;
int frovedis_shmroot_self_rank;
int frovedis_shmroot_comm_size;

// stacks for split_context_execution
std::vector<MPI_Comm> frovedis_comm_rpc_stack;
std::vector<int> frovedis_self_rank_stack;
std::vector<int> frovedis_comm_size_stack;

std::vector<bool> frovedis_shm_init_stack;
std::vector<MPI_Comm> frovedis_shm_comm_stack;
std::vector<int> frovedis_shm_self_rank_stack;
std::vector<int> frovedis_shm_comm_size_stack;
std::vector<MPI_Comm> frovedis_shmroot_comm_stack;
std::vector<int> frovedis_shmroot_self_rank_stack;
std::vector<int> frovedis_shmroot_comm_size_stack;

std::string frovedis_tmpdir;

#ifdef USE_THREAD
pthread_mutex_t mpi_tag_lock = PTHREAD_MUTEX_INITIALIZER;
#endif
int mpi_tag = 1;

int get_mpi_tag() {
#ifdef USE_THREAD
  pthread_mutex_lock(&mpi_tag_lock);
#endif
  int ret = mpi_tag;
  if(mpi_tag == system_tag_fence) mpi_tag = 1;
  else mpi_tag++;
#ifdef USE_THREAD
  pthread_mutex_unlock(&mpi_tag_lock);
#endif
  return ret;
}

bool handle_one_bcast_req() {
  int hdr_count = sizeof(rpc_header);
  char hdr_buf[hdr_count];
  const int root = 0;
  MPI_Bcast(hdr_buf, hdr_count, MPI_CHAR, root, frovedis_comm_rpc);
  rpc_header* hdr = reinterpret_cast<rpc_header*>(hdr_buf);
  string serialized_arg;
  serialized_arg.resize(hdr->count);
  char* recv_data = &serialized_arg[0];
  large_bcast(sizeof(char), recv_data, hdr->count, root, frovedis_comm_rpc);
  if(hdr->type == rpc_type::bcast_rpc_type) {
    typedef void(*wptype)(intptr_t, my_iarchive&, my_oarchive&);
    wptype wpt = (wptype)(hdr->wrapper_addr);
    STRING_TO_ISTREAM(inss,serialized_arg);
    my_iarchive inar(inss);
    my_ostream result;
    my_oarchive outar(result);
    string what;
    int exception_caught = false;
    int all_exception_caught = false;
    try {
      wpt(hdr->function_addr, inar, outar);
    } catch (std::bad_alloc& e) {
      exception_caught = true;
      what = std::string("std::bad_alloc");
    } catch (std::exception& e) {
      exception_caught = true;
      what = e.what();
    }
    MPI_Allreduce(reinterpret_cast<void*>(&exception_caught),
                  reinterpret_cast<void*>(&all_exception_caught),
                  1, MPI_INT, MPI_LOR, frovedis_comm_rpc);
    if(!all_exception_caught) {
      OSTREAM_TO_STRING(result, resultstr);
      size_t send_data_size = resultstr.size();
      char* send_data = &resultstr[0];
      MPI_Gather(reinterpret_cast<void*>(&send_data_size), 
                 sizeof(send_data_size), MPI_CHAR,
                 0, sizeof(send_data_size), MPI_CHAR, root, frovedis_comm_rpc);
      std::vector<size_t> recvcounts; // dummy
      std::vector<size_t> displs; // dummy
      large_gatherv(sizeof(char), send_data, send_data_size,
                    0, recvcounts, displs, root, frovedis_comm_rpc);
    } else {
      size_t send_data_size = what.size();
      char* send_data = const_cast<char*>(what.c_str());
      MPI_Gather(reinterpret_cast<void*>(&send_data_size), 
                 sizeof(send_data_size), MPI_CHAR,
                 0, sizeof(send_data_size), MPI_CHAR, root, frovedis_comm_rpc);
      std::vector<size_t> recvcounts; // dummy
      std::vector<size_t> displs; // dummy
      large_gatherv(sizeof(char), send_data, send_data_size,
                    0, recvcounts, displs, root, frovedis_comm_rpc);
    }
  } else if (hdr->type == rpc_type::bcast_rpc_oneway_type) {
    typedef void(*wptype)(intptr_t, my_iarchive&);
    wptype wpt = (wptype)(hdr->wrapper_addr);
    STRING_TO_ISTREAM(inss, serialized_arg);
    my_iarchive inar(inss);
    string what;
    int exception_caught = false;
    int all_exception_caught = false;
    try {
      wpt(hdr->function_addr, inar);
    } catch (std::bad_alloc& e) {
      exception_caught = true;
      what = std::string("std::bad_alloc");
    } catch (std::exception& e) {
      exception_caught = true;
      what = e.what();
    }
    MPI_Allreduce(reinterpret_cast<void*>(&exception_caught),
                  reinterpret_cast<void*>(&all_exception_caught),
                  1, MPI_INT, MPI_LOR, frovedis_comm_rpc);
    if(all_exception_caught) {
      size_t send_data_size = what.size();
      char* send_data = const_cast<char*>(what.c_str());
      MPI_Gather(reinterpret_cast<void*>(&send_data_size), 
                 sizeof(send_data_size), MPI_CHAR,
                 0, sizeof(send_data_size), MPI_CHAR, root, frovedis_comm_rpc);
      std::vector<size_t> recvcounts; // dummy
      std::vector<size_t> displs; // dummy
      large_gatherv(sizeof(char), send_data, send_data_size,
                    0, recvcounts, displs, root, frovedis_comm_rpc);
    }
  } else if (hdr->type == rpc_type::finalize_type) {
    return false;
  }
  return true;
}

bool handle_one_req() {
  int hdr_count = sizeof(rpc_header);
  char hdr_buf[hdr_count];
  MPI_Status s;
  MPI_Recv(hdr_buf, hdr_count, MPI_CHAR, MPI_ANY_SOURCE, 0, frovedis_comm_rpc,
           &s);
  rpc_header* hdr = reinterpret_cast<rpc_header*>(hdr_buf);
  int from = s.MPI_SOURCE;
  string serialized_arg;
  serialized_arg.resize(hdr->count);
  char* recv_data = &serialized_arg[0];
  for(size_t pos = 0; pos < hdr->count; pos += mpi_max_count) {
    size_t recv_size = std::min(hdr->count - pos, mpi_max_count);
    MPI_Recv(recv_data + pos, recv_size, MPI_CHAR, from, hdr->tag,
             frovedis_comm_rpc, &s);
  }
  if(hdr->type == rpc_type::rpc_async_type) {
    typedef void(*wptype)(intptr_t, my_iarchive&, my_oarchive&);
    wptype wpt = (wptype)(hdr->wrapper_addr);
    STRING_TO_ISTREAM(inss, serialized_arg);
    my_iarchive inar(inss);
    my_ostream result;
    my_oarchive outar(result);
    wpt(hdr->function_addr, inar, outar);
    OSTREAM_TO_STRING(result, resultstr);
    size_t send_data_size = resultstr.size();
    MPI_Send(reinterpret_cast<char*>(&send_data_size), sizeof(send_data_size),
             MPI_CHAR, from, hdr->tag, frovedis_comm_rpc);
    char* send_data = const_cast<char*>(resultstr.c_str());
    for(size_t pos = 0; pos < send_data_size; pos += mpi_max_count) {
      size_t send_size = std::min(send_data_size - pos, mpi_max_count);
      MPI_Send(send_data + pos, send_size, MPI_CHAR, from, hdr->tag,
               frovedis_comm_rpc);
    }
  } else if (hdr->type == rpc_type::rpc_oneway_type) {
    typedef void(*wptype)(intptr_t, my_iarchive&);
    wptype wpt = (wptype)(hdr->wrapper_addr);
    STRING_TO_ISTREAM(inss, serialized_arg);
    my_iarchive inar(inss);
    wpt(hdr->function_addr, inar);
  } else if (hdr->type == rpc_type::finalize_type) {
    return false;
  }
  return true;
}

void handle_req() {
  while (handle_one_bcast_req());
  if(directory_exists(frovedis_tmpdir))
    remove_directory(frovedis_tmpdir);
  MPI_Finalize();
  exit(0);
}

int send_rpcreq(rpc_type type, NID n, intptr_t function_addr,
                intptr_t wrapper_addr, const string& serialized_arg) {
  rpc_header hdr;
  hdr.type = type;
  hdr.tag = get_mpi_tag();
  hdr.function_addr = function_addr;
  hdr.wrapper_addr = wrapper_addr;
  hdr.count = serialized_arg.size();
  char* hdr_buf = reinterpret_cast<char*>(&hdr);
  MPI_Send(hdr_buf, sizeof(rpc_header), MPI_CHAR, n, 0, frovedis_comm_rpc);
  char* send_data = const_cast<char*>(serialized_arg.c_str());
  for(size_t pos = 0; pos < hdr.count; pos += mpi_max_count) {
    size_t send_size = std::min(hdr.count - pos, mpi_max_count);
    MPI_Send(send_data + pos, send_size, MPI_CHAR, n, hdr.tag,
             frovedis_comm_rpc);
  }
  return hdr.tag;
}

void send_bcast_rpcreq(rpc_type type, intptr_t function_addr,
                       intptr_t wrapper_addr, const string& serialized_arg,
                       vector<string>& serialized_result) {
  int node_size = get_nodesize();
  rpc_header hdr;
  hdr.type = type;
  hdr.tag = get_mpi_tag();
  hdr.function_addr = function_addr;
  hdr.wrapper_addr = wrapper_addr;
  hdr.count = serialized_arg.size();
  const int root = 0;
  char* hdr_buf = reinterpret_cast<char*>(&hdr);
  MPI_Bcast(hdr_buf, sizeof(rpc_header), MPI_CHAR, root, frovedis_comm_rpc);
  char* send_data = const_cast<char*>(serialized_arg.c_str());  
  large_bcast(sizeof(char), send_data, hdr.count, root, frovedis_comm_rpc);
  if(type == rpc_type::bcast_rpc_type) {
    typedef void(*wptype)(intptr_t, my_iarchive&, my_oarchive&);
    wptype wpt = (wptype)(hdr.wrapper_addr);
    STRING_TO_ISTREAM(inss, serialized_arg);
    my_iarchive inar(inss);
    my_ostream result;
    my_oarchive outar(result);
    string what;
    int exception_caught = false;
    int all_exception_caught = false;
    try {
      wpt(hdr.function_addr, inar, outar);
    } catch (std::bad_alloc& e) {
      exception_caught = true;
      what = std::string("std::bad_alloc");
    } catch (std::exception& e) {
      exception_caught = true;
      what = e.what();
    }
    MPI_Allreduce(reinterpret_cast<void*>(&exception_caught),
                  reinterpret_cast<void*>(&all_exception_caught),
                  1, MPI_INT, MPI_LOR, frovedis_comm_rpc);
    if(!all_exception_caught) {
      OSTREAM_TO_STRING(result, resultstr);
      size_t send_data_size = resultstr.size();
      char* send_data = &resultstr[0];
      vector<size_t> recv_counts(node_size);
      MPI_Gather(reinterpret_cast<void*>(&send_data_size), 
                 sizeof(send_data_size), MPI_CHAR,
                 &recv_counts[0], sizeof(send_data_size), MPI_CHAR, 
                 root, frovedis_comm_rpc);
      vector<size_t> displs(node_size);
      displs[0] = 0;
      auto displsp = displs.data();
      auto recv_countsp = recv_counts.data();
      for(size_t i = 1; i < node_size; i++) {
        displsp[i] = displsp[i-1] + recv_countsp[i-1];
      }
      size_t total = 0;
      for(size_t i = 0; i < node_size; i++) total += recv_countsp[i];
      string recv_data;
      recv_data.resize(total);
      large_gatherv(sizeof(char), send_data, send_data_size,
                    &recv_data[0], recv_counts, displs, root,
                    frovedis_comm_rpc);
      serialized_result.resize(node_size);
      for(int i = 0; i < node_size; i++) {
        serialized_result[i].resize(recv_counts[i]);
        char* serialized_resultp = &serialized_result[i][0];
        char* recv_datap = &recv_data[displs[i]];
        for(size_t j = 0; j < recv_counts[i]; j++) {
          serialized_resultp[j] = recv_datap[j]; 
        }
      }
    } else {
      size_t send_data_size = what.size();
      char* send_data = const_cast<char*>(what.c_str());
      vector<size_t> recv_counts(node_size);
      MPI_Gather(reinterpret_cast<void*>(&send_data_size), 
                 sizeof(send_data_size), MPI_CHAR,
                 &recv_counts[0], sizeof(send_data_size), MPI_CHAR, 
                 root, frovedis_comm_rpc);
      vector<size_t> displs(node_size);
      displs[0] = 0;
      auto displsp = displs.data();
      auto recv_countsp = recv_counts.data();
      for(size_t i = 1; i < node_size; i++) {
        displsp[i] = displsp[i-1] + recv_countsp[i-1];
      }
      size_t total = 0;
      for(size_t i = 0; i < node_size; i++) total += recv_countsp[i];
      string recv_data;
      recv_data.resize(total);
      large_gatherv(sizeof(char), send_data, send_data_size,
                    &recv_data[0], recv_counts, displs, root,
                    frovedis_comm_rpc);
      string all_what = "exception at any of the ranks:";
      for(int i = 0; i < node_size; i++) {
        string what(recv_data.substr(displs[i], recv_counts[i]));
        if(what != "") {
          all_what.append(string("\n") + string("rank ") + to_string(i) +
                          ": " + what);
        }
      }
      throw std::runtime_error(all_what);
    }
  } else if (type == rpc_type::bcast_rpc_oneway_type) {
    typedef void(*wptype)(intptr_t, my_iarchive&);
    wptype wpt = (wptype)(hdr.wrapper_addr);
    STRING_TO_ISTREAM(inss, serialized_arg);
    my_iarchive inar(inss);
    string what;
    int exception_caught = false;
    int all_exception_caught = false;
    try {
      wpt(hdr.function_addr, inar);
    } catch (std::bad_alloc& e) {
      exception_caught = true;
      what = std::string("std::bad_alloc");
    } catch (std::exception& e) {
      exception_caught = true;
      what = e.what();
    }
    MPI_Allreduce(reinterpret_cast<void*>(&exception_caught),
                  reinterpret_cast<void*>(&all_exception_caught),
                  1, MPI_INT, MPI_LOR, frovedis_comm_rpc);
    if(all_exception_caught) {
      size_t send_data_size = what.size();
      char* send_data = const_cast<char*>(what.c_str());
      vector<size_t> recv_counts(node_size);
      MPI_Gather(reinterpret_cast<void*>(&send_data_size), 
                 sizeof(send_data_size), MPI_CHAR,
                 &recv_counts[0], sizeof(send_data_size), MPI_CHAR, 
                 root, frovedis_comm_rpc);
      vector<size_t> displs(node_size);
      displs[0] = 0;
      auto displsp = displs.data();
      auto recv_countsp = recv_counts.data();
      for(size_t i = 1; i < node_size; i++) {
        displsp[i] = displsp[i-1] + recv_countsp[i-1];
      }
      size_t total = 0;
      for(size_t i = 0; i < node_size; i++) total += recv_countsp[i];
      string recv_data;
      recv_data.resize(total);
      large_gatherv(sizeof(char), send_data, send_data_size,
                    &recv_data[0], recv_counts, displs, root,
                    frovedis_comm_rpc);
      string all_what = "exception at any of the ranks:";
      for(int i = 0; i < node_size; i++) {
        string what(recv_data.substr(displs[i], recv_counts[i]));
        if(what != "") {
          all_what.append(string("\n") + string("rank ") + to_string(i) +
                          ": " + what);
        }
      }
      throw std::runtime_error(all_what);
    }
  } // do nothing for finalize_type
}

void send_bcast_rpcreq_oneway(rpc_type type, intptr_t function_addr,
                              intptr_t wrapper_addr,
                              const string& serialized_arg) {
  vector<string> dummy;
  send_bcast_rpcreq(type, function_addr, wrapper_addr, serialized_arg, dummy);
}


void initfrovedis(int argc, char* argv[]) {
  int provided;
  // for OpenMP hybrid
  int err = MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
  if(err != 0) {
    cerr << "MPI_THREAD_SERIALIZED is not supported. exiting." << endl;
    MPI_Finalize();
    exit(1);
  }
  MPI_Comm_dup(MPI_COMM_WORLD, &frovedis_comm_rpc);
  MPI_Comm_rank(MPI_COMM_WORLD, &frovedis_self_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &frovedis_comm_size);

/*
  On VE/glibc environment, configure not to return memory to VEOS,
  because OS needs to zero clear memory before giving it to user process
  and it takes large amount of time on VE, since it is done from VH
*/
#if defined(__ve__) && defined(M_MMAP_MAX) && defined(M_TRIM_THRESHOLD)
  mallopt(M_MMAP_MAX,0);
  mallopt(M_TRIM_THRESHOLD,-1);
#endif

  auto pidstr = std::to_string(getpid());
  auto rankstr = std::to_string(frovedis_self_rank);
  auto tmpdir = getenv("FROVEDIS_TMPDIR");
  if(tmpdir != NULL) {
    frovedis_tmpdir =
      std::string(tmpdir) + "/frovedis_" + rankstr + "_" + pidstr;
      
  } else {
    frovedis_tmpdir =
      std::string("/var/tmp/frovedis_") + rankstr + "_" + pidstr;
  }

  if(frovedis_self_rank != 0) {
    handle_req();
  }
}

void finalizefrovedis(int code) {
  std::vector<std::string> dummy;
  send_bcast_rpcreq(rpc_type::finalize_type, 0, 0, "", dummy);
  if(directory_exists(frovedis_tmpdir))
    remove_directory(frovedis_tmpdir);
  MPI_Finalize();
  exit(code);
}

}  

