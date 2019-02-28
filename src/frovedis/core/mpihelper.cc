#include "mpi_rpc.hpp" // for mpi_max_count
#include "mpihelper.hpp"
#include <stdexcept>
#include <string.h>
#include <algorithm>

namespace frovedis {

void large_alltoallv(size_t element_size,
                     char* sendbuf_arg,
                     const std::vector<size_t>& sendcounts_arg,
                     const std::vector<size_t>& sdispls_arg,
                     char* recvbuf_arg,
                     const std::vector<size_t>& recvcounts_arg,
                     const std::vector<size_t>& rdispls_arg,
                     MPI_Comm comm) {
  int nodesize;
  MPI_Comm_size(comm, &nodesize);
  std::vector<size_t> sendcounts(nodesize);
  std::vector<size_t> recvcounts(nodesize);
  std::vector<size_t> sdispls(nodesize);
  std::vector<size_t> rdispls(nodesize);
  auto sendcountsp = sendcounts.data();
  auto recvcountsp = recvcounts.data();
  auto sdisplsp = sdispls.data();
  auto rdisplsp = rdispls.data();
  auto sendcounts_argp = sendcounts_arg.data();
  auto recvcounts_argp = recvcounts_arg.data();
  auto sdispls_argp = sdispls_arg.data();
  auto rdispls_argp = rdispls_arg.data();
  for(int i = 0; i < nodesize; i++) {
    sendcountsp[i] = sendcounts_argp[i] * element_size;
    recvcountsp[i] = recvcounts_argp[i] * element_size;
    sdisplsp[i] = sdispls_argp[i] * element_size;
    rdisplsp[i] = rdispls_argp[i] * element_size;
  }
  std::vector<size_t> spos(nodesize);
  std::vector<size_t> rpos(nodesize);
  std::vector<int> each_sendcounts(nodesize);
  std::vector<int> each_recvcounts(nodesize);
  std::vector<int> each_sdispls(nodesize);
  std::vector<int> each_rdispls(nodesize);
  size_t local_max_count = mpi_max_count / nodesize;
  auto sposp = spos.data();
  auto rposp = rpos.data();
  auto each_sendcountsp = each_sendcounts.data();
  auto each_recvcountsp = each_recvcounts.data();
  auto each_sdisplsp = each_sdispls.data();
  auto each_rdisplsp = each_rdispls.data();
  while(true) {
    for(int i = 0; i < nodesize; i++) {
      each_sendcountsp[i] = std::min(sendcountsp[i]-sposp[i], local_max_count);
      each_recvcountsp[i] = std::min(recvcountsp[i]-rposp[i], local_max_count);
    }
    size_t total_send_size = 0;
    for(int i = 0; i < nodesize; i++)
      total_send_size += each_sendcountsp[i];
    each_sdisplsp[0] = 0;
    for(int i = 1; i < nodesize; i++) {
      each_sdisplsp[i] = each_sdisplsp[i-1] + each_sendcountsp[i-1];
    }
    std::string sendbuf;
    sendbuf.resize(total_send_size);
    for(int i = 0; i < nodesize; i++) {
      memcpy(&sendbuf[each_sdisplsp[i]], &sendbuf_arg[sdisplsp[i] + sposp[i]],
             each_sendcountsp[i]);
    }
    size_t total_recv_size = 0;
    for(int i = 0; i < nodesize; i++)
      total_recv_size += each_recvcountsp[i];
    std::string recvbuf;
    recvbuf.resize(total_recv_size);
    each_rdispls[0] = 0;
    for(int i = 1; i < nodesize; i++) {
      each_rdisplsp[i] = each_rdisplsp[i-1] + each_recvcountsp[i-1];
    }
    MPI_Alltoallv(&sendbuf[0], &each_sendcounts[0], &each_sdispls[0],
                  MPI_CHAR, &recvbuf[0], &each_recvcounts[0],
                  &each_rdispls[0], MPI_CHAR, frovedis_comm_rpc);
    for(int i = 0; i < nodesize; i++) {
      memcpy(&recvbuf_arg[rdisplsp[i] + rposp[i]], &recvbuf[each_rdisplsp[i]],
             each_recvcountsp[i]);
    }
    for(int i = 0; i < nodesize; i++) {
      sposp[i] += each_sendcountsp[i];
      rposp[i] += each_recvcountsp[i];
    }
    int done = 1;
    for(int i = 0; i < nodesize; i++) {
      if(sendcountsp[i] != sposp[i] || recvcountsp[i] != rposp[i]) {
        done = 0; break;
      }
    }
    int done_all = 0;
    MPI_Allreduce(&done, &done_all, 1, MPI_INT, MPI_LAND, frovedis_comm_rpc);
    if(done_all) break;
  }
}

void large_gatherv(size_t element_size,
                   char* sendbuf_arg,
                   const size_t sendcount_arg,
                   char* recvbuf_arg,
                   const std::vector<size_t>& recvcounts_arg,
                   const std::vector<size_t>& displs_arg,
                   int root,
                   MPI_Comm comm) {
  // to make it portable with shared vector communicator
  int nodesize, self;
  MPI_Comm_size(comm, &nodesize);
  MPI_Comm_rank(comm, &self);
  size_t sendcount = sendcount_arg * element_size;
  std::vector<size_t> recvcounts(nodesize);
  std::vector<size_t> displs(nodesize);
  auto recvcountsp = recvcounts.data();
  auto displsp = displs.data();
  auto displs_argp = displs_arg.data();
  auto recvcounts_argp = recvcounts_arg.data();
  if(self == root) {
    for(int i = 0; i < nodesize; i++) {
      recvcountsp[i] = recvcounts_argp[i] * element_size;
      displsp[i] = displs_argp[i] * element_size;
    }
  }
  size_t spos = 0;
  std::vector<size_t> rpos(nodesize);
  int each_sendcount = 0;
  std::vector<int> each_recvcounts(nodesize);
  std::vector<int> each_displs(nodesize);
  size_t local_max_count = mpi_max_count / nodesize;
  auto rposp = rpos.data();
  auto each_recvcountsp = each_recvcounts.data();
  auto each_displsp = each_displs.data();
  while(true) {
    each_sendcount = std::min(sendcount-spos, local_max_count);
    char* sendbuf = sendbuf_arg + spos;
    std::string recvbuf;
    if(self == root) {
      for(int i = 0; i < nodesize; i++) {
        each_recvcountsp[i] = std::min(recvcountsp[i]-rposp[i], local_max_count);
      }
      size_t total_recv_size = 0;
      for(int i = 0; i < nodesize; i++)
        total_recv_size += each_recvcountsp[i];
      recvbuf.resize(total_recv_size);
      each_displsp[0] = 0;
      for(int i = 1; i < nodesize; i++) {
        each_displsp[i] = each_displsp[i-1] + each_recvcountsp[i-1];
      }
    }
    MPI_Gatherv(sendbuf, each_sendcount, MPI_CHAR,
                &recvbuf[0], &each_recvcounts[0],
                &each_displs[0], MPI_CHAR, root, comm);
    spos += each_sendcount;
    int done = 1;
    if(self == root) {
      for(int i = 0; i < nodesize; i++) {
        memcpy(&recvbuf_arg[displsp[i] + rposp[i]], &recvbuf[each_displsp[i]],
               each_recvcountsp[i]);
      }
      for(int i = 0; i < nodesize; i++) {
        rposp[i] += each_recvcountsp[i];
      }
      for(int i = 0; i < nodesize; i++) {
        if(recvcountsp[i] != rposp[i]) {
          done = 0; break;
        }
      }
    }
    MPI_Bcast(&done, 1, MPI_INT, root, comm);
    if(done) break;
  }
}

void large_scatterv(size_t element_size,
                    char* sendbuf_arg,
                    const std::vector<size_t>& sendcounts_arg,
                    const std::vector<size_t>& displs_arg,
                    char* recvbuf_arg,
                    size_t recvcount_arg,
                    int root,
                    MPI_Comm comm) {
  int nodesize, self;
  MPI_Comm_size(comm, &nodesize);
  MPI_Comm_rank(comm, &self);
  std::vector<size_t> sendcounts(nodesize);
  size_t recvcount;
  std::vector<size_t> displs(nodesize);
  auto sendcountsp = sendcounts.data();
  auto displsp = displs.data();
  auto sendcounts_argp = sendcounts_arg.data();
  auto displs_argp = displs_arg.data();
  if(self == root) {
    for(int i = 0; i < nodesize; i++) {
      sendcountsp[i] = sendcounts_argp[i] * element_size;
      displsp[i] = displs_argp[i] * element_size;
    }
  }
  recvcount = recvcount_arg * element_size;
  
  std::vector<size_t> spos(nodesize);
  size_t rpos = 0;
  std::vector<int> each_sendcounts(nodesize);
  int each_recvcount = 0;
  std::vector<int> each_displs(nodesize);
  auto each_displsp = each_displs.data();
  size_t local_max_count = mpi_max_count / nodesize;
  auto sposp = spos.data();
  auto each_sendcountsp = each_sendcounts.data();
  while(true) {
    each_recvcount = std::min(recvcount-rpos, local_max_count);
    char* recvbuf = recvbuf_arg + rpos;
    std::string sendbuf;
    if(self == root) {
      for(int i = 0; i < nodesize; i++) {
        each_sendcountsp[i] = std::min(sendcountsp[i]-sposp[i], local_max_count);
      }
      size_t total_send_size = 0;
      for(int i = 0; i < nodesize; i++)
        total_send_size += each_sendcountsp[i];
      each_displsp[0] = 0;
      for(int i = 1; i < nodesize; i++) {
        each_displsp[i] = each_displsp[i-1] + each_sendcountsp[i-1];
      }
      sendbuf.resize(total_send_size);
      for(int i = 0; i < nodesize; i++) {
        memcpy(&sendbuf[each_displsp[i]], &sendbuf_arg[displsp[i] + sposp[i]],
               each_sendcountsp[i]);
      }
    }
    MPI_Scatterv(&sendbuf[0], &each_sendcountsp[0], &each_displsp[0],
                 MPI_CHAR, recvbuf, each_recvcount, MPI_CHAR, root, comm);
    int done = 1;
    if(self == root) {
      for(int i = 0; i < nodesize; i++) {
        sposp[i] += each_sendcountsp[i];
      }
      for(int i = 0; i < nodesize; i++) {
        if(sendcountsp[i] != sposp[i]) {
          done = 0; break;
        }
      }
    }
    rpos += each_recvcount;
    MPI_Bcast(&done, 1, MPI_INT, root, comm);
    if(done) break;
  }
}

void large_bcast(size_t element_size,
                 char* buf,
                 size_t size_arg,
                 int root,
                 MPI_Comm comm) {
  size_t size = size_arg * element_size;
  for(size_t pos = 0; pos < size; pos += mpi_max_count) {
    size_t crnt_size = std::min(size - pos, mpi_max_count);
    MPI_Bcast(buf + pos, crnt_size, MPI_CHAR, root, comm);
  }
}

void large_recv(size_t element_size, char* buf, size_t count_arg,
                int src, int tag, MPI_Comm comm, MPI_Status *s) {
  size_t count = count_arg * element_size;
  for(size_t pos = 0; pos < count; pos += mpi_max_count) {
    size_t recv_size = std::min(count - pos, mpi_max_count);
    MPI_Recv(buf + pos, recv_size, MPI_CHAR, src, tag, comm, s);
  }
}

void large_send(size_t element_size, char* buf, size_t count_arg,
                int dst, int tag, MPI_Comm comm) {
  size_t count = count_arg * element_size;
  for(size_t pos = 0; pos < count; pos += mpi_max_count) {
    size_t send_size = std::min(count - pos, mpi_max_count);
    MPI_Send(buf + pos, send_size, MPI_CHAR, dst, tag, comm);
  }
}

/* // currently not used; leave for future use
template <>
int typed_alltoallv<double>(const double *sendbuf, const int *sendcounts,
                            const int *sdispls, double *recvbuf,
                            const int *recvcounts, const int *rdispls, 
                            MPI_Comm comm) {
  return MPI_Alltoallv(static_cast<void*>(const_cast<double*>(sendbuf)),
                       const_cast<int*>(sendcounts),
                       const_cast<int*>(sdispls),
                       MPI_DOUBLE,
                       static_cast<void*>(recvbuf),
                       const_cast<int*>(recvcounts),
                       const_cast<int*>(rdispls),
                       MPI_DOUBLE,
                       comm);
}

template <>
int typed_alltoallv<float>(const float *sendbuf, const int *sendcounts,
                           const int *sdispls, float *recvbuf,
                           const int *recvcounts, const int *rdispls, 
                           MPI_Comm comm) {
  return MPI_Alltoallv(static_cast<void*>(const_cast<float*>(sendbuf)),
                       const_cast<int*>(sendcounts),
                       const_cast<int*>(sdispls),
                       MPI_FLOAT,
                       static_cast<void*>(recvbuf),
                       const_cast<int*>(recvcounts),
                       const_cast<int*>(rdispls),
                       MPI_FLOAT,
                       comm);
}

template <>
int typed_alltoallv<size_t>(const size_t *sendbuf, const int *sendcounts,
                            const int *sdispls, size_t *recvbuf,
                            const int *recvcounts, const int *rdispls, 
                            MPI_Comm comm) {
  if(sizeof(size_t) == 8) {
    return MPI_Alltoallv(static_cast<void*>(const_cast<size_t*>(sendbuf)),
                         const_cast<int*>(sendcounts),
                         const_cast<int*>(sdispls),
                         MPI_UNSIGNED_LONG_LONG,
                         static_cast<void*>(recvbuf),
                         const_cast<int*>(recvcounts),
                         const_cast<int*>(rdispls),
                         MPI_UNSIGNED_LONG_LONG,
                         comm);
  } else { // == 4
    throw std::runtime_error
      ("size_t is assumed to be 64bit for compatibility with x86");
#if 0
    return MPI_Alltoallv(static_cast<void*>(const_cast<size_t*>(sendbuf)),
                         const_cast<int*>(sendcounts),
                         const_cast<int*>(sdispls),
                         MPI_UNSIGNED,
                         static_cast<void*>(recvbuf),
                         const_cast<int*>(recvcounts),
                         const_cast<int*>(rdispls),
                         MPI_UNSIGNED,
                         comm);
#endif
  }
}

template <>
int typed_alltoallv<int>(const int *sendbuf, const int *sendcounts,
                         const int *sdispls, int *recvbuf,
                         const int *recvcounts, const int *rdispls, 
                         MPI_Comm comm) {
  return MPI_Alltoallv(static_cast<void*>(const_cast<int*>(sendbuf)),
                       const_cast<int*>(sendcounts),
                       const_cast<int*>(sdispls),
                       MPI_INT,
                       static_cast<void*>(recvbuf),
                       const_cast<int*>(recvcounts),
                       const_cast<int*>(rdispls),
                       MPI_INT,
                       comm);
}
*/
template <>
int typed_allgatherv<double>(const double* sendbuf, int sendcount, 
                             double* recvbuf, int *recvcounts,
                             int *displs, MPI_Comm comm) {
  return MPI_Allgatherv(static_cast<void*>(const_cast<double*>(sendbuf)),
                        sendcount,
                        MPI_DOUBLE,
                        static_cast<void*>(recvbuf),
                        recvcounts,
                        displs,
                        MPI_DOUBLE,
                        comm);
}

template <>
int typed_allgatherv<float>(const float* sendbuf, int sendcount, 
                            float* recvbuf, int *recvcounts,
                            int *displs, MPI_Comm comm) {
  return MPI_Allgatherv(static_cast<void*>(const_cast<float*>(sendbuf)),
                        sendcount,
                        MPI_FLOAT,
                        static_cast<void*>(recvbuf),
                        recvcounts,
                        displs,
                        MPI_FLOAT,
                        comm);
}

/* // currently not used; leave for future use
template <>
int typed_gatherv<double>(const double* sendbuf, int sendcount, 
                          double* recvbuf, int *recvcounts,
                          int *displs, int root, MPI_Comm comm) {
  return MPI_Gatherv(static_cast<void*>(const_cast<double*>(sendbuf)),
                     sendcount,
                     MPI_DOUBLE,
                     static_cast<void*>(recvbuf),
                     recvcounts,
                     displs,
                     MPI_DOUBLE,
                     root,
                     comm);
}

template <>
int typed_gatherv<float>(const float* sendbuf, int sendcount, 
                         float* recvbuf, int *recvcounts,
                         int *displs, int root, MPI_Comm comm) {
  return MPI_Gatherv(static_cast<void*>(const_cast<float*>(sendbuf)),
                     sendcount,
                     MPI_FLOAT,
                     static_cast<void*>(recvbuf),
                     recvcounts,
                     displs,
                     MPI_FLOAT,
                     root,
                     comm);
}

template <>
int typed_bcast<double>(double* buffer, int count, int root,
                        MPI_Comm comm) {
  return MPI_Bcast(static_cast<void*>(buffer),
                   count, MPI_DOUBLE, root, comm);
}

template <>
int typed_bcast<float>(float* buffer, int count, int root,
                       MPI_Comm comm){
  return MPI_Bcast(static_cast<void*>(buffer),
                   count, MPI_FLOAT, root, comm);
}
*/
/* // comment out because not used; check before use!
template <>
int typed_allgather<double>(const double* sendbuf, int sendcount,
                            double* recvbuf, int recvcount, MPI_Comm comm) {
  return MPI_Allgather(static_cast<void*>(const_cast<double*>(sendbuf)),
                       sendcount, MPI_DOUBLE,
                       static_cast<void*>(recvbuf), recvcount, MPI_DOUBLE, 
                       comm); 
}

template <>
int typed_allgather(const float* sendbuf, int sendcount,
                    float* recvbuf, int recvcount, MPI_Comm comm) {
  return MPI_Allgather(static_cast<void*>(const_cast<float*>(sendbuf)),
                       sendcount, MPI_FLOAT,
                       static_cast<void*>(recvbuf), recvcount, MPI_FLOAT,
                       comm); 
}
*/

template <>
int typed_allreduce<double>(const double* sendbuf, double* recvbuf, int count,
                            MPI_Op op, MPI_Comm comm) {
  return MPI_Allreduce(static_cast<void*>(const_cast<double*>(sendbuf)),
                       static_cast<void*>(recvbuf),
                       count, MPI_DOUBLE, op, comm);
}

template <>
int typed_allreduce(const float* sendbuf, float* recvbuf, int count,
                    MPI_Op op, MPI_Comm comm) {
  return MPI_Allreduce(static_cast<void*>(const_cast<float*>(sendbuf)),
                       static_cast<void*>(recvbuf),
                       count, MPI_FLOAT, op, comm);

}

}
