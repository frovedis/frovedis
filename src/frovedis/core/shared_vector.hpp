#ifndef SHARED_VECTOR
#define SHARED_VECTOR

// shared vector requires MPI 3.0
#include <mpi.h>
#if MPI_VERSION >= 3

#include "node_local.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <type_traits>
#include <semaphore.h>

namespace frovedis {

template <class T>
struct shared_vector_local {
  shared_vector_local() : size(0), intptr(0) {}
  T* data() {return reinterpret_cast<T*>(intptr);}
  size_t size;
  intptr_t intptr;
  std::string shm_name;
  SERIALIZE(size, intptr, shm_name)
};

int get_new_shmid();
extern pid_t shm_root_pid;

template <class T>
void init_shared_vector(shared_vector_local<T>& sv, size_t size) {
  if(!frovedis_shm_init) {
    int r = MPI_Comm_split_type(frovedis_comm_rpc, MPI_COMM_TYPE_SHARED, 0,
                                MPI_INFO_NULL, &frovedis_shm_comm);
    if(r != 0) throw std::runtime_error("failed to call MPI_Comm_split_type");
    MPI_Comm_rank(frovedis_shm_comm, &frovedis_shm_self_rank);
    MPI_Comm_size(frovedis_shm_comm, &frovedis_shm_comm_size);
    int color = is_shm_root() ? 0 : 1;
    r  = MPI_Comm_split(frovedis_comm_rpc, color, 0, &frovedis_shmroot_comm);
    if(r != 0) throw std::runtime_error("failed to call MPI_Comm_split");
    MPI_Comm_rank(frovedis_shmroot_comm, &frovedis_shmroot_self_rank);
    MPI_Comm_size(frovedis_shmroot_comm, &frovedis_shmroot_comm_size);
    if(frovedis_shm_self_rank == 0) {
      shm_root_pid = getpid();
      MPI_Bcast(reinterpret_cast<char*>(&shm_root_pid), sizeof(pid_t),
                MPI_CHAR, 0, frovedis_shm_comm);
    } else {
      MPI_Bcast(reinterpret_cast<char*>(&shm_root_pid), sizeof(pid_t),
                MPI_CHAR, 0, frovedis_shm_comm);
    }
    frovedis_shm_init = true;;
  }
  size_t bytesize = size * sizeof(T);
  if(frovedis_shm_self_rank == 0) {
    std::string shm_name("/frovedis_shm_");
    shm_name.append(std::to_string(shm_root_pid)).append("_").
      append(std::to_string(get_new_shmid()));
    sv.shm_name = shm_name;
    int fd = shm_open(shm_name.c_str(), O_RDWR | O_CREAT,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if(fd == -1) {
      throw std::runtime_error(std::string("error in shm_open: ") +
                               std::string(strerror(errno)));
    }
    if(ftruncate(fd, bytesize) == -1) {
      throw std::runtime_error(std::string("error in ftruncate: ") +
                               std::string(strerror(errno)));
    }
    sv.size = size;
    void* addr = mmap(0, bytesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == (void*)(-1)) 
      throw std::runtime_error(std::string("error in mmap: ") +
                               std::string(strerror(errno)));
    sv.intptr = reinterpret_cast<intptr_t>(addr);
    MPI_Barrier(frovedis_comm_rpc);
  } else {
    MPI_Barrier(frovedis_comm_rpc);
    std::string shm_name("/frovedis_shm_");
    shm_name.append(std::to_string(shm_root_pid)).append("_").
      append(std::to_string(get_new_shmid()));
    sv.shm_name = shm_name;
    int fd = shm_open(shm_name.c_str(), O_RDWR, 
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if(fd == -1) {
      throw std::runtime_error(std::string("error in shm_open: ") +
                               std::string(strerror(errno)));
    }
    sv.size = size;
    void* addr = mmap(0, bytesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == (void*)(-1)) 
      throw std::runtime_error(std::string("error in mmap: ") +
                               std::string(strerror(errno)));
    sv.intptr = reinterpret_cast<intptr_t>(addr);
  }
}

template <class T>
void free_shared_vector(shared_vector_local<T>& sv) {
  if(sv.intptr != 0) {
    if(munmap(reinterpret_cast<void*>(sv.intptr), sv.size) == -1) {
      throw std::runtime_error(std::string("error in munmap: ")+
                               std::string(strerror(errno)));
    }
    if(frovedis_shm_self_rank == 0) {
      if(shm_unlink(sv.shm_name.c_str()) == -1) {
        throw std::runtime_error(std::string("error in shm_unlink: ")+
                                 std::string(strerror(errno)));
      }
    }
  }
}

template <class T>
struct shared_vector {
  shared_vector() : allocated(false), size(0) {}
  shared_vector(node_local<shared_vector_local<T>>&& svl, size_t size)
    : allocated(true), size(size), data(std::move(svl)) {}
  shared_vector(const shared_vector<T>& sv);
  shared_vector(shared_vector<T>&& sv);
  ~shared_vector();
  shared_vector<T>& operator=(const shared_vector<T>& src);
  shared_vector<T>& operator=(shared_vector<T>&& src);
  std::vector<T> sum();
  void allsum();
  bool allocated;
  size_t size;
  node_local<shared_vector_local<T>> data;
};

template <class T>
shared_vector<T> make_shared_vector(size_t size) {
  auto sv = make_node_local_allocate<shared_vector_local<T>>();
  auto bcast_size = broadcast(size);
  sv.mapv(init_shared_vector<T>, bcast_size);
  return shared_vector<T>(std::move(sv), size);
}

template <class T>
void copy_shared_vector(shared_vector_local<T>& dst,
                        shared_vector_local<T>& src) {
  size_t size = dst.size;
  if(src.size != size)
    throw std::runtime_error("copy_shared_vector: size mismatch");
  T* dstptr = dst.data();
  T* srcptr = src.data();
  for(size_t i = 0; i < size; i++) dstptr[i] = srcptr[i];
}

template <class T>
shared_vector<T>::~shared_vector() {
  if(allocated) {
    // do not want to throw exception
    try {
      data.mapv(free_shared_vector<T>);
    } catch (std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
    }
  }
}

template <class T>
shared_vector<T>::shared_vector(const shared_vector<T>& sv) {
  if(sv.allocated) {
    allocated = true;
    size = sv.size;
    data = make_node_local_allocate<shared_vector_local<T>>();
    auto bcast_size = broadcast(sv.size);
    data.mapv(init_shared_vector<T>, bcast_size);
    data.mapv(copy_shared_vector<T>, sv.data);
  } else {
    size = sv.size;
    allocated = sv.allocated;
  }
}

template <class T>
shared_vector<T>::shared_vector(shared_vector<T>&& sv) {
  allocated = sv.allocated;
  size = sv.size;
  if(sv.allocated) {
    data = std::move(sv.data);
    sv.allocated = false;
  } 
}

template <class T>
shared_vector<T>& shared_vector<T>::operator=(const shared_vector<T>& sv) {
  if(allocated) data.mapv(free_shared_vector<T>);
  allocated = sv.allocated;
  size = sv.size;
  if(sv.allocated) {
    data = make_node_local_allocate<shared_vector_local<T>>();
    auto bcast_size = broadcast(sv.size);
    data.mapv(init_shared_vector<T>, bcast_size);
    data.mapv(copy_shared_vector<T>, sv.data);
  } 
  return *this;
}

template <class T>
shared_vector<T>& shared_vector<T>::operator=(shared_vector<T>&& sv) {
  if(allocated) data.mapv(free_shared_vector<T>);
  allocated = sv.allocated;
  size = sv.size;
  if(sv.allocated) {
    data = std::move(sv.data);
    sv.allocated = false;
  }
  return *this;
}

template <class T>
void make_shared_vector_broadcast_helper(shared_vector_local<T>& sv,
                                         intptr_t vptr) {
  size_t size = sv.size;
  if(is_shm_root()) {
    if(get_selfid() == 0) {
      std::vector<T>& v = *reinterpret_cast<std::vector<T>*>(vptr);
      T* src = v.data();
      T* dst = sv.data();
      for(size_t i = 0; i < size; i++) {
        dst[i] = src[i];
      }
    }
    char* buf = reinterpret_cast<char*>(sv.data());
    large_bcast(sizeof(T), buf, size, 0, frovedis_shmroot_comm);
  }
}

template <class T>
shared_vector<T> make_shared_vector_broadcast(std::vector<T>& v) {
  // should be compilation error, but good compilation error is difficult...
  if(std::is_pod<T>::value == false)
    throw std::runtime_error("cannot maka shared_vector from non PoD vector");
  auto ret = make_shared_vector<T>(v.size());
  intptr_t vptr = reinterpret_cast<intptr_t>(&v);
  ret.data.mapv(make_shared_vector_broadcast_helper<T>, broadcast(vptr));
  return ret;
}

template <class T>
void broadcast_to_shared_vector(std::vector<T>& v, shared_vector<T>& sv) {
  if(v.size() != sv.size)
    throw std::runtime_error("broadcast_to_shared_vector: size mismatch");
  intptr_t vptr = reinterpret_cast<intptr_t>(&v);
  sv.data.mapv(make_shared_vector_broadcast_helper<T>, broadcast(vptr));
}

template <class T>
void shared_vector_sum_helper(shared_vector_local<T>&, intptr_t);

template <>
void shared_vector_sum_helper(shared_vector_local<int>&, intptr_t);
template <>
void shared_vector_sum_helper(shared_vector_local<unsigned int>&, intptr_t);
template <>
void shared_vector_sum_helper(shared_vector_local<long>&, intptr_t);
template <>
void shared_vector_sum_helper(shared_vector_local<unsigned long>&, intptr_t);
template <>
void shared_vector_sum_helper(shared_vector_local<long long>&, intptr_t);
template <>
void shared_vector_sum_helper(shared_vector_local<unsigned long long>&,
                              intptr_t);
template <>
void shared_vector_sum_helper(shared_vector_local<float>&, intptr_t);
template <>
void shared_vector_sum_helper(shared_vector_local<double>&, intptr_t);

template <class T>
std::vector<T> shared_vector<T>::sum() {
  std::vector<T> ret(size);
  intptr_t retptr = reinterpret_cast<intptr_t>(ret.data());
  data.mapv(shared_vector_sum_helper<T>, broadcast(retptr));
  return ret;
}

// TODO: use MPI_Allreduce
template <class T>
void shared_vector<T>::allsum() {
  auto v = sum();
  broadcast_to_shared_vector(v, *this);
}

typedef shared_vector_local<sem_t> shared_lock_local;

struct shared_lock {
  shared_lock(node_local<shared_lock_local>&& d) :
    allocated(true), data(std::move(d)) {}
  shared_lock(const shared_lock&) = delete;
  shared_lock(shared_lock&& s) : data(std::move(s.data)){s.allocated = false;}
  shared_lock& operator=(const shared_lock&) = delete;
  shared_lock& operator=(shared_lock&&) = delete;
  ~shared_lock();
  bool allocated;
  node_local<shared_lock_local> data;
};

shared_lock make_shared_lock(size_t size);
shared_lock make_shared_lock();
void lock(shared_lock_local& sem, int id);
void unlock(shared_lock_local& sem, int id);
void lock(shared_lock_local& sem);
void unlock(shared_lock_local& sem);

}
#endif
#endif
