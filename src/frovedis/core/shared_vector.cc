// shared vector requires MPI 3.0
#include <mpi.h>
#if MPI_VERSION >= 3

#include "shared_vector.hpp"
#include <climits>
#include <stdexcept>

namespace frovedis {

void init_shm_info_local() {
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


pid_t shm_root_pid;

int current_shmid = 1;
int get_new_shmid() {
  int ret = current_shmid;
  if(current_shmid == INT_MAX)
    throw std::runtime_error("shmid exceeded integer");
  else current_shmid++;
  return ret;
}

void init_shared_lock(shared_lock_local& s) {
  if(is_shm_root()) {
    auto ptr = s.data();
    for(size_t i = 0; i < s.size; i++) {
      if(sem_init(ptr + i, 1, 1) == -1) {
        throw std::runtime_error(std::string("error in sem_init: ") +
                                 std::string(strerror(errno)));
      }
    }
  }
}

shared_lock make_shared_lock(size_t size) {
  auto r = make_shared_vector<sem_t>(size);
  r.data.mapv(init_shared_lock);
  r.allocated = false; // will be moved!
  return shared_lock(std::move(r.data));
}

shared_lock make_shared_lock() {
  auto r = make_shared_vector<sem_t>(1);
  r.data.mapv(init_shared_lock);
  r.allocated = false; // will be moved!
  return shared_lock(std::move(r.data));
}

void lock(shared_lock_local& sem, int id) {
  auto ptr = sem.data();
  if(sem_wait(&ptr[id]) == -1) {
    throw std::runtime_error(std::string("error in sem_wait: ") +
                             std::string(strerror(errno)));
  }
}

void lock(shared_lock_local& sem) {
  auto ptr = sem.data();
  if(sem_wait(ptr) == -1) {
    throw std::runtime_error(std::string("error in sem_wait: ") +
                             std::string(strerror(errno)));
  }
}

void unlock(shared_lock_local& sem, int id) {
  auto ptr = sem.data();
  if(sem_post(ptr + id) == -1) {
    throw std::runtime_error(std::string("error in sem_post: ") +
                             std::string(strerror(errno)));
  }
}

void unlock(shared_lock_local& sem) {
  auto ptr = sem.data();
  if(sem_post(ptr) == -1) {
    throw std::runtime_error(std::string("error in sem_post: ") +
                             std::string(strerror(errno)));
  }
}

void destroy_shared_lock(shared_lock_local& s) {
  if(is_shm_root()) {
    auto ptr = s.data();
    for(size_t i = 0; i < s.size; i++) {
      if(sem_destroy(ptr + i) == -1) {
        throw std::runtime_error(std::string("error in sem_destroy: ") +
                               std::string(strerror(errno)));
      }
    }
  }
}

shared_lock::~shared_lock() {
  if(allocated) {
    try {
      data.mapv(destroy_shared_lock); 
    } catch (std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
    }
  }
}

template <>
void shared_vector_sum_helper(shared_vector_local<int>& sv, intptr_t retptr) {
  auto ptr = sv.data();
  if(is_shm_root()) {
    if(get_selfid() == 0) {
      int* ret = reinterpret_cast<int*>(retptr);
      MPI_Reduce((void*)ptr, (void*)ret, sv.size, MPI_INT, MPI_SUM, 0,
                 frovedis_shmroot_comm);
    } else{
      MPI_Reduce((void*)ptr, 0, sv.size, MPI_INT, MPI_SUM, 0,
                 frovedis_shmroot_comm);
    }
  }
}

template <>
void shared_vector_sum_helper(shared_vector_local<unsigned int>& sv,
                              intptr_t retptr) {
  auto ptr = sv.data();
  if(is_shm_root()) {
    if(get_selfid() == 0) {
      unsigned int* ret = reinterpret_cast<unsigned int*>(retptr);
      MPI_Reduce((void*)ptr, (void*)ret, sv.size, MPI_UNSIGNED, MPI_SUM, 0,
                 frovedis_shmroot_comm);
    } else{
      MPI_Reduce((void*)ptr, 0, sv.size, MPI_UNSIGNED, MPI_SUM, 0,
                 frovedis_shmroot_comm);
    }
  }
}

template <>
void shared_vector_sum_helper(shared_vector_local<long>& sv,
                              intptr_t retptr) {
  auto ptr = sv.data();
  if(is_shm_root()) {
    if(get_selfid() == 0) {
      long* ret = reinterpret_cast<long*>(retptr);
      MPI_Reduce((void*)ptr, (void*)ret, sv.size, MPI_LONG, MPI_SUM, 0,
                 frovedis_shmroot_comm);
    } else{
      MPI_Reduce((void*)ptr, 0, sv.size, MPI_LONG, MPI_SUM, 0,
                 frovedis_shmroot_comm);
    }
  }
}

template <>
void shared_vector_sum_helper(shared_vector_local<unsigned long>& sv,
                              intptr_t retptr) {
  auto ptr = sv.data();
  if(is_shm_root()) {
    if(get_selfid() == 0) {
      unsigned long* ret = reinterpret_cast<unsigned long*>(retptr);
      MPI_Reduce((void*)ptr, (void*)ret, sv.size, MPI_UNSIGNED_LONG, MPI_SUM,
                 0, frovedis_shmroot_comm);
    } else{
      MPI_Reduce((void*)ptr, 0, sv.size, MPI_UNSIGNED_LONG, MPI_SUM, 0,
                 frovedis_shmroot_comm);
    }
  }
}

template <>
void shared_vector_sum_helper(shared_vector_local<long long>& sv,
                              intptr_t retptr) {
  auto ptr = sv.data();
  if(is_shm_root()) {
    if(get_selfid() == 0) {
      long long* ret = reinterpret_cast<long long*>(retptr);
      MPI_Reduce((void*)ptr, (void*)ret, sv.size, MPI_LONG_LONG, MPI_SUM,
                 0, frovedis_shmroot_comm);
    } else{
      MPI_Reduce((void*)ptr, 0, sv.size, MPI_LONG_LONG, MPI_SUM, 0,
                 frovedis_shmroot_comm);
    }
  }
}

template <>
void shared_vector_sum_helper(shared_vector_local<unsigned long long>& sv,
                              intptr_t retptr) {
  auto ptr = sv.data();
  if(is_shm_root()) {
    if(get_selfid() == 0) {
      unsigned long long* ret = reinterpret_cast<unsigned long long*>(retptr);
      MPI_Reduce((void*)ptr, (void*)ret, sv.size, MPI_UNSIGNED_LONG_LONG,
                 MPI_SUM, 0, frovedis_shmroot_comm);
    } else{
      MPI_Reduce((void*)ptr, 0, sv.size, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0,
                 frovedis_shmroot_comm);
    }
  }
}

template <>
void shared_vector_sum_helper(shared_vector_local<float>& sv,
                              intptr_t retptr) {
  auto ptr = sv.data();
  if(is_shm_root()) {
    if(get_selfid() == 0) {
      float* ret = reinterpret_cast<float*>(retptr);
      MPI_Reduce((void*)ptr, (void*)ret, sv.size, MPI_FLOAT,
                 MPI_SUM, 0, frovedis_shmroot_comm);
    } else{
      MPI_Reduce((void*)ptr, 0, sv.size, MPI_FLOAT, MPI_SUM, 0,
                 frovedis_shmroot_comm);
    }
  }
}

template <>
void shared_vector_sum_helper(shared_vector_local<double>& sv,
                              intptr_t retptr) {
  auto ptr = sv.data();
  if(is_shm_root()) {
    if(get_selfid() == 0) {
      double* ret = reinterpret_cast<double*>(retptr);
      MPI_Reduce((void*)ptr, (void*)ret, sv.size, MPI_DOUBLE,
                 MPI_SUM, 0, frovedis_shmroot_comm);
    } else{
      MPI_Reduce((void*)ptr, 0, sv.size, MPI_DOUBLE, MPI_SUM, 0,
                 frovedis_shmroot_comm);
    }
  }
}

}
#endif
