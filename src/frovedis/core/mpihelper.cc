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
  int nodesize = get_nodesize();
  std::vector<size_t> sendcounts(nodesize);
  std::vector<size_t> recvcounts(nodesize);
  std::vector<size_t> sdispls(nodesize);
  std::vector<size_t> rdispls(nodesize);
  for(int i = 0; i < nodesize; i++) {
    sendcounts[i] = sendcounts_arg[i] * element_size;
    recvcounts[i] = recvcounts_arg[i] * element_size;
    sdispls[i] = sdispls_arg[i] * element_size;
    rdispls[i] = rdispls_arg[i] * element_size;
  }
  std::vector<size_t> spos(nodesize);
  std::vector<size_t> rpos(nodesize);
  std::vector<int> each_sendcounts(nodesize);
  std::vector<int> each_recvcounts(nodesize);
  std::vector<int> each_sdispls(nodesize);
  std::vector<int> each_rdispls(nodesize);
  size_t local_max_count = mpi_max_count / nodesize;
  while(true) {
    for(int i = 0; i < nodesize; i++) {
      each_sendcounts[i] = std::min(sendcounts[i]-spos[i], local_max_count);
      each_recvcounts[i] = std::min(recvcounts[i]-rpos[i], local_max_count);
    }
    size_t total_send_size = 0;
    for(int i = 0; i < nodesize; i++)
      total_send_size += each_sendcounts[i];
    each_sdispls[0] = 0;
    for(int i = 1; i < nodesize; i++) {
      each_sdispls[i] = each_sdispls[i-1] + each_sendcounts[i-1];
    }
    std::string sendbuf;
    sendbuf.resize(total_send_size);
    for(int i = 0; i < nodesize; i++) {
      memcpy(&sendbuf[each_sdispls[i]], &sendbuf_arg[sdispls[i] + spos[i]],
             each_sendcounts[i]);
    }
    size_t total_recv_size = 0;
    for(int i = 0; i < nodesize; i++)
      total_recv_size += each_recvcounts[i];
    std::string recvbuf;
    recvbuf.resize(total_recv_size);
    each_rdispls[0] = 0;
    for(int i = 1; i < nodesize; i++) {
      each_rdispls[i] = each_rdispls[i-1] + each_recvcounts[i-1];
    }
    MPI_Alltoallv(&sendbuf[0], &each_sendcounts[0], &each_sdispls[0],
                  MPI_CHAR, &recvbuf[0], &each_recvcounts[0],
                  &each_rdispls[0], MPI_CHAR, frovedis_comm_rpc);
    for(int i = 0; i < nodesize; i++) {
      memcpy(&recvbuf_arg[rdispls[i] + rpos[i]], &recvbuf[each_rdispls[i]],
             each_recvcounts[i]);
    }
    for(int i = 0; i < nodesize; i++) {
      spos[i] += each_sendcounts[i];
      rpos[i] += each_recvcounts[i];
    }
    int done = 1;
    for(int i = 0; i < nodesize; i++) {
      if(sendcounts[i] != spos[i] || recvcounts[i] != rpos[i]) {
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
  int nodesize = get_nodesize();
  int self = get_selfid();
  size_t sendcount = sendcount_arg * element_size;
  std::vector<size_t> recvcounts(nodesize);
  std::vector<size_t> displs(nodesize);
  if(self == root) {
    for(int i = 0; i < nodesize; i++) {
      recvcounts[i] = recvcounts_arg[i] * element_size;
      displs[i] = displs_arg[i] * element_size;
    }
  }
  size_t spos = 0;
  std::vector<size_t> rpos(nodesize);
  int each_sendcount = 0;
  std::vector<int> each_recvcounts(nodesize);
  std::vector<int> each_displs(nodesize);
  size_t local_max_count = mpi_max_count / nodesize;
  while(true) {
    each_sendcount = std::min(sendcount-spos, local_max_count);
    char* sendbuf = sendbuf_arg + spos;
    std::string recvbuf;
    if(self == root) {
      for(int i = 0; i < nodesize; i++) {
        each_recvcounts[i] = std::min(recvcounts[i]-rpos[i], local_max_count);
      }
      size_t total_recv_size = 0;
      for(int i = 0; i < nodesize; i++)
        total_recv_size += each_recvcounts[i];
      recvbuf.resize(total_recv_size);
      each_displs[0] = 0;
      for(int i = 1; i < nodesize; i++) {
        each_displs[i] = each_displs[i-1] + each_recvcounts[i-1];
      }
    }
    MPI_Gatherv(sendbuf, each_sendcount, MPI_CHAR,
                &recvbuf[0], &each_recvcounts[0],
                &each_displs[0], MPI_CHAR, root, comm);
    spos += each_sendcount;
    int done = 1;
    if(self == root) {
      for(int i = 0; i < nodesize; i++) {
        memcpy(&recvbuf_arg[displs[i] + rpos[i]], &recvbuf[each_displs[i]],
               each_recvcounts[i]);
      }
      for(int i = 0; i < nodesize; i++) {
        rpos[i] += each_recvcounts[i];
      }
      for(int i = 0; i < nodesize; i++) {
        if(recvcounts[i] != rpos[i]) {
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
  int nodesize = get_nodesize();
  int self = get_selfid();
  std::vector<size_t> sendcounts(nodesize);
  size_t recvcount;
  std::vector<size_t> displs(nodesize);
  if(self == root) {
    for(int i = 0; i < nodesize; i++) {
      sendcounts[i] = sendcounts_arg[i] * element_size;
      displs[i] = displs_arg[i] * element_size;
    }
  }
  recvcount = recvcount_arg * element_size;
  
  std::vector<size_t> spos(nodesize);
  size_t rpos = 0;
  std::vector<int> each_sendcounts(nodesize);
  int each_recvcount = 0;
  std::vector<int> each_displs(nodesize);
  size_t local_max_count = mpi_max_count / nodesize;
  while(true) {
    each_recvcount = std::min(recvcount-rpos, local_max_count);
    char* recvbuf = recvbuf_arg + rpos;
    std::string sendbuf;
    if(self == root) {
      for(int i = 0; i < nodesize; i++) {
        each_sendcounts[i] = std::min(sendcounts[i]-spos[i], local_max_count);
      }
      size_t total_send_size = 0;
      for(int i = 0; i < nodesize; i++)
        total_send_size += each_sendcounts[i];
      each_displs[0] = 0;
      for(int i = 1; i < nodesize; i++) {
        each_displs[i] = each_displs[i-1] + each_sendcounts[i-1];
      }
      sendbuf.resize(total_send_size);
      for(int i = 0; i < nodesize; i++) {
        memcpy(&sendbuf[each_displs[i]], &sendbuf_arg[displs[i] + spos[i]],
               each_sendcounts[i]);
      }
    }
    MPI_Scatterv(&sendbuf[0], &each_sendcounts[0], &each_displs[0],
                 MPI_CHAR, recvbuf, each_recvcount, MPI_CHAR, root, comm);
    int done = 1;
    if(self == root) {
      for(int i = 0; i < nodesize; i++) {
        spos[i] += each_sendcounts[i];
      }
      for(int i = 0; i < nodesize; i++) {
        if(sendcounts[i] != spos[i]) {
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
