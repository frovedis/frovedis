#ifndef MATRIX_MPIHELPER_HPP
#define MATRIX_MPIHELPER_HPP

#include "mpi.h"

namespace frovedis {

void large_alltoallv(size_t element_size,
                     char *sendbuf, const std::vector<size_t>& sendcounts,
                     const std::vector<size_t>& sdispls,
                     char *recvbuf, const std::vector<size_t>& recvcounts,
                     const std::vector<size_t>& rdispls, 
                     MPI_Comm comm);

void large_gatherv(size_t element_size,
                   char* sendbuf,
                   const size_t sendcount,
                   char* recvbuf,
                   const std::vector<size_t>& recvcounts,
                   const std::vector<size_t>& displs,
                   int root,
                   MPI_Comm comm);

void large_scatterv(size_t element_size,
                    char* sendbuf,
                    const std::vector<size_t>& sendcounts,
                    const std::vector<size_t>& displs,
                    char* recvbuf,
                    size_t recvcount,
                    int root,
                    MPI_Comm comm);

void large_bcast(size_t element_size,
                 char* sendbuf,
                 size_t size,
                 int root,
                 MPI_Comm comm);

void large_recv(size_t element_size, char* buf, size_t count_arg,
                int src, int tag, MPI_Comm comm, MPI_Status *s);

void large_send(size_t element_size, char* buf, size_t count_arg,
                int dst, int tag, MPI_Comm comm);

/* // currently not used; leave for future use
template <class T>
int typed_alltoallv(const T *sendbuf, const int *sendcounts,
                    const int *sdispls, T *recvbuf,
                    const int *recvcounts, const int *rdispls, 
                    MPI_Comm comm);

template <>
int typed_alltoallv<double>(const double *sendbuf, const int *sendcounts,
                            const int *sdispls, double *recvbuf,
                            const int *recvcounts, const int *rdispls, 
                            MPI_Comm comm);

template <>
int typed_alltoallv<float>(const float *sendbuf, const int *sendcounts,
                           const int *sdispls, float *recvbuf,
                           const int *recvcounts, const int *rdispls, 
                           MPI_Comm comm);

template <>
int typed_alltoallv<size_t>(const size_t *sendbuf, const int *sendcounts,
                            const int *sdispls, size_t *recvbuf,
                            const int *recvcounts, const int *rdispls, 
                            MPI_Comm comm);

template <>
int typed_alltoallv<int>(const int *sendbuf, const int *sendcounts,
                         const int *sdispls, int *recvbuf,
                         const int *recvcounts, const int *rdispls, 
                         MPI_Comm comm);
*/

template <class T>
int typed_allgatherv(const T* sendbuf, int sendcount, 
                     T* recvbuf, int *recvcounts,
                     int *displs, MPI_Comm comm);

template <>
int typed_allgatherv<double>(const double* sendbuf, int sendcount, 
                             double* recvbuf, int *recvcounts,
                             int *displs, MPI_Comm comm);

template <>
int typed_allgatherv<float>(const float* sendbuf, int sendcount, 
                            float* recvbuf, int *recvcounts,
                            int *displs, MPI_Comm comm);

/* // currently not used; leave for future use
template <class T>
int typed_gatherv(const T* sendbuf, int sendcount, 
                  T* recvbuf, int *recvcounts,
                  int *displs, int root, MPI_Comm comm);

template <>
int typed_gatherv<double>(const double* sendbuf, int sendcount, 
                          double* recvbuf, int *recvcounts,
                          int *displs, int root, MPI_Comm comm);

template <>
int typed_gatherv<float>(const float* sendbuf, int sendcount, 
                         float* recvbuf, int *recvcounts,
                         int *displs, int root, MPI_Comm comm);

template <class T>
int typed_bcast(T* buffer, int count, int root, MPI_Comm comm);

template <>
int typed_bcast<double>(double* buffer, int count, int root,
                        MPI_Comm comm);

template <>
int typed_bcast<float>(float* buffer, int count, int root,
                       MPI_Comm comm);
*/
/* // comment out because not used; check before use!
template <class T>
int typed_allgather(const T* sendbuf, int sendcount,
                    T* recvbuf, int recvcount, MPI_Comm comm);

template <>
int typed_allgather(const double* sendbuf, int sendcount,
                    double* recvbuf, int recvcount, MPI_Comm comm);

template <>
int typed_allgather(const float* sendbuf, int sendcount,
                    float* recvbuf, int recvcount, MPI_Comm comm);
*/

template <class T>
int typed_allreduce(const T* sendbuf, T* recvbuf, int count,
                    MPI_Op op, MPI_Comm comm);

template <>
int typed_allreduce(const double* sendbuf, double* recvbuf, int count,
                    MPI_Op op, MPI_Comm comm);

template <>
int typed_allreduce(const float* sendbuf, float* recvbuf, int count,
                    MPI_Op op, MPI_Comm comm);

}

#endif
